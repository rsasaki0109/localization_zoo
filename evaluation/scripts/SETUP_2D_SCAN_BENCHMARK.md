# 2D laser scan benchmark prep (scan_dogfooding)

Planar `LaserScan` odometry uses `scan_dogfooding` with this layout:

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

The repo ships `evaluation/fixtures/intel_val_73/` (73 frames, 180 beams, ~600 KB) for CI and
the public 2D leaderboard. GT is dataset odometry (scan-matched proxy).

## Run scan_dogfooding

```bash
cmake --build build --target scan_dogfooding

./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_corridor \
  evaluation/fixtures/rf2o_corridor/gt.csv \
  --methods rf2o,pl_icp \
  --summary-json /tmp/rf2o_corridor.json
```

Metrics:

- **ATE [m]**: root-mean-square xy position error vs GT (per frame).
- **Drift [%]**: KITTI-style relative pose error (% of segment length). Segment length
  scales down on short trajectories (same rule as `pcd_dogfooding`).

## Smoke benchmark (committed fixtures)

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/rf2o_smoke \
  evaluation/fixtures/rf2o_smoke/gt.csv \
  60 --methods rf2o,pl_icp,csm,kinematic_icp \
  --wheel-odom-from-gt \
  --summary-json docs/benchmarks/scan2d/rf2o_smoke_60_refresh.json
```

Corridor benchmark artifact: `docs/benchmarks/scan2d/rf2o_corridor_120.json` (after first run).
