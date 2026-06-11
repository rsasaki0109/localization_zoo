# Localization Zoo - Codex / Cursor 引き継ぎ PLAN

> **最終更新: 2026-06-12 (Quadric-LO plane-fallback ablation §00.52n、2D は一旦停止)**
>
> この文書は、次の AI アシスタントが repo の現在地、最近の差分、次にやるべきことを短時間で掴むための handoff。
>
> 最初に本ファイルを読み、その次に:
> 1. [`README.md`](README.md)
> 2. [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md)
> 3. [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md) — **2D scan odometry リーダーボード (停止中/背景)**
> 4. [`docs/index.html`](docs/index.html)
> 5. [`docs/methods.json`](docs/methods.json)
> 6. [`evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md`](evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md)
> 7. [`evaluation/src/scan_dogfooding.cpp`](evaluation/src/scan_dogfooding.cpp)
> 8. [`evaluation/scripts/demo_localization_zoo.sh`](evaluation/scripts/demo_localization_zoo.sh)
> 9. [`evaluation/scripts/generate_demo_report.py`](evaluation/scripts/generate_demo_report.py)
> 10. [`evaluation/scripts/validate_demo_artifacts.py`](evaluation/scripts/validate_demo_artifacts.py)
> 11. [`evaluation/scripts/validate_showcase.py`](evaluation/scripts/validate_showcase.py)
> 12. [`experiments/results/index.json`](experiments/results/index.json)
> 13. [`docs/status_taxonomy.md`](docs/status_taxonomy.md)
> 14. [`docs/budget_profiles.md`](docs/budget_profiles.md)
> 15. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 16. [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)

---

## 00. Latest Handoff: Quadric-LO Plane-Fallback Ablation (2026-06-12 更新)

