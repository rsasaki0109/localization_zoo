#!/bin/bash
# SuMa development check: one sequence, two arms, concurrent, one thread each.
set -u
SEQ=$1
ROOT=/home/user/localization_zoo
OUT=/tmp/claude-0/-home-user-localization-zoo/71b1d203-b71a-5dc6-9879-f7b53e1f1a41/scratchpad/suma_gf/dev/$SEQ
BASE="--methods suma --no-gt-seed --suma-paper"
declare -A EXTRA=(
  [paper_odometry]=""
  [paper_odometry_ground_no_forward]="--suma-paper-set ground_no_forward_vertical_cos=0.866"
)
for arm in paper_odometry paper_odometry_ground_no_forward; do
  mkdir -p $OUT/$arm
  ( cd $OUT/$arm && OMP_NUM_THREADS=1 $ROOT/build/evaluation/pcd_dogfooding \
      /home/user/kitti_data/dataset/sequences/$SEQ/velodyne $ROOT/experiments/reference_data/kitti_seq_${SEQ}_full_gt.csv 0 \
      $BASE ${EXTRA[$arm]} --summary-json $OUT/$arm/summary.json > $OUT/$arm/run.log 2>&1; echo "exit $?" >> $OUT/$arm/run.log ) &
done
wait
