# Hard PCL Localization: indoor_easy_01

GT-backed full-trajectory odometry result for the 2,027-frame Azure Kinect
sequence. This is the easy baseline for the pending `indoor_hard_01`
comparison.

## Result

| Method | Accepted pairs | ATE-XY (m) | RPE-XY (m/frame) | RPE-yaw (deg/frame) | Estimated path (m) | Net XY (m) |
|---|---:|---:|---:|---:|---:|---:|
| KISS keyframe | 1,168 / 2,026 | **8.763** | **0.076** | **2.249** | 137.40 | 7.13 |
| LiTAMIN2 | 322 / 2,026 | 10.815 | 0.358 | 12.233 | 197.84 | 0.52 |
| CT-ICP | 1,462 / 2,026 | 18.767 | 0.284 | 8.837 | 357.13 | 3.75 |
| X-ICP | 232 / 2,026 | 10.037 | 0.320 | 9.815 | 160.59 | 3.63 |
| DegenSense + IMU | 1,932 / 2,026 | 13.201 | 0.567 | 12.330 | 787.93 | 3.34 |
| DegenSense, no IMU | 1,065 / 2,026 | 11.759 | 2.736 | 76.657 | 3,465.33 | 4.36 |

The GT planar path length is 76.31 m and its net XY displacement is only
0.37 m because the trajectory returns near its start. Net displacement is
therefore not a quality metric for this sequence. ATE alone is also insufficient:
DegenSense without IMU has a nominally lower ATE than its IMU run despite a
3.47 km estimated path and catastrophic frame-to-frame error.

The current defaults do not make this nominally "easy" sequence easy for the
window runners. KISS has the best ATE/RPE but still accumulates 8.76 m ATE.
X-ICP accepts only 11.5% of pairs, so its result is dominated by the publish
gate rather than successful tracking. These defaults must be kept unchanged for
the hard-sequence comparison; tuning on the easy result would invalidate the
baseline.

## Reproduction

- Input topic: `/points2/decompressed` (`x/y/z` float32, 167,488 points/frame)
- Extractor: `evaluation/scripts/extract_ros2_lidar_imu.py` (zero intensity
  added for PointXYZI compatibility; no synthetic per-point time)
- GT converter: `evaluation/scripts/tum_to_gt_csv.py`
- GT association: exact header-stamp match, 2,027 / 2,027 frames
- Evaluation: `evaluation/scripts/evaluate_window_odometry_gt.py`, initial
  SE(3) GT pose alignment followed by planar ATE/RPE evaluation in the initial
  LiDAR frame; RPE delta is one frame
- Matrix runner: `evaluation/scripts/run_hard_pcl_odometry_benchmark.py`
  (`--jobs 6` for this run)
- Runner arguments: defaults, `max_frames=-1`; DegenSense additionally run
  with default `imu.csv` and with `--no-imu`

Machine-readable full traces are stored in `full/*.json`. A 200-frame scouting
run was performed first but is intentionally not retained as a canonical result.

## Next comparison

Run the same commands and unchanged defaults on `indoor_hard_01` after its
resumable Zenodo download finishes, then add a side-by-side easy/hard table.