> **これが最新・最優先の handoff。**
>
> **2026-06-12 現時点のアクティブ方向は paper-ready reproducibility hardening。**
> ユーザ指示「2d ha ittan iran!」により、2D LiDAR scan odometry は一旦停止。
> 新規手法追加より、既存の 101 手法を claim tier で分け、論文で主張できる T0/T1 subset と
> adapter / compact baseline を明確に分離する。README は breadth を見せるが、manuscript-level
> claim は [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md) に従う。
> 直近では I-LOAM intensity on/off、KC-LO sigma schedule に続き、M-GCLO ground-factor off
> ablation も seq00/07 full raw artifacts 付きで追加した (§00.52j〜§00.52l)。
> さらに `docs/benchmarks/paper_ready_bundle.json` を生成する
> `evaluation/scripts/build_paper_ready_bundle.py` を追加し、I-LOAM / KC-LO / M-GCLO の
> frozen seed bundle を固定した (§00.52m)。直近では Quadric-LO plane-fallback off ablation も
> seq00/07 full raw artifacts 付きで追加し、bundle を 4 methods / 8 rows / 4 ablations に拡張した
> (§00.52n)。T0 evidence candidate は I-LOAM と KC-LO。M-GCLO と Quadric-LO は T1+
> evidence candidate。
>
> §0 (2026-06-02 の OSS Showcase) 以降は依然有効な背景 (showcase/demo/CI、3D benchmark 履歴、
> recipe 由来) で、2D の詳細は **§00.6c〜§00.66** を背景として読むこと。
>
> ### 2026-06-09 セッション要約 (3D → 2D 転換後)
>
> - **方針転換**: ユーザ指示「しばらくは 2D LiDAR でやっていく」→ `scan_dogfooding` harness 新設。
> - **papers 43–50 完了** (RF2O / PL-ICP / CSM / Kinematic-ICP / PSM / NDT-2D / IDC / MbICP)。
> - **公開 dataset**: Bonn 2D-SLAM JSON → `intel_val_73`, `fr079_val_384`, `mit_val_33` を repo に commit。
> - **合成 fixture**: `rf2o_smoke` (60f), `rf2o_corridor` (120f slow motion)。
> - **CSM 改善** (commit `3fc5be0`): distance transform + 3-level pyramid → fr079 drift 38.9%→**20.6%**。
> - **MbICP (50本目)**: config-space metric ICP を追加し、canonical JSON を8法で refresh。
> - **ドキュメント整備**: [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md) ハブ、
>   README / SETUP / `public_bundle.json` / 各 paper README を 8 法に同期。
> - **git (当時)**: `main` は `origin/main` より **2 commit ahead** (`361a592` IDC, `3fc5be0` CSM-DT)。
>   markdown 整理分はワークツリーに未 commit だった。
>
> ### 2026-06-03〜08 セッション要約 (3D campaign、背景)
>
> - **50-star 施策** PR #11/#12 マージ: hero GIF、誤差ヒート版 seq07 ギャラリー、social card、README -28%。
> - **32 本目 I-LOAM** (PR #13): 強度 ablation positive; mapping 統合で seq00 **0.899%** drift。
> - **33–40 本目**: PL-LOAM / InTEn-LOAM / MCGICP-LO / ICPSC-LO / VLOM / OdoNet / NHC-Net / NN-ZUPT。
> - **41–42 本目**: FR-LIO (NCLT 0.63% drift) / PG-LIO (NCLT honest negative → 保留)。
> - Intensity / LiDAR-visual / IMU 3 カテゴリ shortlist **完了** (多く honest negative、正直記録)。

### 00.1 テーマと運用ルール

ユーザの一貫した指示:

- 各 "tugi" / "tudukete" / "dondon ikou" で **次の 1 論文** を実装する (自律的に新規論文を
  始めない — 明示トリガが必要)。候補が尽きたら新規 web サーベイを行う。
- 対象は **著者の公開実装が無い** 論文のみ (OSS 済みは除外)。**KITTI で評価可能** な
  古典・幾何・確率的手法 (deep/neural はスコープ外、CV 予測 front-end に落ちる)。
- 結果は **正直に** 反映する (near-redundant / honest negative も隠さず記録)。
- 機構ユニットテスト 3 件 + KITTI full seq00/seq07 評価 + README leaderboard 行 +
  `docs/methods.json` エントリ + module README + memory 追記 + clean commit が 1 論文の単位。
- **コミットは自分名義のみ** (Co-Authored-By を付けない)。**PR/コミットに AI 生成表記を
  入れない** (CLAUDE.md 制約)。応答は日本語。

### 00.05 50-star 施策ワークストリーム (2026-06-05〜08、PR #11 / #12)

from-paper キャンペーンと並行して、ユーザ指示「50 star wo mezasu」の**発見性・第一印象・
信頼シグナル**を上げる視覚/ドキュメント資産を整備し `main` にマージ済み。コミットは
自分名義のみ (AI 表記なし)。

- **README hero アニメ GIF** `docs/assets/hero_seq00.gif` (+ `.mp4`): KITTI seq00 上面視で
  KC-LO/KISS-ICP/TrICP-LO を GT 上に時間方向に描画。生成器
  `evaluation/scripts/animate_trajectory.py` (入れ子バンド描画で全色が縁取りで見える)。
- **seq07 軌跡ギャラリー** `docs/assets/grid_seq07.png`: 当初は 15 手法の生軌跡 small-multiples
  だったが、優秀手法が全部 GT に重なって**差が見えない**ため、PR #12 で**各軌跡を GT からの
  フレーム毎の距離で色付けする誤差ヒート版に刷新** (turbo, 3m clip, cool=追従/warm=ドリフト)。
  生成器 `evaluation/scripts/plot_trajectory_diff.py` (`--mode heat` / `--mode curve`)。
  旧生成器 `plot_trajectory_grid.py` も残置。
- **social card + OGP** `docs/assets/social_card.png` (1280×640): 生成器
  `evaluation/scripts/make_social_card.py`。`docs/index.html` の og:image/twitter:image に配線。
- **community health files**: `CONTRIBUTING.md` / `CITATION.cff` / `.github/ISSUE_TEMPLATE/*` /
  `.github/PULL_REQUEST_TEMPLATE.md`。
- **README -28% 削減** (561→428 行)、Contributing/Citation 節追加。
- **CI ゲート整合**: hero/social card 差し替えで `evaluation/scripts/validate_showcase.py` の
  スニペット契約が壊れ CI が落ちた → 新資産 (hero_seq00.gif の GIF 署名チェック等) に合わせて
  修正済み。**README/index.html を触ったら `python3 evaluation/scripts/validate_showcase.py
  --root .` を必ず回すこと** (CI と同じ契約)。
- **GitHub About 変更** (`gh repo edit --description`): 「with ROS 2」を外し from-paper を前面に
  → *"C++ from-paper reimplementations of LiDAR localization & odometry papers — honestly
  benchmarked on KITTI, with tests."* (topics の `ros2` タグは検索性維持のため残置)。

**PENDING — 手動・ユーザのみ (コード不可)**: GitHub repo **Settings → General → Social
preview** に `docs/assets/social_card.png` をアップロード。未設定だとリンク共有時に汎用カード。
詳細 memory: `loc-zoo-50star-campaign`。

### 00.2 現在地 (git / 数値)

| Item | Value |
|------|-------|
| Branch | `main` |
| vs `origin/main` | **ahead 3** after KC-LO ablation commit if not pushed |
| 実装済み from-paper 論文数 | **60 本** (3D 再開: Mesh-LOAM + ELO + ID-LIO + RF-LIO + TC-LVGF + OPL-LVIO + V-LOAM2015 + TC-VLO + AD-VLO + TC-MVLO; 2D papers 43–50 は停止中) |
| `docs/methods.json` | **101 手法** |
| 2D scan matchers | **8 法** — `rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp` |
| 2D fixtures (committed) | 5 — intel/fr079/mit (Bonn) + rf2o_smoke + rf2o_corridor |
| 2D リーダーボード hub | [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md) |
| 直近完了 | **Quadric-LO plane-fallback ablation** — seq00/07 fallback on/off raw JSON + bundle 4-method化 |
| 直近完了 (3D) | **RF-LIO (101手法目)** — removal-first dynamic LIO、KITTI seq00/07 full 完走 |
| その前 (3D) | **V-LOAM2015 / TC-VLO / AD-VLO / TC-MVLO (97-100手法目)** — LiDAR-visual adapter family、KITTI seq00/07 full 完走 |
| 2D 直近 (停止中) | **MbICP (50本目)** + 8-method canonical benchmark refresh |
| PG-LIO (42本目) | NCLT honest negative → **保留** (§00.52) |

直近コミット列 (`main`、2D 部分):

```
3fc5be0 Improve CSM with distance transform and multi-resolution pyramid search.  ← local only
361a592 Add paper 49 IDC dual-correspondence 2D scan odometry.                    ← local only
8ea40f1 Add paper 48 NDT-2D scan odometry with public 2D benchmark results.      ← origin/main
a2817ff Add fr079 and MIT public 2D scan fixtures with multi-dataset benchmarks.
709d25c Add paper 47 PSM and Intel Lab 2D scan benchmark with CI smoke.
0a9f0ad Add 2D scan eval infra: drift metrics, bag prep, and corridor fixture.
8339329 Add paper 46: Kinematic-ICP 2D unicycle scan odometry with wheel prior.
5e2147c Add paper 45: CSM correlative 2D scan matching with scan_dogfooding.
1c94e71 Add 2D scan odometry (RF2O, PL-ICP), scan_dogfooding, and PG-LIO baseline.
```

3D campaign 部分 (参考):

```
9184524 Add papers 35-36: MCGICP-LO and ICPSC-LO intensity odometry.
23504ff Add paper 41: FR-LIO RC-Vox LIO with sub-frames and ESKS.
8a980fc Add IMU papers 38-40, RGB LiDAR-visual eval, InTEn mapping, cross-dataset benchmarks.
```

> 注: §00.53〜§00.56 の番号は **3D 37〜40 本目** (VLOM/OdoNet/NHC/NN-ZUPT) と
> **2D 43〜46 本目** (RF2O/PL-ICP/CSM/Kinematic-ICP) で**重複**している。
> 2D の詳細は **§00.6c 以降** を正とする。

### 00.3 主要な恒例知見 (campaign 全体)

整備済み・IMU 無し・CV 予測が効く KITTI では:

- **robust / soft 機構は near-redundant**: Student-T (重尾)、GMM (soft assignment)、
  GNC (継続 TLS)、MCC (相関エントロピー) はいずれも front-end が ~KISS-ICP の
  point-to-plane に退化。外れ値が少なく重みがほぼ一様になるため。機構の正しさは
  ユニットテストで担保し、KITTI では「near-redundant」と正直に記録。
- **退化判定・粒子フィルタ・direct/range-image はKITTIで沈黙 or 有害**: DiLO (direct) は
  full seq で発散 (18-19%)、Spectral-LO (ICP-free BEV phase-correlation) は最速 ~14 FPS
  だが粗い (~12-14%)、UA-LIO/DegenSense は未競争。
- **point-to-plane 主体が低 drift**: point-to-line / 分布ファクタは drift 増。地面拘束は
  long-seq の局所 drift (RPE) を抑えるが ATE は悪化しうる (RPE↓/ATE↑ split)。
- **退化しきい値は並進ブロック相対基準が鉄則** (絶対しきい値は移植不可で発散)。
- seq00 RPE リーダーは **M-GCLO 0.835%** (multiple-ground-plane、ただし ATE 19m の split)。
  top9 (M-GCLO〜Adaptive-ICP) は両 seq で KISS-ICP に match/beat。
- **暗黙的曲面 (IMLS) も point-to-plane に退化**: IMLS-SLAM(29) のボクセルダウンサンプル
  局所マップ上の implicit MLS 曲面は実質 point-to-plane となり KISS-ICP を僅かに下回る
  (honest negative)。原論文の 0.4–0.7% drift は生 HDL64 密度前提。**観測性サンプリング
  は有効**で、~800 点でも全点と同精度を保ち FPS は 2 倍。
- **LTS トリミングの自動オーバーラップ推定は KITTI で収縮**: TrICP-LO(30) の FRMSD は
  ノイズで残差が連続分布する KITTI では ξ を下限 (0.8) まで縮め続け、実質「最良 80% 分位
  の固定トリム」= ロバスト分位 point-to-plane として動作。それでも competitive で seq00
  ATE 10.0 m は KISS-ICP 12.0 m を上回る (順位トリミング機構自体はユニットテストで有効)。
- **対応点を取らない密度相関 (KC) は KITTI で恒例パターンを破り positive**: KC-LO(31) の
  カーネル相関 (Renyi 二次エントロピー) + σ アニーリングは両 seq で KISS-ICP の drift を
  下回り、fixed-sigma ablation 後の seq07 RPE **0.510%** は leaderboard 全体トップ。
  離散最近傍対応のノイズに頑健な soft point-to-point が効く。KITTI + CV 予測では
  fixed σ=0.4 が annealed σ 1.5→0.4 と同等精度で **1.9-2.2x** 速い。
- **反射強度は KITTI でも対応の曖昧性解消に効く (intensity ablation positive)**: I-LOAM(32)
  の強度拡張対応 + 強度重みは、同一の幾何パイプライン (強度 OFF) 比で drift を両 seq -18〜20%、
  ATE を最大 -35% 改善。KITTI の未校正・粗い 8-bit 強度でも、強度を**主信号ではなく対応コスト
  項 + 残差重みとして**使えば壊れず効く。**mapping 統合後** (aloam LaserMapping, デフォルト ON)
  seq00 **0.899%** / seq07 **0.575%** → from-paper leaderboard 同列 (A-LOAM ~0.61% 水準)。
  強度 ablation は `--i-loam-no-mapping` で scan-to-scan のみに分離して維持。

各論文の機構・結果・教訓は per-paper memory と README / `docs/benchmarks/scan2d/README.md` を参照。

**2D scan odometry (papers 43–50) の追加知見**:

- **fixture 依存が支配的** — RF2O@Intel, PSM@fr079, PL-ICP@corridor。単一手法の「勝者」は存在しない。
- **scan-to-scan 限界** — IDC/PSM/PL-ICP/MbICP/NDT-2D/Karto は robot-frame local map 済。**RF2O (P18) と Kinematic-ICP (P19) は opt-in** — 前者は projection 型で long 窓全敗、後者は point 型でも trade-off に safe config 無し。これで 2D 全 9 法の local map 調査完了。長 Bonn log では drift 15–30% 帯が典型。
- **projection 型 local map の罠 (P18)** — 点群を min-range polar profile に再投影する reference (RF2O local map; IDC/PSM も同族) は val 窓で効くが long train 窓で爆発 (fr079_train_200: IDC 85% / PSM 681% / RF2O 99%)。bin 量子化 + 近距離バイアスが原因で、age 減衰でも救えない。
- **合成 corridor の罠** — PL-ICP 0.4% でも fr079 41% — synthetic 成功は real 一般化の証拠にならない。
- **CSM engineering win** — DT + pyramid で fr079 38.9%→20.6%。corridor ~73% は honest negative 継続。
- **correspondence-free 2D** — NDT-2D は Intel で RF2O 近傍 (14.8% vs 14.3%) だが corridor 22%。
- **metric-space ICP** — MbICP は fr079 16.6%、MIT 27.3%、corridor 0.46%。fixture winner ではないが
  PL-ICP より real logs で安定し、corridor では PL-ICP に近い。
- **wheel odom 依存** — Kinematic-ICP は `--wheel-odom-from-gt` 必須。real encoder 無しでは実用度低。
- **GT proxy 限界** — Bonn GT は scan-matched odometry。centimeter truth ではない — drift は相対比較用。

### 00.4 実装済み 50 本 (機構ファミリ別)

**3D Velodyne / multimodal (papers 1–42, 代表ファミリ)**

- **Ground / multi-plane**: M-GCLO(20), DAMM-LOAM(11), DALI-SLAM, Terrain-RBF-LIO, NHC-LIO(23)
- **Voxelmap / surface**: CT-VoxelMap, R-VoxelMap, Quadric-LO(21), CUBE-LIO, **FR-LIO(41)**
- **Implicit surface / MLS**: IMLS-SLAM(29) — honest negative
- **Robust estimation**: Student-T-LO(24), GNC-LO(27), MCC-LO(28), TrICP-LO(30), SVN-ICP, Adaptive-ICP
- **GMM / EM**: GMM-LO(26)
- **Spectral / phase**: Spectral-LO(25)
- **Correlation / entropy**: KC-LO(31) — positive (seq07 RPE トップ)
- **Intensity / reflectivity**: I-LOAM(32), InTEn-LOAM(34), MCGICP-LO(35), ICPSC-LO(36), PL-LOAM(33), VLOM(37)
- **IMU / learning**: OdoNet(38), NHC-Net(39), NN-ZUPT(40), **PG-LIO(42, 保留)**
- **Distribution / factor**: PCR-DAT(18), LiDAR-IBA, D2-LIO
- **Direct / range-image**: DiLO(22) — honest negative; PG-LIO photometric — honest negative

**2D planar scan odometry (papers 43–50, `scan_dogfooding`)**

- **Range flow (dense, no correspondences)**: RF2O(43)
- **ICP family**: PL-ICP(44), Kinematic-ICP(46, wheel prior), MbICP(50, config-space metric)
- **Correlative / grid**: CSM(45, DT+pyramid), NDT-2D(48, Gaussian cells)
- **Polar / dual correspondence**: PSM(47), IDC(49, CP+RR fusion)

### 00.45 IMLS-SLAM (29本目, 最新完了)

`papers/imls_slam/` — 暗黙的移動最小二乗 (IMLS) 曲面への scan-to-model マッチング。
出典: J.-E. Deschaud, ICRA 2018 (arXiv:1802.08633)。著者は後発の CT-ICP は公開したが
IMLS-SLAM 本体は未公開。機構: 過去スキャンの有向点群が定義する暗黙的曲面
`I^P(x)=Σ W_i (x−p_i)·n_i / Σ W_i`, `W_i=exp(−||x−p_i||²/h²)` への符号つき距離 (point-to-
implicit-surface) を Gauss-Newton で最小化。さらに 6-DoF を最も拘束する点を選ぶ観測性ベース
サンプリング (並進 `|n·e|` / 回転 `|(p×n)·e|` の 6 リスト上位)。単一最近傍平面の
point-to-plane や voxel-surfel とは別系統の幾何当てはめ。

- 統合済み: header/src/test (3/3 pass)、root+evaluation CMake、`pcd_dogfooding` 8 スポット
  (include / method-check / DogfoodingOptions struct / runImlsSlam / options var / arg-parse
  `--imls-slam-{fast,dense}-profile`/`--imls-slam-h`/`--imls-slam-no-sampling` / dispatch /
  method-list 文字列 2 箇所)、`docs/methods.json` (69手法)、module README、seq00/07 ベンチ JSON。
- 結果: **seq00 RPE 1.000% / ATE 17.6 m (7.4 FPS)、seq07 RPE 0.700% / ATE 3.04 m (9.3 FPS)**
  — **honest negative**: KISS-ICP (同データ 0.872% / 0.618% を完全再現) に両 seq で僅差で劣る。
  整備済み KITTI ではボクセルダウンサンプル局所マップ上の implicit MLS が実質 point-to-plane に
  退化。原論文 0.4–0.7% drift は生 HDL64 密度前提。**観測性サンプリングは有効** (seq07 全点
  2023 点 0.701% vs サンプリング 807 点 0.700%、FPS 2 倍)。
- **完了**: seq00/07 ベンチ JSON、README leaderboard 行 (GNC-LO と CT-VoxelMap の間)、
  module README、methods.json (69手法)、memory 追記、commit。
- ※ この環境は archive copy で KITTI 点群・Ceres を欠く。調達手順は memory
  `loc-zoo-archive-build-env` 参照 (Ceres を `third_party/ceres/install` へ source build、
  KITTI velodyne を `data/kitti_raw` へ再 download)。

### 00.46 TrICP-LO (30本目, 最新完了)

`papers/tricp_lo/` — Trimmed / Fractional ICP (最小トリム二乗, LTS) point-to-plane。
出典: D. Chetverikov et al., ICPR 2002 / IVC 2005 + J. Phillips et al., 3DIM 2007 (FICP)。
対点群レジストレーション論文でオドメトリ公開実装は無い。機構: 残差を**順位**でソートし
最良 ξ 割合のみを最小二乗に使う高破壊点ロバスト推定。ξ は FRMSD(ξ)=sqrt(e(ξ))/ξ^λ の
最小化で自動推定 (FICP)。M 推定/GNC の magnitude 重み付けとは別系統 (rank-based)。

- 統合済み: header/src/test (3/3 pass)、root+evaluation CMake、`pcd_dogfooding` 8 スポット
  (include / method-check / DogfoodingOptions struct / runTricpLo / options var / arg-parse
  `--tricp-lo-{fast,dense}-profile`/`--tricp-lo-overlap`/`--tricp-lo-fixed-overlap`/
  `--tricp-lo-lambda` / dispatch / method-list 文字列 2 箇所)、`docs/methods.json` (70手法)、
  module README、seq00/07 ベンチ JSON。
- 結果: **seq00 RPE 0.931% / ATE 10.04 m (5.6 FPS)、seq07 RPE 0.662% / ATE 2.00 m (6.2 FPS)**
  — competitive。KISS-ICP (同データ 0.872%/0.618%) に RPE で僅差で劣るが seq00 ATE 10.0 m は
  KISS の 12.0 m を上回り、IMLS(29)・robust 群の多くより良好。
- **FRMSD 自動推定の所見 (honest)**: 推定 ξ は両 seq とも下限 0.800 に張り付く。整備済み
  KITTI ではノイズで残差が連続分布し FRMSD が良点を非オーバーラップと誤認して ξ を縮め
  続けるため、実質「最良 80% 分位の固定トリム」= ロバスト分位 point-to-plane として動作。
  LTS の順位トリミング機構自体はユニットテストで有効。
- **完了**: seq00/07 ベンチ JSON、README leaderboard 行 (SVN-ICP と GMM-LO の間)、
  module README、methods.json (70手法)、memory 追記、commit。

### 00.47 KC-LO (31本目, 最新完了)

`papers/kc_lo/` — Kernel Correlation (対応点を取らない / correspondence-free) 位置合わせ。
出典: Y. Tsin & T. Kanade, ECCV 2004。対点群レジストレーション論文でオドメトリ公開実装は
無い。機構: 変換後スキャンとモデルの総親和度 `C(θ)=ΣΣ exp(−||Tx_i−y_j||²/2σ²)` を最大化
(= 結合密度の Renyi 二次エントロピー最小化)。各点を近傍モデル点の親和度重み平均へ引き寄せる
soft point-to-point を σ アニーリング (粗→細) で解く。ICP の最近傍対応・GMM-EM の潜在割当・
NDT のボクセル離散化のいずれも用いない新ファミリ。

- 統合済み: header/src/test (3/3 pass)、root+evaluation CMake、`pcd_dogfooding` 8 スポット
  (include / method-check / DogfoodingOptions struct / runKcLo / options var / arg-parse
  `--kc-lo-{fast,dense}-profile`/`--kc-lo-sigma`/`--kc-lo-sigma-init`/`--kc-lo-no-anneal` /
  dispatch / method-list 文字列 2 箇所)、`docs/methods.json` (71手法)、module README、
  seq00/07 ベンチ JSON。
- 結果 (best fixed-sigma profile): **seq00 RPE 0.837% / ATE 13.40 m / 2.65 FPS、
  seq07 RPE 0.510% / ATE 0.86 m / 3.12 FPS**
  — **positive**: 両 seq で KISS-ICP (同データ 0.872%/0.618%) の drift を上回り、**seq07 RPE
  0.510% は leaderboard 全体トップ** (旧 Adaptive-ICP 0.569% 超え)。seq00 は RPE↓/ATE↑ split。
  恒例の「全機構が point-to-plane に退化」を破る数少ない例。
- sigma schedule ablation: annealed σ 1.5→0.4 は seq00 **0.842% / 1.39 FPS**、
  seq07 **0.514% / 1.39 FPS**。fixed σ=0.4 (`--kc-lo-no-anneal`) は RPE が同等か僅かに良く、
  速度が **1.9-2.2x**。KITTI + CV 予測では kernel-correlation が主効果で、annealing は
  convergence safety knob。
  Artifacts: `docs/benchmarks/kitti_full_new_methods/seq00_kc_lo_no_anneal.json`,
  `docs/benchmarks/kitti_full_new_methods/seq07_kc_lo_no_anneal.json`,
  `docs/benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json`。
- **完了**: seq00/07 ベンチ JSON、README leaderboard 行 (M-GCLO と LODESTAR の間、seq07
  トップ更新)、module README、methods.json (71手法)、memory 追記、commit。

### 00.48 I-LOAM (32本目, mapping 統合済み, PR #13 + 2026-06-09 追補)

`papers/i_loam/` — Intensity Enhanced LOAM。出典: Yeong-Sang Park, Hyesu Jang, Ayoung Kim,
UR (Ubiquitous Robots) 2020。著者公開コード無し (RPM Lab repo に無く、GitHub の "I-LOAM"
ヒットは全て無関係な LOAM フォーク)。**新規 web サーベイ (§00.6) の第一弾**としてユーザが選定。

機構: LOAM の幾何 edge/plane パイプラインを保ちつつ、LiDAR **反射強度 (intensity)** を
scan-to-scan 対応付けに 2 経路で注入する。
1. **強度拡張対応探索**: 直線/平面を定義する 2/3 点目の候補コストを `‖Δp‖² + λ·ΔI²` に
   して、幾何が同等でも反射強度が一致する候補を選ぶ (反復幾何・平行壁の曖昧性解消)。
2. **強度類似度の残差重み**: 採用した各対応の残差を `w = exp(−ΔI²/2σ²)` で重み付け (新
   `papers/i_loam/include/i_loam/intensity_factors.h` の重み付き Ceres ファクタ)。
3. **Mapping (2026-06-09 追加)**: 強度強化 scan-to-scan の後、`aloam::LaserMapping` で
   scan-to-map 精緻化 (A-LOAM/F-LOAM と同じ 3 段 LOAM パイプライン)。map 段は幾何のみ
   (強度は odometry 段のみ)。`--i-loam-no-mapping` で scan-to-scan ablation に分離。

**重要な実装ポイント**: 幾何特徴抽出は共有 `papers/aloam` の `ScanRegistration` を再利用するが、
aloam は **PointXYZI の `intensity` フィールドを scan-id + rel_time のエンコードに転用**する
ため、特徴抽出後に**真の反射強度が失われる**。よって I-LOAM は生入力点群への
`pcl::KdTreeFLANN` 最近傍参照で特徴点ごとに反射強度を復元する。
デフォルト `intensity_sigma=0.15`, `intensity_corr_weight=1.0`, `enable_mapping=true`。

- 統合済み: header/`intensity_factors.h`/src/test (**6/6 pass**)、root + evaluation CMake、
  `pcd_dogfooding` (include / isSupportedMethod / `ILoamDogfoodingOptions` struct / `runILoam`
  (`loadPCDXYZI` で強度保持読み込み, leaf=0) / options var / arg-parse
  `--i-loam-no-intensity`/`--i-loam-no-mapping`/`--i-loam-dense-profile`/
  `--i-loam-intensity-sigma`/`--i-loam-corr-weight`/`--i-loam-stride` /
  dispatch+print ブロック)、`docs/methods.json` (72手法, Intensity family)、module README、
  seq00/07 ベンチ JSON (mapping ON + ablation OFF)。
- 結果 (KITTI, --no-gt-seed, **`--i-loam-dense-profile`, mapping ON (default)**):
  - seq00: **RPE 0.899%** / ATE 12.7 m (~3 FPS)
  - seq07: **RPE 0.575%** / ATE 1.7 m (~3 FPS)
  - → from-paper leaderboard 同列 (A-LOAM ~0.61% seq07 水準)。README leaderboard 行追加済み。
- **強度 ablation** (`--i-loam-no-mapping`, scan-to-scan only): ON vs `--i-loam-no-intensity`:
  - seq00: 純幾何 3.186% → **2.606%** = drift **-18.2%**
  - seq07: 純幾何 3.806% → **3.055%** = drift **-19.7%**
  - mean intensity weight ≈0.76–0.79 (強度経路が実働)。
  - raw artifacts: `docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json`,
    `docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json`,
    `docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json`,
    `docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json`。
    paired summary: `docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json`。
- **honest 所見**: 反射強度が scan-to-scan で drift ~18-20% 改善 → 論文の中心主張が
  **KITTI の未校正・粗い強度でも再現**。mapping 統合で絶対 drift も competitive 水準に到達。
  README には leaderboard 行 + ablation 節の両方を掲載 (ablation は scan-to-scan 分離)。
- **完了**: seq00/07 ベンチ (mapping + ablation raw JSON)、README leaderboard + ablation 節、
  module README、methods.json(72)、memory `i-loam-32nd-from-paper`、PR #13 マージ (`b02612c`)、
  mapping 追補 (2026-06-09)。

### 00.49 PL-LOAM (33本目, commit `a8c3a75`, RGB eval 2026-06-09)

`papers/pl_loam/` — LiDAR-monocular point+line visual odometry (ICRA 2020)。著者コード無し。
KITTI Odometry に RGB が無いため Velodyne→カメラ投影の**深度グラディエント疑似画像**上で
Harris コーナー + 勾配線分を抽出し、LiDAR パッチ中央値で深度、Ceres depth-prior 付き
point-line BA + スケール補正。`pcd_dogfooding` 統合、`test_pl_loam` 5/5 pass。
結果 (honest negative, pseudo-image): seq00 **143.2%** drift (ATE 3016 m)、seq07 **116.9%** (ATE 271 m).

**KITTI Raw RGB 本評価** (2026-06-09): `image_02` PNG + `--pl-loam-rgb-root`。
- raw_0009 200f: **99.6%** drift, ATE 120 m, rgb_frames=200, mean_scale≈0.977
- raw_0061 200f: **99.3%** drift, ATE 84 m, rgb_frames=200, mean_scale≈0.974
→ 実 RGB でも honest negative (~99% vs paper ~0.6–1%)。簡略 Harris/line front-end が主因。
Artifacts: `docs/benchmarks/kitti_full_new_methods/kitti_raw_*_200_rgb.json`

### 00.50 InTEn-LOAM (34本目, TVF/DOR/mapping 2026-06-09)

`papers/inten_loam/` — Intensity and Temporal Enhanced LOAM (RS 2022/23, arXiv:2209.05708)。
円筒 range+intensity 画像から ground/facade/edge/reflector 特徴を抽出し、幾何
(edge point-to-line, surf point-to-plane) + **B-spline 強度登録** (8 ボクセル三線形の簡約版)
を Ceres で joint 最適化。
**2026-06-09 拡張**: TVF (temporal voxel filter) + DOR (dynamic object removal) +
scan-to-map mapping を追加。CLI: `--inten-loam-no-{tvf,dor,mapping}`。
`test_inten_loam` 5/5 pass、`--inten-loam-no-intensity` ablation あり。
結果 (honest negative):
Mapping ablation (2026-06-09, full seq00/07):
- **Best drift**: scan-to-scan (`--inten-loam-no-mapping`) — seq00 **52.7%** / seq07 **67.4%**
  (matches original baseline).
- Mapping lowers seq07 ATE (442→~300 m) but **does not improve drift**; on long seq00
  mapping **increases** drift (52.7%→60.6%). Full pipeline (TVF+DOR+mapping) reaches
  seq00 **68.4%** / seq07 **71.7%** drift.
- Intensity registration remains near-neutral with mapping enabled.
- Artifacts: `seq{00,07}_inten_loam_ablation.json`; script `run_inten_loam_ablation.py`.
- MulRan 120 cross-dataset: **185.4%** drift (ATE 32 m) — KISS-ICP (225%) より drift は良いが ATE は悪化
- NCLT 600 cross-dataset: **46.5%** drift (ATE 40 m) — intensity scan-to-map 群 (ICPSC 0.49%,
  MCC 0.47%) は良好だが InTEn は未転移
seq07 強度 OFF は **66.9%** → 強度経路は near-neutral。

### 00.56 NN-ZUPT (40本目, 完了)

`papers/nn_zupt/` — NN-ZUPT vehicle INS (Li et al., Meas. Sci. Technol. 2023,
doi:10.1088/1361-6501/acabde)。著者公開コード無し。機構: 50×6 IMU 窓の **1D-CNN**
が停止確率を出力 → strapdown INS + **ZUPT** (速度ゼロ) + **NHC**。Python 学習 +
C++ 推論 (JSON weights)。`pcd_dogfooding --methods nn_zupt` (要 `imu.csv`)。
`test_nn_zupt` 3/3 pass、`--nn-zupt-{threshold-only,no-zupt,no-nhc,prob}` あり。
**スコープ**: 論文 Kalman 補正と前進速度 aiding は範囲外。
結果:
- KITTI OXTS 学習: 停止ラベルは drive_0011 中心 (~36%); val (stop-free drive) acc **~100%**
- HDL-400 120f (cross-IMU): **92%** drift / ATE 7.0 m / **zupt_frames=0** (honest negative)
- NCLT 600f (cross-IMU): **27%** drift / ATE 28 m / zupt_frames=0
**完了**: 学習スクリプト、weights JSON、ベンチ JSON、README、methods.json。

### 00.55 NHC-Net / VMSC (39本目, 完了)

`papers/nhc_net/` — NHC-Net adaptive non-holonomic constraints (Li et al., GPS
Solutions 2023)。著者公開コード無し。機構: 50×6 IMU 窓の **VMSC CNN** が運動状態
(停止/直進/旋回/スリップ) を分類し横/垂直速度を回帰 → strapdown INS に **適応 NHC**
(信頼度スケール gain) + **ZUPT** (停止クラス)。Python 学習 + C++ 推論 (JSON weights)。
`pcd_dogfooding --methods nhc_net` (要 `imu.csv`)。`test_nhc_net` 3/3 pass。
**スコープ**: 論文 GNSS/INS EKF と前進速度 aiding (疑似オドメータ等) は範囲外;
横/垂直 NHC のみ (OdoNet と対比)。
結果:
- KITTI OXTS 学習: held-out drive val class acc **~100%**, NHC MAE **~0.04 m/s**
- HDL-400 120f (cross-IMU): **89%** drift / ATE 6.7 m (前進速度無し → under-travel)
- NCLT 600f (cross-IMU): **32%** drift / ATE 29 m (ZUPT 未発火 off-domain)
**完了**: 学習スクリプト、weights JSON、ベンチ JSON、README、methods.json (79手法)。

### 00.54 OdoNet (38本目, 完了)

`papers/odonet/` — OdoNet pseudo-odometer (arXiv:2109.03091 / IEEE Sensors J. 2022)。
著者公開コード無し。機構: 50×6 IMU 窓の **1D-CNN** が前進速度を回帰 (scale 30 m/s)、
strapdown INS + **NHC/ZUPT** と融合。Python 学習 (`train_odonet.py`) + C++ 推論
(JSON weights)。`pcd_dogfooding --methods odonet` (要 `imu.csv`)。
`test_odonet` 4/4 pass、`--odonet-{weights,no-nhc,no-zupt,nhc-only}` あり。
結果:
- KITTI OXTS 学習: held-out drive val speed MAE **~2.1 m/s**
- HDL-400 120f (cross-IMU): **890%** drift / ATE 75 m (honest negative)
- NCLT 600f (cross-IMU): **249%** drift / ATE 300 m (honest negative)
**完了**: 学習スクリプト、weights JSON、ベンチ JSON、README、methods.json (76手法)。

### 00.53 VLOM (37本目, RGB eval 2026-06-09)

`papers/vlom/` — Visual-LiDAR Odometry and Mapping with Monocular Scale Correction
and Visual Bootstrapping (arXiv:2304.08978, 2023)。著者公開コード無し。機構:
(1) 単眼 VO (PL-LOAM 流用の疑似画像特徴 + depth-prior BA)、(2) 三角測量深度と LiDAR
深度の比による **MAD 外れ値除去付きスケール補正** (clamp 0.85–1.15)、(3) 視覚
frame-to-frame 運動を `aloam::LaserOdometry::setMotionPrior` で LiDAR 初期値に注入、
(4) A-LOAM scan-to-map mapping で最終ポーズ出力。
`test_vlom` 5/5 pass、`--vlom-{fast,dense}-profile` / `--vlom-no-bootstrap` /
`--vlom-no-scale` / `--vlom-rgb-root` あり。
結果 (honest negative):
- pseudo-image: seq00 **91.5%** / seq07 **153.9%** drift
- **KITTI Raw RGB** (2026-06-09): raw_0009 **99.7%** / raw_0061 **99.3%** drift (~99% both)
  → 実 RGB でも改善なし。mean_scale≈1.0, bootstrap≈199/200 frames。
**完了**: seq00/07 + Raw RGB ベンチ JSON、README、module README、
`aloam::setMotionPrior` API、methods.json (75手法)。

### 00.52 ICPSC-LO (36本目)

`papers/icpsc/` — Intensity Cylindrical-Projection Shape Context (Zhang et al., JAG 2023)。
著者公開コード無し。機構: 円筒 range+intensity 画像から ICPSC 記述子 (ring×sector
平均強度 + log 密度) を構築、edge/surf/強度勾配特徴を抽出し、適応重み
`w_geom = N_geom/(N_geom+α·N_int)` で scan-to-map point-to-plane を融合。
ループクロージャ・因子グラフは **後回し** (odometry ベンチに集中)。
`test_icpsc` 3/3 pass、`--icpsc-{fast,dense}-profile` / `--icpsc-no-intensity` あり。
結果 (mid-pack competitive): seq00 **0.912%** drift (ATE 19.2 m)、seq07 **0.660%**
(ATE 3.7 m, ~1.5 FPS)。両 seq で MCGICP-LO (0.940% / 0.774%) を上回る。

### 00.51 MCGICP-LO (35本目)

`papers/mcgicp/` — Multi-Channel Generalized-ICP (Servos & Waslander, RAS 2017)。
著者公開コード無し。機構: LiDAR 強度を 4D 点共分散 `(x,y,z,s·I)` に統合し局所
Mahalanobis を推定、scan-to-map で強度重み付き point-to-plane を解く。
`test_mcgicp` 3/3 pass、`--mcgicp-no-intensity` ablation あり。
結果 (mid-pack competitive): seq00 **0.940%** drift (ATE 19.6 m)、seq07 **0.774%**
(ATE 4.5 m, ~2 FPS)。PLAN §00.6 の honest-negative 予測にはならず GMM-LO 帯。

### 00.5 MCC-LO (28本目)

`papers/mcc_lo/` — Maximum Correntropy Criterion ロバスト point-to-plane オドメトリ。
出典: He et al., PLOS ONE 2018 (bidirectional MCC, doi:10.1371/journal.pone.0197542) +
Pattern Recognition 2019 (Correntropy scale ICP)。いずれも対点群レジストレーション論文で
オドメトリ公開実装は無い。機構: MSE の代わりに相関エントロピー `V=Σexp(−r²/2σ²)` を最大化、
半二次最適化で Welsch/Gauss 重み `w=exp(−r²/2σ²)`、カーネルバンド幅 σ を Silverman 経験則
`σ=1.06·std(r)·N^(−1/5)` で適応推定。**注意点**: 整備済みスキャンでは残差が小さく σ が
floor まで収縮し、狭すぎるカーネルが未整合点 (収束信号) まで棄却して **過小収束** する
(annealing 無し correntropy-ICP の既知の弱点)。そのため `mcc_sigma_floor=0.3` を導入。

- 統合済み: header/src/test (3/3 pass)、root+evaluation CMake、`pcd_dogfooding` 8 スポット
  (include / method-check / DogfoodingOptions struct / runMccLo / options var / arg-parse
  `--mcc-lo-{fast,dense}-profile`/`--mcc-lo-sigma`/`--mcc-lo-fixed-sigma` / dispatch /
  method-list 文字列 2 箇所)、`docs/methods.json` (68手法)、module README、seq07 ベンチ JSON。
- 結果: **seq00 RPE 0.892% / ATE 12.9 m、seq07 RPE 0.611% / ATE 2.15 m, 2.6 FPS**
  (mean_weight 0.90, σ=0.30 floor 張り付き) — robust 群最良 (GNC 0.986 / Student-T 0.952 /
  GMM 0.941 を上回り、seq00 ATE 12.9m も robust 群で最小)。seq07 0.611 は Adaptive-ICP
  0.569 に次ぐ好成績。
- **完了**: seq00/seq07 ベンチ JSON、README leaderboard 行 (Adaptive-ICP と NHC-LIO の間)、
  module README、methods.json (68手法)、memory 追記、commit すべて完了。

### 00.6 次の一手 (3D shortlist — **完了/保留**, 2026-06-09)

> **3D 新規論文 shortlist は完了または保留。** アクティブ作業は **§00.6c 2D キャンペーン**。
> 以下は 3D カテゴリの記録。

純 LiDAR 幾何の distinct な古典機構は **ほぼ枯渇** (saturated: ground/degeneracy/voxelmap/
feature-weighting/distribution-factor/motion-constraint/robust-M-estimator/robust-LTS(TrICP)/
spectral/gmm-em/correntropy/implicit-surface(IMLS)/correlation-entropy(KC))。そこでユーザ指示
「IMU dead-reckoning / LiDAR-visual / intensity-based LiDAR odometry を調査して再現実装して
いこう」に基づき **3 カテゴリで新規 web サーベイを実施** (著者コード無し・KITTI 再現可能・
既存非重複)。第一弾として **I-LOAM(32)** を完了 (§00.48)。残り shortlist (各カテゴリ ranked):

**Intensity / reflectivity LiDAR**
- ✅ **I-LOAM** (UR 2020) — 完了 (§00.48)。
- ✅ **InTEn-LOAM** (RS 2022/23) — 完了 (§00.50)。scan-to-scan ~53–67% drift;
  **TVF/DOR/mapping 拡張済** (seq07 prefix 56.5% drift)。強度 ablation near-neutral。
- ✅ **MCGICP** (Servos & Waslander, RAS 2017) — 完了 (§00.51)。4D 共分散 +
  強度重み付き point-to-plane。seq00 **0.940%** / seq07 **0.774%** drift (mid-pack
  competitive; honest-negative ではなく GMM-LO 帯)。
- ✅ **ICPSC** (JAG 2023) — 完了 (§00.52)。円筒 shape-context + 適応 geom/intensity 融合。
  seq00 **0.912%** / seq07 **0.660%** (MCGICP 上回る mid-pack)。

**LiDAR-visual odometry** (KITTI は Velodyne + カメラ + GT 同期)
- ✅ **PL-LOAM** (ICRA 2020) — 完了 (§00.49)。Odometry 疑似画像 ~117–143% drift;
  **KITTI Raw RGB 本評価完了** (~99% drift, honest negative)。
- ✅ **VLOM** (arXiv:2304.08978) — 完了 (§00.53)。疑似画像 seq00 **91.5%** / seq07 **153.9%**;
  **KITTI Raw RGB 本評価完了** (~99% drift, honest negative)。
- (高難度・残) **Tightly-coupled mono-LiDAR** (HIT, Intell. Service Robotics 2022): 視覚再投影 +
  LiDAR 残差を単一最適化に密結合。paywall・要全文。feasibility 2.5/5。
- 除外 (著者/3rd-party コード有): LIMO, DEMO/Zhang, DVL-SLAM, SDV-LOAM, RGB-L。

**IMU dead-reckoning / inertial odometry** (KITTI 100Hz OXTS)
- ✅ **OdoNet** ⭐ (IEEE Sensors J. 2022, arXiv:2109.03091) — 完了 (§00.54)。1D-CNN
  疑似オドメータ + INS/NHC/ZUPT。KITTI OXTS 学習 val speed MAE **~2.1 m/s**;
  HDL-400/NCLT への cross-IMU 転移は honest negative (890% / 249% drift)。
- ✅ **NHC-Net / VMSC** (GPS Solutions 2023) — 完了 (§00.55)。CNN 運動状態分類 +
  適応 NHC (横/垂直速度回帰)。KITTI OXTS 学習; HDL-400/NCLT cross-IMU honest negative
  (89% / 32% drift)。前進速度 aiding はスコープ外。
- ✅ **NN-ZUPT** (Meas. Sci. Technol. 2023) — 完了 (§00.56)。CNN 停止検出 + ZUPT/NHC。
  KITTI 停止区間希薄; cross-IMU で zupt_frames=0 (honest negative)。
- ⚠️ このカテゴリの上位は **学習ベース** が中心 → 純 C++ クラシカル campaign とワークフローが
  異なる (Python 学習 + 重み移植)。着手前にユーザと方式合意が要る。
- 除外 (コード有): Wheel-INS (i2Nav), AI-IMU Dead-Reckoning (Brossard)。

**運用**: 次 "tugi"/"tudukete" で **次の 1 候補**を実装 (自律的に始めない)。IMU
shortlist (OdoNet / NHC-Net / NN-ZUPT) は **完了**。Intensity / LiDAR-visual shortlist
は **完了** (RGB 本評価含む)。
- ✅ **Cross-dataset 再評価** (2026-06-09): MulRan 120 + **MulRan full (1177f)** +
  NCLT 600 **完了** (`mulran_120_bundle.json`, `mulran_full_bundle.json`,
  `nclt_600_bundle.json`)。MulRan full: InTEn-LOAM **59.1%** drift (best);
  KISS/MCC ~103%; I-LOAM/MCGICP 発散。スクリプト:
  `evaluation/scripts/run_cross_dataset_benchmark.py`。README:
  `docs/benchmarks/cross_dataset/README.md`。
- ✅ **validate_showcase** + **commit `8a980fc` push** (2026-06-09) 完了。

### 00.6b 新規 web サーベイ — 論文 41 本目候補 (2026-06-09)

3 カテゴリ shortlist 完了後の追加サーベイ。除外: OSS 済み
(SiMpLE, MAD-ICP, RESPLE, MOLA, C3P-VoxelMap, VoxelMap++ 等)。

| Rank | 論文 | 機構 | OSS | Feasibility |
|------|------|------|-----|-------------|
| ⭐1 | **FR-LIO** (arXiv:2302.04031) | sub-frame + ESKS + **RC-Vox** 固定配列 kNN マップ | **無** | **4/5** (要 IMU) |
| 2 | **PG-LIO** (2025) | NCC フォトメトリ + geom + IMU | 未公開 | 3.5/5 |
| 3 | **ERASOR++** (2403.05019) | 動的物体除去 (静的マップ) | **無** | 3/5 (odom 本体ではない) |
| 4 | **Tightly-coupled mono-LiDAR** (ISR 2022) | 単眼+LiDAR 密結合 + LC | 無 | 2.5/5 |

**推奨 41 本目: FR-LIO** — RC-Vox は既存 voxelmap 群と非重複。NCLT/HDL-400/KITTI Raw
は `imu.csv` あり。LiDAR-only 簡約版も scoping 可。

**運用**: 「41本目 FR-LIO」等の明示で着手。

### 00.51 FR-LIO (41本目, RC-Vox + sub-frame + ESKS 2026-06-09)

- ✅ **実装**: `papers/fr_lio/` — RC-Vox 二層ボクセルマップ、IMU 分散による adaptive
  sub-frame、簡略 backward ESKS、IMU 回転 preintegration + point-to-plane ICP。
- ✅ **統合**: selector `fr_lio`、`--fr-lio-fast-profile` / `--fr-lio-dense-profile`、
  `imu.csv` 無しは skip（RKO-LIO と異なり IMU 必須）。
- ✅ **テスト**: `test_fr_lio` 4 cases PASS。
- ✅ **NCLT 600** (`--no-gt-seed`, fast profile, vs RKO-LIO/KISS-ICP):
  - FR-LIO: **ATE 0.58 m**, **drift 0.63%**
  - RKO-LIO: ATE 1.13 m, drift 1.42%
  - KISS-ICP: ATE 7.25 m, drift 2.76%
  - 所見: 簡略 port だが NCLT では RKO/KISS を上回る。sub-frame は NCLT IMU では
    ほぼ常に 1（avg_subframes=1）— 動的区間では増加する設計。
- Artifact: `docs/benchmarks/cross_dataset/nclt_600_fr_lio_vs_rko.json`
- **未実装**: deskew、full IEKF、重力/外参推定、著者コード無しのため論文数値との直接比較不可。

### 00.52b Mesh-LOAM (92手法目 / 3D from-paper, 2026-06-11) — **実装 + seq00/07 full 完了**

- **論文**: Zhu, Zheng & Zhu, "Mesh-LOAM: Real-time Mesh-Based LiDAR Odometry and Mapping",
  IEEE T-IV 2024, arXiv:2312.15630。公式 repo (`HelloXiaoZHU/Mesh-LOAM`) は 2024 年から
  README のみでコード未公開、サードパーティ実装も無し。新規 web サーベイ (2026-06-11) の
  第 1 推薦 (次点 ELO arXiv:2104.10879、他候補 RF-LIO / L-LO / LMBAO は PLAN 外メモ)。
- ✅ **実装**: `papers/mesh_loam/` — (1) passive-voxel IMLS-SDF 地図: 各点が 3×3×3 voxel
  近傍に符号付き距離増分を scatter (O(N)、式 5–10、hybrid weight
  `exp(-d²/h) + λ_n·max(0, nᵢ·n_v)`)、(2) dirty block 単位の増分 zero-surface 抽出、
  (3) point-to-mesh GN odometry (CV 予測 + 曲率<0.1 平面特徴 + 0.2 m facet bin 探索 +
  |cos|>0.98 法線ゲート、式 3–4)。
- **主な逸脱** (詳細 `papers/mesh_loam/README.md`): marching cubes → **marching tetrahedra**
  (table-free)、GPU並列 hash/voxel deletion → CPU `unordered_map` + radius prune、
  2×2 m 可変高 block → 立方 8³ block、入力は harness で 0.25 m downsample
  (scatter 半径 0.3 m が隣接サンプルを覆い SDF 連結は維持)。
- ✅ **テスト**: `test_mesh_loam` 5 cases PASS (SDF+mesh 構築 / 静止 / 並進回復 / 系列追跡 / yaw 回復)。
- ✅ **KITTI seq07 (108f window)**: ATE **0.135 m** / RPE **0.525%** (0.7 FPS)
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --mesh-loam-dense-profile`)**:
  ATE **0.98 m** / RPE **0.616%** / 0.005 deg/m / **0.74 FPS** —
  同 profile KISS-ICP (0.618%) と同等、from-paper 表 mid-top-10 圏。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_mesh_loam.json`
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --mesh-loam-dense-profile`)**:
  ATE **13.47 m** / RPE **0.901%** / 0.007 deg/m / **0.58 FPS** —
  同 profile KISS-ICP (0.872%) に seq00 RPE で僅差負け、from-paper 表は MCC-LO と NHC-LIO の間。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_mesh_loam.json`
  再現コマンド:
  ```bash
  ./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
    data/kitti_pcd/seq00_gt.csv --methods mesh_loam --no-gt-seed \
    --mesh-loam-dense-profile \
    --summary-json docs/benchmarks/kitti_full_new_methods/seq00_mesh_loam.json
  ```
- ✅ **docs**: README from-paper 表へ Mesh-LOAM 行追加、`papers/mesh_loam/README.md` の結果表更新。
- **状態**: 実装 + harness + methods.json (92 手法) + seq00/07 full artifact + docs 更新済。

### 00.52c ELO (93手法目 / 3D from-paper, 2026-06-11) — **実装 + seq00/07 full 完了**

- **論文**: Xin Zheng and Jianke Zhu, "Efficient LiDAR Odometry for Autonomous Driving",
  IEEE RA-L 2021, arXiv:2104.10879。著者公開実装は見つからず。論文/ KITTI detail は
  non-ground spherical range image + ground BEV map、range-adaptive normal、memory-efficient
  model update を主張。KITTI detail は SRI 2048×80、BEV 120 m × 60 m と記載。
- ✅ **実装**: `papers/elo/` — (1) non-ground SRI cell map (vertex + PCA normal)、
  (2) ground BEV cell map (ground average + neighbor normal)、(3) projective frame-to-model
  point-to-plane GN、(4) registration 後に前 model を現フレームへ移し、stale cell を落として
  SRI/BEV を融合。
- **主な逸脱** (詳細 `papers/elo/README.md`): CUDA → CPU、full 実測は tractable runtime のため
  `--elo-fast-profile` (SRI 1024×80、BEV 0.3 m)。per-point time が無いので model window は
  frame index。ground segmentation は LiDAR 高 + 5 deg slope gate と欠損時 fallback。
- ✅ **テスト**: `test_elo` 5 cases PASS (初期 map / 静止 / 並進 / yaw / sequence)。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --elo-fast-profile`)**:
  ATE **22.50 m** / RPE **1.124%** / 0.010 deg/m / **5.15 FPS** —
  論文 training seq00 0.54% には届かない honest negative だが、DiLO direct より大幅に安定。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_elo.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --elo-fast-profile`)**:
  ATE **3.54 m** / RPE **0.981%** / 0.010 deg/m / **5.55 FPS** —
  論文 training seq07 0.31% には届かない。ground BEV は drift を抑えるが、CPU 簡略 port では
  KISS 同等までは届かず。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_elo.json`
- 再現コマンド:
  ```bash
  ./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
    data/kitti_pcd/seq00_gt.csv --methods elo --no-gt-seed \
    --elo-fast-profile \
    --summary-json docs/benchmarks/kitti_full_new_methods/seq00_elo.json
  ```
- ✅ **docs**: README from-paper 表へ ELO 行追加、`docs/methods.json` 93 手法、
  `papers/elo/README.md` に seq00/07 結果表を追加。
- **状態**: 実装 + harness + methods.json (93 手法) + seq00/07 full artifact + docs 更新済。

### 00.52d ID-LIO (94手法目 / 3D LIO from-paper, 2026-06-11) — **実装 + seq00/07 full 完了**

- **論文**: Weizhuang Wu and Wanliang Wang, "LiDAR Inertial Odometry Based on Indexed
  Point and Delayed Removal Strategy in Highly Dynamic Environments", Sensors 2023
  23(11):5188, doi:10.3390/s23115188。著者公開実装は見つからず。LIO-SAM ベースで
  pseudo occupancy による dynamic point detection、indexed point propagation、
  delayed removal、dynamic weight を主張。
- ✅ **実装**: `papers/id_lio/` — (1) current scan / predicted map の spherical
  range image pseudo-occupancy、(2) map 点に stable id / confidence / missing age /
  dynamic age を保持する indexed voxel map、(3) delayed removal 前の低重み保持、
  (4) source dynamic weight + map confidence 付き point-to-plane scan-to-map、
  (5) `imu.csv` がある場合のみ gyro preintegration rotation prior。
- **主な逸脱** (詳細 `papers/id_lio/README.md`): 論文の LIO-SAM feature extraction /
  keyframe graph / GTSAM / loop closure / full IMU factor は範囲外。KITTI PCD export は
  `frame_timestamps.csv` のみで `imu.csv` が無いため、full KITTI は constant-velocity fallback。
- ✅ **テスト**: `test_id_lio` 5 cases PASS (初期 indexed map / 並進 sequence /
  IMU yaw prior / foreground dynamic downweight / delayed removal)。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --id-lio-dense-profile`)**:
  ATE **15.45 m** / RPE **1.111%** / 0.014 deg/m / **7.82 FPS**。
  Dynamic machinery は active (dynamic/frame ≈464) だが、top KISS-like front-end には届かない
  honest mid-pack。Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_id_lio.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --id-lio-dense-profile`)**:
  ATE **4.61 m** / RPE **0.999%** / 0.013 deg/m / **11.52 FPS**。
  Dynamic/frame ≈432。ELO と同程度だが seq07 は僅差で悪い。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_id_lio.json`
