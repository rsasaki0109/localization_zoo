# Localization Zoo - 引き継ぎ PLAN

> 最終更新: 2026-04-06 (第3版)
> この `PLAN.md` は、Cursor 等の AI アシスタントが `Localization Zoo` の
> 現在地と次のアクションを即座に把握するための handoff 文書である。
>
> まずこのファイルを読み、次に
> [`docs/variant_analysis.md`](docs/variant_analysis.md),
> [`docs/paper_comparison.md`](docs/paper_comparison.md),
> [`docs/experiments.md`](docs/experiments.md),
> [`docs/decisions.md`](docs/decisions.md)
> を見れば repo の状態を把握できる。

---

## 0. 1 分で把握すべきこと

- **66 ready problems + 1 blocked** の variant-first benchmark repo
- **4 public dataset families**: Istanbul, HDL-400, MCD (3 windows), KITTI Raw (4 windows)
- **6 method families** がベンチマーク統合済み: LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP, CT-LIO
- **38 本の from-scratch 論文実装** が `papers/` にあり、うち 32 本がまだベンチマーク未統合
- `--no-gt-seed` フラグで GT-seeded vs pure odometry の ablation 実験済み
- `docs/variant_analysis.md` に GT-seed ablation + cross-dataset stability + profile impact の 3 分析
- Docker ビルド検証済み（Ceres 2.1/2.2 両対応 `common/include/common/ceres_compat.h`）
- **次の本筋は「ベンチマーク手法の追加」と「論文執筆」**

---

## 1. Repo 構造

### 1.1 Stable Core

- [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp) — 共有 CLI
  - `--methods litamin2,gicp,ndt,kiss_icp,ct_icp,ct_lio` — 現在統合済みの 6 手法
  - `--no-gt-seed` — scan-to-map メソッドの GT 初期化無効化
  - `--summary-json` — JSON 結果出力
  - 各メソッドの profile flags (`--litamin2-paper-profile`, `--gicp-fast-profile` 等)
- `papers/*` — 38 本の論文実装（全て from-scratch、ライブラリラッパーではない）
- `common/include/common/ceres_compat.h` — Ceres 2.1/2.2 互換ヘルパー

### 1.2 Experiment Layer

- `experiments/*_matrix.json` — problem manifest (variant 定義)
- `experiments/pending/` — KITTI Odometry マニフェスト 10 本（データ待ち）
- `experiments/results/` — aggregate JSON + per-variant summaries
- `experiments/reference_data/` — GT CSV (committed)

### 1.3 Generated Evidence Layer（手編集禁止）

- `docs/experiments.md`, `docs/decisions.md`, `docs/interfaces.md`
- `docs/paper_tracks.md`, `docs/paper_roadmap.md`
- `docs/paper_comparison.md`, `docs/variant_analysis.md`
- `docs/paper_assets.md`, `docs/paper_captions.md`
- `docs/assets/paper/` — CSV, PNG

generator を直して `python3 evaluation/scripts/refresh_study_docs.py` で再生成する。

### 1.4 Paper Skeleton（手編集 OK）

- `docs/paper_draft_outline.md` — 論文構成
- `docs/paper_claim.md` — Core claim + evidence pointers
- `docs/paper_tables_todo.md` — 表・図チェックリスト

### 1.5 Environment

- `Dockerfile` — Ubuntu 22.04, `--target pcd_dogfooding` でビルド
- `.dockerignore` — `build/`, `dogfooding_results/` 除外
- `requirements.txt` — Python 依存

---

## 2. 現在の Evidence Snapshot

### 2.1 問題数

- ready: **66**
- blocked: **1** (CT-LIO GT-backed, scope-out 済み)

### 2.2 Dataset families (4)

| Family | Sensor | Windows | Frames | Contract |
|--------|--------|---------|--------|----------|
| Istanbul | Velodyne (~1.1k pts) | 3 × 108f | 324 | gt-backed |
| HDL-400 | Velodyne HDL-32E | 2 × 120f | 240 | reference-based |
| MCD | Ouster OS1-64/128 (61-84k pts) | 3 × 108f | 324 | gt-backed |
| KITTI Raw | Velodyne HDL-64E (~120k pts) | 2 × 200f + 2 × full (443f, 703f) | 1546 | gt-backed |

追加: no-GT-seed ablation (KITTI Raw 0009 × 200f × 5 methods)

### 2.3 Method families (6 統合済み)

| Method | Type | GT-seed | Variants | 品質 |
|--------|------|---------|----------|------|
| LiTAMIN2 | scan-to-map | あり | 4 | ⭐⭐⭐⭐⭐ |
| GICP | scan-to-map | あり | 3 | ⭐⭐⭐⭐ |
| NDT | scan-to-map | あり | 3 | ⭐⭐⭐⭐ |
| KISS-ICP | odometry | なし | 3 | ⭐⭐⭐⭐ |
| CT-ICP | odometry | なし | 3 | ⭐⭐⭐⭐⭐ |
| CT-LIO | LIO | なし | 3 | HDL-400 のみ |

### 2.4 Key findings (from variant_analysis.md)

1. **GT-seed ablation**: LiTAMIN2/NDT は GT-seed なしで ATE 100m+ に発散。GICP は比較的ロバスト。KISS-ICP/CT-ICP は影響なし（元々 odometry）
2. **Cross-dataset instability**: 5 手法中 3 手法で dataset ごとに default variant が変わる
3. **Profile impact**: fast profile は throughput 2-3x 向上、ATE は dataset 依存で悪化/改善が分かれる

