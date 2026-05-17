# GPT pro 先生への相談用サマリ

> 作成日: 2026-05-17
> 対象 repo: `localization_zoo`
> 現在ブランチ: `wip/profile-expansion-refresh` (HEAD: `aee7611`)

---

## 1. プロジェクトの概要

LiDAR / multimodal SLAM・localization 手法を共通インターフェースで横並びベンチマークする OSS。
論文発表よりも **「ベンチマーク基盤として実際に使える」** ことを優先するフェーズ。

### スタック

- 中核バイナリ
  - `build/evaluation/pcd_dogfooding` — LiDAR-only、27 ファミリ
  - `build/evaluation/multimodal_dogfooding` — カメラ込み、6 ファミリ
- 共通契約 (`--summary-json`)
  - `status / ate_m / rpe_trans_pct / rpe_rot_deg_per_m / frames / time_ms / fps / note`
- 実験層 (`experiments/`)
  - `*_matrix.json` (active manifest)
  - `pending/*_matrix.json` (準備済・未実行)
  - `results/*.json` (集計)
  - `results/index.json` (全 282 problems の現在状態)

### 現在のカバレッジ

| 区分 | 数 |
|------|----|
| LiDAR-only families | 27 |
| Camera-aware families | 6 |
| Indexed problems | 282 |
| Multimodal canonical manifests | 24 |
| ローカル所持データセット | KITTI Raw 0009/0061, HDL-400, MCD KTH/NTU/TUHH |
| 未所持データセット | KITTI Odometry seq 00/07, MulRan, Newer College, Oxford Spires |

### 27 LiDAR-only ファミリ

`aloam, balm2, clins, ct_icp, ct_lio, dlio, dlo, fast_lio2, fast_lio_slam, floam, gicp, hdl_graph_slam, isc_loam, kiss_icp, lego_loam, lins, lio_sam, litamin2, loam_livox, mulls, ndt, point_lio, small_gicp, suma, vgicp_slam, voxel_gicp, xicp`

### 6 Camera-aware ファミリ

`vins_fusion, okvis, orb_slam3, lvi_sam, fast_livo2, r2live`

直近で実用予算内に収まったのは `okvis / vins_fusion / fast_livo2` の 3 つ。`orb_slam3 / lvi_sam / r2live` は実用予算ではタイムアウト気味。

---

## 2. 直近で起きたこと

### 2.1 最後のコミット (`aee7611`)

> MulRan/Newer College benchmarks, RPE short-trajectory fix, docs refresh

- MulRan ParkingLot prep 追加 (pose IO, GT CSV, 120/full manifests)
- Newer College / KITTI multimodal scaffolding
- `pcd_dogfooding` の RPE segment 長を短軌道向けに拡張
- multimodal_dogfooding ターゲット & CI 配線
- `experiments/` の Istanbul / reference_b プロファイル系を整理
- KITTI fusion pending 追加

### 2.2 ブランチ運用上のメモ

- ローカルとリモートで 74 コミット分岐していたが、`git diff HEAD @{u}` はゼロ差分
  → 過去に Co-Authored-By を除去するための rebase + force-push が走っていた痕跡
- `git reset --hard origin/wip/profile-expansion-refresh` でリモートに揃え済み
- 旧 HEAD は `3424f06` (reflog に残存)
- 今後コミットには Co-Authored-By を **付けない** ことが運用ルール

### 2.3 worktree の状態

- `git status` はクリーン
- 過去の PLAN.md は「意図的に dirty」と書いてあるが、今の HEAD では一旦クリーン化されている

---

## 3. 現在の到達点と課題

### 3.1 出来ていること

- LiDAR-only 27 ファミリの横並びは広く取れている
- multimodal は KITTI Raw 4 ウィンドウで 3 ファミリが完走可能
- RPE が `pcd_dogfooding` の安定契約に組み込まれ、aggregate JSON にも保存される
- 再現性ステータスは `docs/reproduction_status.md` で明示
- `tests/` 12/12 pass、`verify_environment.py` pass

### 3.2 まだ弱いところ

| 課題 | 内容 |
|------|------|
| 再現性 | 「論文の数値を再現した」と胸を張れる事例がまだ `LiTAMIN2 / CT-ICP @ KITTI Raw 0009` 程度 |
| データ | KITTI Odometry 公式 sequences/poses がローカルに無いため、フルレンジ再現はコード上のみ準備済 |
| 公開データ多様性 | KITTI に依存しすぎ。MulRan/Newer College/Oxford Spires いずれも未着手 |
| Camera-aware の重さ | `orb_slam3 / lvi_sam / r2live` は現実的予算で回せない |
| 論文系資産 | `docs/paper_*.md` 一式が repo に残っているが、ユーザは現在 paper 優先ではない |

### 3.3 PLAN.md が提示している分岐

