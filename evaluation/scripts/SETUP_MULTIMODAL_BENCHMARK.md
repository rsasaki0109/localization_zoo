# Multimodal Benchmark Setup

`build/evaluation/multimodal_dogfooding` は Visual / Visual-Inertial /
Visual-LiDAR-Inertial 系の compact 実装を同じ `--summary-json` 契約で回すための
CLI です。

## 1. Input Contract

第1引数の `sequence_dir` は既存の `pcd_dogfooding` 用ツリーをそのまま使えます。

必要:

- `sequence_dir/frame_timestamps.csv`
- `gt_csv` (`timestamp,lidar_pose.x,y,z,roll,pitch,yaw` を含む)
- `landmarks.csv`
- `visual_observations.csv`

任意:

- `sequence_dir/00000000/cloud.pcd`, `00000001/cloud.pcd`, ...
- `sequence_dir/imu.csv`

LiDAR を使う `lvi_sam`, `fast_livo2`, `r2live` は `cloud.pcd` が必要です。
`vins_fusion`, `okvis`, `orb_slam3` は observation-only でも動きます。

## 1.5 Bootstrap With Synthetic Observations

raw image frontend がまだ無い場合は、GT から synthetic landmark 観測を作れます。

```bash
python3 evaluation/scripts/generate_synthetic_visual_observations.py \
  --sequence-dir dogfooding_results/kitti_raw_0009_200 \
  --gt-csv experiments/reference_data/kitti_raw_0009_200_gt.csv
```

既定では `sequence_dir` 配下に `landmarks.csv` と `visual_observations.csv` を書くので、
そのまま `multimodal_dogfooding` や manifest から参照できます。

## 1.6 KITTI Raw Real-Image Bootstrap

KITTI Raw では real image track から `landmarks.csv` / `visual_observations.csv` を作れます。

```bash
python3 evaluation/scripts/generate_kitti_visual_observations.py \
  --sequence-dir dogfooding_results/kitti_raw_0009_200 \
  --gt-csv experiments/reference_data/kitti_raw_0009_200_gt.csv
```

既定では sequence 名から drive id を推定して `/tmp/kitti_real` 配下の
`*_drive_0009_sync` を探します。必要なら `--drive-dir` で明示できます。

出力:

- `landmarks.csv`
- `visual_observations.csv`
- `camera_args.txt`

`camera_args.txt` には exporter が使った intrinsics が入るので、
`multimodal_dogfooding` や manifest の `problem.dataset.extra_args` にそのまま渡せます。

canonical な 200-frame windows をまとめて準備する場合:

```bash
python3 evaluation/scripts/prepare_kitti_multimodal_inputs.py
```

full windows も含める場合:

```bash
python3 evaluation/scripts/prepare_kitti_multimodal_inputs.py --include-full
```

selected manifest 群をまとめて回す場合:

```bash
python3 evaluation/scripts/run_multimodal_study.py \
  --method okvis \
  --method orb_slam3 \
  --include-full
```

既定では aggregate と generated docs は `/tmp/localization_zoo_multimodal_results` と
`/tmp/localization_zoo_multimodal_docs` に出ます。repo 直下の generated docs を壊したくない
ときの batch runner として使えます。

## 2. CSV Schemas

`landmarks.csv`

```csv
landmark_id,x,y,z
0,12.0,-3.0,1.5
1,12.0, 3.0,2.5
```

`visual_observations.csv`

```csv
frame_index,landmark_id,u,v
0,0,278.1,241.7
0,1,361.4,250.2
1,0,265.8,241.0
```

補足:

- `frame_index` の代わりに `frame` / `frame_idx` / `index` も可
- `timestamp` 列でも受けられるが、その場合は `frame_timestamps.csv` が必要
- 生画像そのものは読まない。外部 frontend で特徴追跡・landmark 対応付け済みの
  観測列を渡す前提

## 3. Example

```bash
./build/evaluation/multimodal_dogfooding \
  dogfooding_results/kitti_raw_0009_200 \
  experiments/reference_data/kitti_raw_0009_200_gt.csv \
  50 \
  --methods vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live \
  --landmarks-csv dogfooding_results/kitti_raw_0009_200/landmarks.csv \
  --visual-observations-csv dogfooding_results/kitti_raw_0009_200/visual_observations.csv \
  $(cat dogfooding_results/kitti_raw_0009_200/camera_args.txt) \
  --summary-json /tmp/multimodal_summary.json
```

fast / dense profile flags:

- `--vins-fusion-fast-profile`, `--vins-fusion-dense-profile`
- `--okvis-fast-profile`, `--okvis-dense-profile`
- `--orb-slam3-fast-profile`, `--orb-slam3-dense-profile`
- `--lvi-sam-fast-profile`, `--lvi-sam-dense-profile`
- `--fast-livo2-fast-profile`, `--fast-livo2-dense-profile`
- `--r2live-fast-profile`, `--r2live-dense-profile`

## 4. Current Scope

この CLI が扱う visual 入力は `known-landmark reprojection` です。raw image の feature
tracking / stereo / loop closure frontend までは含めていません。

そのため P0 の次段は:

1. real-image exporter を各 KITTI window に回して local observation files を揃える
2. `experiments/{vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live}_kitti_raw_{0009,0061}{,_full}_matrix.json`
   を `stable_interface.binary=build/evaluation/multimodal_dogfooding` で回す
3. 必要なら raw image frontend 側の feature quality / landmark initialization を改善する

multimodal manifests には `problem.variant_timeout_seconds` を入れてあり、既定では
`200-frame=120s`, `full=300s` の wall-clock budget で切ります。override したい場合:

```bash
python3 evaluation/scripts/run_multimodal_study.py \
  --method orb_slam3 \
  --include-full \
  --variant-timeout-seconds 900
```
