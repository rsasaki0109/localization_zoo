# Codex Task 2: 未統合手法のマニフェスト横展開

> **担当**: Codex
> **ブランチ**: `wip/profile-expansion-refresh`
> **PR**: Draft PR #1 に積む

---

## 背景（3行で）

- pcd_dogfooding に 27 method families が統合済みだが、多くは 1 manifest (1 dataset window) しかない
- Istanbul / HDL-400 / MCD / KITTI Raw の 4 dataset families で全手法を比較するには、各手法に全 window のマニフェストが必要
- 現在 13 manifests ある手法 (litamin2, gicp, ndt, kiss_icp, ct_icp) に合わせて、1 manifest しかない手法を横展開する

## 現状

全 window マニフェスト済み (13本): `litamin2`, `gicp`, `ndt`, `kiss_icp`, `ct_icp`

1 manifest のみ: `aloam`, `clins`, `dlio`, `dlo`, `floam`, `lego_loam`, `mulls`

7 manifests (Istanbul + MCD + KITTI 短尺): `balm2`, `fast_lio2`, `fast_lio_slam`, `isc_loam`, `lins`, `lio_sam`, `loam_livox`, `point_lio`, `suma`, `vgicp_slam`, `xicp`

4 manifests: `small_gicp`, `voxel_gicp`

## やること

### Step 1: 1-manifest 手法の横展開 (7 methods × 12 windows = 84 manifests)

以下の 7 手法について、既存の 1 manifest をテンプレートとして全 dataset window 用のマニフェストを生成する:

`aloam`, `clins`, `dlio`, `dlo`, `floam`, `lego_loam`, `mulls`

ただし:
- **IMU 必要な手法** (`clins`, `dlio`, `lio_sam`, `lins`, `point_lio`) は IMU データのない window では作成しない
- 各手法の既存マニフェストから `variants` をそのままコピー
- `dataset.pcd_dir` と `dataset.gt_csv` は window に合わせて差し替え

### Step 2: dataset パスの確認

各 window の dataset パスは以下の既存マニフェストから取得できる:

| Window | pcd_dir の例 | gt_csv の例 |
|--------|-------------|-------------|
| Istanbul A | `litamin2_profile_matrix.json` 参照 |
| Istanbul B | `litamin2_istanbul_window_b_matrix.json` 参照 |
| Istanbul C | `litamin2_istanbul_window_c_matrix.json` 参照 |
| HDL-400 A | `litamin2_hdl_400_reference_matrix.json` 参照 |
| HDL-400 B | `litamin2_hdl_400_reference_b_matrix.json` 参照 |
| MCD TUHH | `litamin2_mcd_tuhh_night_09_matrix.json` 参照 |
| MCD NTU | `litamin2_mcd_ntu_day_02_matrix.json` 参照 |
| MCD KTH | `litamin2_mcd_kth_day_06_matrix.json` 参照 |
| KITTI 0009 200f | `litamin2_kitti_raw_0009_matrix.json` 参照 |
| KITTI 0061 200f | `litamin2_kitti_raw_0061_matrix.json` 参照 |
| KITTI 0009 full | `litamin2_kitti_raw_0009_full_matrix.json` 参照 |
| KITTI 0061 full | `litamin2_kitti_raw_0061_full_matrix.json` 参照 |

### Step 3: マニフェスト命名規則

既存パターンに従う:
```
{method}_{dataset_window}_matrix.json
```

例:
- `aloam_istanbul_window_b_matrix.json`
- `aloam_mcd_tuhh_night_09_matrix.json`
- `aloam_kitti_raw_0009_matrix.json`
- `dlo_kitti_raw_0061_full_matrix.json`

### Step 4: JSON 検証

作成した全マニフェストに対して:
```bash
python3 -c "import json; json.load(open('experiments/<name>.json'))"
```

## やらないこと

- 実際のベンチマーク実行（`run_experiment_matrix.py` は呼ばない）
- pcd_dogfooding.cpp の変更
- 既存マニフェストの変更
- generated docs の変更

## 完了条件

1. 全 LiDAR-only 手法が全 window のマニフェストを持つ
2. IMU 必要手法は IMU あり window のみマニフェストを持つ
3. 全マニフェストが valid JSON
4. commit して PR #1 に push
