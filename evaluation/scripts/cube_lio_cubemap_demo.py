#!/usr/bin/env python3
"""CUBE-LIO slide-faithful preprocessing demo: cubemap + IGM + semi-dense mask.

Implements the projection and feature steps described in:
  Liu Yang, "Cubemap-based LiDAR-inertial odometry using LiDAR reflectance",
  Robotics Symposia slides, Docswell (linked from https://x.com/scomup/status/2035684276591882292).

Scope: intensity cubemap rasterization, Gaussian-smoothed Sobel IGM, quantile feature selection.
Does NOT implement photometric residuals or IMU coupling (full CUBE-LIO).
"""

from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


FACE_NAMES = ("+X", "-X", "+Y", "-Y", "+Z", "-Z")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__)
    mx = p.add_mutually_exclusive_group(required=True)
    mx.add_argument("--pcd", type=Path, help="Single binary PCD v0.7 with x y z intensity (F).")
    mx.add_argument(
        "--sequence-pcd-dir",
        type=Path,
        help="Dogfooding-style tree: 00000000/cloud.pcd, 00000001/… — batch cubemap/IGM.",
    )
    p.add_argument("--output-dir", required=True, type=Path)
    p.add_argument(
        "--max-sequence-frames",
        type=int,
        default=10,
        help="With --sequence-pcd-dir, max frames to process (stops early if cloud.pcd missing).",
    )
    p.add_argument(
        "--sequence-include-face-pngs",
        action="store_true",
        help="With --sequence-pcd-dir, write 12 PNGs per frame (default: montage + CSV only).",
    )
    p.add_argument(
        "--export-features-json",
        action="store_true",
        help="Also write features.json (array of {face,row,col,igm,smoothed_intensity}) for tooling.",
    )
    p.add_argument("--size", type=int, default=512, help="Square face resolution (pixels).")
    p.add_argument(
        "--gaussian-sigma",
        type=float,
        default=1.0,
        help="Sigma in pixels for separable Gaussian blur before Sobel.",
    )
    p.add_argument(
        "--feature-quantile",
        type=float,
        default=0.90,
        help="IGM values above this quantile become semi-dense features.",
    )
    p.add_argument(
        "--min-range",
        type=float,
        default=0.5,
        help="Drop points closer than this (m) to origin.",
    )
    p.add_argument(
        "--max-range",
        type=float,
        default=120.0,
        help="Drop points farther than this (m).",
    )
    p.add_argument(
        "--montage",
        action="store_true",
        help="Write cubemap_montage_intensity.png and cubemap_montage_igm.png (2×3 faces).",
    )
    p.add_argument(
        "--no-per-face-pngs",
        action="store_true",
        help="Skip face_*_intensity.png / face_*_igm.png (still writes CSV, meta, montage if --montage).",
    )
    return p.parse_args()


def read_binary_pcd_xyz_intensity(path: Path) -> tuple[np.ndarray, np.ndarray]:
    raw_hdr = path.read_bytes()
    header_end = raw_hdr.find(b"DATA binary\n")
    if header_end < 0:
        raise RuntimeError(f"{path}: expected DATA binary header")
    header_end += len(b"DATA binary\n")
    header = raw_hdr[:header_end].decode("ascii", errors="replace")
    n_points = 0
    for line in header.splitlines():
        if line.startswith("POINTS "):
            n_points = int(line.split()[1])
            break
    if n_points <= 0:
        raise RuntimeError(f"{path}: could not parse POINTS")

    blob = raw_hdr[header_end:]
    need = n_points * 4 * 4
    if len(blob) < need:
        raise RuntimeError(f"{path}: truncated binary body")
    pts = np.frombuffer(blob[:need], dtype="<f4").reshape(-1, 4)
    xyz = pts[:, :3].astype(np.float64)
    intensity = pts[:, 3].astype(np.float64)
    return xyz, intensity


def directions(xyz: np.ndarray) -> np.ndarray:
    n = np.linalg.norm(xyz, axis=1, keepdims=True)
    n = np.maximum(n, 1e-12)
    return xyz / n


