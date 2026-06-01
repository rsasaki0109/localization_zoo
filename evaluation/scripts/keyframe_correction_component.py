#!/usr/bin/env python3
"""Reusable keyframe-correction component for pair-prior trajectories."""

from __future__ import annotations

import math
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

import numpy as np

from intensity_bev_odometry_demo import compute_metrics, load_gt_xyyaw, se2_matrix


@dataclass(frozen=True)
class KeyframeCorrectionConfig:
    max_keyframe_correction_m: float = 0.5
    max_keyframe_yaw_deg: float = 5.0
    max_keyframe_rmse_m: float = 1.2
    min_keyframe_correspondences: int = 6000

    @classmethod
    def from_recipe(cls, recipe: dict[str, Any]) -> "KeyframeCorrectionConfig":
        gate = recipe.get("keyframe_correction_gate", recipe)
        return cls(
            max_keyframe_correction_m=float(
                gate.get("max_keyframe_correction_m", gate.get("max_keyframe_correction", 0.5))
            ),
            max_keyframe_yaw_deg=float(gate.get("max_keyframe_yaw_deg", 5.0)),
            max_keyframe_rmse_m=float(
                gate.get("max_keyframe_rmse_m", gate.get("max_keyframe_rmse", 1.2))
            ),
            min_keyframe_correspondences=int(gate.get("min_keyframe_correspondences", 6000)),
        )


def finite_float(value: Any, default: float) -> float:
    try:
        out = float(value)
    except (TypeError, ValueError):
        return default
    return out if math.isfinite(out) else default


@dataclass(frozen=True)
class KeyframeCorrectionCandidate:
    attempted: bool
    anchor_accepted: bool
    correction_m: float
    correction_yaw_deg: float
    rmse_m: float
    correspondences: int
    dx_m: float
    dy_m: float
    yaw_deg: float

    @classmethod
    def from_pair(cls, pair: dict[str, Any]) -> "KeyframeCorrectionCandidate":
        return cls(
            attempted=bool(pair.get("keyframe_attempted", False)),
            anchor_accepted=bool(
                pair.get("keyframe_anchor_accepted", pair.get("keyframe_accepted", False))
            ),
            correction_m=finite_float(pair.get("keyframe_correction_m"), float("inf")),
            correction_yaw_deg=finite_float(pair.get("keyframe_correction_yaw_deg"), float("inf")),
            rmse_m=finite_float(pair.get("keyframe_rmse_m"), float("inf")),
            correspondences=int(pair.get("keyframe_correspondences", 0)),
            dx_m=finite_float(pair.get("keyframe_correction_dx_m"), 0.0),
            dy_m=finite_float(pair.get("keyframe_correction_dy_m"), 0.0),
            yaw_deg=finite_float(pair.get("keyframe_correction_transform_yaw_deg"), 0.0),
        )

    @property
    def has_transform(self) -> bool:
        return all(math.isfinite(value) for value in (self.dx_m, self.dy_m, self.yaw_deg))


@dataclass(frozen=True)
class KeyframeCorrectionDecision:
    accepted: bool
    reason: str
    candidate: KeyframeCorrectionCandidate

    def to_log(self, source_keyframe_json: str | None) -> dict[str, Any]:
        return {
            "attempted": self.candidate.attempted,
            "accepted": self.accepted,
            "reason": self.reason,
            "source_keyframe_json": source_keyframe_json,
            "keyframe_rmse_m": self.candidate.rmse_m
            if math.isfinite(self.candidate.rmse_m)
            else None,
            "keyframe_correspondences": self.candidate.correspondences,
            "keyframe_correction_m": self.candidate.correction_m
            if math.isfinite(self.candidate.correction_m)
            else None,
            "keyframe_correction_yaw_deg": self.candidate.correction_yaw_deg
            if math.isfinite(self.candidate.correction_yaw_deg)
            else None,
        }


def pair_delta(pair: dict[str, Any]) -> np.ndarray:
    return se2_matrix(
        float(pair.get("used_dx_curr_to_prev_m", pair.get("dx_curr_to_prev_m", 0.0))),
        float(pair.get("used_dy_curr_to_prev_m", pair.get("dy_curr_to_prev_m", 0.0))),
        math.radians(
            float(pair.get("used_yaw_curr_to_prev_deg", pair.get("yaw_curr_to_prev_deg", 0.0)))
        ),
    )


