# Localization Zoo - 引き継ぎ PLAN（Claude / 次エージェント向け）

> **最終更新: 2026-04-08（第 6 版）**  
> 本ファイルは **リポジトリの現在地・契約・落とし穴・再利用スクリプト** を一文脈にまとめた handoff 用ドキュメントである。  
> **先にここを通読**し、次に [`docs/experiments.md`](docs/experiments.md)、[`docs/decisions.md`](docs/decisions.md)、[`docs/interfaces.md`](docs/interfaces.md)、[`experiments/results/index.json`](experiments/results/index.json) を見ると全体像に届く。

---

## Claude 向け: 最短読書順（15 分コース）

1. **本 `PLAN.md`**（§0〜3, §4 の罠, §5 コマンド）
2. [`README.md`](README.md) の「Three-step sanity check」と Experiment-Driven の段落
3. [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md)（Istanbul / HDL / KITTI / ミニフィクスチャ）
4. [`evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md`](evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md)（**KITTI 必須ではない**が、OXTS→`imu.csv` の代表例）
5. [`papers/cube_lio_repro/README.md`](papers/cube_lio_repro/README.md)（CUBE-LIO **前処理スライス**の範囲と限界）

---

## 0. 1 分サマリ（2026-04-08 時点）

| 項目 | 値 / 状態 |
|------|-----------|
| **Ready problems** | **80**（`experiments/results/index.json` 内 `status: "ready"`） |
| **Blocked** | **1**（CT-LIO GT-backed 公共問題 — 方針は `docs/` に既存） |
| **総 problem 行** | **81**（index の `problems` 配列長） |
| **Dataset families** | **4**: Istanbul, HDL-400, **MCD（KTH / NTU / TUHH の 3 窓）**, KITTI Raw |
| **`pcd_dogfooding` 統合** | **27 families**: LiTAMIN2, GICP, Small-GICP, Voxel-GICP, NDT, KISS-ICP, DLO, DLIO, CT-ICP, CT-LIO, A-LOAM, F-LOAM, LeGO-LOAM, MULLS, X-ICP, FAST-LIO2, HDL-Graph-SLAM, VGICP-SLAM, **SuMa**, **BALM2**, **ISC-LOAM**, **LOAM-Livox**, **LIO-SAM**, **LINS**, **FAST-LIO-SLAM**, **Point-LIO**, **CLINS** |
| **コミット済みミニデータ** | `evaluation/fixtures/mcd_kth_smoke/` — **MCD KTH 由来 3 フレーム**（計 ~3MB）、**CI で `pcd_dogfooding` スモークに使用** |
| **`dogfooding_results/`** | `.gitignore`** — 本番 MCD / KITTI ツリーは**各自用意**。マニフェスト・集計 JSON はコミット対象 |
| **Paper-facing 生成物** | `docs/assets/paper/*`, `docs/paper_*.md` 等 — **手編集禁止**（generator 修正 → `refresh_study_docs.py`） |
| **直近の Git 主題** | 27手法統合完了（Tier 1+2全統合）、CI修復（libunwind-dev + ROS2 shell fix）、CIスモーク拡張（全手法対応） |

---

## 1. Repo 構造（再掲 + 追記）

### 1.1 Stable Core

