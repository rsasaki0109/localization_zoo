# 2D laser scan benchmark prep (scan_dogfooding)

Planar `LaserScan` odometry uses `scan_dogfooding`. **Leaderboard and artifact index:**
[`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md).

Fixture layout:

```
<scan_dir>/
  scan_meta.json          # angle_min, angle_max, angle_increment, range_min, range_max, scan_rate_hz
  00000000/scan.csv       # comma-separated ranges (one row)
  00000001/scan.csv
  gt.csv                  # timestamp,x,y,yaw [rad]
  frame_timestamps.csv    # optional audit (written by prep script)
```

## Synthetic fixtures (no download)

```bash
# Fast mixed-motion smoke (60 frames, box world)
python3 evaluation/scripts/generate_rf2o_smoke_fixture.py

# Slow-motion fixture (120 frames, ~0.08 m/frame, bounded box)
python3 evaluation/scripts/generate_rf2o_corridor_fixture.py
```

Slow-motion fixture: same bounded box raycast as `rf2o_smoke` (0.08 m/frame mixed motion,
120 frames, ~9.5 m). Pure parallel-wall corridors without near forward structure are **degenerate**
for 2D scan odometry — use ROS bag exports for real hallway data instead.

Validate layout:

```bash
python3 evaluation/scripts/prepare_2d_scan_inputs.py \
  --output-dir evaluation/fixtures/rf2o_corridor \
  --validate-only
```

## ROS1 bag export

Requires `pip install rosbags`.

List topics:

```bash
python3 evaluation/scripts/prepare_2d_scan_inputs.py \
  --ros1-bag /path/to/log.bag \
  --output-dir /tmp/scan2d_out \
  --inspect
```

Export scans + interpolated GT:

```bash
python3 evaluation/scripts/prepare_2d_scan_inputs.py \
  --ros1-bag /path/to/log.bag \
  --scan-topic /scan \
  --gt-csv /path/to/gt.csv \
  --output-dir dogfooding_results/intel_hallway_300 \
  --max-frames 300
```

GT CSV must include `timestamp,x,y,yaw` (or `x,y,yaw` with implicit index timestamps).
TUM poses are also supported via `--gt-tum`.

## Public 2D datasets (manual download)

| Dataset | Notes |
|---------|--------|
| [Intel Research Lab](https://www.intelrealsense.com/solution-areas/robotics/) / Radish | Classic 2D hallway logs; often distributed as ROS bags or `.flog` |
| [Freiburg 2D](https://www2.informatik.uni-freiburg.de/~stachnis/datasets/) | Raw `.log` + ground truth; convert externally to bag or CSV |
| [Radish (UMD)](http://radish.sourceforge.net/) | `.flog` format; use community converters to ROS bag first |
| [Bonn 2D-SLAM JSON](https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip) | Pre-converted Intel / fr079 / MIT splits (`train.json`, `val.json`, `test.json`) |

### Bonn JSON → committed fixture (Intel Lab val)

```bash
wget -O /tmp/2dslam.zip \
  https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip
unzip /tmp/2dslam.zip intel/val.json -d /tmp/2dslam
python3 evaluation/scripts/prepare_bonn_2dslam_inputs.py \
  --json /tmp/2dslam/intel/val.json \
  --output-dir evaluation/fixtures/intel_val_73
```

The repo ships these Bonn JSON fixtures for CI and the public 2D leaderboard:

| Fixture | Source | Frames | Beams | Traj [m] | Size |
|---------|--------|--------|-------|----------|------|
| `intel_val_73` | `intel/val.json` | 73 | 180 | ~378 | ~600 KB |
| `fr079_val_384` | `fr079/val.json` | 384 | 360 | ~373 | ~3.1 MB |
| `mit_val_33` | `mit/val.json` | 33 | 360 | ~267 | ~280 KB |

GT is dataset odometry (scan-matched proxy, not centimeter truth).

### fr079 (Freiburg building 079)

```bash
unzip /tmp/2dslam.zip fr079/val.json -d /tmp/2dslam
python3 evaluation/scripts/prepare_bonn_2dslam_inputs.py \
  --json /tmp/2dslam/fr079/val.json \
  --output-dir evaluation/fixtures/fr079_val_384
```

### MIT CSAIL

```bash
unzip /tmp/2dslam.zip mit/val.json -d /tmp/2dslam
python3 evaluation/scripts/prepare_bonn_2dslam_inputs.py \
  --json /tmp/2dslam/mit/val.json \
  --output-dir evaluation/fixtures/mit_val_33
```

## Run scan_dogfooding

```bash
cmake --build build --target scan_dogfooding

./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 \
  evaluation/fixtures/intel_val_73/gt.csv \
  --methods rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp,karto_matcher \
  --wheel-odom-from-gt \
  --summary-json docs/benchmarks/scan2d/intel_val_73.json
```

Methods (papers 43–50 + Karto extension): `rf2o`, `pl_icp`, `csm`, `kinematic_icp`, `psm`, `ndt_2d`, `idc`, `mb_icp`, `karto_matcher`.
Kinematic-ICP requires `--wheel-odom-from-gt` (or real wheel odometry when available).

Metrics:

- **ATE [m]**: root-mean-square xy position error vs GT (per frame).
- **Drift [%]**: KITTI-style relative pose error (% of segment length). Segment length
  scales down on short trajectories (same rule as `pcd_dogfooding`).

## Smoke benchmark (committed fixtures)

CI runs all nine methods on a 20-frame Intel slice:

```bash
bash evaluation/scripts/smoke_scan2d_fixture.sh
```

Full fixture refresh (4 benchmarks → `docs/benchmarks/scan2d/*.json`):

```bash
bash evaluation/scripts/run_scan2d_benchmark.sh
```

See [`docs/benchmarks/scan2d/README.md`](../../docs/benchmarks/scan2d/README.md) for the
leaderboard and per-method notes.
