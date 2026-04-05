# Localization Zoo - Claude 引き継ぎ PLAN

> 最終更新: 2026-04-06 (第2版)
> この `PLAN.md` は、現在の `Localization Zoo` を
> 「実験駆動の比較基盤 + 論文化準備中の artifact」
> として引き継ぐための handoff 文書である。
>
> Claude はまずこのファイルを読み、次に
> [`docs/paper_tracks.md`](docs/paper_tracks.md),
> [`docs/paper_roadmap.md`](docs/paper_roadmap.md),
> [`docs/experiments.md`](docs/experiments.md),
> [`docs/decisions.md`](docs/decisions.md),
> [`docs/paper_comparison.md`](docs/paper_comparison.md)
> を見れば、今の repo の状態と次の優先タスクを把握できる。

---

## 0. まず 1 分で把握すべきこと

- この repo はもう単なる「38本の論文実装集」ではない。
- 現在の主戦場は `evaluation/src/pcd_dogfooding.cpp` を stable core に置いた
  `variant-first benchmark` である。
- つまり、最初に完璧な抽象設計を作るのではなく、
  `同一 interface / 同一入力 / 同一 metric` の下で複数 variant を並べ、
  比較し、勝ち筋だけを暫定 default に採用する流れに切り替わっている。
- 論文化の主軸も「正しい 1 実装」ではなく、
  `variant-first benchmarking exposes Pareto fronts`
  という empirical study / reproducibility artifact に寄っている。
- 現在の repo 状態は、
  `36 ready problems + 1 blocked problem`
  まで進んでいる。
- 3つ目の public dataset family (MCD: Multi-Campus Dataset) が追加済み。
  - MCD TUHH night-09 (Ouster OS1-64, 61k pts/frame)
  - MCD NTU day-02 (Ouster OS1-128, 84k pts/frame)
  - MCD KTH day-06 (Ouster OS1-64) — 追加中
- CT-LIO GT-backed は main study から scope-out 済み。
- KITTI Odometry 用のマニフェスト 10 本が `experiments/pending/` に待機中（データ要ダウンロード）。
- 論文化基盤が整った:
  - `docs/paper_draft_outline.md` — 論文構成
  - `docs/paper_claim.md` — Core claim + evidence pointers
  - `docs/paper_comparison.md` — 論文報告値 vs repo defaults
  - `Dockerfile` + `requirements.txt` — 再現ビルド環境
- 次の本筋は
  `論文草稿の執筆`
  と
  `evidence の window 数をさらに増やす`
  である。

---

## 1. このファイルが置き換えるもの

旧 `PLAN.md` は、以下の意味で既に古い。

- 「38実装を追加する」が主テーマになっていた
- ROS2 node 追加が主目的だった
- `README` 更新や `CTest all pass` が中心タスクだった
- repo を静的な実装集として扱っていた

今の repo は違う。

- `papers/` は実装群として一通り揃っている
- 重要なのは「どの variant を残し、どの variant を reference 扱いにするか」
- `docs/experiments.md` と `docs/decisions.md` が、今の設計判断の実態
- `docs/paper_tracks.md` と `docs/paper_roadmap.md` が、論文化の方針

このため、これ以降は
`実装数の増加` より
`比較可能性の維持`
と
`公開可能な実験証拠の厚み`
を優先する。

---

## 2. 現在の repo の役割

この repo は現在、次の 3 層で構成されている。

### 2.1 Stable Core

安定契約として扱う部分。