def dir_to_cube_face_uv(d: np.ndarray) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Vectorized cubemap mapping. One row per point.

    Returns face index 0..5, u, v in [-1, 1] on that face (OpenGL-style layout).
    """
    x = d[:, 0]
    y = d[:, 1]
    z = d[:, 2]
    ax = np.abs(x)
    ay = np.abs(y)
    az = np.abs(z)

    face = np.zeros(len(d), dtype=np.int32)
    u = np.zeros(len(d), dtype=np.float64)
    v = np.zeros(len(d), dtype=np.float64)

    m = (ax >= ay) & (ax >= az)
    if np.any(m):
        ma = ax[m]
        pos = x[m] > 0
        neg = ~pos
        idx = np.where(m)[0]
        if np.any(pos):
            i = idx[pos]
            face[i] = 0
            u[i] = -z[i] / np.maximum(np.abs(x[i]), 1e-12)
            v[i] = -y[i] / np.maximum(np.abs(x[i]), 1e-12)
        if np.any(neg):
            i = idx[neg]
            face[i] = 1
            u[i] = z[i] / np.maximum(np.abs(x[i]), 1e-12)
            v[i] = -y[i] / np.maximum(np.abs(x[i]), 1e-12)

    m = (~((ax >= ay) & (ax >= az))) & (ay >= az)
    if np.any(m):
        idx = np.where(m)[0]
        pos = y[m] > 0
        neg = ~pos
        if np.any(pos):
            i = idx[pos]
            face[i] = 2
            u[i] = x[i] / np.maximum(np.abs(y[i]), 1e-12)
            v[i] = z[i] / np.maximum(np.abs(y[i]), 1e-12)
        if np.any(neg):
            i = idx[neg]
            face[i] = 3
            u[i] = x[i] / np.maximum(np.abs(y[i]), 1e-12)
            v[i] = -z[i] / np.maximum(np.abs(y[i]), 1e-12)

    m = (~((ax >= ay) & (ax >= az))) & (~(ay >= az))
    if np.any(m):
        idx = np.where(m)[0]
        pos = z[m] > 0
        neg = ~pos
        if np.any(pos):
            i = idx[pos]
            face[i] = 4
            u[i] = x[i] / np.maximum(np.abs(z[i]), 1e-12)
            v[i] = -y[i] / np.maximum(np.abs(z[i]), 1e-12)
        if np.any(neg):
            i = idx[neg]
            face[i] = 5
            u[i] = -x[i] / np.maximum(np.abs(z[i]), 1e-12)
            v[i] = -y[i] / np.maximum(np.abs(z[i]), 1e-12)

    return face, u, v


def uv_to_pixel(u: np.ndarray, v: np.ndarray, size: int) -> tuple[np.ndarray, np.ndarray]:
    col = ((u + 1.0) * 0.5 * (size - 1)).clip(0, size - 1)
    row = ((v + 1.0) * 0.5 * (size - 1)).clip(0, size - 1)
    return row.astype(np.int32), col.astype(np.int32)


def rasterize_max_intensity(
    face: np.ndarray,
    row: np.ndarray,
    col: np.ndarray,
    intensity: np.ndarray,
    size: int,
) -> np.ndarray:
    flat = face * size * size + row * size + col
    grids_flat = np.full(6 * size * size, -np.inf, dtype=np.float64)
    np.maximum.at(grids_flat, flat, intensity)
    grids = grids_flat.reshape(6, size, size)
    return np.where(np.isfinite(grids), grids, 0.0)


def gaussian_kernel1d(sigma: float) -> np.ndarray:
    if sigma <= 0:
        return np.array([1.0], dtype=np.float64)
    radius = max(1, int(np.ceil(3 * sigma)))
    x = np.arange(-radius, radius + 1, dtype=np.float64)
    k = np.exp(-0.5 * (x / sigma) ** 2)
    k /= k.sum()
    return k


def convolve_separable(img: np.ndarray, k: np.ndarray) -> np.ndarray:
    kh = k.shape[0]
    pad = kh // 2
    tmp = np.pad(img, ((0, 0), (pad, pad)), mode="edge")
    acc = np.zeros_like(img)
    for i in range(img.shape[0]):
        acc[i, :] = np.convolve(tmp[i, :], k, mode="valid")
    tmp2 = np.pad(acc, ((pad, pad), (0, 0)), mode="edge")
    out = np.zeros_like(img)
    for j in range(img.shape[1]):
        out[:, j] = np.convolve(tmp2[:, j], k, mode="valid")
    return out


def sobel_magnitude(img: np.ndarray) -> np.ndarray:
    kx = np.array([[-1.0, 0.0, 1.0], [-2.0, 0.0, 2.0], [-1.0, 0.0, 1.0]], dtype=np.float64)
    ky = kx.T

    def conv2_same(x: np.ndarray, k: np.ndarray) -> np.ndarray:
        r, c = k.shape
        pr, pc = r // 2, c // 2
        p = np.pad(x, ((pr, pr), (pc, pc)), mode="edge")
        out = np.zeros_like(x)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                out[i, j] = (p[i : i + r, j : j + c] * k).sum()
        return out

    gx = conv2_same(img, kx)
    gy = conv2_same(img, ky)
    return np.sqrt(gx * gx + gy * gy)


def save_png(path: Path, gray01: np.ndarray) -> None:
    plt.imsave(path, gray01, cmap="gray", vmin=0.0, vmax=1.0)


def save_montage_panels(panels: list[np.ndarray], path: Path, suptitle: str) -> None:
    fig, axes = plt.subplots(2, 3, figsize=(10.5, 7.0))
    for ax, fi in zip(axes.flat, range(6)):
        ax.imshow(panels[fi], cmap="gray", vmin=0.0, vmax=1.0)
        ax.set_title(FACE_NAMES[fi], fontsize=9)
        ax.axis("off")
    fig.suptitle(suptitle, fontsize=11)
    fig.tight_layout()
    fig.savefig(path, dpi=140)
    plt.close(fig)


def process_one_cloud(
    pcd_path: Path,
    out_dir: Path,
    args: argparse.Namespace,
    *,
    frame_index: int | None,
) -> int:
    out_dir.mkdir(parents=True, exist_ok=True)
    xyz, intensity = read_binary_pcd_xyz_intensity(pcd_path)
    rng = np.linalg.norm(xyz, axis=1)
    m = (rng >= args.min_range) & (rng <= args.max_range)
    xyz, intensity = xyz[m], intensity[m]
    d = directions(xyz)
    face, u, v = dir_to_cube_face_uv(d)
    row, col = uv_to_pixel(u, v, args.size)
    grids = rasterize_max_intensity(face, row, col, intensity, args.size)

    imax = max(float(grids.max()), 1e-9)
    k = gaussian_kernel1d(args.gaussian_sigma)
    feature_rows: list[tuple[str, int, int, float, float]] = []
    intensity_panels: list[np.ndarray] = []
    igm_panels: list[np.ndarray] = []

    for fi in range(6):
        name = FACE_NAMES[fi].replace("+", "pos_").replace("-", "neg_")
        raw = grids[fi].astype(np.float64)
        if k.size > 1:
            sm = convolve_separable(raw, k)
        else:
            sm = raw
        igm = sobel_magnitude(sm)
        valid = raw > 0
        if np.any(valid):
            thr = float(np.quantile(igm[valid], args.feature_quantile))
            feat_mask = (igm >= thr) & valid
        else:
            feat_mask = np.zeros_like(igm, dtype=bool)
        rows, cols = np.where(feat_mask)
        for r, c in zip(rows.tolist(), cols.tolist(), strict=False):
            feature_rows.append((FACE_NAMES[fi], int(r), int(c), float(igm[r, c]), float(sm[r, c])))

        inten_n = (raw / imax).clip(0.0, 1.0)
        igm_n = igm / max(float(igm.max()), 1e-9)
        intensity_panels.append(inten_n)
        igm_panels.append(igm_n)
        if not args.no_per_face_pngs:
            save_png(out_dir / f"face_{name}_intensity.png", inten_n)
            save_png(out_dir / f"face_{name}_igm.png", igm_n)

    tag = f" frame={frame_index}" if frame_index is not None else ""
    if args.montage:
        save_montage_panels(
            intensity_panels,
            out_dir / "cubemap_montage_intensity.png",
            f"Cubemap intensity (6 faces){tag}",
        )
        save_montage_panels(
            igm_panels,
            out_dir / "cubemap_montage_igm.png",
            f"IGM after Gaussian + Sobel (6 faces){tag}",
        )

    csv_path = out_dir / "features.csv"
    with csv_path.open("w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["face", "row", "col", "igm", "smoothed_intensity"])
        w.writerows(feature_rows)

    if getattr(args, "export_features_json", False):
        fj = out_dir / "features.json"
        payload = {
            "pcd": str(pcd_path.resolve()),
            "frame_index": frame_index,
            "n_features": len(feature_rows),
            "features": [
                {
                    "face": fe[0],
                    "row": fe[1],
                    "col": fe[2],
                    "igm": fe[3],
                    "smoothed_intensity": fe[4],
                }
                for fe in feature_rows
            ],
        }
        fj.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")

    meta = out_dir / "run_meta.txt"
    meta.write_text(
        "\n".join([
            f"pcd: {pcd_path}",
            f"frame_index: {frame_index}",
            f"points_used: {xyz.shape[0]}",
            f"face_size: {args.size}",
            f"gaussian_sigma: {args.gaussian_sigma}",
            f"feature_quantile: {args.feature_quantile}",
            f"features: {len(feature_rows)}",
            f"montage: {args.montage}",
            f"no_per_face_pngs: {args.no_per_face_pngs}",
            f"export_features_json: {getattr(args, 'export_features_json', False)}",
            "",
            "Reference: Docswell slides by Liu Yang (CUBE-LIO), Robotics Symposia.",
        ]),
        encoding="utf-8",
    )

    extra = []
    if not args.no_per_face_pngs:
        extra.append("12 face PNGs")
    if args.montage:
        extra.append("2 montage PNGs")
    print(
        f"[done]{tag} {' + '.join(extra) or 'no PNGs'} + {csv_path} ({len(feature_rows)} feature pixels)"
    )
    return len(feature_rows)


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    if args.sequence_pcd_dir is not None:
        seq_args = argparse.Namespace(**vars(args))
        seq_args.montage = True
        seq_args.no_per_face_pngs = not args.sequence_include_face_pngs
        n_ok = 0
        for i in range(args.max_sequence_frames):
            pcd_path = args.sequence_pcd_dir / f"{i:08d}" / "cloud.pcd"
            if not pcd_path.is_file():
                print(f"[sequence] stop: missing {pcd_path} (processed {n_ok} frames)")
                break
            frame_out = args.output_dir / f"{i:08d}"
            process_one_cloud(pcd_path, frame_out, seq_args, frame_index=i)
            n_ok += 1
        print(f"[sequence] done: {n_ok} frames under {args.output_dir}")
        return 0

    assert args.pcd is not None
    process_one_cloud(args.pcd, args.output_dir, args, frame_index=None)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
