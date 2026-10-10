#!/usr/bin/env python3
"""Score place_recognition_benchmark output against KITTI ground truth.

Protocol (top-1 retrieval, the usual LiDAR loop-closure evaluation):

* A query frame i is a *revisit* when some earlier frame j <= i - exclude
  lies within ``--revisit-radius`` metres of it (ground-truth positions).
* Each method reports its best earlier frame j* and a confidence score.
  At a threshold t the query is predicted positive when score >= t.
* A positive prediction is a true positive when ||p_i - p_j*|| is within the
  radius, otherwise a false positive. Recall divides true positives by the
  number of revisit queries, so a revisit whose top-1 is wrong counts as
  missed at every threshold.

Reported per method: maximum F1, recall at 100 % precision, and the
precision-recall curve. Ground truth is read only here, never by the
descriptor binary.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
from pathlib import Path

import numpy as np


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--results", required=True, nargs="+", type=Path,
                        help="JSON files from place_recognition_benchmark.")
    parser.add_argument("--gt-csv", required=True, nargs="+", type=Path,
                        help="Zoo lidar_pose GT CSV, one per --results file.")
    parser.add_argument("--label", nargs="+",
                        help="Display label per results file (default: GT stem).")
    parser.add_argument("--revisit-radius", type=float, default=4.0)
    parser.add_argument("--output-json", type=Path, required=True)
    parser.add_argument("--output-md", type=Path)
    parser.add_argument("--output-png", type=Path)
    return parser.parse_args()


def load_positions(path: Path) -> np.ndarray:
    with path.open(newline="") as handle:
        rows = list(csv.DictReader(handle))
    return np.array(
        [[float(r["lidar_pose.x"]), float(r["lidar_pose.y"]), float(r["lidar_pose.z"])]
         for r in rows]
    )


def revisit_mask(positions: np.ndarray, exclude: int, radius: float) -> np.ndarray:
    """True for frames with a ground-truth neighbour at least ``exclude`` frames older."""
    mask = np.zeros(len(positions), dtype=bool)
    radius_sq = radius * radius
    for i in range(exclude, len(positions)):
        older = positions[: i - exclude + 1]
        mask[i] = bool(np.any(np.sum((older - positions[i]) ** 2, axis=1) <= radius_sq))
    return mask


def pr_curve(scores: np.ndarray, correct: np.ndarray, num_revisits: int) -> dict:
    """Sweep the score threshold over the predicted (query, top-1) pairs."""
    if num_revisits == 0 or len(scores) == 0:
        return {"precision": [], "recall": [], "f1_max": 0.0,
                "recall_at_100_precision": 0.0, "threshold_at_f1_max": None}
    order = np.argsort(-scores, kind="stable")
    sorted_scores = scores[order]
    sorted_correct = correct[order]
    tp = np.cumsum(sorted_correct)
    fp = np.cumsum(~sorted_correct)
    # Only cut between distinct scores so ties are accepted or rejected together.
    last_of_tie = np.r_[sorted_scores[1:] != sorted_scores[:-1], True]
    tp, fp, thresholds = tp[last_of_tie], fp[last_of_tie], sorted_scores[last_of_tie]
    precision = tp / (tp + fp)
    recall = tp / num_revisits
    with np.errstate(invalid="ignore", divide="ignore"):
        f1 = np.where(precision + recall > 0,
                      2 * precision * recall / (precision + recall), 0.0)
    best = int(np.argmax(f1))
    perfect = recall[precision >= 1.0]
    return {
        "precision": precision.round(6).tolist(),
        "recall": recall.round(6).tolist(),
        "f1_max": float(f1[best]),
        "precision_at_f1_max": float(precision[best]),
        "recall_at_f1_max": float(recall[best]),
        "threshold_at_f1_max": float(thresholds[best]),
        "recall_at_100_precision": float(perfect.max()) if len(perfect) else 0.0,
    }


def score_method(queries: list, positions: np.ndarray, revisits: np.ndarray,
                 radius: float) -> dict:
    query_ids, scores, correct = [], [], []
    for i, entry in enumerate(queries):
        match, score = entry
        if match is None or match < 0 or score is None or not math.isfinite(score):
            continue
        query_ids.append(i)
        scores.append(score)
        correct.append(bool(np.linalg.norm(positions[i] - positions[match]) <= radius))
    scores_np = np.asarray(scores, dtype=float)
    correct_np = np.asarray(correct, dtype=bool)
    num_revisits = int(revisits.sum())
    curve = pr_curve(scores_np, correct_np, num_revisits)
    # A correct top-1 on a non-revisit query is impossible by construction,
    # but keep the count explicit as a sanity check.
    curve["queries_with_answer"] = len(query_ids)
    curve["correct_top1"] = int(correct_np.sum())
    curve["revisit_queries"] = num_revisits
    curve["top1_recall_no_threshold"] = (
        float(correct_np.sum()) / num_revisits if num_revisits else 0.0
    )
    return curve


def evaluate(results_path: Path, gt_csv: Path, label: str, radius: float) -> dict:
    payload = json.loads(results_path.read_text())
    positions = load_positions(gt_csv)
    if len(positions) < payload["frames"]:
        raise ValueError(
            f"{gt_csv} has {len(positions)} poses, results cover {payload['frames']} frames"
        )
    positions = positions[: payload["frames"]]
    exclude = int(payload["exclude_frames"])
    revisits = revisit_mask(positions, exclude, radius)
    methods = {}
    for name, block in payload["methods"].items():
        scored = score_method(block["queries"], positions, revisits, radius)
        scored["score"] = block["score"]
        scored["seconds"] = block["seconds"]
        scored["ms_per_frame"] = 1000.0 * block["seconds"] / payload["frames"]
        methods[name] = scored
    return {
        "label": label,
        "results": str(results_path),
        "gt_csv": str(gt_csv),
        "frames": payload["frames"],
        "exclude_frames": exclude,
        "revisit_radius_m": radius,
        "revisit_queries": int(revisits.sum()),
        "methods": methods,
    }


def markdown(sequences: list[dict]) -> str:
    method_names = sorted({m for s in sequences for m in s["methods"]})
    lines = [
        "| Sequence | Revisits | " + " | ".join(
            f"{m} F1max / R@100P / ms" for m in method_names) + " |",
        "|---|---:|" + "---:|" * len(method_names),
    ]
    for seq in sequences:
        cells = []
        for m in method_names:
            r = seq["methods"].get(m)
            cells.append("-" if r is None else
                         f"{r['f1_max']:.3f} / {r['recall_at_100_precision']:.3f}"
                         f" / {r['ms_per_frame']:.1f}")
        lines.append(f"| {seq['label']} | {seq['revisit_queries']} | " +
                     " | ".join(cells) + " |")
    return "\n".join(lines) + "\n"


def plot(sequences: list[dict], path: Path) -> None:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, axes = plt.subplots(1, len(sequences), figsize=(4.2 * len(sequences), 4),
                             squeeze=False)
    for ax, seq in zip(axes[0], sequences):
        for name, r in sorted(seq["methods"].items()):
            ax.plot(r["recall"], r["precision"],
                    label=f"{name} (F1 {r['f1_max']:.2f})")
        ax.set_title(f"{seq['label']} ({seq['revisit_queries']} revisits)")
        ax.set_xlabel("Recall")
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1.02)
        ax.grid(alpha=0.3)
        ax.legend(loc="lower left", fontsize=8)
    axes[0][0].set_ylabel("Precision")
    fig.tight_layout()
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=130)


def main() -> int:
    args = parse_args()
    if len(args.results) != len(args.gt_csv):
        raise SystemExit("--results and --gt-csv need the same number of entries")
    labels = args.label or [p.stem for p in args.gt_csv]
    if len(labels) != len(args.results):
        raise SystemExit("--label needs one entry per --results file")
    sequences = [
        evaluate(r, g, label, args.revisit_radius)
        for r, g, label in zip(args.results, args.gt_csv, labels)
    ]
    report = {
        "schema_version": 1,
        "protocol": {
            "retrieval": "top-1 among frames at least exclude_frames older",
            "revisit_radius_m": args.revisit_radius,
            "true_positive": "predicted match within revisit radius (GT positions)",
            "recall_denominator": "queries that have a GT revisit",
        },
        "sequences": sequences,
    }
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(report, indent=1) + "\n")
    if args.output_md:
        args.output_md.write_text(markdown(sequences))
    if args.output_png:
        plot(sequences, args.output_png)
    print(markdown(sequences), end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
