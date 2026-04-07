# `imu.csv` と DLIO / CT-LIO のすばやい検証（KITTI 以外でも可）

**KITTI 必須ではありません。** `pcd_dogfooding` が期待する `imu.csv` と `frame_timestamps.csv` の対応さえ合っていれば、**任意のデータセット**のツリーで DLIO / CT-LIO を試せます。以下の KITTI Raw + OXTS はその **生成例の一つ**です。

## 0. KITTI Raw を使う場合（公開・再現性）

このリポの OXTS / Velodyne 変換は **KITTI Vision Benchmark の Raw Data（同期・整流版）** を想定しています。他公開データ（EuRoC 等）の CSV 形式はそのままでは使えません。

- **公式ページ（データ説明・利用規約）**: [KITTI Raw Data](https://www.cvlibs.net/datasets/kitti/raw_data.php)
- **入手**: 同サイトで **無料アカウント登録 → ログイン** のうえ、日付 **2011_09_26** のシーケンス **drive 0009** の **Synced & rectified**（ページ上の *sync* パッケージ、ZIP 名は例: `2011_09_26_drive_0009_sync.zip`）を入手します。IMU 成分は OXTS パケットに含まれます。
- **展開後のディレクトリ例**（ZIP の作り方で 1 段違うことがあるので **`velodyne_points/data` と `oxts/data` が直接その下にあるフォルダ**を `--drive-dir` に指定）:
  - `…/2011_09_26_drive_0009_sync/velodyne_points/data/*.bin`
  - `…/2011_09_26_drive_0009_sync/oxts/data/*.txt`
- **パスが分からないとき**（ローカルに既に展開済みの場合）:

```bash
# 一例: ホーム以下で sync フォルダを探す
find "$HOME" -type d -name '2011_09_26_drive_0009_sync' 2>/dev/null | head
```

見つかったパスを `--drive-dir` にそのまま渡します。

**参考（LiDAR のみの別ベンチ）**: [KITTI Odometry](https://www.cvlibs.net/datasets/kitti/eval_odometry.php) の velodyne は `evaluation/scripts/setup_kitti_benchmark.sh` 向けで、**Raw の OXTS 整列 `*_sync` は含みません**。DLIO の `imu.csv` 検証には **Raw の sync パッケージ**が必要です。

## 1. OXTS から `imu.csv` を置く

同期済み KITTI Raw の `*_sync` フォルダが手元に有る場合:

```bash
python3 evaluation/scripts/kitti_raw_to_benchmark.py \
  --drive-dir /path/to/2011_09_26_drive_0009_sync \
  --output-dir dogfooding_results/kitti_raw_0009_full \
  --gt-csv experiments/reference_data/kitti_raw_0009_full_gt.csv \
  --write-imu-csv
```

既に PCD がある場合のみ IMU を足す:

```bash
python3 evaluation/scripts/kitti_oxts_imu_for_dogfooding.py \
  --drive-dir /path/to/2011_09_26_drive_0009_sync \
  --pcd-dir dogfooding_results/kitti_raw_0009_full
```

## 2. DLIO / CT-LIO で IMU が読めているか

`imu.csv` を **PCD ツリーのルート**に置いたあと:

```bash
chmod +x evaluation/scripts/smoke_dlio_imu_when_ready.sh
# 既定は公開 MCD KTH（imu が無ければエラーで終了）
./evaluation/scripts/smoke_dlio_imu_when_ready.sh

# 別ツリーを試す例（KITTI でも MCD でも可）
export PCD_DIR=/path/to/your_dogfooding_tree
export GT_CSV=/path/to/matching_gt.csv
export NFR=40
# プロファイル名に KITTI とあるが、フラグ程度のプリセットです。外す: export USE_DLIO_PROFILE=0
./evaluation/scripts/smoke_dlio_imu_when_ready.sh
```

単体 DLIO の例（パスは任意）:

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/kitti_raw_0009_full \
  experiments/reference_data/kitti_raw_0009_full_gt.csv \
  30 --methods dlio --summary-json /tmp/dlio_imu.json --dlio-kitti-profile
```

標準出力に `IMU samples: 0` ではなく **正の件数**が出ていれば、`imu.csv` は契約どおり見えている。各スキャンのログに `imu=y` が増えるかは実装・窓の取り方に依存。

## 3. フルマトリックス

```bash
python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing
# または commit 前に
python3 evaluation/scripts/refresh_study_docs.py
```