- **Option A**: ローカル完結。MCD / HDL-400 で既存の `LiTAMIN2 / CT-ICP` を RPE 付きで再評価
- **Option B**: KITTI Odometry root を入手 → pending manifests を実行 (再現性パス)
- **Option C**: MulRan ingestion を新規追加 (OSS としての価値を広げる)
- **Option D (やらない)**: 論文ドラフト、PR 整形、大規模 git 整理

---

## 4. GPT pro 先生に聞きたい論点

### Q1. 次の打ち手の優先順位

「ベンチマーク OSS として実用的」を最大化するなら、A / B / C のうちどれが期待効用が高い?

- **A (ローカル MCD/HDL-400 再評価)**: 既存データで RPE まで揃える → 既出ファミリの定量化が深まるが新規性は薄い
- **B (KITTI Odometry 取得 → seq 00/07 フル)**: 公式ベンチマークの再現性が立つ → 論文クレームに最も直結
- **C (MulRan ingestion)**: 公開データ多様化 → OSS の射程が広がるが、初期投資が大きい

工数感:
- A: 1〜2 日
- B: データ DL に数時間 + フル run に半日〜1 日 (シーケンス長次第)
- C: import 層から書くので 3〜5 日

### Q2. Camera-aware 系の「実用予算」設計

現在の budget は 200-frame で 120s、full で 300s。
`orb_slam3 / lvi_sam / r2live` が完走できない状態をどう扱うべき?

- 選択肢:
  1. budget を相手に合わせて緩める (公平性が損なわれる)
  2. budget は固定し、未完走を「条件下では実用外」と明記する (現状)
  3. ファミリごとに budget profile を分け、それを論文/docs で明示する

OSS の honesty と publishable 性のバランスをどう取るのが業界標準的に良いか?

### Q3. 「再現したと言ってよい閾値」

`LiTAMIN2 @ KITTI Raw 0009 (短軌道, 443 frames)` で
- ATE 1.053 m
- RPE trans 0.742 %
- RPE rot 0.004224 deg/m

が出ている。これは KITTI Odometry の論文値とは別の窓なので、「再現」と言うのは厳しい。

業界的に「reproduced」「approximate」「indicative」のような言葉をどう使い分けるべきか?
`docs/reproduction_status.md` のような明示文書を持つ運用は妥当か?

### Q4. 27 LiDAR ファミリの絞り込み

研究者向け OSS として有用なのは「広く 27」と「精選 10 程度」のどちらか?

- 広い派の利点: comparative study の論文受けが良い、leaderboard 的価値
- 絞り派の利点: maintenance コストが下がる、各ファミリのチューニング深度を保てる

### Q5. 公開データの優先順位

- **MulRan**: LiDAR + GPS/INS GT。シーケンス豊富。フォーマット親和性高
- **Newer College**: LiDAR + IMU + camera。multimodal を強化したいならここ
- **Oxford Spires**: 新しく見栄えするが ingestion コスト高
- **KITTI-360**: 既存 KITTI infra の延長で取り込みやすい

OSS としての「次の 1 公開データ」はどれを優先すべきか?

### Q6. 論文化のタイミング

今は「論文より基盤」フェーズ。
ベンチマーク OSS 系の論文 (KISS-ICP, SLAMBench, OpenLORIS-Scene 等) の通った時期を考えると、どの段階で論文化に踏み切るのが筋がいいか?

- 何ファミリ揃ったら?
- 何データセット揃ったら?
- 再現性をどこまで詰めたら?

---

## 5. 参考: ファイル位置

| 役割 | パス |
|------|------|
| ハンドオフ計画 | `PLAN.md` |
| 再現性ステータス | `docs/reproduction_status.md` |
| 論文比較 | `docs/paper_comparison.md` |
| バリアント分析 | `docs/variant_analysis.md` |
| 全 problem 状態 | `experiments/results/index.json` |
| LiDAR ベンチ本体 | `evaluation/src/pcd_dogfooding.cpp` |
| Multimodal ベンチ本体 | `evaluation/src/multimodal_dogfooding.cpp` |
| マトリクス実行ランナー | `evaluation/scripts/run_experiment_matrix.py` |
| MulRan 準備 (新規) | `evaluation/scripts/SETUP_MULRAN_BENCHMARK.md` |
| Newer College 準備 (新規) | `evaluation/scripts/SETUP_NEWER_COLLEGE_BENCHMARK.md` |
| KITTI Odometry 準備 | `evaluation/scripts/prepare_kitti_odometry_inputs.py` |

---

## 6. 要約

- LiDAR ベンチは広く取れたが、**「再現したと言える」事例が薄い**。
- multimodal は 3/6 ファミリで実用予算内、残り 3 は重い。
- KITTI 偏重を脱したい (MulRan / Newer College が候補)。
- 論文化は急がない。OSS としての実用性を先に固めたい。
- 次の打ち手として A/B/C の選択に外部の知見が欲しい。
