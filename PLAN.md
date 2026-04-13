# Localization Zoo - Codex 引き継ぎ PLAN

> **最終更新: 2026-04-14**
>
> この文書は Codex / AI アシスタントが repo の現在地と次のアクションを把握するための handoff。
> PR #1 は squash merge 済みで main に統合された。
>
> 最初に本ファイルを読み、次に:
> 1. [`experiments/results/index.json`](experiments/results/index.json)
> 2. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 3. [`docs/variant_analysis.md`](docs/variant_analysis.md)

---

## 1. 現在地サマリ

| 項目 | 値 |
|------|----|
| Branch | `main` (PR #1 merged) + `wip/profile-expansion-refresh` (開発用) |
| HEAD (wip branch) | `13c5d3a` |
| Ready problems | **248** |
| Blocked | **1** (CT-LIO GT-backed, scope-out 済み) |
| Skipped | **1** (hdl_graph_slam KITTI 0061 full) |
| Active manifests | **250** |
| Pending manifests | **90** (Istanbul/HDL-400 B のローカルデータ待ち) |
| CLI method families | **27** |
| papers/ 実装数 | **39** (27 統合済み + 12 未統合) |
| Unit tests | **38/38 passed** |
| Docker build | verified (Ceres 2.1/2.2 両対応) |

---

## 2. Repo 構造

### 2.1 Stable Core
- `evaluation/src/pcd_dogfooding.cpp` — 27 method 統合 CLI
  - `--no-gt-seed` — GT 初期化なしモード
  - `--methods` — カンマ区切りで手法選択
  - 各手法に fast/dense/paper profile flags
- `common/include/common/ceres_compat.h` — Ceres 2.1/2.2 互換
- `papers/*/` — 39 本の from-scratch 論文実装

### 2.2 Experiment Layer
- `experiments/*_matrix.json` — 250 active manifests
- `experiments/pending/` — 90 pending manifests (データ待ち)
- `experiments/results/` — aggregate JSON + per-variant summaries
- `experiments/reference_data/` — GT CSV (committed)

### 2.3 Generated Docs (手編集禁止)
- `docs/experiments.md`, `docs/decisions.md`, `docs/interfaces.md`
- `docs/paper_tracks.md`, `docs/paper_roadmap.md`
- `docs/paper_comparison.md`, `docs/variant_analysis.md`
- `docs/assets/paper/` — Pareto plot, variant fronts 等

### 2.4 手編集 OK な Docs
- `docs/paper_draft_outline.md` — 論文構成
- `docs/paper_claim.md` — Core claim
- `docs/implementation_notes.md` — 27 method 品質監査
- `docs/native_time_provenance.md` — HDL-400 time provenance

### 2.5 Environment
- `Dockerfile` + `.dockerignore` + `requirements.txt`

---

## 3. Dataset Families (4)

| Family | Sensor | Windows | Max Frames | Contract |
|--------|--------|---------|------------|----------|
| Istanbul | Velodyne (~1.1k pts) | 3 × 108f | 108 | gt-backed |
| HDL-400 | Velodyne HDL-32E | 2 × 120f | 120 | reference-based |
| MCD | Ouster OS1-64/128 (61-84k pts) | 3 × 108f | 108 | gt-backed |
| KITTI Raw | Velodyne HDL-64E (~120k pts) | 2×200f + 2×full (443/703f) + nogt | 703 | gt-backed |

MCD: Google Drive 直接 DL（登録不要）。変換: `extract_ros1_lidar.py` + `mcd_gt_to_csv.py`
KITTI Raw: `/tmp/kitti_real/` にローカルデータあり。変換: `kitti_raw_to_benchmark.py`

---

## 4. 27 統合済み Methods

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

---

## 5. 12 未統合 papers/

| 名前 | LOC | 種類 | 未統合理由 |
|------|-----|------|-----------|
| `vins_fusion` | 615 | Visual-Inertial | カメラ必要 |
| `okvis` | 633 | Visual-Inertial | カメラ必要 |
| `orb_slam3` | 894 | Visual SLAM | カメラ必要 |
| `lvi_sam` | 762 | LiDAR-Visual-Inertial | カメラ必要 |
| `r2live` | 513 | LiDAR-Visual-Inertial | カメラ必要 |
| `fast_livo2` | 387 | LiDAR-Visual-Inertial | カメラ必要 |
| `vilens` | 708 | Visual-Inertial-Leg | 特殊ハードウェア |
| `relead` | 499 | CT-ICP variant | ct_icp_relead と重複 |
| `ct_icp_relead` | 119 | CT-ICP+ReLead | 薄いラッパー |
| `scan_context` | 294 | Place Recognition | odometry ではない |
| `imu_preintegration` | 237 | utility library | ベンチマーク対象外 |
| `cube_lio_repro` | 0 | 空 | 未実装 |

**LiDAR-only ベンチマーク対象は全て統合済み。** 残りは Visual 系 or 特殊用途。

---

## 6. Key Findings

### 6.1 GT-Seed Ablation (docs/variant_analysis.md)
- LiTAMIN2/NDT: GT-seed なしで ATE 100m+ に発散
- GICP: 比較的ロバスト (1.46→2.21m)
- KISS-ICP/CT-ICP: pure odometry なので影響なし

### 6.2 Cross-Dataset Default Instability
- 5 手法中 3 手法で dataset ごとに default variant が変わる
- "universal default" は存在しない → variant-first thesis の根拠

### 6.3 HDL-400 Native-Time Provenance
- `docs/native_time_provenance.md` に明文化済み
- synthetic-time ≠ native-time、CT 系の exact reproduction には native bag が必要

---

## 7. 次にやるべきこと

### P0: Visual 系手法のベンチマーク統合

pcd_dogfooding を拡張してカメラデータ対応にするか、別ベンチマークツールを作る。
対象: `vins_fusion`, `okvis`, `orb_slam3`, `lvi_sam`, `r2live`, `fast_livo2`
これにより 27→33 methods に拡大可能。

### P1: 論文執筆

248 ready, 27 methods, 4 dataset families の evidence で草稿を書く。
- `docs/paper_draft_outline.md` がベース
- Workshop / RA-L Artifact track が現実的ターゲット

### P2: Istanbul データ取得

90 pending manifests を有効化 → 248→338 ready。
Autoware Istanbul open data が必要。

---

## 8. Workflow ルール

### 8.1 比較軸を壊さない
- 同一 CLI, 同一 pcd_dir, 同一 gt_csv, 同一 metrics

### 8.2 generated docs を手編集しない
- generator を直して `python3 evaluation/scripts/refresh_study_docs.py`

### 8.3 targeted run の罠
`--manifest` で subset 実行すると **index.json が部分状態になる**。
commit 前に必ず full refresh:
```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 8.4 dogfooding_results/ は commit しない
ローカルデータは `.gitignore` 対象。

### 8.5 HDL-400 注意
- synthetic-time と native-time は別物
- current reference aggregates を canonical native-time truth と見なさない
- `docs/native_time_provenance.md` 参照

---

## 9. コマンド集

```bash
# ビルド
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc) --target pcd_dogfooding

# 全テスト
ctest --test-dir build --output-on-failure -j$(nproc)

# Full refresh (commit 前に必ず)
python3 evaluation/scripts/refresh_study_docs.py

# Smoke benchmark
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> --methods litamin2,gicp,ndt --summary-json /tmp/smoke.json

# No-GT-seed
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> --methods litamin2,gicp,ndt --no-gt-seed

# MCD データ準備
python3 evaluation/scripts/extract_ros1_lidar.py --bag <bag> --pointcloud-topic /os_cloud_node/points --output-dir <out> --start-frame 10 --max-frames 108
python3 evaluation/scripts/mcd_gt_to_csv.py --input <pose_inW.csv> --output <gt.csv> --frame-timestamps <out>/frame_timestamps.csv

# KITTI Raw データ準備
python3 evaluation/scripts/kitti_raw_to_benchmark.py --drive-dir <drive_sync> --output-dir <out> --gt-csv <gt.csv>

# Docker build
docker build -t localization_zoo:test .

# Script syntax check
python3 -m py_compile evaluation/scripts/*.py
```

---

*End of PLAN.md*
