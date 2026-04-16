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

### HDL-400 ROS2 native-time B window

`experiments/*_hdl_400_reference_b_matrix.json` が参照する B window は、native per-point `time` を持つ ROS2 bag から以下の範囲を抽出する契約です。

```bash
python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag /path/to/hdl_400_ros2 \
  --pointcloud-topic /velodyne_points \
  --imu-topic /gpsimu_driver/imu_data \
  --output-dir dogfooding_results/hdl_400_open_ct_lio_120_b \
  --start-frame 360 \
  --max-frames 120
```

この window は `experiments/reference_data/hdl_400_public_reference_b.csv` と timestamp が対応します。先頭フレームは `1509348833.091089964`、末尾フレームは `1509348845.001411915` です。

## KITTI Raw（フル PCD + IMU）

- `evaluation/scripts/setup_kitti_benchmark.sh` … **Odometry** velodyne（Raw の OXTS なし）
- `evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md` … **Raw *sync*** から GT + PCD + `imu.csv` を作る例（**KITTI 以外でも**同じ `imu.csv` 契約なら `PCD_DIR` / `GT_CSV` を差し替えて `smoke_dlio_imu_when_ready.sh` 可）

## 手元ですぐ試せる最小データ

CI と同じ **3 フレーム**が `evaluation/fixtures/mcd_kth_smoke/` にあります（MCD KTH 由来、コミット対象）。

```bash
./build/evaluation/pcd_dogfooding \
  evaluation/fixtures/mcd_kth_smoke \
  evaluation/fixtures/mcd_kth_smoke_gt.csv \
  3 --methods gicp
```