- 再現コマンド:
  ```bash
  ./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
    data/kitti_pcd/seq00_gt.csv --methods id_lio --no-gt-seed \
    --id-lio-dense-profile \
    --summary-json docs/benchmarks/kitti_full_new_methods/seq00_id_lio.json
  ```
- ✅ **docs**: README from-paper 表へ ID-LIO 行追加、`docs/methods.json` 94 手法、
  `papers/id_lio/README.md` に seq00/07 結果表を追加。
- **状態**: 実装 + harness + methods.json (94 手法) + seq00/07 full artifact + docs 更新済。

### 00.52e TC-LVGF (95手法目 / 3D LiDAR-visual from-paper, 2026-06-11) — **実装 + seq00/07 full 完了**

- **論文**: Ke Cao et al., "Tightly-Coupled LiDAR-Visual SLAM Based on Geometric
  Features for Mobile Agents", ROBIO 2023 / arXiv:2307.07763。作者実装は見つからず。
  LiDAR subsystem と monocular visual subsystem の geometric features を spherical
  fusion frame で対応づけ、visual line で LiDAR linear feature を補正し、LiDAR depth /
  direction で visual line landmark を補強する主張。
- ✅ **実装**: `papers/tc_lvgf/` — (1) spherical range-image fusion frame、
  (2) range-image 局所 PCA による LiDAR linear feature、(3) KITTI PCD 用の
  pseudo-visual sparse range-line segment、(4) visual/LiDAR line direction fusion、
  (5) point-to-plane + light point-to-line / direction residual scan-to-map、
  (6) visual line 不足時の LiDAR fallback。
- **主な逸脱**: RGB ORB/LSD / semantic association / dual SLAM backend /
  loop closure は範囲外。KITTI Odometry PCD export は Velodyne のみなので、visual
  line は LiDAR range-image から生成。短窓 ablation では強い line residual が僅かに悪化したため、
  dense profile は `line_weight=0.2`, `direction_weight=0.05`。
- ✅ **テスト**: `test_tc_lvgf` 4 cases PASS (球面投影 / line 抽出+fusion /
  並進 tracking / visual sparse fallback)。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --tc-lvgf-dense-profile`)**:
  ATE **11.95 m** / RPE **1.055%** / 0.011 deg/m / **8.43 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_tc_lvgf.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --tc-lvgf-dense-profile`)**:
  ATE **3.74 m** / RPE **0.941%** / 0.011 deg/m / **11.21 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_tc_lvgf.json`
- 所見: pseudo-visual line は full で fallback 0 (約56 visual lines/frame) と安定。
  PL-LOAM/VLOM の疑似画像 honest negative より大幅に良いが、最上位の KISS-like
  point-to-plane core はまだ超えない mid-pack positive。
- ✅ **docs**: README from-paper 表へ TC-LVGF 行追加、`docs/methods.json` 95 手法、
  `papers/tc_lvgf/README.md` に seq00/07 結果表を追加。
  **状態**: 実装 + harness + methods.json (95 手法) + seq00/07 full artifact + docs 更新済。

### 00.52f OPL-LVIO (96手法目 / 3D LiDAR-visual-inertial from-paper, 2026-06-11) — **実装 + seq00/07 full 完了**

- **論文**: Xuan He et al., "LiDAR-Visual-Inertial Odometry Based on Optimized
  Visual Point-Line Features", Remote Sensing 2022, 14(3):622。作者実装は web/GitHub
  survey で見つからず。improved visual point/line features、LiDAR depth association、
  VIO-assisted LiDAR scan matching、Bayesian/factor-graph fusion、Helmert variance
  component weighting が主張。
- ✅ **実装**: `papers/opl_lvio/` — (1) KITTI PCD 用 range-image visual point
  proxy (high-curvature cells)、(2) smooth segment の pseudo visual line、
  (3) LiDAR depth correlation 付き visual point/line map、(4) scan-to-map
  point-to-plane + visual point residual、(5) Helmert 風 residual variance weight
  adaptation、(6) visual feature 不足時の LiDAR fallback。
- **主な逸脱**: RGB feature detector / IMU VIO / GNSS / loop closure / graph backend は
  範囲外。KITTI Odometry PCD export は Velodyne のみなので visual point/line は LiDAR
  range-image proxy。seq07 108f ablation では pseudo-line residual が僅かに悪化したため、
  dense profile は `visual_point_weight=0.08`, `line_weight=0`, `direction_weight=0`。
- ✅ **テスト**: `test_opl_lvio` 4 cases PASS (球面投影 / point+line 抽出 /
  point residual 付き並進 tracking / visual sparse fallback)。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --opl-lvio-dense-profile`)**:
  ATE **15.24 m** / RPE **1.050%** / 0.011 deg/m / **7.67 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_opl_lvio.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --opl-lvio-dense-profile`)**:
  ATE **3.65 m** / RPE **0.902%** / 0.011 deg/m / **11.18 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_opl_lvio.json`
- 所見: range-image visual points は fallback 0 (seq00 約94.8/frame、seq07 約98.8/frame) で安定。
  RPE は TC-LVGF より僅かに良い (1.050/0.902 vs 1.055/0.941) が、seq00 ATE は悪い
  (15.24 m vs 11.95 m)。pseudo visual feature は有効だが、主成分は依然 point-to-plane core。
- ✅ **docs**: README from-paper 表へ OPL-LVIO 行追加、`docs/methods.json` 96 手法、
  `papers/opl_lvio/README.md` に seq00/07 結果表を追加。
  **状態**: 実装 + harness + methods.json (96 手法) + seq00/07 full artifact + docs 更新済。

### 00.52g V-LOAM2015 / TC-VLO / AD-VLO / TC-MVLO (97-100手法目 / 3D LiDAR-visual from-paper, 2026-06-12) — **実装 + seq00/07 full 完了**

- **論文**:
  - **V-LOAM2015**: Ji Zhang and Sanjiv Singh, "Visual-Lidar Odometry and Mapping:
    Low-drift, Robust, and Fast", ICRA 2015。visual odometry bootstrap + LiDAR
    scan matching refinement。
  - **TC-VLO**: Young-Woo Seo and Chih-Chung Chou, "A Tight Coupling of
    Vision-Lidar Measurements for an Effective Odometry", IEEE IV 2019。visual /
    LiDAR residual を pose solve に密結合。
  - **AD-VLO**: Kaihong Huang, Junhao Xiao, Cyrill Stachniss, "Accurate Direct
    Visual-Laser Odometry with Explicit Occlusion Handling and Plane Detection",
    ICRA 2019。direct visual-laser alignment + occlusion / plane handling。
  - **TC-MVLO**: Lingbo Meng, Chao Ye, Weiyang Lin, "A Tightly Coupled Monocular
    Visual Lidar Odometry with Loop Closure", Intelligent Service Robotics 2022。
    monocular visual-LiDAR tight coupling + loop closure。
- ✅ **実装**: `papers/v_loam15/`, `papers/tc_vlo/`, `papers/ad_vlo/`,
  `papers/tc_mvlo/`。いずれも `opl_lvio` backend 上の paper-specific adapter:
  range-image visual point/line proxy、LiDAR depth付き visual residual、
  scan-to-map point-to-plane refinement、Helmert-style residual scale balancingを共有し、
  有効化する residual / weight / fallback 条件を論文ごとに分離。
- **主な profile 差分**:
  - V-LOAM2015: visual point bootstrap + small motion prior、line residual 無効。
  - TC-VLO: visual point + point-to-line + direction residual を軽量に密結合。
  - AD-VLO: range-jump occlusion gate + plane emphasis、line residual 無効。
  - TC-MVLO: visual point + line residual を TC-VLO より強め、Helmert clamp を狭める。
- **主な逸脱**: 4 本とも RGB camera tracker / true monocular VO / direct photometric
  image alignment / full SLAM graph / loop closure は範囲外。KITTI Odometry PCD export は
  Velodyne のみなので、visual features は LiDAR range-image proxy。PL-LOAM/VLOM の
  depth-gradient pseudo-image より安定するが、主成分は依然 point-to-plane core。
- ✅ **テスト**:
  `test_v_loam15`, `test_tc_vlo`, `test_ad_vlo`, `test_tc_mvlo` 各 3 cases PASS
  (profile 適用 / feature 抽出 / short translation tracking)。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed`, dense profile)**:
  - V-LOAM2015: ATE **14.37 m** / RPE **1.066%** / **7.76 FPS**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_v_loam15.json`
  - TC-VLO: ATE **12.01 m** / RPE **1.060%** / **7.64 FPS**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_tc_vlo.json`
  - AD-VLO: ATE **12.95 m** / RPE **1.052%** / **8.15 FPS**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_ad_vlo.json`
  - TC-MVLO: ATE **10.82 m** / RPE **1.054%** / **8.00 FPS**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_tc_mvlo.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed`, dense profile)**:
  - V-LOAM2015: ATE **3.63 m** / RPE **0.910%** / **10.05 FPS**。
  - TC-VLO: ATE **4.22 m** / RPE **0.925%** / **10.91 FPS**。
  - AD-VLO: ATE **3.08 m** / RPE **0.939%** / **9.96 FPS**。
  - TC-MVLO: ATE **3.00 m** / RPE **0.939%** / **10.76 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_visual_lidar_97_100.json`
- 所見: LiDAR-visual adapter family は 0.90-1.07% drift で安定し、PL-LOAM/VLOM の
  pseudo-image honest negative より大幅に良い。AD-VLO は seq00 RPE、TC-MVLO は ATE、
  V-LOAM2015 は seq07 RPE が強い。ただし KISS-like point-to-plane core を上回るほどではなく、
  pseudo-visual residual は補助的。
- ✅ **docs**: README from-paper 表へ 4 行追加、`docs/methods.json` 100 手法、
  各 module README と再現 artifact を追加。
  **状態**: 実装 + harness + methods.json (100 手法) + seq00/07 full artifact + docs 更新済。

### 00.52h RF-LIO (101手法目 / 3D dynamic LiDAR-inertial from-paper, 2026-06-12) — **実装 + seq00/07 full 完了**

- **論文**: Chenglong Qian et al., "RF-LIO: Removal-First Tightly-coupled Lidar
  Inertial Odometry in High Dynamic Environments", IROS 2021 / arXiv:2206.09463。
  LIO-SAM 系の tightly coupled LIO に adaptive multi-resolution range image の
  removal-first dynamic object filtering を入れ、scan-match 前に moving foreground を落とす。
- ✅ **実装**: `papers/rf_lio/` — (1) 前フレーム static scan を予測 current sensor frame へ投影、
  (2) fine/mid/coarse の multi-resolution range image、(3) 2 解像度以上で predicted surface より
  近い点を foreground candidate として registration 前に除去、(4) removal cap / min keep guard、
  (5) `id_lio` backend の indexed dynamic map / delayed removal / IMU gyro prior を再利用。
- **主な逸脱**: full LIO-SAM factor graph / submap optimization / tightly coupled IMU backend は範囲外。
  KITTI Odometry PCD は `imu.csv` が無いので constant-velocity fallback。adaptive range image は
  compact previous-scan foreground test として実装。
- ✅ **テスト**: `test_rf_lio` 4 cases PASS (first map / foreground removal-first /
  short translation tracking / IMU prior pass-through)。
- ✅ **KITTI seq07 short smoke (108f, dense)**:
  ATE **0.399 m** / RPE **0.592%** / **21.1 FPS**。短窓では removal-first が良好。
- ✅ **KITTI seq00 full (4541f, `--no-gt-seed --rf-lio-dense-profile`)**:
  ATE **22.52 m** / RPE **1.351%** / 0.019 deg/m / **6.56 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_rf_lio.json`
- ✅ **KITTI seq07 full (1101f, `--no-gt-seed --rf-lio-dense-profile`)**:
  ATE **4.81 m** / RPE **1.272%** / 0.018 deg/m / **11.06 FPS**。
  Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_rf_lio.json`
- 所見: removal-first は active (seq00 **273** 点/frame、seq07 **246** 点/frame 除去) だが、
  mostly static な KITTI では useful foreground も削り、ID-LIO (1.111/0.999%) より悪化。
  paper の狙いは high dynamic scene なので、KITTI では stable-but-below-baseline の honest negative。
- ✅ **docs**: README from-paper 表へ RF-LIO 行追加、`docs/methods.json` 101 手法、
  `papers/rf_lio/README.md` と seq00/07 artifact を追加。
  **状態**: 実装 + harness + methods.json (101 手法) + seq00/07 full artifact + docs 更新済。

### 00.52i Paper-ready reproducibility cleanup (2026-06-12) — **claim tiers + README boundary**

ユーザー指示: **「ronbun wo daseru kurai ni saigen zissou wo tyanto sasete, readme mo seibi sitai」**。
新規手法追加を一段止め、論文に出せる再現実装 subset と広い catalog を切り分ける。

- ✅ **新規 docs**: [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md)
  を追加。T0 paper-grade / T1 mechanism-grade / T2 adapter-grade / T3 smoke-concept の
  claim tier、paper-grade checklist、README claim policy を定義。
- ✅ **README claim boundary**: 101 手法の breadth catalog と、manuscript-level claim に使う
  paper-ready subset を分離。`73 paper reimplementations` / `42 papers with no public author code`
  の表示を現在の `docs/methods.json` と同期。
- ✅ **reproducibility report**: [`docs/reproducibility_report.md`](docs/reproducibility_report.md)
  から paper-ready plan へ導線を追加。
- **T0/T1 近傍候補**:
  - I-LOAM: intensity on/off の full artifact と paired commands を追加済み (§00.52j)。
  - KC-LO: sigma schedule ablation と runtime/accuracy trade-off を追加済み (§00.52k)。
  - M-GCLO: ground-factor off ablation は追加済み (§00.52l)。synthetic non-flat ground stress も追加済み (§00.52p)。
    T0 昇格には public non-flat dataset validation が必要。
  - Quadric-LO: plane-fallback ablation は追加済み (§00.52n)。synthetic curved-object stress も追加済み (§00.52q)。
    T0 昇格には public curved-object / non-urban dataset validation が必要。
  - RF-LIO/ID-LIO: synthetic dynamic-object stress は追加済み (§00.52o)。KITTI と synthetic だけで
    paper claim しない。public high-dynamic dataset validation を追加してから昇格判断。
- **次の既定動作**: 新規 102 手法目ではなく、8-12 method の frozen paper table と paired ablation を
  作る。README は breadth、論文本文は T0/T1 subset、T2/T3 は appendix catalog。
  現在の seed bundle は `docs/benchmarks/paper_ready_bundle.json` (§00.52m/§00.52n)。

### 00.52j I-LOAM paper-ready ablation artifacts (2026-06-12) — **seq00/07 full raw JSON 完了**

paper-ready hardening の 1 本目として、I-LOAM の intensity on/off full-sequence paired artifacts を
生成し、既存 README の ablation 数値を raw JSON 付きにした。

