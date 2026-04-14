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
#   ./evaluation/scripts/setup_kitti_benchmark.sh <kitti_root> [--include-full]
#
# This script:
#   1. Converts KITTI sequences 00 and 07 to PCD format (108-frame windows by default)
#   2. Generates GT CSV files
#   3. Validates the output structure
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <kitti_root> [--include-full]"
    echo "  kitti_root: directory containing sequences/ and poses/"
    exit 1
fi

KITTI_ROOT="$1"
shift

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

python3 "$SCRIPT_DIR/prepare_kitti_odometry_inputs.py" \
    --kitti-root "$KITTI_ROOT" \
    --sequence 00 \
    --sequence 07 \
    --window-size 108 \
    "$@"