- [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
  - 共有 CLI 契約
  - 共有 `--summary-json` 出力契約
  - method family ごとの profile flag surface
- `papers/*`
  - コア実装群
- `build/evaluation/pcd_dogfooding`
  - 比較を回す実体

この層は「他の全実験が依存する共通土台」なので、
不用意に壊さないこと。

### 2.2 Experiment Layer

捨てられる探索層。

- [`experiments/`](experiments/)
  - `*_matrix.json` manifests
  - problem 定義
  - variant 群
  - `experiments/results/*.json`
  - `experiments/results/runs/.../summary.json`

ここでは 1 つの方法に対して
最低 3 variant を並べるのが基本。

### 2.3 Generated Evidence Layer

generator から再生成される docs / assets。

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_roadmap.md`](docs/paper_roadmap.md)
- [`docs/paper_assets.md`](docs/paper_assets.md)
- [`docs/paper_captions.md`](docs/paper_captions.md)
- [`docs/assets/paper/`](docs/assets/paper/)

原則として、この層は手編集しない。
変更したいなら generator 側を直し、再生成する。

---

## 3. いま push 対象に含まれる主要変更

現在の worktree には、主に次の変更がある。

### 3.1 新しい実験スクリプト

- [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
  - manifest 実行
  - aggregate 生成
  - `docs/experiments.md`, `docs/decisions.md`, `docs/interfaces.md` 再生成
- [`evaluation/scripts/refresh_study_docs.py`](evaluation/scripts/refresh_study_docs.py)
  - experiment docs + publication docs + paper assets の一括 refresh
- [`evaluation/scripts/generate_publication_docs.py`](evaluation/scripts/generate_publication_docs.py)
  - `docs/paper_tracks.md`, `docs/paper_roadmap.md` 生成
- [`evaluation/scripts/export_paper_assets.py`](evaluation/scripts/export_paper_assets.py)
  - paper-ready CSV / figure / caption 出力
- [`evaluation/scripts/pose_trace_to_gt_csv.py`](evaluation/scripts/pose_trace_to_gt_csv.py)
  - public `pose_trace.csv` を dogfooding 用 CSV に変換
- [`evaluation/scripts/slice_trajectory_csv_by_frames.py`](evaluation/scripts/slice_trajectory_csv_by_frames.py)
  - extracted `frame_timestamps.csv` に合わせて trajectory CSV を切り出す

### 3.2 Manifest 群

`experiments/` 以下に、各 method family の比較マニフェストがある。

いま ready / blocked に関与する主要 manifest は次。

- Istanbul x 3 windows
  - `litamin2_profile_matrix.json`
  - `litamin2_istanbul_window_b_matrix.json`
  - `litamin2_istanbul_window_c_matrix.json`
  - 同様に `gicp`, `ndt`, `kiss_icp`, `ct_icp`
- HDL-400 reference x 2 windows
  - `litamin2_hdl_400_reference_matrix.json`
  - `litamin2_hdl_400_reference_b_matrix.json`
  - 同様に `gicp`, `ndt`, `kiss_icp`, `ct_icp`
- CT-LIO
  - `ct_lio_reference_profile_matrix.json`
  - `ct_lio_public_readiness_matrix.json` (blocked)

### 3.3 Paper-facing docs / assets

現在の論文化関係は以下に集約されている。

- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_roadmap.md`](docs/paper_roadmap.md)
- [`docs/paper_assets.md`](docs/paper_assets.md)
- [`docs/paper_captions.md`](docs/paper_captions.md)
- [`docs/assets/paper/ready_defaults.csv`](docs/assets/paper/ready_defaults.csv)
- [`docs/assets/paper/manuscript_core_defaults.csv`](docs/assets/paper/manuscript_core_defaults.csv)
- [`docs/assets/paper/ready_defaults_pareto.png`](docs/assets/paper/ready_defaults_pareto.png)
- [`docs/assets/paper/variant_fronts_by_selector.png`](docs/assets/paper/variant_fronts_by_selector.png)
- [`docs/assets/paper/manuscript_core_methods.png`](docs/assets/paper/manuscript_core_methods.png)

---

## 4. 現在の evidence snapshot

`python3 evaluation/scripts/refresh_study_docs.py`
を full に回した最新集約では、次の状態になっている。

### 4.1 問題数

- ready: `36`
- blocked: `1`

### 4.2 内訳