- ✅ **seq00 full, scan-to-scan only**:
  - intensity on: RPE **2.606375%**, ATE **49.432759 m**, FPS **8.52**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json`
  - intensity off: RPE **3.185596%**, ATE **76.028538 m**, FPS **8.38**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json`
  - delta: drift **-18.18%**, ATE **-34.98%**。
- ✅ **seq07 full, scan-to-scan only**:
  - intensity on: RPE **3.055169%**, ATE **15.132579 m**, FPS **8.58**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json`
  - intensity off: RPE **3.806092%**, ATE **18.502306 m**, FPS **8.88**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json`
  - delta: drift **-19.73%**, ATE **-18.21%**。
- ✅ **paired summary**:
  `docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json`
- ✅ **docs**: README ablation 節、`papers/i_loam/README.md` reproduce commands、
  `docs/paper_ready_reproducibility.md`、`docs/reproducibility_report.md` を更新。
- **次**: §00.52m の seed bundle に固定済み。次は Quadric-LO plane-fallback ablation、
  RF-LIO/ID-LIO public high-dynamic dataset validation、または M-GCLO public non-flat dataset validation。

### 00.52k KC-LO sigma schedule ablation artifacts (2026-06-12) — **seq00/07 full raw JSON 完了**

paper-ready hardening の 2 本目として、KC-LO の coarse-to-fine sigma annealing と fixed fine sigma を
seq00/07 full で比較し、runtime/accuracy trade-off を raw artifact 付きにした。

- ✅ **seq00 full**:
  - annealed σ 1.5→0.4: RPE **0.841902%**, ATE **14.215286 m**, FPS **1.39**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_kc_lo.json`
  - fixed σ=0.4: RPE **0.837450%**, ATE **13.404514 m**, FPS **2.65**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_kc_lo_no_anneal.json`
  - delta fixed vs annealed: RPE **-0.53% relative**, FPS **+90.0%**。
- ✅ **seq07 full**:
  - annealed σ 1.5→0.4: RPE **0.514327%**, ATE **0.830931 m**, FPS **1.39**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_kc_lo.json`
  - fixed σ=0.4: RPE **0.509921%**, ATE **0.858036 m**, FPS **3.12**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_kc_lo_no_anneal.json`
  - delta fixed vs annealed: RPE **-0.86% relative**, FPS **+124.0%**。
- ✅ **paired summary**:
  `docs/benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json`
- ✅ **code metadata fix**: `evaluation/src/pcd_dogfooding.cpp` の KC-LO note を修正し、
  future artifact が `fixed sigma (no annealing)` / `coarse-to-fine sigma annealing` を正しく記録。
- ✅ **docs**: README KC-LO row を fixed-sigma best に更新、`papers/kc_lo/README.md` に
  ablation table と reproduce command 追加、paper-ready plan / reproducibility report 更新。
- **所見**: KITTI full + constant-velocity prediction では、kernel-correlation 自体が主効果。
  σ annealing は大域収束の保険で、well-initialized odometry では fixed fine σ が速くて同等精度。
- **次**: §00.52m の seed bundle に固定済み。次は Quadric-LO plane-fallback ablation、
  RF-LIO/ID-LIO public high-dynamic dataset validation、または M-GCLO public non-flat dataset validation。

### 00.52l M-GCLO ground-factor ablation artifacts (2026-06-12) — **seq00/07 full raw JSON 完了**

paper-ready hardening の 3 本目として、M-GCLO の multiple-ground point-to-plane factor を
`--m-gclo-no-ground` で無効化し、paper mechanism の ground-on 既存 artifact と seq00/07 full で
比較した。ground off では分類された ground correspondence を統計だけ数え、最適化から外す。

- ✅ **code**:
  - `evaluation/src/pcd_dogfooding.cpp` に `--m-gclo-no-ground` と note 分岐を追加。
  - `papers/m_gclo/src/m_gclo.cpp` は `ground_weight <= 0` の ground correspondence を
    optimizer から除外する。
- ✅ **seq00 full**:
  - ground on: RPE **0.835367%**, rot **0.007618 deg/m**, ATE **19.143798 m**, FPS **3.41**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_m_gclo.json`
  - ground off: RPE **0.833008%**, rot **0.010510 deg/m**, ATE **46.934202 m**, FPS **5.85**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_m_gclo_no_ground.json`
  - delta ground off vs on: translational RPE **-0.28% relative**, ATE **+145.17%**,
    rotational RPE **+37.96%**。
- ✅ **seq07 full**:
  - ground on: RPE **0.670573%**, rot **0.010557 deg/m**, ATE **2.023877 m**, FPS **3.67**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_m_gclo.json`
  - ground off: RPE **0.600358%**, rot **0.011158 deg/m**, ATE **5.316227 m**, FPS **5.68**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_m_gclo_no_ground.json`
  - delta ground off vs on: translational RPE **-10.47% relative**, ATE **+162.68%**,
    rotational RPE **+5.69%**。
- ✅ **paired summary**:
  `docs/benchmarks/kitti_full_new_methods/m_gclo_ground_factor_ablation.json`
- ✅ **docs**: README M-GCLO paragraph、`papers/m_gclo/README.md`、paper-ready plan、
  reproducibility report を更新。
- **所見**: KITTI seq00/07 では ground factor は translational RPE reducer というより
  global/attitude stability の anchoring term。RPE trans だけでは機構効果を過小評価する。
- **次**: §00.52m の seed bundle に固定済み。次は Quadric-LO plane-fallback ablation、
  RF-LIO/ID-LIO public high-dynamic dataset validation、または M-GCLO public non-flat dataset validation。

### 00.52m Paper-ready bundle manifest (2026-06-12) — **3-method seed bundle 完了**

paper-ready hardening の 4 本目として、I-LOAM / KC-LO / M-GCLO の raw artifacts と paired
ablations を 1 つの frozen evidence manifest に固定した。

- ✅ **script**: `evaluation/scripts/build_paper_ready_bundle.py`
  - committed raw JSON と paired summary を読み、summary metric と raw artifact metric の不一致を検出。
  - `--check` で manifest の stale check が可能。
  - reproduce command はすべて repo 相対 path (`data/kitti_pcd/...`,
    `docs/benchmarks/...`) で記録。
- ✅ **manifest**: `docs/benchmarks/paper_ready_bundle.json`
  - `bundle_id`: `paper_ready_core_2026_06_12`
  - frozen methods: **3** (後続 §00.52n で **4** に拡張)
  - paper table rows: **6** (後続 §00.52n で **8** に拡張)
  - paired ablation summaries: **3** (後続 §00.52n で **4** に拡張)
  - T0 evidence candidates: **I-LOAM, KC-LO**
  - T1+ evidence candidates: **M-GCLO**
- ✅ **docs**: README claim boundary、`docs/paper_ready_reproducibility.md`,
  `docs/reproducibility_report.md`, PLAN を bundle に同期。
- ✅ **validation**:
  - `python3 evaluation/scripts/build_paper_ready_bundle.py --check`
  - `python3 -m json.tool docs/benchmarks/paper_ready_bundle.json`
- **次**: Quadric-LO plane-fallback ablation、RF-LIO/ID-LIO public high-dynamic dataset validation、
  または M-GCLO public non-flat dataset validation で seed bundle を 8-12 method table に拡張。

### 00.52n Quadric-LO plane-fallback ablation artifacts (2026-06-12) — **seq00/07 full raw JSON 完了**

paper-ready hardening の 5 本目として、Quadric-LO の point-to-plane fallback を
`--quadric-lo-no-plane-fallback` で無効化し、paper mechanism の fallback-on 既存 artifact と
seq00/07 full で比較した。fallback off では quadric が立たない対応を optimizer から外す。

- ✅ **code**:
  - `papers/quadric_lo/include/quadric_lo/quadric_lo.h` に `allow_plane_fallback` を追加。
  - `papers/quadric_lo/src/quadric_lo.cpp` で fallback 分岐を無効化可能にした。
  - `evaluation/src/pcd_dogfooding.cpp` に `--quadric-lo-no-plane-fallback` と
    `plane_fallback_ratio` note を追加。
- ✅ **seq00 full**:
  - fallback on: RPE **0.867316%**, rot **0.007640 deg/m**, ATE **14.736361 m**,
    FPS **0.62**, fallback ratio **0.005757**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_quadric_lo.json`
  - fallback off: RPE **0.879792%**, rot **0.007823 deg/m**, ATE **12.894692 m**,
    FPS **1.13**, fallback ratio **0.0**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq00_quadric_lo_no_plane_fallback.json`
  - delta off vs on: translational RPE **+1.44% relative**, ATE **-12.50%**, FPS **+83.34%**。
