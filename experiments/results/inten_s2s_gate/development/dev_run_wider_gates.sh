#!/bin/bash
# Development check: one sequence, three arms, concurrent, one OpenMP thread each.
set -u
SEQ=$1
ROOT=/home/user/localization_zoo
OUT=/tmp/claude-0/-home-user-localization-zoo/71b1d203-b71a-5dc6-9879-f7b53e1f1a41/scratchpad/inten_gf/dev4/$SEQ
BASE="--methods inten_loam --no-gt-seed --inten-loam-paper-odometry --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 --inten-loam-paper-set s2m_plane_max_dist=0.05"
declare -A EXTRA=(
  [gate100_ground]="--inten-loam-paper-set s2s_gate_sq=100 --inten-loam-paper-set s2s_ground_no_forward=1"
  [gate400_ground]="--inten-loam-paper-set s2s_gate_sq=400 --inten-loam-paper-set s2s_ground_no_forward=1"
  [gate25_ground]="--inten-loam-paper-set s2s_gate_sq=25 --inten-loam-paper-set s2s_ground_no_forward=1"
)
for arm in gate100_ground gate400_ground; do
  mkdir -p $OUT/$arm
  ( cd $OUT/$arm && OMP_NUM_THREADS=1 $ROOT/build/evaluation/pcd_dogfooding \
      /home/user/kitti_data/dataset/sequences/$SEQ/velodyne $ROOT/experiments/reference_data/kitti_seq_${SEQ}_full_gt.csv 0 \
      $BASE ${EXTRA[$arm]} --summary-json $OUT/$arm/summary.json > $OUT/$arm/run.log 2>&1; echo "exit $?" >> $OUT/$arm/run.log ) &
done
wait