- LiDAR-only / gt-backed / Istanbul x 3 windows
  - `5 method families x 3 windows = 15 ready`
- LiDAR-only / reference-based / HDL-400 x 2 windows
  - `5 method families x 2 windows = 10 ready`
- LiDAR-only / gt-backed / MCD TUHH night-09
  - `5 method families x 1 window = 5 ready`
- LiDAR-only / gt-backed / MCD NTU day-02
  - `5 method families x 1 window = 5 ready`
- CT-LIO / reference-based / HDL-400
  - `1 ready`
- CT-LIO / GT-backed / HDL-400
  - `1 blocked` (scope-out 済み: main study 対象外)

### 4.3 Dataset family 数

- non-IMU LiDAR-only 系で `3 public dataset families`
  - Autoware Istanbul (Velodyne, ~1.1k pts/frame, 都市走行)
  - HDL-400 (Velodyne HDL-32E, reference-based)
  - MCD Multi-Campus Dataset (Ouster OS1-64/128, 61-84k pts/frame, 大学キャンパス)

MCD は登録不要で Google Drive から直接ダウンロード可能。
KITTI Odometry のマニフェスト 10 本が `experiments/pending/` に待機中。

---

## 5. method family ごとの現在地

ここでは「今どの variant が default 扱いになっているか」をざっくりまとめる。
厳密な採否理由は [`docs/decisions.md`](docs/decisions.md) を見ること。

### 5.1 LiTAMIN2

- Istanbul A: `fast_icp_only_half_threads`
- Istanbul B: `fast_icp_only_half_threads`
- Istanbul C: `paper_icp_only_half_threads`
- HDL-400 A: `paper_icp_only_half_threads`
- HDL-400 B: `fast_icp_only_half_threads`

観察:

- dataset ごとに勝つ profile が少し揺れる
- `paper profile` が常に勝つわけではない
- `fast_icp_only_half_threads` は throughput 側の強い default
- ただし「論文 reported result に近いか」と「この repo 内での Pareto front」は別問題

### 5.2 GICP

- ほぼ全 window で `fast_recent_map`

観察:

- dense は精度が良くても遅い
- balanced は中途半端になりやすい
- 現状は `fast_recent_map` がこの repo の実用 default

### 5.3 NDT

- ほぼ全 window で `fast_coarse_map`

観察:

- この repo の現条件では `fast_coarse_map` が精度 / 速度の両面で勝ちやすい
- dense/local map を default に戻す根拠は今のところ弱い

### 5.4 KISS-ICP

- Istanbul A: `fast_recent_map`
- Istanbul B: `dense_local_map`
- Istanbul C: `fast_recent_map`
- HDL-400 A: `fast_recent_map`
- HDL-400 B: `fast_recent_map`

観察:

- odometry-only で dataset 依存が大きい
- HDL-400 B では `fast_recent_map` が `0.218 m / 0.447 FPS` と比較的良かった
- Istanbul では absolute ATE が依然大きい

### 5.5 CT-ICP

- Istanbul A: `fast_window`
- Istanbul B: `balanced_window`
- Istanbul C: `balanced_window`
- HDL-400 A: `fast_window`
- HDL-400 B: `fast_window`

観察:

- HDL-400 系では `fast_window` が throughput 側 default
- Istanbul B/C では `balanced_window` が粘る
- continuous-time odometry としては比較可能性は出ているが、まだ paper claim の核ではない

### 5.6 CT-LIO

- reference-based ready:
  - `seed_only_fast`
- GT-backed:
  - blocked

観察:

- 参照軌跡ベースの比較はできている
- しかし「独立 GT に対して勝った」という story にはまだなっていない

---

## 6. 代表値

manuscript core として今 pick されている代表 default は
[`docs/assets/paper/manuscript_core_defaults.csv`](docs/assets/paper/manuscript_core_defaults.csv)
に出る。

現時点の representative defaults は次。