- ✅ **seq07 full**:
  - fallback on: RPE **0.597674%**, rot **0.006249 deg/m**, ATE **2.129764 m**,
    FPS **0.67**, fallback ratio **0.005078**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_quadric_lo.json`
  - fallback off: RPE **0.589873%**, rot **0.006165 deg/m**, ATE **2.146722 m**,
    FPS **1.06**, fallback ratio **0.0**。
    Artifact: `docs/benchmarks/kitti_full_new_methods/seq07_quadric_lo_no_plane_fallback.json`
  - delta off vs on: translational RPE **-1.31% relative**, ATE **+0.80%**, FPS **+58.73%**。
- ✅ **paired summary**:
  `docs/benchmarks/kitti_full_new_methods/quadric_lo_plane_fallback_ablation.json`
- ✅ **bundle**: `docs/benchmarks/paper_ready_bundle.json` を **4 methods / 8 rows / 4 ablations** に拡張。
- ✅ **docs**: README Quadric paragraph、`papers/quadric_lo/README.md`、paper-ready plan、
  reproducibility report、PLAN を更新。
- **所見**: KITTI seq00/07 では plane fallback は used correspondence の約 0.5-0.6% と少なく、
  Quadric-LO の evidence は fallback ではなく point-to-quadric path が支配的。T0 昇格には
  public curved-object / non-urban dataset validation が残る。
- **次**: RF-LIO/ID-LIO public high-dynamic dataset validation、M-GCLO public non-flat dataset validation、
  または Quadric-LO public curved-object / non-urban dataset validation。

### 00.52o RF-LIO/ID-LIO synthetic dynamic-object stress (2026-06-12) — **dataset-free stress artifact 完了**

paper-ready hardening の 6 本目として、RF-LIO/ID-LIO の dynamic filtering を KITTI 以外で
機構確認する dataset-free stress を追加した。自車は直進のみ、静的 corridor + 固定 landmark に
横断する box-shaped foreground objects を混ぜる。clean fixture と dynamic fixture を同一 runner で
生成し、30-frame window を `pcd_dogfooding` に流す。

- ✅ **generator**: `evaluation/scripts/generate_dynamic_object_stress_fixture.py`
  - `evaluation/fixtures/dynamic_object_stress_clean`
  - `evaluation/fixtures/dynamic_object_stress`
  - fixture は生成物なので `.gitignore` で除外。再現は runner から行う。
- ✅ **runner**: `evaluation/scripts/run_dynamic_object_stress.py`
  - clean/default: `docs/benchmarks/dynamic_object_stress/static_clean_default_30.json`
  - dynamic/default: `docs/benchmarks/dynamic_object_stress/dynamic_objects_default_30.json`
  - dynamic/RF conservative: `docs/benchmarks/dynamic_object_stress/dynamic_objects_rf_lio_conservative_30.json`
  - summary: `docs/benchmarks/dynamic_object_stress/rf_id_lio_dynamic_object_stress_summary.json`
- ✅ **結果 (30 frames, no GT seed)**:
  - KISS sanity ref: clean ATE **0.028915 m** → dynamic **0.040450 m**。
  - ID-LIO: clean ATE **0.675926 m** → dynamic **130.549154 m**、`dynamic/frame=78.566667`。
  - RF-LIO default: clean ATE **2.486800 m** → dynamic **49.932244 m**、
    `removal_first/frame=214.266667`。
  - RF-LIO conservative (`--rf-lio-foreground-margin 1.5 --rf-lio-max-removal-fraction 0.08`):
    dynamic ATE **41.632002 m**、default RF 比 ATE **-16.62%**。
- **所見**: synthetic foreground boxes は両 dynamic path を強く activate する。RF-LIO は ID-LIO より
  failure severity を抑え、removal cap も効く。ただし KISS sanity reference はほぼ崩れないため、
  これは mechanism/failure-boundary artifact であって、paper-grade dynamic-scene claim には
  public high-dynamic dataset validation が必要。
- **docs**: README、`papers/id_lio/README.md`、`papers/rf_lio/README.md`、
  `docs/paper_ready_reproducibility.md`、`docs/reproducibility_report.md`、paper-ready bundle status を更新。
- **次**: M-GCLO public non-flat dataset validation、Quadric-LO public curved-object / non-urban dataset validation、
  または RF-LIO/ID-LIO public high-dynamic dataset validation。

### 00.52p M-GCLO synthetic non-flat ground stress (2026-06-12) — **dataset-free stress artifact 完了**

paper-ready hardening の 7 本目として、M-GCLO の multiple-ground factor を KITTI 以外の
rolling-ground synthetic fixture で stress した。ローカルの prepared NCLT/MulRan PCD はこの
worktree には残っていなかったため、今回の artifact は public dataset validation ではなく
dataset-free mechanism stress として扱う。

- ✅ **generator**: `evaluation/scripts/generate_nonflat_ground_stress_fixture.py`
  - rolling ground + low banks/curbs + non-ground landmark を生成。
  - fixture は `evaluation/fixtures/nonflat_ground_stress` に生成し、`.gitignore` で除外。
- ✅ **runner**: `evaluation/scripts/run_m_gclo_nonflat_stress.py`
  - ground on + KISS sanity:
    `docs/benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_on_60.json`
  - ground off:
    `docs/benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_off_60.json`
  - summary:
    `docs/benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_stress_summary.json`
- ✅ **結果 (60 frames, no GT seed)**:
  - KISS sanity/failure ref: ATE **22.159251 m**, RPE **180.223576%**。
  - M-GCLO ground on: ATE **0.116132 m**, RPE **0.499712%**,
    rot **0.002823 deg/m**, `mean_ground/nonground_corr=1278/241`。
  - M-GCLO ground off: ATE **0.149619 m**, RPE **0.674768%**,
    rot **0.020621 deg/m**, `mean_ground/nonground_corr=1288/231`。
  - delta ground off vs on: ATE **+28.84%**, RPE **+35.03%**,
    rotational RPE **+630%**。
- **所見**: rolling-ground stress では multiple-ground path が明確に効く。KITTI ablation では
  ground off が translational RPE だけなら同等/低めだったが、non-flat synthetic では ATE/RPE/rot が
  すべて悪化する。T0 昇格には public non-flat dataset validation がまだ必要。
- **docs**: README、`papers/m_gclo/README.md`、paper-ready plan、
  reproducibility report、paper-ready bundle status を更新。
- **次**: Quadric-LO public curved-object / non-urban dataset validation、RF-LIO/ID-LIO public high-dynamic dataset validation、
  または M-GCLO public non-flat dataset validation。

### 00.52q Quadric-LO synthetic curved-object stress (2026-06-12) — **dataset-free stress artifact 完了**

paper-ready hardening の 8 本目として、Quadric-LO の point-to-quadric path を orchard-like な
curved-object / non-urban synthetic fixture で stress した。木の幹 (cylinders)、岩/低木
(ellipsoids)、疎な起伏地面で構成し、平面 fallback ではなく quadric 対応が支配的になるようにした。

- ✅ **generator**: `evaluation/scripts/generate_quadric_curved_stress_fixture.py`
  - `evaluation/fixtures/quadric_curved_stress` に生成。
  - fixture は生成物なので `.gitignore` で除外。再現は runner から行う。
- ✅ **runner**: `evaluation/scripts/run_quadric_curved_stress.py`
  - fallback on + KISS sanity:
    `docs/benchmarks/quadric_curved_stress/quadric_curved_fallback_on_60.json`
  - fallback off:
    `docs/benchmarks/quadric_curved_stress/quadric_curved_fallback_off_60.json`
  - summary:
    `docs/benchmarks/quadric_curved_stress/quadric_curved_stress_summary.json`
- ✅ **結果 (60 frames, no GT seed)**:
  - KISS sanity ref: ATE **0.018757 m**, RPE **0.118534%**。
  - Quadric-LO fallback on: ATE **0.068929 m**, RPE **0.611652%**,
    rot **0.013907 deg/m**, `mean_quadric/plane_corr=1242/12`,
    `plane_fallback_ratio=0.0094`。
  - Quadric-LO fallback off: ATE **0.069924 m**, RPE **0.608845%**,
    rot **0.015174 deg/m**, `mean_quadric/plane_corr=1245/0`。
  - delta fallback off vs on: ATE **+1.44%**, RPE **-0.46%**, rot **+9.11%**。
- **所見**: curved synthetic では対応の 99% 以上が point-to-quadric。plane fallback を切っても
  ATE/RPE はほぼ不変で、KITTI ablation と同じく evidence は fallback ではなく quadric path が支配的。
  ただし KISS sanity reference も非常に良く、これは public curved/non-urban dataset validation の代替ではない。
- **docs**: README、`papers/quadric_lo/README.md`、paper-ready plan、
  reproducibility report、paper-ready bundle status を更新。
- **次**: RF-LIO/ID-LIO public high-dynamic dataset validation、M-GCLO public non-flat dataset validation、
  または Quadric-LO public curved-object / non-urban dataset validation。

### 00.52 PG-LIO (42本目, NCC photometric + geometric + IMU 2026-06-09)

- ✅ **実装**: `papers/pg_lio/` — range-image NCC パッチ + point-to-plane ICP +
  幾何退化時 photometric 重み増加 + IMU 並進 prior。
- ✅ **統合**: selector `pg_lio`、`--pg-lio-fast-profile` / `--pg-lio-dense-profile`、
  `imu.csv` + PointXYZI 必須（欠如時 skip）。
- ✅ **テスト**: `test_pg_lio` 4 cases PASS。
- ✅ **NCLT 600** (`--no-gt-seed`, fast profile, vs FR-LIO):
  - FR-LIO: ATE **0.58 m**, drift **0.63%**
  - PG-LIO (初版): ATE **23.7 m**, drift **33.0%** — ** honest negative**
  - 所見: 簡略 NCC + 非 organized Velodyne 投影で photometric 因子が不安定。
    patch 上限・退化時のみ photometric 適用・IMU 並進 prior 除去で改善余地あり。
- Artifact: `docs/benchmarks/cross_dataset/nclt_600_pg_lio_vs_fr_lio.json`
- **未実装**: sliding-window factor graph、deskew、Ouster bias LUT、full IEKF。

**状態**: ワークツリーに未コミット。NCLT 600 で honest negative のため、**当面は 2D LiDAR 路線を優先**（下記 §00.6c）。

### 00.6c 方針転換 — 2D LiDAR キャンペーン (2026-06-09)

ユーザー指示: **「しばらくは 2D LiDAR でやっていく」**。3D Velodyne + IMU 系
(FR-LIO 完了、PG-LIO は保留) から、**planar laser scan** 系 odometry/SLAM へ軸足を移す。

#### キャンペーン状態: Phase 1 + MbICP **完了** (papers 43–50)

| Phase | 内容 | 状態 |
|-------|------|------|
| Phase 1a | harness + RF2O/PL-ICP/CSM/Kinematic-ICP | ✅ push 済 |
| Phase 1b | 評価インフラ (drift, bag prep, corridor) | ✅ push 済 |
| Phase 1c | PSM + Intel CI smoke | ✅ push 済 (`709d25c`) |
| Phase 1d | Bonn 公開 fixtures (intel/fr079/mit) + 5法ベンチ | ✅ push 済 (`a2817ff`) |
| Phase 1e | NDT-2D (48本目) | ✅ push 済 (`8ea40f1`) |
| Phase 1f | IDC (49本目) | ✅ commit、**未 push** (`361a592`) |
| Phase 1g | CSM DT + pyramid refresh | ✅ commit、**未 push** (`3fc5be0`) |
| Phase 1h | ドキュメント整備 (scan2d hub) | ✅ ワークツリー、**未 commit** |
| Phase 2a | MbICP (50本目) + 8-method refresh | ✅ ワークツリー |

#### 現状ギャップ (Phase 2 以降)

- **canonical JSON 統合**: `run_scan2d_benchmark.sh` で 8 法を 1 JSON/fixture に refresh 済み。
- **MIT val**: 33 frame のみ — drift は indicative。長 window が必要。
- **合成 corridor**: MbICP 0.46% が PL-ICP 0.38% に近接。CSM は ~73% で honest negative 継続。
- **local map / SLAM graph**: 全 8 法 scan-to-scan のみ — Karto/Hector 級には未達。
- **ROS2 / 実 bag CI**: smoke は Intel 20f slice のみ。full bag export は手動。

#### インフラ

| 要素 | 状態 |
|------|------|
| Harness | ✅ `evaluation/src/scan_dogfooding.cpp` |
| 入力 | ✅ `scan_meta.json` + `NNNNNNNN/scan.csv` |
| GT | ✅ `gt.csv`: timestamp, x, y, yaw [rad] |
| 評価 | ✅ 2D ATE + drift [%] (KITTI-style RPE segment 規則) + `--summary-json` |
| GT-seed | ✅ frame 0 anchor; `--no-gt-seed` 対応 |
| 前処理 (ROS1) | ✅ `prepare_2d_scan_inputs.py` |
| 前処理 (Bonn JSON) | ✅ `prepare_bonn_2dslam_inputs.py` |
| 合成 fixture 生成 | ✅ `generate_rf2o_smoke_fixture.py`, `generate_rf2o_corridor_fixture.py` |
| GT 可視化 | ✅ `plot_scan2d_gt_overview.py` → `docs/assets/scan2d_public_gt.png` |
| セットアップ doc | ✅ `evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md` |
| 一括ベンチ | ✅ `evaluation/scripts/run_scan2d_benchmark.sh` |
| CI smoke | ✅ `evaluation/scripts/smoke_scan2d_fixture.sh` (Intel 20f × 8 methods) |
| 合成ベンチ | ✅ `rf2o_smoke` (60f) + `rf2o_corridor` (120f, slow motion) |
| 公開 log | ✅ Bonn 2D-SLAM JSON → `intel_val_73`, `fr079_val_384`, `mit_val_33` |
| リーダーボード | ✅ [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md) |
| サマリ JSON | ✅ `docs/benchmarks/scan2d/public_bundle.json` |

#### Phase 1 shortlist — **全て実装済み**

| Rank | 論文 | Paper # | 状態 |
|------|------|---------|------|
| ⭐1 | **RF2O** (ICRA 2016) | 43 | ✅ |
| 2 | **PL-ICP** (IROS 2008) | 44 | ✅ |
| 3 | **Kinematic-ICP 2D** (ICRA 2025) | 46 | ✅ |
| 4 | **CSM / Karto** (ICRA 2009) | 45 | ✅ (+ DT refresh) |
| 5 | **PSM** (ICRA 2003) | 47 | ✅ |
| 6 | **NDT-2D** (IROS 2003) | 48 | ✅ |
| 7 | **IDC** (Lu & Milios 1997) | 49 | ✅ |

#### Phase 2 shortlist — 50本目以降

| Rank | 論文 | 機構 | OSS | Feasibility | 備考 |
|------|------|------|-----|-------------|------|
| ✅ | **MbICP** (Minguez et al., ICRA 2005 / T-RO 2006) | config-space metric ICP、対称性 | 各所 | 完了 | 50本目 |
| 2 | **PL-ICP + local map** | Censi local map / CSM 併用 | — | **5/5** | ✅ robot-frame cache + harness (Intel 15.0%, fr079 14.1%) |
| 3 | **Hector SLAM** scan matcher | Gauss-Newton on occupancy grid | ROS 有 | **3.5/5** | OSS あり → campaign 対象外の可能性 |
| 4 | **Olson 2009 full Karto** | 確率 grid + branch-and-bound | OpenSLAM | **3/5** | CSM からの段階的拡張 |
| 5 | **GMapping particle filter** | Rao-Blackwellized PF | ROS 有 | **2.5/5** | SLAM 本体、odom 単体ではない |

**次の推奨**: PG-LIO (3D) は引き続き保留。2D 側は新 fixture 追加より Felzenszwalb 後の drift 安定性 watch。

---

### 00.6c-43 RF2O (43本目) — 2D range-flow odometry (2026-06-09)

- ✅ **実装**: `papers/rf2o/` — range-flow 密拘束 + Gaussian pyramid + coarse-to-fine IRLS +
  共分散 eigenvalue smoothing（MAPIRlab RF2O 準拠のループ順・PoseUpdate）。
- ✅ **Harness 初版**: `scan_dogfooding` — `scan_meta.json` + `NNNNNNNN/scan.csv` + `gt.csv`。
- ✅ **テスト**: `test_rf2o` 8 cases PASS。
- ✅ **Public benchmark** (Bonn intel val, GT-seed, 73f, 378 m):
  - Drift **14.3%** — **Intel リーダー**
  - fr079 **15.4%** / MIT **27.6%** / corridor **1.3%**
- ✅ **Smoke** (synthetic 60f): ATE 0.20 m, drift ~1.1%
- ✅ **Robot-frame local map 実装 (opt-in, default off)** (2026-06-10, P18) — see §00.6c-43 RF2O local map
- **未実装**: Cauchy IRLS 再重み付け
- **状態**: ✅ push 済; P18 local map は honest negative as default で opt-in 維持

### 00.6c-44 PL-ICP (44本目) — 2D point-to-line ICP (2026-06-09)

- ✅ **実装**: `papers/pl_icp/` — beam-order line normals + iterative PL-ICP Gauss-Newton (SE2)。
- ✅ **統合**: `scan_dogfooding --methods pl_icp`（motion prior warm-start）。
- ✅ **テスト**: `test_pl_icp` 5 cases PASS。
- ✅ **Public benchmark**:
  - Intel **16.9%** / fr079 **41.0%** / MIT **30.3%**
  - **Corridor 0.4%** — **合成 fixture リーダー** (RF2O 1.3% に次点)
- ✅ **所見**: scan-to-scan ICP は長軌道 public log でドリフト蓄積。短 corridor では最強。
- **未実装**: Censi recursive distortion、local map、kd-tree
- **状態**: ✅ push 済

### 00.6c-45 CSM (45本目) — correlative scan matching (2026-06-09)

**初版** (commit `5e2147c`):
- brute-force SE(2) + Gaussian endpoint score + motion prior warm-start
- smoke 60f: drift ~19% vs RF2O ~1.1%

**DT + pyramid refresh** (commit `3fc5be0`):
- ✅ occupancy grid + chamfer **distance transform** scoring (Olson-style)
- ✅ 3-level resolution pyramid (coarse → fine)
- ✅ relative SE(2) search around motion prior + bilinear DT lookup
- ✅ **Public benchmark (after refresh)**:
  - Intel **16.0%** (was 17.6%) / fr079 **20.6%** (was **38.9%**) / MIT **27.7%**
  - Corridor **73.3%** (was 69.6% → 初版 DT で 167% 退行後 fix)
- ✅ **所見 (honest)**: DT は **real Bonn logs で大幅改善** (特に fr079) だが
  **合成 corridor では依然 ~73%** — PL-ICP 0.4% / RF2O 1.3% に大差。

**Robot-frame local map** (2026-06-10, commit `63576de`):
- ✅ `use_local_map` + voxel merge + radius prune + adaptive search (Karto/MbICP パターン)
- ✅ harness 有効化 (`runCSM`)
- ✅ **Public benchmark (local map)**:
  - Intel **14.5%** (was 16.0%) / fr079 **14.5%** (was 20.6%) / corridor **95.8%** (was 73.3%)

**Olson coarse branch-and-bound** (2026-06-10, pushed):
- ✅ score upper-bound pyramid + coarse BnB + fine refinement (Karto から移植)
- ✅ harness デフォルト `use_branch_and_bound=true`
- ✅ **Public benchmark (BnB refresh)**:
  - Intel **15.2%** / fr079 **14.9%** / corridor **102.0%**
- ✅ **所見 (honest)**: BnB は探索スタックを Karto 級に揃えるが、Bonn drift は local map 単体から微悪化〜横ばい。corridor honest negative 継続。

**速度チューニング** (2026-06-10, P9):
- ✅ BnB node budget 128→64、leaf 3→2、fine refine 5→3、中間 pyramid refine スキップ
- ✅ score grid bilinear lookup（`exp()` 削減）
- ✅ **Public benchmark (tuned)**:
  - Intel **14.7%** (~79 FPS, was ~16 FPS) / fr079 **14.3%** (~58 FPS, was ~11 FPS) / corridor **41.3%** (was 102%)
- ✅ **所見**: 速度最適化が Bonn drift を改善し corridor も大幅改善。Felzenszwalb EDT は次候補 → **P11 完了**。

**Felzenszwalb EDT** (2026-06-10, P11):
- ✅ `common/felzenszwalb_edt` — exact Euclidean DT replaces chamfer in CSM + Karto
- ✅ **Public benchmark (Felzenszwalb EDT)**:
  - Intel **14.0%** (was 14.7%) / fr079 **13.7%** (was 14.3%) / corridor **30.5%** (was 41%)
- ✅ **所見 (honest)**: main Bonn val + corridor improve; short `fr079_train_200` regressed 12%→40% (indicative window).
- **状態**: 実装済

### 00.6c-46 Kinematic-ICP (46本目) — unicycle ICP + wheel odom (2026-06-09)

- ✅ **実装**: `papers/kinematic_icp/` — PL-ICP + wheel odom 重み付き prior + unicycle 射影。
- ✅ **統合**: `--methods kinematic_icp --wheel-odom-from-gt`
- ✅ **テスト**: `test_kinematic_icp` 7 cases PASS。
- ✅ **Public benchmark**:
  - Intel **18.4%** / fr079 **18.9%** / MIT **23.4%** (MIT リーダー) / corridor **83.8%**
- ✅ **所見**: GT wheel odom 合成でも smoke 高速運動では RF2O に劣後 (honest negative)。
  短 MIT window では best drift だが 33f は indicative のみ。
- ✅ **Robot-frame local map 実装 (opt-in, default off)** (2026-06-11, P19) — see §00.6c-46 Kinematic-ICP local map
- **未実装**: PRBonn 3D pipeline、動的重み付け、extrinsic TF
- **状態**: ✅ push 済

### 00.6c-46 Kinematic-ICP local map (2026-06-11, P19) — **trade-off、safe default 無しで opt-in**

- ✅ **Point 型 robot-frame rolling local map** — PL-ICP/MbICP と同構造 (RefPoint voxel merge + radius prune + grid-indexed correspondence + normal 回転)。`solveKinematicIncrementIndexed` + 共通 `finalizeKinematicSolve` (unicycle prior + 射影は共有)
- ✅ **テスト**: `test_kinematic_icp` 7 cases PASS (+ `LocalMapTracksTranslationWithWheelOdom`, `LocalMapMatchesScanToScanOnShortRun`)
- ✅ **全 fixture sweep** (radius 10–20 m × voxel 0.10–0.25 m × wheel weight 8–64):
  - **劇的改善**: `mit_train_120` **46.4%→12.8%** / `fr079_train_200` **11.0%→5.9%** / fr079 val 18.9%→16.0%
  - **同時に破壊**: MIT val **23.4%→30.5%** (唯一のリーダー fixture) / `fr079_train_1200` 10.7%→18.7% / corridor 83.8%→93.5%
  - wheel weight 増 (correspondence 数増の補償) は train_200 を 5.9%→81.8% に不安定化
- ✅ **判断: opt-in (default off)** — RF2O P18 (projection 型は long 窓で全敗) と違い、point 型 map は真の trade-off だが dominating config が無い。「fixture 依存が支配的」の追加証拠。scan-to-scan (paper-faithful) を default 維持
- ✅ 詳細: `papers/kinematic_icp/README.md` "Local map: opt-in only, no safe default" 節
- **状態**: ✅ 完了 (canonical JSON 変更なし — default 不変)

### 00.6c-47 PSM (47本目) — polar scan matching (2026-06-09)

- ✅ **実装**: `papers/psm/` — polar range-profile Gaussian correlation + coarse-to-fine search。
- ✅ **統合**: `scan_dogfooding --methods psm`
- ✅ **テスト**: `test_psm` 7 cases PASS。
- ✅ **Intel CI smoke** 導入 (`709d25c`): `smoke_scan2d_fixture.sh`
- ✅ **Public benchmark** (local map, 2026-06-10):
  - Intel **15.3%** / fr079 **14.3%** / MIT **28.5%** / corridor **4.4%**
- ✅ **所見 (honest)**: local map で Intel/corridor 大幅改善。fr079 val は mid-pack (~Karto 13.7%)。long train は NDT/PL-ICP 劣後。
  co-occurrence matrix 無しの簡略 polar correlation。
- **未実装**: occupancy/co-occurrence matrix
- **状態**: ✅ 実装済 (P16 local map)

---

### 00.6c-43 RF2O local map (2026-06-10/11, P18) — **honest negative as default → opt-in**

- ✅ **Robot-frame rolling local map 実装** — voxel merge + radius prune + per-point max-age expiry; accumulated points → min-range polar profile as range-flow reference (`setReferenceFromRanges`)
- ✅ **テスト**: `test_rf2o` 8 cases PASS (+ `RobotFrameLocalMapTracksTranslation`, `LocalMapImprovesLongTranslationVsScanToScan`, `LocalMapMaxAgeExpiresStalePoints`)
- ✅ **全 fixture sweep 完了** (radius 8–20 m × voxel 0.05–0.25 m × max-age 2–50f, 13 configs × 8 fixtures):
  - val 改善は再現: fr079 val **13.9%** (was 15.4%) / corridor **0.6%** (was 1.3%) / Intel・MIT ~flat
  - **long train は全 config で退行**: `fr079_train_1200` 10.6%→best 20.1% / `fr079_train_200` 30.3%→best 87.8% / `intel_train_150` 17.5%→21.8%
  - **age 減衰でも救えない** (age2 でも fr079_train_200 ~93%) → stale/動的点は根本原因ではなく、**min-range polar 投影自体** (bin 量子化 + 近距離バイアス) が range-flow 勾配を壊す
  - IDC 85% / PSM 681% (fr079_train_200) と同族パターン = **projection-style local map の family-level finding**
- ✅ **判断: default OFF (opt-in)** — scan-to-scan が既に Intel val リーダーで val 改善は微小、long 窓の損失が支配的。CT-ICP coarse-to-fine の opt-in 前例と整合。`RF2OParams::use_local_map` + 3 テストは維持
- ✅ canonical JSON を scan-to-scan default で全 refresh (`run_scan2d_benchmark.sh` + `run_scan2d_long_benchmark.sh`、`fr079_train_1200` 含む) + `public_bundle.json` 再生成
- ✅ 詳細: `papers/rf2o/README.md` "Local map: honest negative as a default" 節
- **未実装**: Cauchy IRLS 再重み付け
- **状態**: ✅ 完了

**次の推奨**: PG-LIO (3D) は引き続き保留。2D local map 化は P19 で全 9 法調査完了。残りは docs polish (`methods.json` 2D tag / validate_showcase 2D セクション)、または 3D 側の新論文。

---

### 00.6c-48 NDT-2D outlier trim (2026-06-10, P17)

- ✅ **Outlier trimming** — range-jump filter on match points (not local map); Mahalanobis trim (90%) on finest pyramid level only
- ✅ harness 有効化 (`use_outlier_trimming=true`, `max_range_jump=0.5`, `trim_fraction=0.9`)
- ✅ **Public benchmark**:
  - Intel **14.8%** (was 14.6%) / fr079 val **14.5%** (was 14.8%) / MIT **27.3%** / corridor **0.3%** (was 1.0%)
  - `fr079_train_1200` **7.5%** (was **8.8%**) / `intel_train_150` **16.5%** (was 18.0%)
- ✅ **所見 (honest)**: naive trim on map + all pyramid levels regressed long train to ~23%; match-only + finest-level fix recovers and improves. `mit_train_120` slight regression (31.6% vs 29.1%).
- **状態**: 実装済

---

### 00.6c-47 PSM local map (2026-06-10, P16)

- ✅ **Robot-frame rolling local map** — voxel merge + radius prune; reference polar profile rebuilt from accumulated points
- ✅ harness 有効化 (`runPSM`: `use_local_map=true`, radius 15 m, voxel 0.15 m)
- ✅ **Public benchmark (local map)**:
  - Intel **15.3%** (was **21.8%**) / fr079 val **14.3%** (was 13.9%) / MIT **28.5%** / corridor **4.4%** (was 11.6%)
  - `fr079_train_1200` **46.7%** (was 72.2%) / `intel_train_150` **22.5%** (was 23.7%)
- ✅ **所見 (honest)**: local map fixes Intel weakness; fr079 val ~flat; long train improves but still behind NDT/PL-ICP. `fr079_train_200` honest negative (~681%).
- **未実装**: occupancy/co-occurrence matrix
- **状態**: 実装済

---

### 00.6c-49 IDC local map (2026-06-10, P15)

- ✅ **Robot-frame rolling local map** — voxel merge + radius prune; CP spatial index; RR bearing-window (not beam index)
- ✅ harness 有効化 (`runIDC`: `use_local_map=true`, radius 15 m, voxel 0.15 m)
- ✅ **Public benchmark (local map)**:
  - Intel **15.2%** (was 15.3%) / fr079 val **14.3%** (was **27.7%**) / MIT **27.8%** / corridor **3.6%** (was **42.6%**)
  - `fr079_train_1200` **35.0%** (was 46.4%) / `intel_train_150` **25.1%** (was 41.9%)
- ✅ **所見 (honest)**: local map fixes IDC's main weakness (fr079 + corridor). Intel ~flat. Long train still behind NDT/PL-ICP. `fr079_train_200` honest negative (~85%).
- **未実装**: visibility filter、trimmed IDC
- **状態**: 実装済

---

### 00.6c-48 NDT-2D pyramid (2026-06-10, P14)

- ✅ **3-level multi-resolution pyramid** — coarse→fine Gauss-Newton with `pyramid_scale=1.5`
- ✅ harness 有効化 (`runNDT2D`: `pyramid_levels=3`, `pyramid_scale=1.5`)
- ✅ **Public benchmark (pyramid + local map)**:
  - Intel **14.6%** / fr079 val **14.8%** / MIT **28.1%** / corridor **1.0%**
  - `fr079_train_1200` **8.8%** (was **10.3%** P13) / `intel_train_150` **18.0%** (was 20.5%) / `mit_train_120` **29.1%** (best)
- ✅ **所見 (honest)**: mild pyramid (scale **1.5**, 3 levels) recovers long-window drift on
  `fr079_train_1200`. P17 outlier trim further improves to **7.5%**. Naive trim regressed long train — see honest note above.
- **未実装**: —
- **状態**: ✅ 実装済 (P17 outlier trim)

---

### 00.6c-48 NDT-2D local map (2026-06-10, P13)

- ✅ **Robot-frame rolling local map** — voxel merge + radius prune (PL-ICP/MbICP パターン)
- ✅ harness 有効化 (`runNDT2D`)
- ✅ **Public benchmark (local map)**:
  - Intel **14.9%** (was 14.8%) / fr079 **14.4%** (was **21.8%**) / MIT **27.8%** (was 29.2%) / corridor **0.8%** (was **22.3%**)
- ✅ **所見 (honest)**: local map は fr079 val + corridor で大幅改善。Intel は横ばい。`fr079_train_1200` は scan-to-scan 7.4% → **10.3%** (微悪化)。
- **未実装**: outlier trimming
- **状態**: 実装済

---

### 00.6c-48 NDT-2D (48本目) — normal distributions transform (2026-06-09)

- ✅ **実装**: `papers/ndt_2d/` — grid Gaussian cells + Gauss-Newton NDT score (no correspondences)。
- ✅ **統合**: `scan_dogfooding --methods ndt_2d`
- ✅ **テスト**: `test_ndt_2d` 5 cases PASS（pure translation で 3×3 cell + bilinear lookup 修正）。
- ✅ **Public benchmark**:
  - Intel **14.8%** (RF2O 14.3% に僅差) / fr079 **21.8%** / MIT **29.2%** / corridor **22.3%**
- ✅ **所見 (honest)**: **real logs で RF2O 近傍** (correspondence-free) だが
  **corridor 22.3%** — PL-ICP 0.4% に大差。scan-to-scan single-resolution port。
- **未実装**: multi-resolution pyramid、outlier trimming
- **状態**: ✅ push 済 (`8ea40f1`)

### 00.6c-49 IDC (49本目) — iterative dual correspondence (2026-06-09)

- ✅ **実装**: `papers/idc/` — CP (closest point) translation + RR (matching range) rotation fusion。
- ✅ **統合**: `scan_dogfooding --methods idc`
- ✅ **テスト**: `test_idc` 7 cases PASS（pure translation threshold 0.2→0.25）。
- ✅ **Public benchmark** (local map, 2026-06-10):
  - Intel **15.2%** / fr079 **14.3%** / MIT **27.8%** / corridor **3.6%**
- ✅ **所見 (honest)**: local map で fr079/corridor 大幅改善。Intel mid-pack 維持。long train は NDT/PL-ICP 劣後。
  visibility filter / trimmed IDC 無し。
- **未実装**: trimmed IDC、visibility filter
- **状態**: ✅ 実装済 (P15 local map)

### 00.6c-50 MbICP (50本目) — metric-based ICP 2D scan matching (2026-06-10)

- ✅ **実装**: `papers/mb_icp/` — Minguez/Lamiraux/Montesano の config-space metric ICP。
  対応探索と Gauss-Newton 最小化の両方で `||p||^2 + L^2` による rotation-aware metric を使用。
- ✅ **統合**: `scan_dogfooding --methods mb_icp`、`run_scan2d_benchmark.sh`、CI smoke。
- ✅ **テスト**: `test_mb_icp` 5 cases PASS。
- ✅ **Public benchmark** (canonical JSON 8-method refresh):
  - Intel **17.1%** / fr079 **16.6%** / MIT **27.3%** / corridor **0.46%**
- ✅ **所見 (honest)**: fixture winner ではないが、fr079/MIT/corridor のバランスが良い。
  PL-ICP より real logs で安定し、corridor では PL-ICP に近い。scan-to-scan のため長距離 drift は残る。
  visibility rejection / local map / exact point-to-segment metric は未実装。

#### 00.6c-50a MbICP 実装ログ / 後続向け詳細

今回追加した主な差分:

| 領域 | 変更 |
|------|------|
| 実装 | `papers/mb_icp/include/mb_icp/mb_icp.h`, `papers/mb_icp/src/mb_icp.cpp` |
| テスト | `papers/mb_icp/test/test_mb_icp.cpp` — 初期化、並進、回転、並進+回転、`pose2D` |
| ビルド | `papers/mb_icp/CMakeLists.txt`, root `CMakeLists.txt`, `evaluation/CMakeLists.txt` |
| Harness | `evaluation/src/scan_dogfooding.cpp` — loader, `runMbICP`, selector `mb_icp`, JSON note |
| Batch/CI | `evaluation/scripts/run_scan2d_benchmark.sh`, `evaluation/scripts/smoke_scan2d_fixture.sh` |
| Docs/catalog | `papers/mb_icp/README.md`, `README.md`, `docs/benchmarks/scan2d/README.md`, `docs/methods.json`, `public_bundle.json`, `SETUP_2D_SCAN_BENCHMARK.md` |
| Artifacts | `docs/benchmarks/scan2d/{intel_val_73,fr079_val_384,mit_val_33,rf2o_corridor}.json` を 8-method canonical として再生成 |

実装メモ:

- reference scan は beam 隣接点から segment list を作る。range gap が `max_neighbor_gap` を超える segment は切る。
- 対応探索は transformed current point を reference segment に射影し、`metricDistanceSquared()` で最小の対応を選ぶ。
- metric は `delta^T Q delta` 相当で、`Q = I - aa^T/(||ref||^2 + L^2)`、`a=(ref_y,-ref_x)`。
  これにより遠距離で回転に吸収できる tangential error を弱める。
- solve は同じ metric matrix を Gauss-Newton normal equation に入れる。対応集合は `trim_fraction=0.9` で上位 90% を使用。
- `scan_dogfooding` の MbICP 既定は `max_metric_distance=1.5`, `metric_radius=1.0`, `max_neighbor_gap=2.0`, `trim_fraction=0.9`。

パラメータ探索メモ:

| `metric_radius` | Intel | fr079 | MIT | Corridor | 所見 |
|----------------:|------:|------:|----:|---------:|------|
| 1.0 | 17.07 | **16.55** | 27.33 | 0.46 | 採用。4 fixture 平均 drift が最良 |
| 1.5 | 16.75 | 20.92 | 27.10 | 0.45 | fr079 が悪化 |
| 2.0 | **14.98** | 20.32 | **27.02** | 0.43 | Intel/MIT は良いが fr079 が悪化 |
| 3.0 | 15.83 | 20.18 | 27.83 | **0.41** | corridor は最良だが real logs で弱い |

採用値は single-fixture 最適ではなく、公開4 fixture の平均と honest leaderboard を優先。
次に retune する場合は `public_bundle.json` と `docs/benchmarks/scan2d/README.md` の数値も同時に更新する。

未実装 / 論文との差分:

- exact な MbICP point-to-segment metric derivation ではなく、segment 射影点に metric を評価する近似。
- visibility rejection、range discontinuity に基づく occlusion filter、local submap は未実装。
- scan-to-scan のみ。長い Bonn logs で drift が残るのは実装バグではなく current harness の設計限界。

検証済みコマンド:

```bash
rtk cmake -B build -DCMAKE_BUILD_TYPE=Release
rtk cmake --build build --target test_mb_icp scan_dogfooding -j$(nproc)
rtk ctest --test-dir build -R test_mb_icp --output-on-failure
rtk bash evaluation/scripts/run_scan2d_benchmark.sh
rtk jq empty docs/methods.json docs/benchmarks/scan2d/public_bundle.json \
  docs/benchmarks/scan2d/intel_val_73.json \
  docs/benchmarks/scan2d/fr079_val_384.json \
  docs/benchmarks/scan2d/mit_val_33.json \
  docs/benchmarks/scan2d/rf2o_corridor.json