def correction_delta(candidate: KeyframeCorrectionCandidate) -> np.ndarray:
    return se2_matrix(candidate.dx_m, candidate.dy_m, math.radians(candidate.yaw_deg))


def load_keyframe_pairs(payload: dict[str, Any]) -> dict[int, dict[str, Any]]:
    return {int(pair["index"]): pair for pair in payload.get("pairs", [])}


def decide_keyframe_correction(
    pair: dict[str, Any],
    config: KeyframeCorrectionConfig,
) -> KeyframeCorrectionDecision:
    candidate = KeyframeCorrectionCandidate.from_pair(pair)
    if not candidate.attempted:
        return KeyframeCorrectionDecision(False, "not_attempted", candidate)
    if not candidate.anchor_accepted:
        return KeyframeCorrectionDecision(False, "anchor_rejected", candidate)
    if candidate.correction_m > config.max_keyframe_correction_m:
        return KeyframeCorrectionDecision(False, "correction_translation_gate", candidate)
    if abs(candidate.correction_yaw_deg) > config.max_keyframe_yaw_deg:
        return KeyframeCorrectionDecision(False, "correction_yaw_gate", candidate)
    if candidate.rmse_m > config.max_keyframe_rmse_m:
        return KeyframeCorrectionDecision(False, "rmse_gate", candidate)
    if candidate.correspondences < config.min_keyframe_correspondences:
        return KeyframeCorrectionDecision(False, "correspondence_gate", candidate)
    if not candidate.has_transform:
        return KeyframeCorrectionDecision(False, "missing_correction_transform", candidate)
    return KeyframeCorrectionDecision(True, "accepted", candidate)


def apply_keyframe_corrections(
    prior_payload: dict[str, Any],
    keyframe_payload: dict[str, Any],
    config: KeyframeCorrectionConfig,
    *,
    source_prior_json: str | None = None,
    source_keyframe_json: str | None = None,
    gt_csv: Path | None = None,
) -> dict[str, Any]:
    started_at = time.perf_counter()
    anchors = load_keyframe_pairs(keyframe_payload)

    pose = np.eye(3, dtype=np.float64)
    poses = [[0.0, 0.0, 0.0]]
    pairs = []
    correction_counts: dict[str, int] = {}

    for pair in prior_payload.get("pairs", []):
        index = int(pair["index"])
        pose = pose @ pair_delta(pair)
        decision = decide_keyframe_correction(anchors.get(index, {}), config)
        correction_counts[decision.reason] = correction_counts.get(decision.reason, 0) + 1
        if decision.accepted:
            pose = correction_delta(decision.candidate) @ pose
        pairs.append({
            **pair,
            "keyframe_replay": decision.to_log(source_keyframe_json),
        })
        poses.append([
            float(pose[0, 2]),
            float(pose[1, 2]),
            float(math.atan2(pose[1, 0], pose[0, 0])),
        ])

    pose_arr = np.asarray(poses, dtype=np.float64)
    effective_gt_csv = gt_csv
    if effective_gt_csv is None and prior_payload.get("gt_csv"):
        effective_gt_csv = Path(str(prior_payload["gt_csv"]))
    gt = load_gt_xyyaw(effective_gt_csv) if effective_gt_csv is not None else None
    metrics = compute_metrics(pose_arr, gt)

    return {
        "sequence_pcd_dir": prior_payload.get("sequence_pcd_dir"),
        "gt_csv": str(effective_gt_csv) if effective_gt_csv is not None else None,
        "frames": int(pose_arr.shape[0]),
        "method": "keyframe_corrected_pair_prior_odometry",
        "component": "keyframe_correction",
        "source_prior_json": source_prior_json,
        "source_keyframe_json": source_keyframe_json,
        "parameters": asdict(config),
        "runtime_s": time.perf_counter() - started_at,
        "metrics": metrics,
        "correction_counts": correction_counts,
        "poses_xyyaw": [
            {"index": i, "x_m": float(p[0]), "y_m": float(p[1]), "yaw_deg": math.degrees(float(p[2]))}
            for i, p in enumerate(pose_arr)
        ],
        "pairs": pairs,
    }
