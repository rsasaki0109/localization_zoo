# リポジトリが参照する「別途用意する」公開データ

`dogfooding_results/` は `.gitignore` 対象です。以下はマニフェストが指す**代表パス**と、入手の手掛かりです。

## Istanbul（Autoware 系ウィンドウ）

マニフェスト例: `experiments/gicp_istanbul_window_b_matrix.json`

- **PCD ディレクトリ**: `dogfooding_results/autoware_istanbul_open_108_b`
- **GT**: `dogfooding_results/autoware_istanbul_open_108_b_gt.csv`（同梱前提の相対パス）

データそのものは **元プロジェクトのライセンスに従って取得**し、上記パスに展開してください。ファイル名はマニフェストの `dataset.pcd_dir` / `gt_csv` と一致させる必要があります。

## HDL-400 参照窓

マニフェスト例: `experiments/ct_icp_hdl_400_reference_matrix.json`

- **PCD**: `dogfooding_results/hdl_400_open_ct_lio_120`
- **GT**: `experiments/reference_data/hdl_400_public_reference.csv`

公開元・利用条件は問題タイトル（`public HDL-400 reference`）に合わせたデータセット説明を確認してください。

ROS1 公開 bag は `/velodyne_points` に **native の `time` field を含みません**。`ct_lio` / `ct_icp` を public bag から復元する場合は、`evaluation/scripts/extract_ros1_lidar_imu.py` に `--time-mode index` か `--time-mode azimuth` を付けて synthetic per-point time を埋めてください。

```bash
python3 evaluation/scripts/extract_ros1_lidar_imu.py \
  --pointcloud-bag /path/to/hdl_400.bag \
  --pointcloud-topic /velodyne_points \
  --imu-bag /path/to/hdl_400.bag \
  --imu-topic /gpsimu_driver/imu_data \
  --output-dir dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index \
  --start-frame 1 \
  --max-frames 120 \
  --time-mode index
```

この synthetic time で **non-CT 系はほぼ再現**できますが、`ct_lio` / `ct_icp` の stored reference aggregate までは一致しません。stored CT reference を正確に再現したい場合は、元の `hdl_400_ros2` 抽出窓のような **native per-point `time` 付き点群**が必要です。

## KITTI Raw / Odometry

- `evaluation/scripts/prepare_kitti_odometry_inputs.py` … **Odometry** velodyne + poses から `kitti_seq_<seq>_<window>` と `kitti_seq_<seq>_full` を生成
- `evaluation/scripts/setup_kitti_benchmark.sh` … 上の薄い wrapper。既定では `00/07` の `108-frame` 窓を作り、`--include-full` もそのまま渡せる
- `evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md` … **Raw *sync*** から GT + PCD + `imu.csv` を作る例（**KITTI 以外でも**同じ `imu.csv` 契約なら `PCD_DIR` / `GT_CSV` を差し替えて `smoke_dlio_imu_when_ready.sh` 可）

## 手元ですぐ試せる最小データ

### MCD の参照座標系に注意

公開元の `pose_inW.csv` は **body frame** の姿勢です。raw Ouster 点群と
比較する場合は `T_world_lidar = T_world_body * T_body_lidar` の変換が必要です。
既存の CI fixture と旧結果は回帰確認用として保持し、精度比較にそのまま
使わないでください。新しい変換は明示指定できます:

```bash
python3 evaluation/scripts/mcd_gt_to_csv.py \
  --input /path/to/pose_inW.csv --output /tmp/mcd_lidar_gt.csv \
  --frame-timestamps /path/to/frame_timestamps.csv \
  --sensor-to-body-json experiments/results/kiss_matcher_mcd_transfer/hhs_ouster_sensor_to_body.json
```

この例の校正は公開元が案内する KTH/TUHH の HHS Ouster 用です。
別センサや変換済み点群に重ねて適用しないでください。旧既定値は変更していません。
[公開元の定義・校正根拠と再採点結果](../../experiments/results/kiss_matcher_mcd_transfer/README.md)
には、時刻対応や fixture の由来に残る制約も記載しています。

CI と同じ **3 フレーム**が `evaluation/fixtures/mcd_kth_smoke/` にあります（MCD KTH 由来、コミット対象）。

```bash
./build/evaluation/pcd_dogfooding \
  evaluation/fixtures/mcd_kth_smoke \
  evaluation/fixtures/mcd_kth_smoke_gt.csv \
  3 --methods gicp
```