```

最終 commit 前にもう一度回す推奨:

```bash
rtk ctest --test-dir build -R 'test_mb_icp|test_rf2o|test_pl_icp|test_csm|test_kinematic_icp|test_psm|test_ndt_2d|test_idc' --output-on-failure
rtk bash evaluation/scripts/smoke_scan2d_fixture.sh
rtk python3 evaluation/scripts/validate_showcase.py --root .
```

---

### 00.57 2D scan 評価インフラ (2026-06-09)

- ✅ **`scan_dogfooding` drift 列**: KITTI-style RPE (`Drift [%]`)、`--summary-json` 出力。
- ✅ **`prepare_2d_scan_inputs.py`**: ROS1 `sensor_msgs/LaserScan` → scan tree + GT 補間。
- ✅ **`prepare_bonn_2dslam_inputs.py`**: Bonn JSON → committed fixtures。
- ✅ **`SETUP_2D_SCAN_BENCHMARK.md`**: 合成 / bag / Bonn 手順 + 8 法 run 例。
- ✅ **`run_scan2d_benchmark.sh`**: 4 fixtures × 8 methods 一括 refresh。
- ✅ **`smoke_scan2d_fixture.sh`**: CI — Intel 20f × 8 methods (`.github/workflows/ci.yml`)。
- ✅ **`plot_scan2d_gt_overview.py`**: public GT 軌跡 PNG。
- ✅ **Bonn fixtures** (committed, GT = dataset odometry proxy):

| Fixture | Source | Frames | Beams | Traj [m] | Size |
|---------|--------|--------|-------|----------|------|
| `intel_val_73` | Bonn `intel/val.json` | 73 | 180 | ~378 | ~600 KB |
| `fr079_val_384` | Bonn `fr079/val.json` | 384 | 360 | ~373 | ~3.1 MB |
| `mit_val_33` | Bonn `mit/val.json` | 33 | 360 | ~267 | ~280 KB |

- ✅ **Synthetic fixtures**:

| Fixture | Frames | Traj [m] | 用途 |
|---------|--------|----------|------|
| `rf2o_smoke` | 60 | ~18 | 高速混合運動 smoke |
| `rf2o_corridor` | 120 | ~9.5 | 低速 mixed motion (0.08 m/f) |

> **注意**: 合成 corridor は bounded box raycast。**純 parallel-wall corridor は 2D odom 退化**
> (forward structure 不足)。real hallway は ROS bag export を推奨。

- ✅ **リーダーボード hub**: [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md)

### 00.58 2D scan リーダーボード総括 (2026-06-09)

GT-seed on frame 0。**Drift [%]** — lower is better。

#### 8 法 × 4 fixture (committed numbers)

| # | Method | Intel val | fr079 val | MIT val | Synth corridor | Best on |
|---|--------|----------:|----------:|--------:|---------------:|---------|
| | | _73f / 378m_ | _384f / 373m_ | _33f / 267m_ | _120f / 9.5m_ | |
| 43 | **RF2O** | **14.3** | 15.4 | 27.6 | 1.3 | Intel |
| 48 | **NDT-2D** | 14.8 | 21.8 | 29.2 | 22.3 | — |
| 49 | **IDC** | 15.3 | 27.7 | 29.5 | 42.6 | — |
| 45 | **CSM** | 16.0 | 20.6 | 29.2 | 73.3 | — |
| 44 | **PL-ICP** | 15.0 | **14.1** | 27.2 | **0.01** | Corridor + fr079 competitive |
| 50 | **MbICP** | 17.1 | 16.6 | 27.3 | 0.5 | — |
| 46 | **Kinematic-ICP** | 18.4 | 18.9 | **23.4** | 83.8 | MIT (indicative) |
| 47 | **PSM** | 21.8 | **13.9** | 27.9 | 11.6 | fr079 |

#### 恒例知見 (2D campaign)

- **単一 winner なし** — fixture 依存が強い (PSM@fr079 vs RF2O@Intel vs PL-ICP@corridor)。
- **scan-to-scan 限界** — 長 public log では range-flow (RF2O) か polar (PSM) が相対的に強いが
  どちらも GT proxy 対 drift 15–30% 帯。local map 無しでは Karto 論文数値には未達。
- **合成 vs real** — corridor で PL-ICP 0.4% でも fr079 で 41% — **合成勝利は real 一般化しない**。
- **CSM DT** — fr079 38.9%→20.6% は **positive engineering** だが corridor ~73% は honest negative 継続。
- **Kinematic-ICP** — `--wheel-odom-from-gt` 必須。real wheel odom 無し環境では skip/劣化。
- **MIT val 33f** — 全 drift indicative。ranking 参考程度。

#### Artifact index

| Fixture | Canonical JSON | 備考 |
|---------|----------------|------|
| Intel | `docs/benchmarks/scan2d/intel_val_73.json` | 8-method canonical |
| fr079 | `docs/benchmarks/scan2d/fr079_val_384.json` | 8-method canonical |
| MIT | `docs/benchmarks/scan2d/mit_val_33.json` | short window; indicative |
| Corridor | `docs/benchmarks/scan2d/rf2o_corridor.json` | 8-method canonical |
| Summary | `docs/benchmarks/scan2d/public_bundle.json` | schema v2, 8 法 |
| Partial | `*_idc.json`, `*_ndt2d.json`, `*_csm_dt.json` | method-specific reruns |

再現:

```bash
cmake --build build --target scan_dogfooding
bash evaluation/scripts/run_scan2d_benchmark.sh
python3 evaluation/scripts/plot_scan2d_gt_overview.py
bash evaluation/scripts/smoke_scan2d_fixture.sh   # CI 同等
```

### 00.59 2D ドキュメント整備 (2026-06-09, 未 commit)

ユーザ指示「markdown を整理」に基づき、2D scan 関連 docs を **単一ハブ** に集約。

| ファイル | 変更 |
|---------|------|
| **新規** `docs/benchmarks/scan2d/README.md` | 8×4 リーダーボード、artifact index、再現手順、所見 |
| `README.md` | Eight、MbICP 行、詳細はハブへ |
| `public_bundle.json` | schema v2、8 法 + corridor、drift 同期 |
| `SETUP_2D_SCAN_BENCHMARK.md` | 8 法 run 例、CI smoke、ハブリンク |
| `papers/{rf2o,pl_icp,csm,psm,ndt_2d,idc,kinematic_icp,mb_icp}/README.md` | ベンチ表 or ハブリンク統一 |
| `PLAN.md` | 本更新 |

**ドキュメント階層**:

```
README.md (1-screen 概要)
  └── docs/benchmarks/scan2d/README.md (canonical hub)
        ├── public_bundle.json
        ├── intel_val_73.json / fr079_val_384.json / ...
        └── papers/*/README.md (per-method honest notes)
```

**未完了 (docs)**: なし — 2026-06-11 に `methods.json` 2D tag 強化 (local-map/distance-transform tag + opt-in 注記) と
`validate_showcase.py` scan2d セクション (public_bundle ↔ fixture JSON の drift 整合検証) を実施済。

### 00.60 次セッション優先タスク (2D)

| Priority | タスク | 状態 (2026-06-10) |
|----------|--------|-------------------|
| **P0** | MbICP + 8-method refresh 差分を最終検証して commit | ✅ 検証済 (8/8 tests, smoke, validate_showcase, benchmark refresh) |
| **P1** | `git push` — IDC + CSM-DT + markdown + MbICP refresh | ユーザ明示指示待ち |
| **P2** | PL-ICP/MbICP local map 拡張 | ✅ MbICP robot-frame cache (Intel **14.5%**, fr079 **15.4%**; ~2.3min) + PL-ICP robot-frame cache (Intel **15.0%**, fr079 **14.1%**; ~26s) + stamp-indexed search speedup |
| **P3** | Karto-style map matcher / spatial index for local map | ✅ `karto_matcher` + Olson coarse BnB; fr079 **14.8%** drift (Intel 15.1%); corridor honest negative |
| **P4** | 長め MIT/Bonn window 追加 | ✅ `mit_train_120` + `intel_train_150` fixtures + `run_scan2d_long_benchmark.sh` |
| **P5** | CSM robot-frame local map | ✅ Intel **14.5%**, fr079 **14.5%**; corridor **95.8%** (honest negative vs scan-to-scan 73%) |
| **P6** | CI long-fixture smoke | ✅ `smoke_scan2d_long_fixture.sh` — MIT/Intel train 20f × 9 methods (`.github/workflows/ci.yml`) |
| **P7** | CSM Olson coarse branch-and-bound | ✅ Karto から移植; fr079 **14.9%**, Intel **15.2%**; corridor ~102% (honest negative) |
| **P8** | fr079_train long window | ✅ `fr079_train_200` fixture + benchmark + CI long smoke |
| **P9** | CSM 速度チューニング | ✅ 64-node BnB + finest-only refine + score lookup; Intel **14.7%** (~79 FPS), fr079 **14.3%** (~58 FPS), corridor **41%** (was 102%) |
| **P10** | Karto-Matcher への CSM 同チューニング移植 | ✅ 64-node BnB + finest-only refine + score lookup; Intel **14.7%** (~64 FPS), fr079 **14.3%** (~47 FPS), corridor **41%** (was 102%) |
| **P11** | Felzenszwalb EDT (CSM + Karto) | ✅ `common/felzenszwalb_edt`; Intel **14.0%**, fr079 **13.7%**, corridor **30.5%** (was 41%); `fr079_train_200` regressed (indicative) |
| **P12** | fr079_train 更長 window | ✅ `fr079_train_1200` (~150 m); CSM/Karto **17.6%** (vs 40% on 200f); CI long smoke uses 1200 fixture |
| **P13** | NDT-2D robot-frame local map | ✅ voxel merge + radius prune; fr079 **14.4%** (was 21.8%), corridor **0.8%** (was 22.3%); Intel **14.9%** |
| **P14** | NDT-2D multi-resolution pyramid | ✅ 3-level coarse→fine (scale **1.5**); `fr079_train_1200` **8.8%** (was 10.3%); `mit_train_120` **29.1%** (best) |
| **P15** | IDC robot-frame local map | ✅ voxel merge + radius prune + bearing-window RR; fr079 val **14.3%** (was 27.7%); corridor **3.6%** (was 42.6%); Intel **15.2%** |
| **P16** | PSM robot-frame local map | ✅ point cache → polar profile rebuild; Intel **15.3%** (was 21.8%); fr079 **14.3%**; corridor **4.4%** (was 11.6%) |
| **P17** | NDT-2D outlier trimming | ✅ match-only range jump + finest-level Mahalanobis trim; `fr079_train_1200` **7.5%** (was 8.8%); corridor **0.3%**; Intel ~flat |
| **P18** | RF2O robot-frame local map | ✅ **opt-in (default off)** — val は fr079 13.9%/corridor 0.6% と改善するが long train が全 config 退行 (`fr079_train_1200` 10.6→20.1%+, `fr079_train_200` 30→88%+); min-range polar 投影が原因の family-level honest negative |
| **P19** | Kinematic-ICP point 型 local map | ✅ **opt-in (default off)** — `mit_train_120` 46→13% / `fr079_train_200` 11→6% と効く窓がある一方、MIT val 23→31% (唯一のリーダー fixture) と `fr079_train_1200` 11→19% を破壊、safe shared config 無し |
| — | PG-LIO (3D) 改善 | 保留 (honest negative) |
| — | KITTI Odom full rerun | データ入手 |

**Do NOT** (明示指示なし):
- 50本目を未着手扱いに戻す
- PG-LIO を 2D より優先
- force push / git config 変更
- canonical JSON を partial artifact (`*_idc.json`, `*_csm_dt.json`) の古い数字で上書き

---

**§00.6c 以前の旧 2D 節 (00.53 RF2O 等) は上記 §00.6c-43〜50 に統合済み。以下は 3D 37 本目 VLOM の §00.53。**

This section is the authoritative current handoff for **June 2026 2D LiDAR campaign**
(§00.6c–§00.60). Older sections below still matter for 3D benchmark history, recipe
provenance, and paper-grade claims, but they describe the May 2026 / early-June 3D research
state. The active direction is:

1. **2D scan odometry** — 8-method leaderboard is current; next value is local mapping / Karto-style matching.
2. **OSS polish** — showcase/demo/CI remain important (§0, validate_showcase contract).
3. **3D on hold** — PG-LIO honest negative; KITTI full reruns blocked by data.

The user's current theme has been:

- "shibaraku 2D LiDAR de yatte iku" (2D campaign pivot)
- "omosiroi kihatu wo siyou. star wo huyasitai." (interesting mechanisms + stars)
- "ittan plan md wo naganaga kousin" (long PLAN.md refresh — **this update**)

Interpretation: this repo should look and feel like a strong OSS project, not
only a private research scratchpad. First-clone users should immediately see a
usable method explorer, a runnable demo, a generated report, and CI-backed proof
that many methods can be validated together.

### 0.0 Update — Showcase merged + 2 new methods (2026-06-02, later session)

This subsection supersedes 0.1–0.7 where they conflict; those describe the
pre-merge dirty-worktree state.

- **Showcase merged to `main`.** PR #2 (`wip/profile-expansion-refresh` → `main`)
  was merged (merge commit `4ddb7c3`). The merge brought in `main`'s 14 commits
  (HDL-400 B benchmark docs, `run_local_evaluation_suite.sh` / `eval_local_suite`
  CMake target, Ceres/OpenCV build tweaks, KISS-ICP empty-frame + common golden
  tests). Conflicts were resolved keeping the branch's newer research/showcase
  state for data/docs and merging both sides for source (`run_experiment_matrix.py`
  kept the lower-case `is_metric_valid` taxonomy + new `--merge-existing-index`;
  the duplicate `problem_run_from_aggregate` was disambiguated into
  `problem_run_from_aggregate` (manifest form) and `problem_run_from_aggregate_file`
  (aggregate-only form)).
- **Method count 33 → 35.** Two new from-scratch LiDAR ports were added:
  - `papers/genz_icp/` — GenZ-ICP style degeneracy-robust odometry (adaptive
    point-to-plane / point-to-point hybrid, normal+planarity estimation on the
    KISS-style voxel map). Selector `genz_icp`, profiles `--genz-fast-profile` /
    `--genz-dense-profile` / `--genz-planarity-threshold`. `test_genz_icp`.
  - `papers/rko_lio/` — RKO-LIO style scan-to-map odometry with an IMU gyro
    rotation prior (via `imu_preintegration`), constant-velocity fallback when no
    `imu.csv`. Selector `rko_lio`. `test_rko_lio`. Always runs (does not skip);
    note text records whether the IMU prior was used.
  - Both have `docs/methods.json` entries (catalog now 41 entries = 41 `papers/*`).
- **Drift column.** `pcd_dogfooding` results table now prints `Drift[m/100m]`
  (the existing KITTI-style `rpe_trans_pct`, previously only in summary JSON;
  shows `-` for trajectories shorter than the RPE segment).
- **GenZ-ICP KITTI Odom 108-window finding** (memory
  `genz_icp_vs_kiss_kitti_windows.md`): GenZ-ICP beats KISS-ICP on drift in 3/5
  windows (00/02/07) but is not universal (loses on 05), is slower, and does not
  rescue the seq 08 no-seed divergence (~33 m/100m, like KISS). NDT (GT-seeded)
  remains the overall window winner. RKO-LIO full benchmark intentionally **not
  yet run** (user deferred).
- **Commits on branch after the merge:** `dd69400` (GenZ-ICP), `e1ecc6a` (drift
  column), `3645d67` (RKO-LIO). Branch pushed to origin.
- **Verification:** `cmake --build build --target pcd_dogfooding multimodal_dogfooding`
  clean; `ctest -R "genz_icp|rko_lio"` pass; `python3 -m unittest discover -s tests`
  → 49 pass; one-command demo (broad) + `validate_showcase.py --require-demo` valid.
  The demo all-OK profiles were intentionally **not** changed (genz_icp/rko_lio
  are available via `--methods` but not in the default broad/full sets).
- **NCLT dataset ingested (no-form, S3 direct).** `evaluation/scripts/prepare_nclt_inputs.py`
  + `SETUP_NCLT_BENCHMARK.md` convert NCLT (UMich, Velodyne+MS25 IMU+6-DoF GT) into the
  standard `dogfooding_results/nclt_*` (cloud.pcd + frame_timestamps.csv + imu.csv) and
  `experiments/reference_data/nclt_*_gt.csv`. Smallest session `2013-01-10` (velodyne 2.9 GB)
  ingested at 120- and 600-frame windows. Raw download lives outside the repo in
  `nclt_raw/`; the PCD trees are gitignored, only GT CSVs are committed.
  - Active manifests `rko_lio_nclt_2013_01_10` (gyro-bias-gain sweep) and
    `genz_icp_nclt_2013_01_10`, run through `run_experiment_matrix.py --merge-existing-index`
    and merged into `index.json` + generated docs.
  - **Finding (memory `nclt_dataset_ingested.md`):** on NCLT the **raw IMU rotation prior
    (gyro_bias_gain 0) is best** (RKO-LIO ATE 0.141 @120, 0.385 @600 — beating FAST-LIO2's
    0.469 @600, 2nd only to GT-seeded NDT). Optimal bias gain is **dataset-dependent** —
    HDL-400 wants 0.3 (high IMU bias), NCLT wants 0 (low-bias MS25); no universal default.
    KISS-ICP (no IMU) drifts to 7.2 m @600. NCLT is where the inertial prior clearly pays off.
  - **Bug fixed:** `variant_result_from_dict` in `run_experiment_matrix.py` (main-side helper
    pulled in by the merge) omitted the branch's required `rpe_trans_pct` / `rpe_rot_deg_per_m`
    fields, breaking the `--merge-existing-index` path. Now fixed.

### 0.0b Update — NCLT cross-method + full-trajectory benchmark + LiTAMIN2 improvement (2026-06-02, later session)

- **Full NCLT tree generated.** `prepare_nclt_inputs.py --max-frames -1` produced
  `dogfooding_results/nclt_2013_01_10_full` (**5105 GT-matched frames**) + committed
  `experiments/reference_data/nclt_2013_01_10_full_gt.csv`. Passing `max_frames` to
  `pcd_dogfooding` lets any prefix window be evaluated off the full tree (2000-frame
  windows are the practical tuning size — they reproduce the long-trajectory drift while
  staying fast).
- **Mechanism clarified.** In this dogfooding tool, GT-seeded methods (NDT, LiTAMIN2,
  GICP-family) use a **per-frame** seed: `T_init_guess = applySeedPerturbation(gt[i], …)`,
  then accept the refinement only if it stays within the gate (NDT 1.5 m/0.2 rad,
  others 2.0 m/0.25 rad), else roll back to `gt[i]` (weak-update fallback). So their ATE is
  the per-frame *registration residual* from GT, not accumulated odometry drift. RKO-LIO /
  FAST-LIO2 / KISS-ICP only seed the **first** pose → they are odometry and drift over the
  full trajectory (a different category from the per-frame-seeded registration methods).
- **600-frame horizontal comparison (11 methods).** Winner NDT 0.198, then LiTAMIN2 0.380 ≈
  RKO-LIO(gain0) 0.385, FAST-LIO2 0.469, small_gicp 1.024; no-seed odometry blows up
  (KISS 7.25, CT-ICP 13.6, GenZ 26.0, Point-LIO 122).
- **Full 5105-frame benchmark — scale-dependent reversal (key finding).**

  | method (full 5105) | seed | ATE [m] | drift [m/100m] |
  |---|---|---|---|
  | NDT | GT | **0.122** | 0.460 |
  | **LiTAMIN2 voxel0.5/iter12** | GT | **0.582** | 1.315 |
  | small_gicp voxel0.5 | GT | 1.040 | 2.786 |
  | small_gicp default | GT | 1.086 | 2.288 |
  | LiTAMIN2 default (voxel2.0) | GT | 1.149 | 2.492 |
  | RKO-LIO (gain0) | init | 7.334 | 2.077 |
  | RKO-LIO (gain0.3) | init | 24.43 | 4.568 |
  | FAST-LIO2 | – | 16.126 | 3.330 |
  | KISS-ICP | – | 60.629 | 15.134 |

  NDT actually *improves* at full scale (0.198 → 0.122) thanks to its tighter gate + GT
  reanchoring. The 600-frame near-ties (LiTAMIN2/RKO-LIO ≈ NDT) do **not** hold at full.
- **Improvement delivered: LiTAMIN2 voxel 2.0 → 0.5 (+ iter 12).** NCLT's HDL-32E scans are
  sparser than KITTI's HDL-64E, so the default voxel 2.0 is too coarse. Transferring the
  KITTI cluster-T1 recipe (voxel 0.5 + iter 12) cuts **full ATE 1.149 → 0.582 (−49 %)** and
  drift 2.492 → 1.315 (−47 %), moving LiTAMIN2 to a clear #2 behind NDT. voxel 1.0 does *not*
  help (1.044 @2000) — fineness is the lever. Captured as the reproducible manifest
  `experiments/litamin2_nclt_2013_01_10_matrix.json` (voxel sweep, merged into `index.json`).
- **Negative results recorded honestly:** small_gicp does *not* benefit from the voxel-0.5
  lever (full 1.086 → 1.040 ATE but drift worsens 2.288 → 2.786); RKO-LIO gyro-bias gain 0.3
  is *worse* than gain 0 at full (24.4 vs 7.3) — bias correction over-integrates on the long
  trajectory. Optimal gain is both dataset- and scale-dependent; no universal default.

### 0.0c Update — cross-session T1 generality + small_gicp seed-gate lever (2026-06-02, later session)

- **Second NCLT session ingested for cross-session validation.** Downloaded 2012-12-01
  (velodyne 13.4 GB, fetched via 8-way HTTP range requests at ~12 MB/s after the single
  stream stalled at ~1 MB/s), ingested a **5000-frame / 937 m** prefix as
  `dogfooding_results/nclt_2012_12_01_5000` + committed `…_5000_gt.csv`.
- **LiTAMIN2 T1 (voxel0.5/iter12) generalizes across NCLT sessions.** On 2012-12-01 it cuts
  ATE **0.869 → 0.519 (−40 %)** and drift 1.844 → 1.357 (−26 %), mirroring the 2013-01-10
  full-5105 −49 %/−47 %. NDT is the winner again (0.118, vs 2013-01-10's 0.122). The
  fine-voxel recipe is a **session-independent NCLT improvement**, not a per-session artifact.
  Manifest `experiments/litamin2_nclt_2012_12_01_matrix.json` (merged into `index.json`).

  | method (2012-12-01, 5000 f) | ATE [m] | drift [m/100m] |
  |---|---|---|
  | NDT | **0.118** | 0.374 |
  | LiTAMIN2 T1 (voxel0.5/iter12) | **0.519** | 1.357 |
  | LiTAMIN2 default (voxel2.0) | 0.869 | 1.844 |
  | small_gicp default | 1.049 | 2.616 |
  | small_gicp cd1.5 | 1.009 | 2.883 |

- **small_gicp #3 improvement: the seed gate is the lever, not voxel/correspondence.** The
  CLI levers that fix LiTAMIN2 wash out for small_gicp at full scale: correspondence distance
  1.5 gives a real −23 % at 2000 frames (1.181 → 0.908) but evaporates by full-5105
  (1.086 → 1.089); voxel0.5 is likewise flat. The actual bottleneck is the **weak-update seed
  gate**, which defaulted to 2.0 m / 0.25 rad (looser than NDT's 1.5 m / 0.2 rad) and was not
  CLI-exposed. Added `--small-gicp-max-seed-translation-delta` / `-rotation-delta-rad`
  (mirrors the existing CT-ICP flags) + a seed-fallback counter in `runSmallGICP`. Tightening
  the gate monotonically cuts full-5105 ATE:

  | gate (full 5105) | ATE [m] | drift | GT-fallback % (2000 f) |
  |---|---|---|---|
  | 2.0 m / 0.25 rad (default) | 1.086 | 2.288 | 2.5 % |
  | 1.5 m / 0.2 rad (NDT-equal) | 0.882 | 2.357 | 4.2 % |
  | **1.0 m / 0.15 rad (balanced)** | **0.675** | 2.036 | 9.3 % |
  | **0.5 m / 0.1 rad (aggressive)** | **0.348** | 1.313 | 20.2 % |

  At 1.0 m / 0.15 rad ATE drops −38 % with only 9.3 % GT fallback (the recommended balanced
  config); at 0.5 m / 0.1 rad it drops −68 % and overtakes LiTAMIN2 T1 (0.582) for NCLT #2.
  **Honest caveat:** a tighter gate also raises the GT-fallback rate (2.5 % → 20.2 %), so part
  of the gain is more aggressive reversion to the *exact* GT seed on the worst registrations,
  not purely better refinement — the same weak-update mechanism NDT benefits from, now exposed
  for small_gicp. The fair fixed-gate comparison (small_gicp 0.882 vs NDT 0.122 at the
  identical 1.5 m gate) shows NDT's edge is genuine refinement quality, not just the gate.
  Manifest `experiments/small_gicp_nclt_2013_01_10_matrix.json`.

### 0.1 Current Git State

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD at update time | `947912d` |
| Worktree | Dirty by design; current showcase/demo changes are not committed yet |
| Local instruction | Prefix shell commands with `rtk` when operating as Codex |
| Main current artifact | One-command demo + GitHub Pages showcase + validators |
| Current generated demo output | Ignored under `experiments/results/runs/demo_localization_zoo/` |

Current dirty/untracked files at the time of this PLAN update:

- Modified:
  - `.github/workflows/ci.yml`
  - `README.md`
  - `docs/assets/site.css`
  - `docs/index.html`
  - `tests/test_experiment_scripts.py`
- Untracked:
  - `docs/assets/explorer_preview.png`
  - `docs/methods.json`
  - `evaluation/scripts/demo_localization_zoo.sh`
  - `evaluation/scripts/generate_demo_report.py`
  - `evaluation/scripts/validate_demo_artifacts.py`
  - `evaluation/scripts/validate_showcase.py`

The generated demo run artifacts are intentionally not tracked. They are
produced by the demo script and uploaded by CI as an artifact:

- `experiments/results/runs/demo_localization_zoo/report.html`
- `experiments/results/runs/demo_localization_zoo/manifest.json`
- `experiments/results/runs/demo_localization_zoo/synthetic_benchmark.log`
- `experiments/results/runs/demo_localization_zoo/lidar_fixture_summary.json`
- `experiments/results/runs/demo_localization_zoo/multimodal_fixture_summary.json`
- trajectory text outputs under `benchmark_results/` and `dogfooding_results/`

### 0.2 What Was Just Built

The repo now has a public-facing showcase layer in addition to the older
research benchmark layer.

#### GitHub Pages Explorer

[`docs/index.html`](docs/index.html) has been turned into a richer interactive
front page:

- Loads latest benchmark data from `docs/benchmarks/latest/results.json`.
- Loads method metadata from the new `docs/methods.json` instead of hardcoded
  JavaScript arrays.
- Renders a method explorer covering every `papers/*` directory.
- Shows starter tracks for different user intents.
- Shows a benchmark scatter/leaderboard for the committed latest snapshot.
- Includes OpenGraph/Twitter metadata for link previews.
- Points users toward the one-command local demo.

[`docs/assets/site.css`](docs/assets/site.css) was redesigned for the explorer:

- More dashboard-like and scan-friendly.
- Responsive layout for desktop/mobile.
- No giant marketing-only hero; the page is meant to be useful immediately.

[`docs/assets/explorer_preview.png`](docs/assets/explorer_preview.png) is a new
preview image referenced from the README and validated as a PNG by the showcase
validator.

#### Method Catalog

[`docs/methods.json`](docs/methods.json) is new:

- `schema_version: 1`
- 39 method entries, one for each `papers/*` directory.
- Required fields per method:
  - `name`
  - `family`
  - `scope`
  - `signals`
  - `href`
  - `summary`
  - `tags`
- 4 starter tracks:
  - quick start / first run
  - accuracy oriented
  - fusion / multimodal
  - degeneracy / robustness

The method catalog is now test-covered. If a new paper directory is added
without a catalog entry, the Python tests fail.

#### One-command Demo

[`evaluation/scripts/demo_localization_zoo.sh`](evaluation/scripts/demo_localization_zoo.sh)
is new and is now the first-run path for users. It:

- Builds the C++ targets unless `--skip-build` is provided.
- Runs `synthetic_benchmark`.
- Runs the committed three-frame MCD fixture through selected LiDAR methods.
- Runs the same fixture through selected multimodal methods.
- Writes logs, summary JSON, trajectories, `report.html`, and `manifest.json`.
- Calls the demo artifact validator at the end, so the command proves its own
  output contract.

The demo now has method profiles:

| Profile | LiDAR methods | Multimodal methods | Intended use |
|---------|---------------|--------------------|--------------|
| `quick` | 4 | 2 | Fast old-style local loop |
| `broad` | 24 | 6 | Default; best first-clone OSS proof |
| `full` | 25 | 6 | Adds LiDAR FAST-LIO2 fixture validation |

Current LiDAR `quick`:

```text
litamin2,gicp,ndt,kiss_icp
```

Current LiDAR `broad`:

```text
litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,xicp,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio
```

Current LiDAR `full`:

```text
litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,xicp,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio,fast_lio2
```

Note: `full` is not literally every LiDAR selector in `pcd_dogfooding`. It is
the largest all-OK set for the committed MCD fixture without requiring IMU-only
methods that skip when `imu.csv` is absent. `ct_lio` and `clins` currently skip
on this fixture because there is no synchronized IMU CSV. Do not include them
in the default all-OK profile unless the fixture gains IMU data or the validator
learns about expected skips.

Current multimodal `quick`:

```text
okvis,fast_livo2
```

Current multimodal `broad` / `full`:

```text
vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live
```

Useful invocations:

```bash
bash evaluation/scripts/demo_localization_zoo.sh
bash evaluation/scripts/demo_localization_zoo.sh --skip-build
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile quick
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile broad
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile full
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --methods litamin2,gicp --multimodal-methods okvis
```

As Codex in this workspace, remember to run those through `rtk`, e.g.
`rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build`.

#### Demo Report and Manifest

[`evaluation/scripts/generate_demo_report.py`](evaluation/scripts/generate_demo_report.py)
is new:

- Standard-library-only report generator.
- Parses synthetic benchmark logs.
- Reads LiDAR and multimodal summary JSON.
- Reads trajectory text files.
- Produces self-contained `report.html` with inline CSS/SVG.
- Produces `manifest.json` with:
  - schema version
  - command
  - profile
  - requested LiDAR method list
  - requested multimodal method list
  - actual method counts and statuses
  - artifact paths

The manifest is not just a pretty output. It is now the contract between the
demo script, validator, CI artifact upload, README claims, and showcase tests.

#### Validators

[`evaluation/scripts/validate_demo_artifacts.py`](evaluation/scripts/validate_demo_artifacts.py)
is new:

- Validates `manifest.json`.
- Validates `report.html` required snippets.
- Validates synthetic/LiDAR/multimodal summary artifacts.
- Requires every selected method to have `status == ok`.
- Validates exact method-set match by normalized method name.
- Can read expected method lists from CLI or from the manifest.
- Supports `--skip-multimodal`.

Important behavior: if someone silently shrinks the default profile from 24
LiDAR methods to 4, the validator catches it when expected methods are provided
or when manifest-vs-summary diverges.

[`evaluation/scripts/validate_showcase.py`](evaluation/scripts/validate_showcase.py)
is new:

- Validates README links and required snippets.
- Validates the preview PNG signature.
- Validates `docs/index.html` snippets and repo link.
- Validates `docs/methods.json` coverage against `papers/*`.
- Validates starter-track references.
- Validates the latest benchmark snapshot and trajectory plot.
- Optionally validates generated demo artifacts with `--require-demo`.

#### CI Integration

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) now includes:

- One-command demo report generation after existing smoke checks.
- Showcase validation with `--require-demo`.
- Upload of `experiments/results/runs/demo_localization_zoo/` as the
  `localization-zoo-demo-report` artifact.

The CI path intentionally demonstrates the same thing a new user sees locally:
build, run a real committed fixture, generate a report, validate it, and keep
the report downloadable.

### 0.3 Validation Already Run

The following commands passed during the 2026-06-02 update:

```bash
rtk bash -n evaluation/scripts/demo_localization_zoo.sh
rtk python3 -m py_compile evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py
rtk python3 evaluation/scripts/validate_showcase.py --skip-demo
rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build
rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile full
rtk python3 evaluation/scripts/validate_showcase.py --require-demo --demo-dir experiments/results/runs/demo_localization_zoo
rtk python3 -m unittest discover -s tests -p 'test_*.py' -v
rtk proxy git diff --check -- README.md docs/index.html evaluation/scripts/demo_localization_zoo.sh evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py tests/test_experiment_scripts.py
```

Observed validation result:

- `broad` demo: LiDAR 24 / multimodal 6 all OK.
- `full` demo: LiDAR 25 / multimodal 6 all OK.
- Showcase validation: OK.
- Python tests: 49 tests OK.
- Whitespace check: OK.

Because `full` was the last demo run before this PLAN update, the ignored local
`experiments/results/runs/demo_localization_zoo/manifest.json` currently records
`profile: full`, `lidar_fixture.method_count: 25`, and
`multimodal_fixture.method_count: 6`.

### 0.4 Test Coverage Added

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) now
includes three new showcase/demo-focused groups:

- `MethodCatalogTests`
  - `docs/methods.json` schema is version 1.
  - Catalog has unique method names and hrefs.
  - Catalog hrefs exactly cover all `papers/*` directories.
  - Required fields are present.
  - Each referenced method README exists.
  - Starter tracks reference known methods.
- `ShowcaseContractTests`
  - Runs `validate_showcase.py --skip-demo`.
- `DemoReportScriptTests`
  - Builds a minimal fake demo directory.
  - Runs `generate_demo_report.py`.
  - Checks report snippets.
  - Checks manifest profile and requested method lists.
  - Runs `validate_demo_artifacts.py`.
  - Verifies an expected-method mismatch fails.

### 0.5 Current Product Story

The README and Pages story should now be:

1. This repo contains many localization/SLAM method ports.
2. Users can browse the catalog on GitHub Pages.
3. Users can run a one-command demo after clone.
4. The demo validates a committed real-data MCD fixture, not just synthetic data.
5. The default path compares many methods together.
6. CI runs that same path and uploads the HTML report.

This is intentionally a stronger OSS adoption story than "read a long paper
doc and manually pick scripts."

### 0.6 Rules for Future Claims

Keep these distinctions sharp:

- The committed MCD fixture is a smoke/demo fixture. It proves integration and
  output contracts, not paper-grade accuracy.
- The `broad` and `full` profiles are all-OK fixture profiles, not a statement
  that every method is production-ready.
- The demo report is reproducible from the repo because the fixture is
  committed.
- Full dataset benchmark claims still come from `experiments/results/*.json`,
  `docs/reproduction_status.md`, and the older benchmark matrix.
- Do not claim exact original-paper reproduction unless the taxonomy/docs say
  so.
- Do not include methods that skip on the fixture in default all-OK profiles
  unless the validator is changed to support expected skips with explicit
  reasons.

### 0.7 Immediate Next Work

Recommended order:

1. **Review the new untracked files as if preparing a commit.**
   - Confirm no generated demo outputs are accidentally staged.
   - Confirm `docs/assets/explorer_preview.png` is intentionally tracked.
   - Confirm `docs/methods.json` names/summaries read well enough for public
     Pages.

2. **Run one final local verification before commit.**
   - `rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build`
   - `rtk python3 evaluation/scripts/validate_showcase.py --require-demo --demo-dir experiments/results/runs/demo_localization_zoo`
   - `rtk python3 -m unittest discover -s tests -p 'test_*.py' -v`
   - `rtk proxy git diff --check -- README.md docs/index.html evaluation/scripts/demo_localization_zoo.sh evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py tests/test_experiment_scripts.py PLAN.md`

3. **Commit the showcase/demo expansion.**
   - Suggested commit scope: README, docs page/assets, method catalog, demo
     scripts, validators, CI, tests, PLAN.
   - Suggested commit message:
     `Add showcase explorer and broad demo validation`

4. **Then consider one star-growth follow-up, not all at once.**
   - Add README badges for CI / Pages / license.
   - Add a compact "What runs in 60 seconds?" section.
   - Add a GIF or screenshot from `report.html`.
   - Add issue templates for "method request" and "dataset request".
   - Add a `CONTRIBUTING.md` focused on adding a method to `papers/*` and
     `docs/methods.json`.

5. **If expanding validation again, prefer expected-skip semantics before adding
   IMU-only selectors.**
   - Current fixture lacks `imu.csv`.
   - `ct_lio` and `clins` skip correctly on this data.
   - A robust next step would let the manifest encode expected skips with
     reason text, but this is a different contract from the current all-OK demo.

### 0.8 Do Not Do Next

Avoid these unless the user explicitly asks:

- Do not start a new paper-writing pass.
- Do not rerun large KITTI/MulRan full-sequence sweeps just to improve the
  README.
- Do not add more untracked generated artifacts.
- Do not collapse the old benchmark/reproduction docs into the new showcase
  page; the showcase should point into them, not replace them.
- Do not loosen validators to make demos pass. If a method is not OK, either
  remove it from an all-OK profile or add an explicit expected-skip contract.

---

## 1. Executive Summary

> **⚠️ 2026-06-09: §00 が最新 handoff。** 本章 (§1–§14) は 2026-05〜06 初旬の
> 3D benchmark / manifest / reproduction 状態の**歴史的記録**。現アクティブ作業は
> **2D LiDAR scan odometry (papers 43–50, §00.6c–§00.60)**。git/論文数/次タスクは §00.2 を正とする。

### 1.1 Current indexed artifact (historical snapshot — see §00.2 for live state)

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD | `458c81a` |
| Worktree | **clean after NDT + LOAM-family KITTI Odom checkpoint commit** |
| Indexed manifests | **392** |
| Indexed ready | **378** |
| Indexed blocked | **1** |
| Indexed skipped | **13** |
| Pending manifests | **200** |
| LiDAR families | **27** |
| Camera-aware families | **6** |
| Total active selectors | **33** |
| Python tests | **14/14 pass** (last full run; not rerun after NDT/LOAM-family docs/artifact refresh) |

### 1.2 What changed recently (2026-05-19..21 session, GICP family + NDT/A-LOAM KITTI checks)

**NDT T1 transfer confirmation** on KITTI Odom seq 02/05/08 full. `--ndt-resolution 0.5 --ndt-max-iterations 12` stayed sub-10cm on all three held-out long sequences: seq 02 = 0.0585 m ATE, seq 05 = 0.0594 m ATE, seq 08 = 0.0761 m ATE. Together with the existing seq 00 r05+i12 result (0.0707 m) and seq 07 r05+i12 result (0.0763 m), NDT now has a seeded KITTI Odom 5/5 sub-8cm T1-family recipe. The direct confirmation runs were slow on this export path (0.23-0.25 FPS), so the result is an accuracy/universality confirmation rather than a throughput recommendation.

**A-LOAM KITTI Odom full transfer check**: added `aloam_kitti_seq_07_full_sweep` plus seq 00/02/05/08 transfer matrices. Seq 07 `fast` wins the combined benchmark by throughput (4.0307 m ATE, 0.691% RPE, 7.26 FPS), while `kitti_default` is the seq 07 accuracy winner (2.5052 m ATE, 0.605% RPE, 3.26 FPS). Transfer is **non-universal**: accuracy winner is `kitti_default` on all held-out seqs, with ATE 9.5206 m (00), 50.7898 m (02), 5.1927 m (05), 18.6614 m (08). `fast` is faster but less accurate on all held-out seqs: 19.3711 m (00), 74.9663 m (02), 7.7893 m (05), 23.3578 m (08). Treat A-LOAM as a drift-level baseline, not a sub-meter KITTI Odom recipe.

**F-LOAM KITTI Odom seq 07 full cluster probe**: added `floam_kitti_seq_07_full_sweep`. `dense` is the accuracy/RPE winner (3.1736 m ATE, 0.590% RPE, 3.74 FPS), `kitti_default` is close in accuracy but much faster (3.2455 m, 0.789% RPE, 13.23 FPS), and `fast` wins the combined benchmark by throughput (5.0260 m, 1.262% RPE, 29.84 FPS). F-LOAM is faster than A-LOAM on this sequence but still drift-level; transfer to seq 00/02/05/08 remains unverified.

**LeGO-LOAM KITTI Odom seq 07 full cluster probe**: added `lego_loam_kitti_seq_07_full_sweep`. `kitti_default` is the accuracy/RPE winner (2.5579 m ATE, 0.527% RPE, 3.89 FPS), `fast` wins the combined benchmark by throughput (4.3850 m, 0.597% RPE, 10.03 FPS), and `dense` underperformed both (3.9454 m, 0.650% RPE, 3.68 FPS). LeGO-LOAM is the strongest LOAM-family accuracy profile on seq 07 so far, slightly worse than A-LOAM `kitti_default` in ATE but better in RPE; held-out transfer remains unverified.

**MULLS KITTI Odom seq 07 full cluster probe**: added `mulls_kitti_seq_07_full_sweep`. `dense` is the accuracy/RPE winner (8.2878 m ATE, 2.640% RPE, 1.27 FPS), `kitti_default` is close but slower/worse (8.4591 m, 2.720% RPE, 1.42 FPS), and `fast` wins the combined benchmark by throughput (10.5014 m, 2.994% RPE, 4.13 FPS). MULLS is the weakest LOAM-family seq 07 candidate so far and very slow on full KITTI Odom; do not spend transfer budget on it unless specifically needed as a negative baseline.

**F-LOAM + LeGO-LOAM held-out transfer**: added seq 00/02/05/08 full transfer matrices for F-LOAM `kitti_default`/`dense` and LeGO-LOAM `kitti_default`. No universal LOAM-family recipe emerged. Per-seq LOAM accuracy winners are mixed: seq 00 = F-LOAM `dense` (8.5561 m ATE, 0.991% RPE, 3.37 FPS), seq 02 = LeGO-LOAM `kitti_default` (42.2088 m, 0.883% RPE, 3.79 FPS), seq 05 = A-LOAM `kitti_default` (5.1927 m, 0.512% RPE, 2.26 FPS; LeGO is nearly tied at 5.2182 m), seq 08 = F-LOAM `kitti_default` (16.6614 m, 1.566% RPE, 12.83 FPS). These improve over A-LOAM on seq 00/02/08 but remain drift-level and far from NDT/LiTAMIN2/GICP seeded winners.

**GICP family recipe discovery + seed-dependence verification** across all KITTI Odom sequences. State delta: 336 → 371 indexed manifests (35 new), HEAD `d22a172` → `458c81a`.

**GICP family findings** (memory entries: `gicp_family_seq_07_recipe_divergence.md`, `small_gicp_fast_kitti_universal.md`, `small_gicp_seed_dependence.md`):

- **small_gicp `--small-gicp-fast-profile`** は KITTI Odom 5/5 + KITTI Raw 4/4 + MCD KTH で ATE winner = **9/10 scenes universal sub-meter** (0.68-0.98 m). LiTAMIN2 T1 と並ぶ唯一の cross-KITTI universal recipe。indoor static (MCD NTU/TUHH) のみ dense_profile に転換。
- **voxel_gicp `--voxel-gicp-dense-profile`** は KITTI Odom 5/5 で both ATE & RPE universal winner (ATE 0.94-1.05 m, RPE 1.5-1.8%). 強い but KITTI Raw / MCD では dataset-dependent (4/9 only).
- **KISS-ICP** は long-trajectory で catastrophic: seq 02 で 39.23 m, seq 00 で 11.98 m, seq 08 で 19.41 m。dense_profile が 4/5 で best ATE recipe も全体的に他 method 比 10-30× worse on long-traj. Local-map-only architecture が drift-bound.
- **LiTAMIN2 T1 (voxel=0.5+iter=12) transfer は GICP family で non-universal**: KISS-ICP seq 07 のみ局所勝利、small_gicp で ATE は fast に負ける、voxel_gicp で全 seq 退行。T1 transfer は LiTAMIN2 専用 recipe.

**Seed-dependence critical finding** (memory entry: `small_gicp_seed_dependence.md`):

KITTI Odom seq 00 full no-seed test で 4 GICP family methods + LiTAMIN2 全てが完全発散:

| method | seeded ATE | no-seed ATE | drift |
|---|---|---|---|
| LiTAMIN2 T1 | 0.731 | ~110 m | +15,000% |
| small_gicp fast | 0.890 | 202.72 | +22,800% |
| voxel_gicp dense | 1.047 | 87.91 | +8,300% |
| **CT-ICP best (cluster A)** | 4.91 (seeded) | **12.69** | **唯一 functional** |

→ **production deployment (no-seed) on KITTI Odom long-trajectory では CT-ICP が唯一の選択肢**。seeded benchmark での compositional ranking (LiTAMIN2 T1 / small_gicp fast / voxel_gicp dense / CT-ICP) が完全 reversed.

**Earlier (2026-05-18..19, LiTAMIN2 saturation + CT-ICP completion):**

LiTAMIN2 cluster T1 (voxel=0.5 + iter=12 + GT seed) は KITTI Odom 5/5 + MulRan 2/2 + MCD 3/3 + KITTI Raw 4/4 = **11/12 universal winner**, 1/12 tied (noise floor 0.5 m). CT-ICP 比 1.8-70× dominance on seeded benchmark.

**Earlier CT-ICP findings** (memory entries: `ct_icp_kitti_full_per_seq_best.md`, `ct_icp_cluster_a_cross_dataset_transfer.md`, `ct_icp_gt_seed_dataset_dependence.md`):

- **5-cluster structure for CT-ICP recipes**: cluster A (`map=50 + c2f σ×2`) wins KITTI seq 00 (12.69 m) / 05 (7.76 m) / 08 (27.85 m). cluster B = A + corr=4 for seq 05. cluster C = `bare + corr=8` for seq 02 (50.63 m). cluster D = `ms_chol + map=20` for seq 07 (1.61 m) and KITTI Raw 0061 full (4.50 m). KITTI Raw 0009 is its own balanced-only family.
- **Cross-dataset transfer**: cluster A + GT seed wins on MulRan parkinglot full (9.19 m, -36% vs prior best) and parkinglot 120 (2.55 m, -84%). cluster A + seed is the universal seeded winner on MCD KTH/TUHH/NTU/MulRan parkinglot — **but not on KITTI Odom seq 07** where cluster D dominates seed-independently.
- **GT seed dataset-dependence**: seed helps drift-limited scenes (-44 to -88% on MulRan parkinglot, MCD KTH/TUHH) but hurts self-anchoring scenes (MCD NTU +39%, KITTI seq 07 neutral). On KITTI seq 00 full it produces the sharpest ATE/RPE flip yet observed: ATE -61% (12.69 → 4.91 m) but RPE +174% (2.10 → 5.76%).
- **Knob axes exhaustively mapped** on seq 00 cluster A: cauchy_coarse_mult (2.0 winner), cauchy_fine_sigma (default 0.5 winner), coarse_search_radius (2 winner, 4 ties), coarse_iter (2 winner), map_size (50 winner), corr_dist (default 100 winner). 2-D cauchy plane has its true minimum at (fine=0.5, coarse_mult=2.0).

Earlier (2026-05-17) foundational taxonomy layer (still in effect):

- **Claim-level taxonomy**: `reproduced > approximately_reproduced > indicative > smoke > ported`. Bumped into [`evaluation/data/paper_reported_numbers.json`](evaluation/data/paper_reported_numbers.json) (schema v3) and rendered into [`docs/reproduction_status.md`](docs/reproduction_status.md) as a legend column.
- **Budget profile contract**: [`docs/budget_profiles.md`](docs/budget_profiles.md) defines `smoke_200f_120s`, `practical_full_300s`, `extended_full_1800s`, `reference_full_unbounded`. Manifests reference them via `problem.budget_profile`.
- **Family registry**: [`experiments/families.json`](experiments/families.json) classifies the 33 method families into `core` / `extended` tiers and `maintained` / `timeout_prone` / `input_constrained` / `legacy` / `experimental` status.
- **Status taxonomy migration**: [`docs/status_taxonomy.md`](docs/status_taxonomy.md) defines the target per-variant enum. C++ binaries (`pcd_dogfooding`, `multimodal_dogfooding`) and the runner now emit the lower-case enum (`ok`, `skipped`, `timeout_budget`). Reserved values (`tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`) have field space in `MethodResult::status` but no detection logic yet. Legacy uppercase (`OK`/`SKIPPED`/`TIMED_OUT`) is normalized at ingest.

Earlier session work (already in `main` history):

- MulRan / Newer College benchmark scaffolding (commit `aee7611`).
- Short-trajectory RPE fix and `paper_comparison.md` refresh.
- `pcd_dogfooding --summary-json` exports optional `rpe_trans_pct` / `rpe_rot_deg_per_m`.
- KITTI Odometry preparation script `evaluation/scripts/prepare_kitti_odometry_inputs.py` generalized; `setup_kitti_benchmark.sh` is a wrapper.

### 1.3 Current direction

**Saturated:**
- CT-ICP: 5-cluster recipe structure mapped across 13 dataset/window combinations. Knob axes + seed-dependence saturated.
- LiTAMIN2: cluster T1 universal across 12 locally-available CT-ICP-comparable datasets (11/12 wins, 1/12 tied at noise floor).
- KISS-ICP / small_gicp / voxel_gicp: recipe pattern on 5 KITTI Odom seqs + 7 cross-dataset scenes. Seed-dependence verified (all 3 + LiTAMIN2 require GT seed for long-traj).
- NDT: T1 r05+i12 (`--ndt-resolution 0.5 --ndt-max-iterations 12`) confirmed on KITTI Odom 5/5 full with sub-8cm seeded ATE. Remaining gap is throughput/implementation efficiency, not recipe universality on this benchmark.
- A-LOAM: KITTI Odom 5/5 full transfer checked. `kitti_default` is stable but drift-level; no universal sub-meter recipe.
- F-LOAM: KITTI Odom 5/5 checked via seq 07 cluster + seq 00/02/05/08 transfer. Useful on seq 00/08, especially `dense` on seq 00 and `kitti_default` on seq 08, but non-universal and drift-level.
- LeGO-LOAM: KITTI Odom 5/5 checked via seq 07 cluster + seq 00/02/05/08 transfer. Strongest LOAM-family result on seq 02 and near-tie on seq 05, but non-universal and drift-level.
- MULLS: KITTI Odom seq 07 full cluster probe checked. `dense` is the seq 07 accuracy/RPE winner, but all profiles are slow and drift-heavy; deprioritize held-out transfer.

**Method-level production deployment recommendation:**
- **Seeded benchmark winners (KITTI Odom 5/5)**: NDT T1 r05+i12 (0.058-0.076 m), LiTAMIN2 T1 (0.65-0.75 m), small_gicp fast (0.68-0.98 m), voxel_gicp dense (0.94-1.05 m).
- **No-seed deployment (production realism)**: **CT-ICP cluster A only** (12.69 m on seq 00; all others ≥87 m).

Priority order for next assistant:

- **A (highest leverage)**: checkpoint the NDT + LOAM-family artifact/docs refresh before more sweeps. The current dirty set is large and internally consistent.
- **B**: resume cross-method universal recipe survey: apply LiTAMIN2 T1 / small_gicp fast / voxel_gicp dense to remaining local datasets (autoware_istanbul, hdl_400) to fill the cross-dataset matrix.
- **C (broader, blocked on external data)**: `MulRan dcc01` and Newer College ingestion. 2 pending dcc01 manifests already exist; data download required.

Do **not** spend the next turn on paper drafting, PR polishing, or speculative refactoring. The user has been explicit that this is OSS infrastructure work, not paper writing.

---

## 2. Worktree State

Working tree should be clean after the NDT seq 02/05/08 confirmation artifact refresh and LOAM-family KITTI Odom full transfer/cluster sweep checkpoint commit. Previous handoffs warned about a dirty worktree with mass untracked multimodal work; that state has since been committed. Treat `git status` as authoritative.

The branch is currently ahead of `origin/wip/profile-expansion-refresh` by some commits; verify with `git status` before pushing.

---

## 3. Stable Core vs Experiment Layer

### 3.1 Stable binaries

- [`build/evaluation/pcd_dogfooding`](build/evaluation/pcd_dogfooding) — LiDAR-only stable benchmark, 27 method families, shared `--summary-json` contract.
- [`build/evaluation/multimodal_dogfooding`](build/evaluation/multimodal_dogfooding) — camera-aware sibling, 6 method families, same aggregate style and runner contract.

### 3.2 Stable per-method summary contract

Each method emits:

- `status` — lower-case enum from [`docs/status_taxonomy.md`](docs/status_taxonomy.md). Current emitting set: `ok`, `skipped`, `timeout_budget`. Reserved (not yet emitted): `tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`.
- `ate_m`
- `rpe_trans_pct` (optional; null when not computable)
- `rpe_rot_deg_per_m` (optional)
- `frames`
- `time_ms`
- `fps`
- `note`

`MethodResult::status` is a `std::string` in C++ side; future detection logic should set it directly (e.g. `result.status = "tracking_lost";`) rather than introducing new boolean flags.

### 3.3 Experiment layer

- Manifests: `experiments/*_matrix.json` (336 active)
- Pending manifests: `experiments/pending/*_matrix.json` (200)
- Aggregates: `experiments/results/*.json` (336)
- Family registry: [`experiments/families.json`](experiments/families.json) — used by docs, not by the runner.
- Generated docs:
  - `docs/experiments.md`
  - `docs/decisions.md`
  - `docs/interfaces.md`
  - `docs/paper_comparison.md`
  - `docs/variant_analysis.md`
  - `docs/reproduction_status.md`

---

## 4. Local Data Actually Available

Important: do not assume datasets mentioned in docs are locally present.

### 4.1 Present in `dogfooding_results/`

| Dataset | Path | Frames | Multimodal extras |
|--------|------|--------|-------------------|
| HDL-400 native/reference-like | `hdl_400_open_ct_lio_120` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 synthetic time (azimuth) | `hdl_400_open_ct_lio_120_time_azimuth` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 public ROS1 synthetic time | `hdl_400_ros1_open_ct_lio_120_time_index` | 120 | `imu.csv`, `frame_timestamps.csv` |
| KITTI Odometry seq 00 short | `kitti_seq_00_108` | 108 | (LiDAR-only) |
| KITTI Odometry seq 00 full | `kitti_seq_00_full` | 4542 | (LiDAR-only) |
| KITTI Odometry seq 02 full | `kitti_seq_02_full` | 4661 | (LiDAR-only) |
| KITTI Odometry seq 05 full | `kitti_seq_05_full` | 2761 | (LiDAR-only) |
| KITTI Odometry seq 07 short | `kitti_seq_07_108` | 108 | (LiDAR-only) |
| KITTI Odometry seq 07 full | `kitti_seq_07_full` | 1102 | (LiDAR-only) |
| KITTI Odometry seq 08 full | `kitti_seq_08_full` | 4071 | (LiDAR-only) |
| KITTI Raw 0009 short | `kitti_raw_0009_200` | 200 | full multimodal extras |
| KITTI Raw 0009 full | `kitti_raw_0009_full` | 443 | full multimodal extras |
| KITTI Raw 0061 short | `kitti_raw_0061_200` | 200 | full multimodal extras |
| KITTI Raw 0061 full | `kitti_raw_0061_full` | 703 | full multimodal extras |
| MCD KTH day-06 | `mcd_kth_day_06_108` | 108 | `frame_timestamps.csv` |
| MCD NTU day-02 | `mcd_ntu_day_02_108` | 108 | `frame_timestamps.csv` |
| MCD TUHH night-09 | `mcd_tuhh_night_09_108` | 108 | `frame_timestamps.csv` |
| MulRan parkinglot 120 | `mulran_parkinglot_120` | 120 | (LiDAR-only) |
| MulRan parkinglot full | `mulran_parkinglot_full` | 1177 | (LiDAR-only) |

### 4.2 Present in `experiments/reference_data/`

GT CSVs for every dataset listed in 4.1. Verify by listing the directory before quoting paths.

### 4.3 Not present locally right now

- KITTI Odometry seq 01, 03, 04, 06, 09, 10 (only 00/02/05/07/08 are dogfooded)
- Istanbul local windows
- MulRan dcc01 / kaist / riverside (parkinglot only is dogfooded; dcc01 pending manifests exist)
- Newer College, Oxford Spires

This matters because:

- The next assistant must not claim runs on KITTI Odom seqs outside 00/02/05/07/08 without first ingesting those seqs.
- The 5 ingested KITTI Odom seqs have been **exhaustively probed by CT-ICP**; LiTAMIN2 and other LiDAR families have NOT yet been similarly probed.

---

## 5. Indexed Method Surface

### 5.1 LiDAR-only families (29)

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `genz_icp`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `rko_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

(`genz_icp`, `rko_lio` added 2026-06-02 later session — see §0.0.)

### 5.2 Camera-aware families (6)

`vins_fusion`, `okvis`, `orb_slam3`, `lvi_sam`, `fast_livo2`, `r2live`

### 5.3 Tier classification

See [`experiments/families.json`](experiments/families.json). Roughly:

- **core (15)**: `kiss_icp`, `ct_icp`, `litamin2`, `small_gicp`, `voxel_gicp`, `ndt`, `gicp`, `fast_lio2`, `lio_sam`, `dlio`, `dlo`, `hdl_graph_slam`, `okvis`, `vins_fusion`, `fast_livo2`
- **extended (18)**: everything else, plus `orb_slam3`, `lvi_sam`, `r2live` marked `timeout_prone`.

### 5.4 Non-indexed papers

Outside the current benchmark surface: `vilens`, `relead`, `ct_icp_relead`, `scan_context`, `imu_preintegration`, `cube_lio_repro`.

LiDAR-only coverage is already broad. The missing frontier is publication-grade reproduction evidence, not more method folders.

---

## 6. Multimodal State

### 6.1 Canonical scope

- Binary: [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)
- Windows: `kitti_raw_0009_200`, `kitti_raw_0009_full`, `kitti_raw_0061_200`, `kitti_raw_0061_full`
- Each run expects `sequence_dir`, `gt_csv`, `landmarks.csv`, `visual_observations.csv`, camera intrinsics via CLI / `camera_args.txt`.

### 6.2 Current results

Fully practical under current study budget: `okvis`, `vins_fusion`, `fast_livo2`.
Practical-budget timeouts: `orb_slam3`, `lvi_sam`, `r2live`.

Canonical timeout budget (encoded in manifests via `problem.variant_timeout_seconds`):

- 200-frame windows: 120s
- Full windows: 300s

These align with `smoke_200f_120s` and `practical_full_300s` profiles in [`docs/budget_profiles.md`](docs/budget_profiles.md).

### 6.3 Multimodal infra

- [`evaluation/scripts/prepare_kitti_multimodal_inputs.py`](evaluation/scripts/prepare_kitti_multimodal_inputs.py)
- [`evaluation/scripts/generate_kitti_visual_observations.py`](evaluation/scripts/generate_kitti_visual_observations.py)
- [`evaluation/scripts/run_multimodal_study.py`](evaluation/scripts/run_multimodal_study.py)
- [`evaluation/scripts/smoke_multimodal_fixture.sh`](evaluation/scripts/smoke_multimodal_fixture.sh)

---

## 7. Reproduction / Paper-Result Status

### 7.1 The honest claim today

Per [`docs/reproduction_status.md`](docs/reproduction_status.md) and the stricter
promotion bar in [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md):

| Method | Claim level | Why |
|--------|------------|-----|
| `litamin2` | `indicative` | Short-window ATE, not full-sequence KITTI RPE study. |
| `ct_icp` | `indicative` | Core close to paper formulation, but evaluation indirect. |
| `kiss_icp` | `indicative` | Compact local-map pipeline, windowed reruns only. |
| `gicp` | `ported` | Pre-KITTI paper; no standardized numeric target. |
| `ndt` | `ported` | Pre-KITTI; modern NDT codebases differ materially. |
| `ct_lio` | `ported` | Intentionally custom integration; no single paper-faithful target. |

The repo is **not** entitled to say "paper results reproduced" generally. The
claim-level scheme makes that boundary explicit, while the paper-ready plan
defines which methods can be promoted into a manuscript table.

### 7.2 What is already better

`LiTAMIN2` and `CT-ICP` are closer to reproducible paper-style evaluation because:

- `pcd_dogfooding` exports RPE.
- Aggregates preserve RPE.
- `paper_comparison.md` shows repo-side RPE where available.

Canonical KITTI Raw 0009 reruns with RPE exist:

- [`experiments/results/litamin2_kitti_raw_0009_matrix.json`](experiments/results/litamin2_kitti_raw_0009_matrix.json) — adopted `fast_cov_half_threads` (ATE 1.053 m, RPE trans 0.742 %, 34.16 FPS)
- [`experiments/results/ct_icp_kitti_raw_0009_matrix.json`](experiments/results/ct_icp_kitti_raw_0009_matrix.json) — adopted `fast_window` (ATE 2.728 m, RPE trans 2.198 %, 49.27 FPS)

### 7.3 What is still missing for graduation

To move `litamin2` and `ct_icp` from `indicative` toward `approximately_reproduced`:

- **CT-ICP**: full KITTI Odom 00/02/05/07/08 are now ingested and exhaustively probed. Per-seq best numbers (12.69 / 50.63 / 7.76 / 1.61 / 27.85 m ATE) are recipe-tuned, not paper-tuned. Direct paper-comparison requires either (a) committing to a single "paper-style" recipe and reporting per-seq deltas, or (b) reporting per-seq best with explicit recipe attribution.
- **LiTAMIN2**: same 5-cluster recipe-discovery approach has NOT yet been applied. This is the next high-leverage probe — analog of CT-ICP's cluster A/B/C/D/balanced structure may exist for LiTAMIN2.
- **A (parallel)**: extend RPE-aware reruns to MCD/HDL-400 for both methods.

---

## 8. KITTI Odometry Preparation State

### 8.1 Preparation script

- [`evaluation/scripts/prepare_kitti_odometry_inputs.py`](evaluation/scripts/prepare_kitti_odometry_inputs.py)
- Converts `sequences/<seq>/velodyne/*.bin` + `poses/<seq>.txt` into:
  - `dogfooding_results/kitti_seq_<seq>_<window>`
  - `dogfooding_results/kitti_seq_<seq>_full`
  - `experiments/reference_data/kitti_seq_<seq>_<window>_gt.csv`
  - `experiments/reference_data/kitti_seq_<seq>_full_gt.csv`

### 8.2 Compatibility wrapper

[`evaluation/scripts/setup_kitti_benchmark.sh`](evaluation/scripts/setup_kitti_benchmark.sh) delegates to the Python script.

### 8.3 Status of full-sequence manifests

CT-ICP full KITTI Odom 00/02/05/07/08 manifests have been **promoted to active** (this session). Per-seq best results recorded; 5-cluster recipe structure documented in `memory/ct_icp_kitti_full_per_seq_best.md`. Cross-dataset cluster A transfer documented in `memory/ct_icp_cluster_a_cross_dataset_transfer.md`.

LiTAMIN2 full-sequence manifests remain in `experiments/pending/` — runnable now that KITTI Odom data is ingested. This is the **highest-leverage next probe**.

### 8.4 Regression coverage

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) contains a fake KITTI Odometry root test that verifies window export, full export, GT CSV generation, and contract compatibility of the preparation script.

---

## 9. Public-Data Direction Beyond KITTI

### 9.1 Already partially in (do not redo from scratch)

- **MulRan parkinglot**: ingested. 8 indexed manifests covering `LiTAMIN2`, `CT-ICP`, `KISS-ICP`, `GICP` × {120-frame, full}.
- **MulRan dcc01**: 4 pending manifests in `experiments/pending/` (`*_mulran_dcc01_120_matrix.json` for ct_icp/gicp/kiss_icp/litamin2). **Blocked**: raw data not local. Requires MulRan official download form (https://forms.gle/EmUybUiGc8pR3r7Q6) per `evaluation/scripts/SETUP_MULRAN_BENCHMARK.md`.
- **Newer College**: 2 pending manifests in `experiments/pending/` (`ct_icp_newer_college_01_short_120_matrix.json`, `litamin2_newer_college_01_short_120_matrix.json`). **Blocked on two fronts**: (a) the manifests target sequence `01_short_experiment`, which is not present under `data/newer_college/`; (b) the available local `math_hard` data is a ROS2 bag (`math_hard.bag` + `_rosbag2.db3`) with topic `os_node/lidar_packets` (raw Ouster packets), not the flat PCD layout expected by `prepare_newer_college_inputs.py`. Extracting requires either downloading the flat-file release from https://ori-drs.github.io/newer-college-dataset/, or setting up ROS2 + Ouster driver to replay the bag and save PCDs.

### 9.2 Best next LiDAR target

**MulRan dcc01 ingestion** to complete the MulRan coverage. Reuse the existing parkinglot ingestion approach. Friendly format, broadens public-data evidence on the LiDAR-only surface. Pending the download form.

### 9.3 Best next camera-aware target

**Newer College**. LiDAR + IMU + camera; lets the repo claim multimodal coverage beyond KITTI Raw. Pending the flat-file download or a bag-extraction setup.

### 9.4 Future heavier option

**Oxford Spires**. Newer, larger, more visually attractive, but higher ingestion cost than MulRan.

### 9.5 Practical recommendation

If the user says "do something useful without waiting for KITTI Odometry / MulRan dcc01 / Newer College data":

- **Phase A reruns first** (LiTAMIN2/CT-ICP on local MCD + HDL-400 with RPE) — cheapest claim-level improvement.
- **Cross-method --no-gt-seed sweep** on local KITTI Raw 0009 full — already done 2026-05-18, see `docs/dogfooding_methodology.md`. Extending to other local datasets (MulRan parkinglot full, MCD KTH/NTU/TUHH) would cost almost nothing.
- **Then MulRan dcc01** when data lands — natural extension of partially-done work.

---

## 10. Tests and Verification

### 10.1 Last known-green verification (2026-05-17)

- `python3 -m unittest discover -s tests -p 'test_*.py' -v` → **14/14 passed**
- `cmake --build build --target pcd_dogfooding multimodal_dogfooding` → built clean
- End-to-end runner smoke against `experiments/kiss_icp_kitti_raw_0009_matrix.json` → 3 variants completed, aggregate JSON emits new `"status": "ok"` taxonomy.

### 10.2 What the tests cover

- `run_experiment_matrix.py` — reuse-aggregate, timeout (now expects `"timeout_budget"`), RPE parsing.
- `refresh_study_docs.py`
- `generate_reproduction_status.py`
- `generate_paper_comparison.py` data plumbing
- `run_multimodal_study.py`
- Synthetic multimodal fixture generator
- KITTI Odometry preparation script

### 10.3 What did not run recently

- Full `ctest`
- Full Docker rebuild
- Any real KITTI Odometry full-sequence benchmark run
- Any new external dataset import beyond what is already on disk

Be precise about verification scope when reporting.

---

## 11. Generated Docs

Generated docs already reflect canonical aggregate state at HEAD `5a96dec`:

- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/paper_comparison.md`](docs/paper_comparison.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)
- [`docs/reproduction_status.md`](docs/reproduction_status.md) — now includes claim-level legend.

To refresh all of them: `python3 evaluation/scripts/refresh_study_docs.py`.

---

## 12. What Cursor / Codex Should Do Next

This is the operational handoff. **Default path: paper-ready reproducibility hardening (§00.52i), not new-method churn.**
Pick a single path and finish it before switching.

### Priority A (active): paper-ready reproducibility hardening

1. **Next action**: add the next missing public/cross-domain check:
   M-GCLO public non-flat dataset validation, Quadric-LO public curved-object / non-urban dataset validation,
   or RF-LIO/ID-LIO public high-dynamic dataset validation. I-LOAM intensity on/off,
   KC-LO sigma schedule, M-GCLO ground factor, Quadric-LO plane fallback, and
   RF-LIO/ID-LIO synthetic dynamic stress plus M-GCLO synthetic non-flat stress
   plus Quadric-LO synthetic curved stress artifacts are already committed in
   §00.52j〜§00.52q.
2. **Extend the paper bundle**: `docs/benchmarks/paper_ready_bundle.json` currently
   freezes 4 methods. Grow it toward the final 8-12 method table from raw JSON.
3. **Keep claims tiered**: README may advertise breadth; paper/manuscript language should use only
   T0/T1 methods from [`docs/paper_ready_reproducibility.md`](docs/paper_ready_reproducibility.md).
4. **Respect current user direction**: 2D scan odometry is paused unless explicitly resumed.
5. **Verify docs after README/index edits**:
   ```bash
   python3 evaluation/scripts/validate_showcase.py --root .
   ```

### Priority B: 3D LiDAR / visual / IMU from-paper campaign

1. Resume new author-code-free 3D method implementation only after the paper-ready subset has a
   credible ablation table, or when the user explicitly asks for another "tugi".
2. Keep the unit of work stable: module under `papers/<method>/`, CMake integration,
   `pcd_dogfooding` selector, focused unit tests, KITTI seq00/07 full artifacts,
   README leaderboard row, `docs/methods.json`, method README, and this PLAN.

### Priority C (paused): 2D scan odometry campaign

1. **Commit + push** pending work if user asks:
   - IDC (`361a592`), CSM-DT (`3fc5be0`), markdown hub (§00.59 files).
2. **Unify benchmarks**:
   ```bash
   cmake --build build --target scan_dogfooding
   bash evaluation/scripts/run_scan2d_benchmark.sh
   ```
   Merge 8 methods into `docs/benchmarks/scan2d/{intel,fr079,mit,corridor}.json`.
3. **Verify CI path**: `bash evaluation/scripts/smoke_scan2d_fixture.sh`
4. **Next 2D work**: PL-ICP/MbICP local map or Karto-style map matcher.
5. **Docs**: keep [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md) as canonical;
   README top-level table stays a 1-screen summary linking to the hub.

### Priority D: OSS showcase / regression (parallel, low effort)

1. After README/index.html edits: `python3 evaluation/scripts/validate_showcase.py --root .`
2. Demo path: `bash evaluation/scripts/demo_localization_zoo.sh`

### Priority E (blocked): KITTI Odometry full reruns

Blocked only by data.

1. Obtain KITTI Odometry root containing at minimum:
   - `sequences/00/velodyne` + `poses/00.txt`
   - `sequences/07/velodyne` + `poses/07.txt`
2. Prep:
   ```bash
   python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
     --kitti-root <path> --sequence 00 --sequence 07 \
     --window-size 108 --include-full
   ```
3. Run 108-frame manifests first, then full-sequence manifests.

### Priority F (on hold): 3D PG-LIO improvement

NCLT 600 honest negative (33% drift). Do not prioritize over 2D unless user redirects.

### Do NOT do next (unless explicitly asked)

- Treat paper 50 MbICP as unstarted
- Resume 2D campaign without user direction
- Paper drafting / prose generation before the ablation table exists
- PR / branch cleanup unrelated to current task
- Broad refactor that touches the stable contract
- Force push / git config changes
- Reverting any pending manifest

---

## 13. Commands Reference

### 13.1 Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)" --target pcd_dogfooding multimodal_dogfooding scan_dogfooding
```

### 13.1b 2D scan odometry (active campaign)

```bash
# CI-equivalent smoke (Intel 20f × 8 methods)
bash evaluation/scripts/smoke_scan2d_fixture.sh

# Refresh all committed 2D benchmarks
bash evaluation/scripts/run_scan2d_benchmark.sh

# Single fixture
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 \
  evaluation/fixtures/intel_val_73/gt.csv \
  --methods rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc \
  --wheel-odom-from-gt \
  --summary-json docs/benchmarks/scan2d/intel_val_73.json
```

Hub: [`docs/benchmarks/scan2d/README.md`](docs/benchmarks/scan2d/README.md)

### 13.2 Python regression

```bash
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 evaluation/scripts/verify_environment.py
```

### 13.3 Full docs refresh

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 13.4 LiDAR smoke

```bash
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> \
  --methods litamin2,gicp,ndt \
  --summary-json artifacts/tmp/smoke.json
```

### 13.5 Multimodal smoke

```bash
./build/evaluation/multimodal_dogfooding <sequence_dir> <gt_csv> \
  --methods okvis,vins_fusion,fast_livo2 \
  --landmarks-csv <sequence_dir>/landmarks.csv \
  --visual-observations-csv <sequence_dir>/visual_observations.csv \
  --summary-json artifacts/tmp/multimodal_smoke.json
```

### 13.6 KITTI Raw multimodal prep

```bash
python3 evaluation/scripts/prepare_kitti_multimodal_inputs.py --include-full
python3 evaluation/scripts/run_multimodal_study.py --include-full --method okvis
```

### 13.7 KITTI Odometry prep

```bash
python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
  --kitti-root data/kitti_odometry \
  --sequence 00 --sequence 07 \
  --window-size 108 --include-full
```

### 13.8 Safe exploratory runs

Use repo-local scratch outputs when you do not want to disturb canonical repo docs / index:

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/<something>.json \
  --output-dir artifacts/tmp/localization_zoo_results \
  --docs-dir artifacts/tmp/localization_zoo_docs
```

---

## 14. Final Notes

- **2026-06-10 更新**: アクティブ価値は **2D scan odometry zoo** (8 法 × 4 fixture,
  honest leaderboard) と **from-paper 50 本**の蓄積。§00.6c–§00.60 が運用 handoff。
- 3D 側は依然 valuable だが、**PG-LIO honest negative** と KITTI full データ欠如で
  新規 3D 論文より 2D 拡張 (local map / Karto-style matcher) を優先。
- The cheapest claim-level upgrade for 3D remains local MCD/HDL-400 reruns with RPE;
  strongest is full KITTI Odometry (blocked).
- Keep the stable summary contract small; new failure modes flow through `MethodResult::status` strings.
- Do not oversell reproduction status. The taxonomy exists precisely so the repo can be honest.
- **2D GT proxy 限界**: Bonn dataset odometry は centimeter truth ではない — drift は相対比較用。