- **[`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)** — 単一 CLI の比較エントリ
  - `--methods` … カンマ区切り（`isSupportedMethod()` と整合）
  - `--summary-json` … 集計・CI・行列ランナーが読む契約
  - `--no-gt-seed` … scan-to-map 系の ablation
  - 各種 `--*-fast-profile` / `--*-dense-profile` 等
- **`papers/*`** — 論文単位の C++ 実装（from-scratch が基本方針）
- **`common/include/common/ceres_compat.h`** — Ceres 2.1 / 2.2 互換

### 1.2 Voxel-GICP（統合メモ・落とし穴）

- 実体: [`papers/voxel_gicp/`](papers/voxel_gicp/) + `pcd_dogfooding` の `runVoxelGICP`
- **重要**: 内部 `VoxelGICPMap` は **`min_points_per_voxel` 未満のボクセルを捨てる**。粗い `voxel_resolution` + 間引き点群だと **ターゲット／ソースが空→ ATE=0 かつ corr=0** の偽陽性になっていた。  
  - **Dogfooding 既定は `min_points_per_voxel=1`**（[`pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp) 内 `VoxelGICPDogfoodingOptions`）  
  - fast profile も **1** に設定済み
- CLI: `--voxel-gicp-fast-profile`, `--voxel-gicp-dense-profile`, 個別 `--voxel-gicp-*`

### 1.3 Experiment Layer

- `experiments/*_matrix.json` — manifest（dataset + `stable_interface` + `variants[]`）
- `experiments/pending/` — KITTI Odometry 系などデータ待ち
- `experiments/results/*_matrix.json` — **aggregate（variant ごとの ate_m, fps, decision 等）**
- `experiments/results/index.json` — **全 manifest の索引**（**`--manifest` 部分実行だけで上書きされると危険** → §4.4）
- `experiments/reference_data/*.csv` — コミット GT

### 1.4 生成ドキュメント（手編集禁止ゾーン）

- `docs/experiments.md`, `docs/decisions.md`, `docs/interfaces.md`
- `docs/paper_tracks.md`, `docs/paper_roadmap.md`, `docs/paper_comparison.md`, `docs/variant_analysis.md`
- `docs/paper_assets.md`, `docs/paper_captions.md`
- `docs/assets/paper/` — CSV / PNG（Pareto、default×dataset 行列など）

再生成の正規ルート:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

（内部で `run_experiment_matrix.py --reuse-existing` → publication / export / comparison / variant_analysis を続ける）

### 1.5 検証・スモーク用スクリプト（2026-04 追加）

| スクリプト | 役割 |
|------------|------|
| [`evaluation/scripts/smoke_ci_fixture.sh`](evaluation/scripts/smoke_ci_fixture.sh) | **コミット済み** `evaluation/fixtures/mcd_kth_smoke`（3f）で **全主要メソッド**を短実行。GitHub Actions の `build-and-test` 最後に呼ばれる |
| [`evaluation/scripts/smoke_mcd_pcd_dogfooding.sh`](evaluation/scripts/smoke_mcd_pcd_dogfooding.sh) | 手元の **`dogfooding_results/mcd_*`** 向け：`kth` / `ntu` / `tuhh` / `all`、フレーム数指定。`imu.csv` が無い限り **ct_lio はリストに入れない** |
| [`evaluation/scripts/smoke_dlio_imu_when_ready.sh`](evaluation/scripts/smoke_dlio_imu_when_ready.sh) | **`PCD_DIR/imu.csv` が存在するとき** DLIO + CT-LIO。既定パスは MCD KTH（imu 無ければ exit 2）。**KITTI 不要** — env で任意ツリー指定可。`USE_DLIO_PROFILE=0` で `--dlio-kitti-profile` を外せる |
| [`evaluation/scripts/cube_lio_cubemap_demo.py`](evaluation/scripts/cube_lio_cubemap_demo.py) | CUBE-LIO スライド準拠の **cubemap + IGM + semi-dense 特徴**。**単一 `--pcd`** または **`--sequence-pcd-dir`**。**`--montage`**, `--export-features-json` 等 |

### 1.6 CI（`.github/workflows/ci.yml`）

- `ubuntu-22.04`: apt で PCL/Ceres 等 → `cmake` → **`ctest`** → **`smoke_ci_fixture.sh`**
- **フル MCD / KITTI は CI に含めない**（`dogfooding_results/` は ignore のため）。**3 フレーム・フィクスチャのみ**が常時回帰対象

---

## 2. 「MCD」とは（本 repo の呼び方）

**KITTI ではない**公開系ウィンドウの総称として、マニフェスト上 **「MCD KTH / NTU / TUHH」** とラベル付き:

- `dogfooding_results/mcd_kth_day_06_108`
- `dogfooding_results/mcd_ntu_day_02_108`
- `dogfooding_results/mcd_tuhh_night_09_108`

各 **108 フレーム**（Ouster 系、点密度は KITTI より高い）。**元データの正式名称・引用は配布元の論文/ライセンスに従う**（リポはパスと実験契約を主に固定している）。

---

## 3. 再現実装の状態（期待値のキャリブレーション）

### 3.1 **できている**

- **14 手法**の統一ベンチ（ATE / FPS / variant 採択ルール）
- **Voxel-GICP** の scan-to-map dogfooding 経路
- **Small-GICP / Voxel-GICP × MCD 3 本**のマニフェスト + 集計（`refresh` 済み前提）
- **CUBE-LIO** については **スライド Stage 1–2 相当**（投影・IGM・特徴・JSON 吐き）— [`papers/cube_lio_repro/README.md`](papers/cube_lio_repro/README.md)

### 3.2 **まだ「論文フル再現」ではない**

- **CUBE-LIO 全体**（光度残差＋幾何＋IMU の joint GN、ENWIDE 等の数値一致）— **未実装**。ICRA 2026 論文・公式コードの公開待ち or 別途大規模実装
- **各 `papers/*` と原論文の bit-exact 一致** — 一部は compact baseline として意図的に簡略化（§6 参照）

---

## 4. 現在の Evidence スナップショット（要約）

### 4.1 問題数（確定値の取り方）

```bash
python3 -c "import json;d=json.load(open('experiments/results/index.json'));print('total',len(d['problems']));print('ready',sum(1 for p in d['problems'] if p['status']=='ready'))"
```

**2026-04-07**: total **81**, ready **80**, blocked **1**。

### 4.2 Dataset families（4）

| Family | 内容 | 備考 |
|--------|------|------|
| Istanbul | Autoware 系ウィンドウ | `dogfooding_results/` に実体配置が必要 |
| HDL-400 | 参照軌道付き窓 | 同上 |
| MCD | KTH / NTU / TUHH × 108f | **CI はミニ 3f のみ**；本番は手元 |
| KITTI Raw | 短縮 + full | `imu.csv` は Raw *sync* + スクリプトで生成可能 |

### 4.3 Method families（27）

（§0 表と同じ — `pcd_dogfooding` / `isSupportedMethod` と一致させること）

**2026-04-08 追加 Tier 1**: X-ICP, FAST-LIO2, HDL-Graph-SLAM, VGICP-SLAM
**2026-04-08 追加 Tier 2**: SuMa, BALM2, ISC-LOAM, LOAM-Livox, LIO-SAM, LINS, FAST-LIO-SLAM, Point-LIO, CLINS

### 4.4 Variant analysis の示唆（詳細は `docs/variant_analysis.md`）

- GT-seed ablation、cross-dataset での default 不安定、profile の speed-accuracy トレードオフ
- LOAM 系は **`frame_timestamps.csv` と連番スキャンの整合**が崩れると評価が破綻しうる

---

## 5. 優先タスク（更新版）

### P0 — ベンチに載せていない `papers/` の発掘（進行中）

**2026-04-08**: Tier 1+2 全13手法を統合済み。**27 families**。papers/ の LiDAR/LIO 系はほぼ網羅。残りは visual 系（orb_slam3, vins_fusion 等）・ユーティリティ（scan_context, relead 等）・特殊（cube_lio_repro）で統合不要。統合手順の型は固定されている:

1. `runXxx()` + CLI + `isSupportedMethod` + CMake `target_link_libraries`
2. smoke（フィクスチャ or 手元 MCD）
3. manifest（≥3 variants）→ **full** `refresh_study_docs.py`（§7）

### P0.5 — データの物理配置

- Istanbul / HDL: [`SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md) のパスに合わせて配置し、`run_experiment_matrix.py --reuse-existing` で集計を更新

### P1 — IMU 付き LIO の実測

- `imu.csv` を任意 `PCD_DIR` に置き、[`smoke_dlio_imu_when_ready.sh`](evaluation/scripts/smoke_dlio_imu_when_ready.sh) で DLIO/CT-LIO を確認（**KITTI 以外でも可**）

### P2 — 論文 prose

- `docs/paper_draft_outline.md` から執筆。Evidence は常に aggregate に紐づける

### P3 — CUBE-LIO の行く末

- 現状: **前処理デモのみ**。論文・コードが出たら **別バイナリ統合 or サブモジュール**を判断

### P4 — CI とローカルの乖離監視

- PR ごとに **Actions が緑か**（`smoke_ci_fixture` が落ちたら **ビルド or 実行パス or フィクスチャ**から切り分け）

---

## 6. 実験 Workflow ルール（必読）

### 6.1 variant-first

3+ variants。負け組もすぐ消さない（比較と説明のため）

### 6.2 比較軸の保全

同一 `pcd_dir`, `gt_csv`, metrics, binary 契約

### 6.3 生成 docs の手編集禁止

generator 修正 → `refresh_study_docs.py`

### 6.4 **`--manifest` 部分実行の罠**

subset だけ `run_experiment_matrix.py` すると **`index.json` がその分割だけになる**。  
**コミット前は必ず**（引数なしの）full refresh か:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 6.5 `dogfooding_results/` はコミットしない

中身は巨大・個人パス。コミットするのは **manifest / results JSON / reference CSV / generated docs / fixtures（小さいもののみ）**

---

## 7. コマンド集（増補）

```bash
# --- ビルド（ローカル）---
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"

# --- CI と同じ超短スモーク（コミット済み 3 フレーム）---
bash evaluation/scripts/smoke_ci_fixture.sh

# --- 本番 MCD（手元に dogfooding_results が必要）---
bash evaluation/scripts/smoke_mcd_pcd_dogfooding.sh all 30

# --- 論文・研究ドキュメント全部更新（コミット前の定番）---
python3 evaluation/scripts/refresh_study_docs.py

# --- manifest だけ試す（最後に必ず full refresh）---
python3 evaluation/scripts/run_experiment_matrix.py --manifest experiments/foo_matrix.json

# --- default×dataset 行列だけ再生成（export からも呼ばれる）---
python3 evaluation/scripts/generate_default_variant_matrix.py

# --- 単発 smoke ---
./build/evaluation/pcd_dogfooding evaluation/fixtures/mcd_kth_smoke \
  evaluation/fixtures/mcd_kth_smoke_gt.csv 3 --methods gicp --summary-json /tmp/s.json

# --- KITTI Raw → dogfooding + 任意で imu ---
python3 evaluation/scripts/kitti_raw_to_benchmark.py \
  --drive-dir /path/to/*_sync --output-dir dogfooding_results/kitti_raw_0009_full \
  --gt-csv experiments/reference_data/kitti_raw_0009_full_gt.csv --write-imu-csv

python3 evaluation/scripts/kitti_oxts_imu_for_dogfooding.py \
  --drive-dir /path/to/*_sync --pcd-dir dogfooding_results/kitti_raw_0009_full

# --- Cubemap / IGM デモ（CUBE-LIO スライド準拠の前処理のみ）---
python3 evaluation/scripts/cube_lio_cubemap_demo.py \
  --sequence-pcd-dir dogfooding_results/mcd_kth_day_06_108 \
  --output-dir /tmp/cube_out --max-sequence-frames 5 --size 256 --export-features-json

# Docker
docker build -t localization_zoo:test .
```

---

## 8. 実装品質・既知の deviation（要約）

- **NDT**: neighbor interpolation 等を意図的に省略した lightweight 版
- **KISS-ICP**: adaptive 回りを簡略化しうる
- **GICP 系**: regularization 等、論文より短い経路がありうる
- **LiTAMIN2, CT-ICP**: 相対的に忠実度が高い

詳細は各 `papers/*/README.md` を優先し、不足は `docs/implementation_notes.md`（あれば）へ追記する運用がよい。

---

## 9. 結論（handoff メッセージ）

**80 ready problems・4 dataset families・27 method families・81 index 行**という比較基盤は稼働している。  
**ミニ MCD フィクスチャ + CI スモーク**により「clone してビルドすれば最低限の回帰」がかかる状態になった。

次の Claude / メンテ担当は次を優先するとよい:

1. **Actions の緑維持**と **`smoke_ci_fixture` 失敗時の切り分け**（CI修復済み: libunwind-dev + ROS2 shell fix）
2. **新手法のマニフェスト作成**（27手法 × MCD/KITTI の matrix → refresh_study_docs.py）
3. **Istanbul / HDL 実データ配置**または **blocked 問題の解除条件の再確認**
4. **CUBE-LIO**は前処理まで — **フル本体は別プロジェクト扱い**で論文・公式実装を待つ

---

*End of PLAN.md 第 6 版*