| selector | dataset | contract | variant | ATE [m] | FPS |
|---|---|---|---|---:|---:|
| `ct_icp` | `autoware_istanbul_open_108_b` | gt-backed | `balanced_window` | 6.819773 | 1.582405 |
| `ct_lio` | `hdl_400_open_ct_lio_120` | reference-based | `seed_only_fast` | 0.411644 | 0.443120 |
| `gicp` | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 0.981916 | 4.265101 |
| `kiss_icp` | `autoware_istanbul_open_108_c` | gt-backed | `fast_recent_map` | 131.691761 | 3.744606 |
| `litamin2` | `autoware_istanbul_open_108_c` | gt-backed | `paper_icp_only_half_threads` | 0.741485 | 17.249127 |
| `ndt` | `autoware_istanbul_open_108_c` | gt-backed | `fast_coarse_map` | 0.005456 | 1.947388 |

注意:

- これは「全 dataset を通じた universal default」ではない
- generator が manuscript 用に representative 1 本を選んでいるだけ
- HDL-400 では別 variant が勝つことがある

---

## 7. 論文化の現在地

詳細は [`docs/paper_tracks.md`](docs/paper_tracks.md) と
[`docs/paper_roadmap.md`](docs/paper_roadmap.md) を参照。

要点だけ書く。

### 7.1 Track A: Empirical Study

- 現在の主軸
- readiness: `Medium`
- 主張:
  `variant-first localization benchmarking reveals Pareto fronts hidden by single-canonical repositories`

これが一番筋が良い。

### 7.2 Track B: Artifact / Reproducibility

- 平行で狙える
- readiness: `High`
- 近い将来に一番出しやすい

### 7.3 Track C: Focused Method Paper

- まだ保留
- どれか 1 手法が multi-dataset で一貫して勝つ状態ではない

結論:

- 草稿はもう書ける
- workshop / artifact / reproducibility は現実的
- 強い full paper には、もう 1 段の evidence 増強が必要

---

## 8. Claude が次にやるべきこと

優先度順に書く。

### P0-1: 第3の public dataset family を追加 — **完了**

MCD (Multi-Campus Dataset) を第3 family として追加済み。
- MCD TUHH night-09 (Ouster OS1-64) — 5 methods ready
- MCD NTU day-02 (Ouster OS1-128) — 5 methods ready
- MCD KTH day-06 (Ouster OS1-64) — 追加中
- 登録不要、Google Drive 直接ダウンロード
- 変換スクリプト: `extract_ros1_lidar.py`, `mcd_gt_to_csv.py`
- KITTI Odometry マニフェスト 10 本が `experiments/pending/` に待機中

### P0-2: CT-LIO の GT-backed をどう扱うか決める — **完了**

main study から scope-out。
- `ct_lio_public_readiness_matrix.json` に `scope_decision` を追記済み
- 理由: KITTI に IMU なし、HDL-400 に独立 GT なし
- CT-LIO は reference-based のみで artifact に残る

### P1-1: original-paper comparison sheets — **完了**

- `evaluation/data/paper_reported_numbers.json` — 6手法の論文報告値
- `evaluation/scripts/generate_paper_comparison.py` — 自動生成
- `docs/paper_comparison.md` — generated doc
- `refresh_study_docs.py` に統合済み

### P1-2: pinned environment / container — **完了**

- `Dockerfile` — Ubuntu 22.04 ベース、全依存込み
- `requirements.txt` — Python 依存固定
- `evaluation/scripts/verify_environment.py` — 環境検証

### P1-3: paper skeleton — **完了**

- `docs/paper_draft_outline.md` — 7 section + appendix
- `docs/paper_claim.md` — Core claim + 4 sub-claims + evidence pointers
- `docs/paper_tables_todo.md` — 表・図チェックリスト

---

### 新 P0: 論文草稿の執筆

36 ready problems、3 dataset families の evidence が揃った。
次は `docs/paper_draft_outline.md` をベースに actual prose を書く。

### 新 P1: evidence window をさらに増やす

