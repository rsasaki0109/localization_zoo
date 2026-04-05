#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# KITTI Odometry Benchmark Setup Script
#
# Prerequisites:
#   1. Download from https://www.cvlibs.net/datasets/kitti/eval_odometry.php
#      - "Download odometry data set (velodyne laser data, 80 GB)" => data_odometry_velodyne.zip
#      - "Download odometry ground truth poses (4 MB)" => data_odometry_poses.zip
#   2. Extract both zips so you have:
#      <kitti_root>/sequences/00/velodyne/000000.bin ...
#      <kitti_root>/poses/00.txt ...
#
# Usage:
#   ./evaluation/scripts/setup_kitti_benchmark.sh <kitti_root>
#
# This script:
#   1. Converts KITTI sequences 00 and 07 to PCD format (108-frame windows)
#   2. Generates GT CSV files
#   3. Validates the output structure
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <kitti_root>"
    echo "  kitti_root: directory containing sequences/ and poses/"
    exit 1
fi

KITTI_ROOT="$1"

# Validate KITTI directory structure
for seq in 00 07; do
    if [ ! -d "$KITTI_ROOT/sequences/$seq/velodyne" ]; then
        echo "ERROR: $KITTI_ROOT/sequences/$seq/velodyne not found"
        echo "Please download and extract KITTI odometry velodyne data first."
        exit 1
    fi
    if [ ! -f "$KITTI_ROOT/poses/$seq.txt" ]; then
        echo "ERROR: $KITTI_ROOT/poses/$seq.txt not found"
        echo "Please download and extract KITTI odometry poses first."
        exit 1
    fi
done

echo "=== KITTI Odometry Benchmark Setup ==="
echo "KITTI root: $KITTI_ROOT"
echo "Repo root:  $REPO_ROOT"

# Convert sequence 00 (frames 0-107, 108 frames)
echo ""
echo "--- Converting sequence 00 (frames 0-107) ---"
python3 "$SCRIPT_DIR/kitti_bin_to_pcd_dir.py" \
    --velodyne-dir "$KITTI_ROOT/sequences/00/velodyne" \
    --output-dir "$REPO_ROOT/dogfooding_results/kitti_seq_00_108" \
    --start-frame 0 \
    --max-frames 108

python3 "$SCRIPT_DIR/kitti_poses_to_gt_csv.py" \
    --poses-file "$KITTI_ROOT/poses/00.txt" \
    --output "$REPO_ROOT/experiments/reference_data/kitti_seq_00_108_gt.csv" \
    --start-frame 0 \
    --max-frames 108

# Convert sequence 07 (frames 0-107, 108 frames)
echo ""
echo "--- Converting sequence 07 (frames 0-107) ---"
python3 "$SCRIPT_DIR/kitti_bin_to_pcd_dir.py" \
    --velodyne-dir "$KITTI_ROOT/sequences/07/velodyne" \
    --output-dir "$REPO_ROOT/dogfooding_results/kitti_seq_07_108" \
    --start-frame 0 \
    --max-frames 108

python3 "$SCRIPT_DIR/kitti_poses_to_gt_csv.py" \
    --poses-file "$KITTI_ROOT/poses/07.txt" \
    --output "$REPO_ROOT/experiments/reference_data/kitti_seq_07_108_gt.csv" \
    --start-frame 0 \
    --max-frames 108

# Validate outputs
echo ""
echo "=== Validation ==="
for seq in 00 07; do
    dir="$REPO_ROOT/dogfooding_results/kitti_seq_${seq}_108"
    csv="$REPO_ROOT/experiments/reference_data/kitti_seq_${seq}_108_gt.csv"
    n_pcd=$(ls -d "$dir"/0* 2>/dev/null | wc -l)
    n_gt=$(wc -l < "$csv")
    echo "  seq $seq: $n_pcd PCD frames, $((n_gt - 1)) GT poses"
done

echo ""
echo "=== Setup complete ==="
echo ""
echo "Next steps:"
echo "  1. Build:  cmake -B build && cmake --build build -j\$(nproc)"
echo "  2. Run:    python3 evaluation/scripts/run_experiment_matrix.py --manifest experiments/*kitti*.json"
echo "  3. Refresh: python3 evaluation/scripts/refresh_study_docs.py"