---

## 3. 次にやるべきこと（優先度順）

### P0: ベンチマーク手法の追加

**最優先。** 現在 6 手法だが、`papers/` に 32 本の未統合実装がある。
pcd_dogfooding に統合して比較手法を増やす。

**高優先度（LiDAR-only、IMU 不要）:**

| Method | Dir | LOC | 特徴 | 統合難度 |
|--------|-----|-----|------|----------|
| A-LOAM | `papers/aloam/` | 965 | Feature-based LOAM 基準実装 | 中 |
| F-LOAM | `papers/floam/` | - | Fast LOAM variant | 中 |
| Small-GICP | `papers/small_gicp/` | 295 | 効率改善版 GICP | 低 |
| DLO | `papers/dlo/` | - | Keyframe scan-to-map | 中 |
| LeGO-LOAM | `papers/lego_loam/` | - | Ground-aware LOAM | 中 |
| MULLS | `papers/mulls/` | - | Multi-metric 拡張 | 中 |

**統合手順（1 手法あたり）:**

1. `papers/{method}/` の registration/odometry interface を確認
2. `pcd_dogfooding.cpp` に `runXxx()` 関数を追加
3. CLI flags (profile variants) を追加
4. `isSupportedMethod()` に登録
5. ビルド & smoke test
6. 3+ variant のマニフェスト作成
7. 全 dataset で実験実行
8. full refresh

**目標: 10+ method families で variant-first 比較。**
これにより "38 本の実装から 10+ 手法を統一フレームワークで比較" という artifact 貢献が強まる。

### P1: 論文草稿の執筆

`docs/paper_draft_outline.md` をベースに prose を書く。
P0 で手法が増えた後の方が story が強い。

### P2: 再現性の再確認

- 各実装が元論文のアルゴリズムを忠実に再現しているかの検証記録
- `docs/implementation_notes.md` に手法ごとの deviation を明文化
- テスト (`papers/*/test/`) の全パス確認

### P3: README 更新

- 現在の 66 ready problems, 4 dataset families を反映
- MCD / KITTI Raw の追加手順を記載

---

## 4. 実験 Workflow ルール

### 4.1 variant を先に増やし、後で削る
- 3+ variant を並べる。負け variant もすぐ消さない

### 4.2 比較軸を壊さない
- 同一 CLI, 同一 pcd_dir, 同一 gt_csv, 同一 metrics

### 4.3 generated docs を手編集しない
- generator 側を直して再生成

### 4.4 targeted run の罠（重要）
`--manifest` で subset 実行すると **index.json と docs が部分状態になる**。
commit 前には必ず:
```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 4.5 local data と committed artifact を混同しない
- `dogfooding_results/` は `.gitignore` 対象
- commit するのは manifests, results JSON, reference CSV, generated docs

---

## 5. コマンド集

```bash
# ビルド
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc) --target pcd_dogfooding

# Full refresh (commit 前に必ず)
python3 evaluation/scripts/refresh_study_docs.py

# Targeted run (局所探索)
python3 evaluation/scripts/run_experiment_matrix.py --manifest experiments/<name>_matrix.json

# Smoke benchmark
./build/evaluation/pcd_dogfooding dogfooding_results/<data> experiments/reference_data/<gt>.csv \
  --methods litamin2,gicp,ndt,kiss_icp,ct_icp --summary-json /tmp/smoke.json

# No-GT-seed モード
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> --methods litamin2,gicp,ndt --no-gt-seed

# MCD データ準備
python3 evaluation/scripts/extract_ros1_lidar.py --bag <bag> --pointcloud-topic /os_cloud_node/points --output-dir <out> --start-frame 10 --max-frames 108
python3 evaluation/scripts/mcd_gt_to_csv.py --input <pose_inW.csv> --output <gt.csv> --frame-timestamps <frame_timestamps.csv>

# KITTI Raw データ準備
python3 evaluation/scripts/kitti_raw_to_benchmark.py --drive-dir <drive_sync> --output-dir <out> --gt-csv <gt.csv> --max-frames 200

# Script syntax check
python3 -m py_compile evaluation/scripts/*.py

# Docker build
docker build -t localization_zoo:test .
```

---

## 6. 実装品質メモ

全 38 実装は from-scratch（外部ライブラリのラッパーではない）。
テストカバレッジあり (`papers/*/test/`)。
主な deviation:

- **NDT**: Magnusson 論文の neighbor interpolation は未実装（lightweight 版として意図的）
- **KISS-ICP**: adaptive threshold を簡略化（機能的には同等）
- **GICP**: covariance regularization を若干簡略化
- **LiTAMIN2, CT-ICP**: 論文忠実度高い

---

## 7. 結論

**66 ready problems, 4 dataset families, 6 method families** の比較基盤が稼働中。
GT-seed ablation と variant analysis により、単なる hyperparameter sweep ではない分析が出ている。

次のフェーズは：
1. **手法数を 10+ に拡大** — papers/ の未統合実装を pcd_dogfooding に追加
2. **論文執筆** — 手法数が増えた後に story を書く
3. **再現性検証の明文化** — 各実装の deviation を文書化
