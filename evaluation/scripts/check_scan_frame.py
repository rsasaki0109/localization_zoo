#!/usr/bin/env python3
"""Estimate where the spinning lidar sits in the frame a dataset stores its scans in.

Range-image methods (SuMa, DiLO, ELO, InTEn-LOAM) assume a sensor-centred,
z-up scan with known field of view; some datasets store scans in a body frame
instead (NCLT: z down, HDL-32E 0.957 m above the origin). For a sensor with
evenly spaced beams, elevations measured from the right origin and axis show
the beam spacing as a sharp periodic pattern; this script searches the height
offset and the sign of z for the strongest pattern and prints the elevation
range seen from there (NCLT scores about 30; a score below about 5, e.g.
KITTI's HDL-64E with two blocks of different spacing, means no clear
pattern and no answer). Run it on a few scans (ASCII or binary PCD, or KITTI
.bin).

  python3 evaluation/scripts/check_scan_frame.py --beam-spacing-deg 1.333 \
      dogfooding_results/nclt_2013_01_10_full/00000100/cloud.pcd ...

The answer feeds `pcd_dogfooding --input-sensor-pose x y z roll pitch yaw`:
an offset c with the sign -1 means the sensor is at z = c and gravity-up is
-z, i.e. `--input-sensor-pose 0 0 c 180 0 0`.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np


def load_points(path: Path) -> np.ndarray:
    if path.suffix == ".bin":
        return np.fromfile(path, dtype=np.float32).reshape(-1, 4)[:, :3].astype(np.float64)
    raw = path.read_bytes()
    end = raw.index(b"DATA")
    header = raw[:end].decode("ascii", "replace").split("\n")
    fields = sizes = types = counts = None
    points = 0
    for line in header:
        if not line.split() or line.startswith("#"):
            continue
        key, *values = line.split()
        if key == "FIELDS":
            fields = values
        elif key == "SIZE":
            sizes = [int(v) for v in values]
        elif key == "TYPE":
            types = values
        elif key == "COUNT":
            counts = [int(v) for v in values]
        elif key == "POINTS":
            points = int(values[0])
    data_line_end = raw.index(b"\n", end)
    mode = raw[end:data_line_end].decode().split()[1]
    body = raw[data_line_end + 1:]
    ix = [fields.index(name) for name in ("x", "y", "z")]
    if mode == "ascii":
        rows = np.loadtxt(body.decode().splitlines(), ndmin=2)
        return rows[:, ix]
    if mode != "binary":
        raise SystemExit(f"{path}: unsupported PCD data mode {mode}")
    counts = counts or [1] * len(fields)
    dtype = np.dtype([(f, {"F": "f", "U": "u", "I": "i"}[t] + str(s), (c,) if c > 1 else ())
                      for f, s, t, c in zip(fields, sizes, types, counts)])
    rows = np.frombuffer(body, dtype=dtype, count=points)
    return np.stack([rows[n].astype(np.float64) for n in ("x", "y", "z")], axis=1)


def periodicity(elevation_deg: np.ndarray, spacing_deg: float) -> float:
    hist, _ = np.histogram(elevation_deg, bins=np.arange(-60.0, 60.0, 0.1))
    spectrum = np.abs(np.fft.rfft(hist - hist.mean()))
    freqs = np.fft.rfftfreq(len(hist), 0.1)
    k = int(np.argmin(np.abs(freqs - 1.0 / spacing_deg)))
    return float(spectrum[max(k - 2, 0):k + 3].max() / max(np.median(spectrum), 1e-9))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("scans", nargs="+", type=Path)
    parser.add_argument("--beam-spacing-deg", type=float, required=True,
                        help="vertical beam spacing (HDL-32E 1.333, VLP-16 2.0)")
    parser.add_argument("--max-offset-m", type=float, default=2.5)
    args = parser.parse_args()

    points = np.vstack([load_points(p) for p in args.scans])
    horizontal = np.hypot(points[:, 0], points[:, 1])
    keep = horizontal > 0.5
    points, horizontal = points[keep], horizontal[keep]

    results = []
    for sign in (1, -1):
        for offset in np.arange(-args.max_offset_m, args.max_offset_m + 1e-9, 0.05):
            up = sign * (points[:, 2] - offset)
            elevation = np.degrees(np.arctan2(up, horizontal))
            results.append((periodicity(elevation, args.beam_spacing_deg), sign, offset,
                            np.percentile(elevation, [0.2, 99.8])))
    results.sort(key=lambda r: -r[0])
    print("score  up-axis  sensor z  elevation 0.2-99.8 %")
    for score, sign, offset, (low, high) in results[:6]:
        print(f"{score:5.1f}  {'+z' if sign > 0 else '-z':>7}  {offset:+8.2f}  {low:+6.1f} .. {high:+6.1f} deg")
    print("The two up-axis signs score the same; take the one whose lower elevations hit the ground "
          "(the largest flat surface below the sensor).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
