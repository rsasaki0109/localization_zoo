#!/usr/bin/env python3

"""Prepare NCLT (UMich North Campus) velodyne_sync + ground truth + IMU for pcd_dogfooding.

NCLT (http://robots.engin.umich.edu/nclt/) ships per-session:
  - velodyne_data/<date>_vel.tar.gz  -> velodyne_sync/<utime>.bin (one motion-corrected
    body-frame scan per file; each point is 8 bytes: uint16 x,y,z + uint8 intensity + uint8 layer,
    decoded as value * 0.005 - 100.0 metres)
  - ground_truth/groundtruth_<date>.csv  -> utime, x, y, z, roll, pitch, yaw (NaN where lost)
  - sensor_data/<date>_sen.tar.gz -> <date>/ms25.csv (utime, mag x/y/z, accel x/y/z, gyro x/y/z)

This writes the repository's standard layout:
  - dogfooding_results/nclt_<date>_<N>/00000000/cloud.pcd, ... (+ frame_timestamps.csv, imu.csv)
  - experiments/reference_data/nclt_<date>_<N>_gt.csv
See evaluation/scripts/SETUP_NCLT_BENCHMARK.md.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import math
import struct
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


def read_velodyne_sync(path: Path) -> list[tuple[float, float, float, float]]:
    """Decode one NCLT velodyne_sync .bin into a list of (x, y, z, intensity)."""
    points: list[tuple[float, float, float, float]] = []
    scaling = 0.005
    offset = -100.0
    data = path.read_bytes()
    n = len(data) // 8
    for k in range(n):
        base = k * 8
        x_s, y_s, z_s = struct.unpack_from("<HHH", data, base)
        intensity, _layer = struct.unpack_from("<BB", data, base + 6)
        x = x_s * scaling + offset
        y = y_s * scaling + offset
        z = z_s * scaling + offset
        points.append((x, y, z, float(intensity)))
    return points


def write_pcd_ascii(points: list[tuple[float, float, float, float]], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    n = len(points)
    with path.open("w") as f:
        f.write("# .PCD v0.7 - Point Cloud Data file format\n")
        f.write("VERSION 0.7\n")
        f.write("FIELDS x y z intensity\n")
        f.write("SIZE 4 4 4 4\n")
        f.write("TYPE F F F F\n")
        f.write("COUNT 1 1 1 1\n")
        f.write(f"WIDTH {n}\n")
        f.write("HEIGHT 1\n")
        f.write("VIEWPOINT 0 0 0 1 0 0 0\n")
        f.write(f"POINTS {n}\n")
        f.write("DATA ascii\n")
        for x, y, z, i in points:
            f.write(f"{x:.4f} {y:.4f} {z:.4f} {i:.1f}\n")


class GroundTruth:
    """NCLT ground truth indexed by microsecond utime, NaN rows dropped."""

    def __init__(self, csv_path: Path) -> None:
        self.utimes: list[int] = []
        self.poses: list[tuple[float, float, float, float, float, float]] = []
        with csv_path.open() as f:
            for row in csv.reader(f):
                if len(row) < 7:
                    continue
                try:
                    utime = int(float(row[0]))
                    vals = [float(v) for v in row[1:7]]
                except ValueError:
                    continue
                if any(math.isnan(v) for v in vals):
                    continue
                self.utimes.append(utime)
                self.poses.append(tuple(vals))  # x, y, z, roll, pitch, yaw
        if not self.utimes:
            raise RuntimeError(f"No valid (non-NaN) GT rows in {csv_path}")

    def nearest(self, utime: int) -> tuple[tuple[float, ...], int]:
        """Return (pose, abs_dt_microseconds) for the nearest GT to utime."""
        idx = bisect.bisect_left(self.utimes, utime)
        best = None
        for j in (idx - 1, idx):
            if 0 <= j < len(self.utimes):
                dt = abs(self.utimes[j] - utime)
                if best is None or dt < best[1]:
                    best = (self.poses[j], dt)
        return best


def load_ms25_imu(path: Path) -> list[tuple[int, float, float, float, float, float, float]]:
    """Load NCLT ms25.csv -> (utime, gyro x/y/z, accel x/y/z). Columns:
    utime, mag x/y/z, accel x/y/z, rot_rate(gyro) x/y/z."""
    out = []
    with path.open() as f:
        for row in csv.reader(f):
            if len(row) < 10:
                continue
            try:
                utime = int(float(row[0]))
                accel = (float(row[4]), float(row[5]), float(row[6]))
                gyro = (float(row[7]), float(row[8]), float(row[9]))
            except ValueError:
                continue
            out.append((utime, gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2]))
    out.sort(key=lambda r: r[0])
    return out


def export_window(
    *,
    velodyne_dir: Path,
    gt: GroundTruth,
    imu: list,
    output_root: Path,
    reference_data_dir: Path,
    start_frame: int,
    max_frames: int | None,
    stem: str,
    gt_tolerance_s: float,
) -> tuple[str, Path, Path, int]:
    bins = sorted(velodyne_dir.glob("*.bin"), key=lambda p: int(p.stem))
    if not bins:
        raise RuntimeError(f"No velodyne .bin files in {velodyne_dir}")

    # Keep only scans with a nearby valid GT pose.
    tol_us = int(gt_tolerance_s * 1e6)
    good = []
    for b in bins:
        utime = int(b.stem)
        pose, dt = gt.nearest(utime)
        if dt <= tol_us:
            good.append((utime, b, pose))
    if not good:
        raise RuntimeError("No velodyne scans have a GT pose within tolerance")

    end_idx = len(good) if max_frames is None or max_frames < 0 else min(start_frame + max_frames, len(good))
    selected = good[start_frame:end_idx]
    if not selected:
        raise RuntimeError(f"No frames in range [{start_frame}, {end_idx})")

    output_dir = output_root / stem
    output_dir.mkdir(parents=True, exist_ok=True)
    gt_csv = reference_data_dir / f"{stem}_gt.csv"
    gt_csv.parent.mkdir(parents=True, exist_ok=True)

    ts_path = output_dir / "frame_timestamps.csv"
    imu_idx = 0
    with gt_csv.open("w", newline="") as gf, ts_path.open("w", newline="") as tf, \
            (output_dir / "imu.csv").open("w", newline="") as imf:
        gw = csv.writer(gf)
        gw.writerow(["timestamp", "lidar_pose.x", "lidar_pose.y", "lidar_pose.z",
                     "lidar_pose.roll", "lidar_pose.pitch", "lidar_pose.yaw"])
        tw = csv.writer(tf)
        tw.writerow(["frame_idx", "timestamp", "points"])
        iw = csv.writer(imf)
        iw.writerow(["stamp", "angular_vel.x", "angular_vel.y", "angular_vel.z",
                     "linear_acc.x", "linear_acc.y", "linear_acc.z"])

        first_utime = selected[0][0]
        last_utime = selected[-1][0]

        for i, (utime, src, pose) in enumerate(selected):
            pts = read_velodyne_sync(src)
            dst = output_dir / f"{i:08d}"
            write_pcd_ascii(pts, dst / "cloud.pcd")
            ts_sec = utime * 1e-6
            gw.writerow([f"{ts_sec:.6f}"] + [f"{v:.10f}" for v in pose])
            tw.writerow([i, f"{ts_sec:.6f}", len(pts)])

        # IMU rows spanning the window time range (seconds).
        for (utime, gx, gy, gz, ax, ay, az) in imu:
            if utime < first_utime:
                continue
            if utime > last_utime:
                break
            iw.writerow([f"{utime * 1e-6:.6f}",
                         f"{gx:.9f}", f"{gy:.9f}", f"{gz:.9f}",
                         f"{ax:.9f}", f"{ay:.9f}", f"{az:.9f}"])

    return stem, output_dir, gt_csv, len(selected)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Convert NCLT session into pcd_dogfooding inputs.")
    p.add_argument("--velodyne-dir", required=True,
                   help="Directory with velodyne_sync/*.bin (extracted <date>_vel.tar.gz).")
    p.add_argument("--ground-truth", required=True, help="groundtruth_<date>.csv path.")
    p.add_argument("--ms25", default="", help="ms25.csv path (IMU). Optional; imu.csv emitted when given.")
    p.add_argument("--date", default="", help="Session date tag for the output stem (e.g. 2013-01-10).")
    p.add_argument("--start-frame", type=int, default=0)
    p.add_argument("--max-frames", type=int, default=120, help="-1 for all (full).")
    p.add_argument("--include-full", action="store_true")
    p.add_argument("--gt-tolerance-s", type=float, default=0.05)
    p.add_argument("--output-root", default=str(REPO_ROOT / "dogfooding_results"))
    p.add_argument("--reference-data-dir", default=str(REPO_ROOT / "experiments" / "reference_data"))
    return p.parse_args()


def main() -> int:
    args = parse_args()
    velodyne_dir = Path(args.velodyne_dir).resolve()
    if velodyne_dir.name != "velodyne_sync" and (velodyne_dir / "velodyne_sync").is_dir():
        velodyne_dir = velodyne_dir / "velodyne_sync"
    gt = GroundTruth(Path(args.ground_truth).resolve())
    imu = load_ms25_imu(Path(args.ms25).resolve()) if args.ms25 else []

    output_root = Path(args.output_root)
    reference_data_dir = Path(args.reference_data_dir)
    output_root.mkdir(parents=True, exist_ok=True)
    reference_data_dir.mkdir(parents=True, exist_ok=True)

    date = args.date or "session"
    slug = f"nclt_{date}".replace("-", "_").replace(" ", "_")

    exports = []
    window_n = args.max_frames if args.max_frames >= 0 else "full"
    exports.append(export_window(
        velodyne_dir=velodyne_dir, gt=gt, imu=imu, output_root=output_root,
        reference_data_dir=reference_data_dir, start_frame=args.start_frame,
        max_frames=args.max_frames, stem=f"{slug}_{window_n}", gt_tolerance_s=args.gt_tolerance_s))
    if args.include_full:
        exports.append(export_window(
            velodyne_dir=velodyne_dir, gt=gt, imu=imu, output_root=output_root,
            reference_data_dir=reference_data_dir, start_frame=0, max_frames=-1,
            stem=f"{slug}_full", gt_tolerance_s=args.gt_tolerance_s))

    print("=== NCLT exports ===")
    for stem, seq_dir, gcsv, n in exports:
        print(f"{stem}: {n} frames -> {seq_dir}")
        print(f"  GT: {gcsv}")
    print("")
    print("Smoke benchmark:")
    print(f"  ./build/evaluation/pcd_dogfooding {exports[0][1]} {exports[0][2]} "
          f"--methods litamin2,kiss_icp,genz_icp,ndt --summary-json /tmp/nclt_smoke.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