- MCD KTH day-06 追加中
- KITTI Odometry データダウンロード後に `experiments/pending/` を有効化 → +10 ready
- 他の MCD sequence (e.g. `ntu_night_04`, `tuhh_day_02`) も追加可能

### 新 P2: Docker clean-machine 再現性検証

`Dockerfile` のビルド + `refresh_study_docs.py` の実行確認。

### 新 P3: Ceres 2.2 互換修正の横展開確認

19 ファイルで `EigenQuaternionParameterization` → `EigenQuaternionManifold` に修正済み。
全テストが通ることの確認。

---

## 9. 実験 workflow のルール

Claude 向けに、ここが一番重要。

### 9.1 まず variant を増やし、後で削る

禁止:

- 最初から 1 つの正解実装に収束しようとすること
- 全 method family を統一抽象で縛ろうとすること

やること:

- 3 variant 以上を並べる
- 結果を `docs/decisions.md` に外部化する
- 負け variant もすぐ消さない

### 9.2 比較軸を壊さない

新 variant を入れるときは、必ず同じ問題定義で回す。

- 同一 CLI
- 同一 `pcd_dir`
- 同一 `gt_csv`
- 同一 metrics

### 9.3 generated docs を手編集しない

以下は原則 hand-edit しない。

- `docs/experiments.md`
- `docs/decisions.md`
- `docs/interfaces.md`
- `docs/paper_tracks.md`
- `docs/paper_roadmap.md`
- `docs/paper_assets.md`
- `docs/paper_captions.md`

generator を直して再生成する。

### 9.4 targeted run の罠

重要:

`python3 evaluation/scripts/refresh_study_docs.py --manifest ...`
や
`python3 evaluation/scripts/run_experiment_matrix.py --manifest ...`
は、
**その subset だけで `index.json` と generated docs を上書きする**。

つまり、
subset 実験を回した直後は
repo 全体の docs が一時的に「部分状態」になる。

したがって、

1. 局所探索では `--manifest` を使ってよい
2. しかし commit / push 前には必ず

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

を full で回して、全 manifest 状態に戻すこと

これは今回の作業でも実際に踏んだ罠なので、忘れないこと。

### 9.5 local dataset と committed artifact を混同しない

`dogfooding_results/` は local fixture であり、git には載せない。

repo に commit するのは主に:

- manifests
- stored summaries
- aggregate JSON
- generated docs
- paper assets

clean checkout でも `--reuse-existing` により docs の再生成は可能だが、
新しい rerun には raw / extracted data が要る。

---

## 10. 実用コマンド集

### 10.1 full refresh

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

用途:

- 全 manifest を `--reuse-existing` で再集約
- docs / paper docs / paper assets を全更新
- commit 前の最終整形

### 10.2 targeted manifest run

```bash
python3 evaluation/scripts/refresh_study_docs.py \
  --manifest experiments/<name>_matrix.json
```

用途:

- 1 問題だけの局所探索

注意:

- そのままだと global docs が subset 状態になる
- 最後に full refresh を必ずやる

### 10.3 public HDL-400 の新 window 切り出し

```bash
python3 evaluation/scripts/extract_ros2_lidar_imu.py \
  --bag ../lidarloc_ws/data/official/hdl_localization/hdl_400_ros2 \
  --pointcloud-topic /velodyne_points \
  --imu-topic /gpsimu_driver/imu_data \
  --output-dir dogfooding_results/hdl_400_open_ct_lio_120_b \
  --start-frame 360 \
  --max-frames 120

python3 evaluation/scripts/slice_trajectory_csv_by_frames.py \
  --input experiments/reference_data/hdl_400_public_reference.csv \
  --frame-timestamps dogfooding_results/hdl_400_open_ct_lio_120_b/frame_timestamps.csv \
  --output experiments/reference_data/hdl_400_public_reference_b.csv
```

