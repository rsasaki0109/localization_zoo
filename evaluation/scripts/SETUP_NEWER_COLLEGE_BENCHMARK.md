# Newer College benchmark prep (flat PCD + GT CSV)

The [Newer College Dataset](https://ori-drs.github.io/newer-college-dataset/) provides **ROS bags** and a **flat-file** layout (EuRoC-style) with:

- Ouster LiDAR as **`.pcd`** per scan under `os1_cloud_node/points/` (stereo rig)
- Ground truth as **`.csv`**: `seconds since epoch`, `nanoseconds`, `x`, `y`, `z`, `qx`, `qy`, `qz`, `qw` (see [Ground Truth](https://ori-drs.github.io/newer-college-dataset/ground-truth/))

Download sequences from the official [Download](https://ori-drs.github.io/newer-college-dataset/download) page (browser / wget / rclone as offered there), then unpack so you have a folder such as:

```text
01_short_experiment/
  os1_cloud_node/points/*.pcd
  ground_truth.csv
```

The exact folder names depend on the release archive; if LiDAR lives elsewhere, pass `--lidar-subdir`. If the GT file has another name, pass `--ground-truth /path/to/file.csv`.

## Export for `pcd_dogfooding`

From the repository root:

```bash
python3 evaluation/scripts/prepare_newer_college_inputs.py \
  --sequence-dir /path/to/01_short_experiment \
  --max-frames 120 \
  --include-full
```

This writes:

- `dogfooding_results/newer_college_<folder>_<N>/` with `00000000/cloud.pcd`, …
- `experiments/reference_data/newer_college_<folder>_<N>_gt.csv`
- `frame_timestamps.csv` (timestamps inferred from each PCD filename stem)

PCD filenames are parsed as either a **single integer nanosecond timestamp**, or `sec_nsec` with an underscore (e.g. `1590000000_123456789`).

## Smoke benchmark

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/newer_college_01_short_experiment_120 \
  experiments/reference_data/newer_college_01_short_experiment_120_gt.csv \
  --methods litamin2,gicp,kiss_icp \
  --summary-json /tmp/newer_college_smoke.json
```

Adjust paths to match the `--stem` / folder name produced by the prep script.
