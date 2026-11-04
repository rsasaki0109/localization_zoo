#!/bin/bash
# Development check: one sequence, three arms, concurrent, one OpenMP thread each.
set -u
SEQ=$1
ROOT=/home/user/localization_zoo
OUT=/tmp/claude-0/-home-user-localization-zoo/71b1d203-b71a-5dc6-9879-f7b53e1f1a41/scratchpad/inten_gf/dev2/$SEQ
BASE="--methods inten_loam --no-gt-seed --inten-loam-paper-odometry --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 --inten-loam-paper-set s2m_plane_max_dist=0.05"
declare -A EXTRA=(
  [ground]="--inten-loam-paper-set s2s_ground_no_forward=1"
  [ground_facade_along]="--inten-loam-paper-set s2s_ground_no_forward=1 --inten-loam-paper-set s2s_facade_no_forward_below=0.5"
  [ground_facade_all]="--inten-loam-paper-set s2s_ground_no_forward=1 --inten-loam-paper-set s2s_facade_no_forward_below=1.5"
)
for arm in ground ground_facade_along ground_facade_all; do
  mkdir -p $OUT/$arm
  ( cd $OUT/$arm && OMP_NUM_THREADS=1 $ROOT/build/evaluation/pcd_dogfooding \
      /home/user/kitti_data/dataset/sequences/$SEQ/velodyne $ROOT/experiments/reference_data/kitti_seq_${SEQ}_full_gt.csv 0 \
      $BASE ${EXTRA[$arm]} --summary-json $OUT/$arm/summary.json > $OUT/$arm/run.log 2>&1; echo "exit $?" >> $OUT/$arm/run.log ) &
done
wait