### 10.4 smoke benchmark

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/hdl_400_open_ct_lio_120_b \
  experiments/reference_data/hdl_400_public_reference_b.csv \
  --methods litamin2,gicp,ndt,kiss_icp,ct_icp \
  --summary-json dogfooding_results/hdl_400_open_lidar_120_b_summary.json
```

### 10.5 script sanity

```bash
python3 -m py_compile \
  evaluation/scripts/slice_trajectory_csv_by_frames.py \
  evaluation/scripts/generate_publication_docs.py \
  evaluation/scripts/export_paper_assets.py \
  evaluation/scripts/refresh_study_docs.py \
  evaluation/scripts/run_experiment_matrix.py
```

---

## 11. 今回追加・更新した HDL-400 window B

Claude が見落としやすいので明示する。

新たに追加された local dataset / reference は次。

- local sequence:
  - `dogfooding_results/hdl_400_open_ct_lio_120_b`
- sliced reference CSV:
  - `experiments/reference_data/hdl_400_public_reference_b.csv`

追加済み manifest:

- `experiments/ct_icp_hdl_400_reference_b_matrix.json`
- `experiments/gicp_hdl_400_reference_b_matrix.json`
- `experiments/kiss_icp_hdl_400_reference_b_matrix.json`
- `experiments/litamin2_hdl_400_reference_b_matrix.json`
- `experiments/ndt_hdl_400_reference_b_matrix.json`

この window B での default は以下。

| Method | Default | ATE [m] | FPS |
|---|---|---:|---:|
| CT-ICP | `fast_window` | 1.210946 | 2.358097 |
| GICP | `fast_recent_map` | 0.283982 | 1.709203 |
| KISS-ICP | `fast_recent_map` | 0.218431 | 0.446613 |
| LiTAMIN2 | `fast_icp_only_half_threads` | 0.168429 | 5.198294 |
| NDT | `fast_coarse_map` | 0.064833 | 0.858834 |

観察:

- HDL-400 B では `KISS-ICP` が Istanbul よりかなりまとも
- `LiTAMIN2` は `paper profile` より `fast_icp_only_half_threads` が良い
- `NDT` はここでも `fast_coarse_map` 優勢

---

## 12. README / docs の整合性

README は今の experiment-driven state に合わせて更新済み。

特に以下は反映済み:

- experiment-driven セクション
- generated docs の導線
- HDL-400 reference trajectory と slicing helper の説明
- paper docs / paper assets の導線

ただし benchmark セクション先頭の表は
依然として Istanbul snapshot を例示している。
これは意図的で、
`paper-facing defaults table` は別に
[`docs/paper_assets.md`](docs/paper_assets.md)
へ逃がしている。

README をさらに更新するなら、
「single snapshot table」と
「cross-problem decision docs」を混同しないように書くこと。

---

## 13. commit / push 前の最終チェック

Claude がこの先作業して commit する前には、最低限これをやること。

```bash
python3 evaluation/scripts/refresh_study_docs.py
git status --short
```

必要なら追加で:

```bash
python3 -m py_compile evaluation/scripts/*.py
```

そして push。

このファイルを書いている時点では、
`main` branch で作業している。

---

## 14. 現時点の結論

この repo は
「比較を回しながら設計を進化させる場所」
として十分に成熟した。

達成済み:

1. ~~public evidence の family 数を増やす~~ → 3 families, 36 ready
2. ~~blocked な CT-LIO の扱いを決める~~ → scope-out 決定済み
3. ~~original-paper comparison を作る~~ → `docs/paper_comparison.md`
4. ~~artifact として clean-machine 再現性を上げる~~ → `Dockerfile` + `requirements.txt`
5. ~~empirical paper の skeleton を起こす~~ → `docs/paper_draft_outline.md` 他

次の本筋は:

1. 論文草稿の執筆（prose を書く段階）
2. evidence window のさらなる拡大（MCD KTH, KITTI 等）
3. Docker clean-machine 再現性の実地検証
4. 論文投稿先の選定と submission 準備

