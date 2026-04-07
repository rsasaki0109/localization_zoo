# Localization Zoo - Codex 向け引き継ぎ PLAN

> **最終更新: 2026-04-08**
>  
> この文書は「次の Codex / メンテ担当が、いま何が終わっていて、何が終わっておらず、どの数字を信じるべきか」を最短で掴むための handoff である。
>  
> 先に本ファイルを読み、その後に [`experiments/results/index.json`](experiments/results/index.json)、[`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)、[`docs/experiments.md`](docs/experiments.md) を見るのが最短。

---

## 0. この文書の役割

- これは「将来計画」ではなく、**現在地の正確な説明 + 次に触る順番 + 罠一覧**である。
- 特にこの repo は **CLI 側の統合数** と **manifest / result 側の被覆数** が完全には一致しないため、その差を埋めて書く。
- また、`README.md` や一部 `docs/paper_*.md` に **古い数字**が残っている。次の作業者がそこに引っ張られないよう、**真実源泉の優先順位**を明示する。

---

## 1. 今の HEAD と作業ツリー

| 項目 | 値 |
|------|----|
| Branch | `main` |
| HEAD | `a52d0f83ea44f35c078ff7b55c37c4203125f988` (`a52d0f8`) |
| Worktree | dirty (`public ROS1 synthetic-time` split + generated docs/results を含む) |
| 直近の PLAN 専用コミット | `a52d0f8 Update PLAN.md v6 final: 164 ready problems, all tasks completed` |
| 直近の docs / index 再生成コミット | `08df6b8 Refresh study docs and index: 164 ready problems, 27 method families` |

### 1.1 直近コミット列の意味

重要なのは次の流れ:

1. `1d1c26b`  
   Tier 1 と CI 修復。`X-ICP`, `FAST-LIO2`, `HDL-Graph-SLAM`, `VGICP-SLAM` を bench 側に接続。
2. `e5d9d71`  
   Tier 2 と build 修正。`SuMa`, `BALM2`, `ISC-LOAM`, `LOAM-Livox`, `LIO-SAM`, `LINS`, `FAST-LIO-SLAM`, `Point-LIO`, `CLINS` を追加。
3. `7b3c429`  
   MCD manifest 36 件。
4. `14c63b6`  
   KITTI Raw manifest 48 件。
5. `64354f9`  
   `primary_method` 名の修正。
6. `5253d55`  
   `balm2`, `fast_lio2`, `fast_lio_slam` の結果追加。
7. `77b03e0`  
   一時点の PLAN 更新。
8. `c6ad89f`  
   新規手法ぶんの実験結果を追加。
9. `08df6b8`  
   `refresh_study_docs.py` で docs / index を再生成。
10. `a52d0f8`  
    PLAN 最終版更新。

### 1.2 背景コマンド `exit 144` について

前セッションで `refresh_study_docs.py` / index 更新をバックグラウンドで回し、その後 kill されたジョブが `exit code 144 (SIGKILL)` を出している。  
**これは失敗の痕跡ではなく、直接実行系で再生成が完了した後の kill 済みジョブの終了コード**である。  
**正とみなすべき根拠は `08df6b8` の内容**であり、`144` のログではない。

---

## 2. 数で見る現状

この節の数字は **2026-04-08 時点でファイルを直接読んで再確認した値**。

### 2.1 ベンチ全体

| 指標 | 値 | 備考 |
|------|----|------|
| manifest 数 | **167** | `experiments/*_matrix.json` |
| aggregate 数 | **167** | `experiments/results/*_matrix.json` |
| stored summary 数 | **440** | `experiments/results/runs/**/summary.json` |
| `index.json` 上の ready | **165** | `status == "ready"` の件数 |
| `index.json` 上の blocked | **1** | `ct_lio_public_gt_readiness` |
| `index.json` 上の skipped | **1** | `hdl_graph_slam_kitti_raw_0061_full` |
| dataset families | **4** | Istanbul / HDL-400 / MCD / KITTI Raw |
| `pcd_dogfooding` 対応 method families | **27** | CLI 実装済み |
| manifests を持つ method families | **26** | `clins` はまだ manifest なし |

### 2.2 manifest の内訳

`167` の中身は次のとおり:

| 区分 | 件数 | 説明 |
|------|------|------|
| KITTI Raw | **81** | short / full を含む主戦場 |
| MCD | **57** | `kth` / `ntu` / `tuhh` |
| Istanbul | **10** | 主に core methods + tradeoff |
| HDL-400 ready | **12** | native-time reference 10 + public ROS1 synthetic-time 2 |
| profile tradeoff studies | **5** | `litamin2`, `gicp`, `ndt`, `kiss_icp`, `ct_icp` |
| CT-LIO reference tradeoff | **1** | `ct_lio_reference_tradeoff` |
| blocked readiness | **1** | `ct_lio_public_gt_readiness` |

### 2.3 ここで注意すべきこと

- 以前は **`hdl_graph_slam_kitti_raw_0061_full` が phantom ready** だったが、この turn で runner を修正し、**全 variant `SKIPPED` の problem は `skipped` として index / aggregate に落ちる**ようにした。
- その結果、現在の全体像は:
  - **ready = 165**
  - **blocked = 1**
  - **skipped = 1**
  である。
- `skipped` は **manifest 設計上は対象だが、有効な benchmark result がまだ無い**ことを意味する。

---

## 3. 真実源泉の優先順位

この repo は docs が多いが、**全部を同じ重みで信じてはいけない**。

### 3.1 最上位の真実源泉

1. [`experiments/results/index.json`](experiments/results/index.json)  
   ベンチ全体の数え上げ・問題一覧。
2. 個々の [`experiments/results/*_matrix.json`](experiments/results)  
   各 problem の実際の variant 状態。`index.json` より具体的。
3. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)  
   実装済み method / CLI flags / profile flags の真実源泉。
4. [`experiments/*_matrix.json`](experiments/)  
   各 manifest の variant 設計と dataset 契約。

### 3.2 便利だが二次資料

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)

これらは読む価値があるが、**最終確認は JSON を見る**こと。特に status の境界ケースは JSON の方が明確。

### 3.3 現時点で stale 扱いすべき資料

少なくとも次は古い数字が残っている:

- [`README.md`](README.md)
- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_captions.md`](docs/paper_captions.md)

たとえば `README.md` はまだ **70+ ready / 14 methods** 時代の説明になっている。  
次の作業者は **README の数は信用しない**こと。

### 3.4 生成 docs の扱い

次は **手編集禁止**:

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)
- `docs/paper_*.md`
- `docs/assets/paper/*`

編集したい場合は **generator を直して**:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

---

## 4. method 統合の状態

### 4.1 CLI に統合済みの 27 families

`pcd_dogfooding` が受け付ける method は現時点で次の 27:

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

真実源泉は [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp) の `isSupportedMethod()`。

### 4.2 manifests / results の被覆

method ごとの manifest 数:

| 件数 | methods |
|------|---------|
| **14** | `ct_icp` |
| **13** | `litamin2`, `gicp`, `ndt`, `kiss_icp` |
| **7** | `balm2`, `fast_lio2`, `fast_lio_slam`, `hdl_graph_slam`, `isc_loam`, `lins`, `lio_sam`, `loam_livox`, `point_lio`, `suma`, `vgicp_slam`, `xicp` |
| **4** | `small_gicp`, `voxel_gicp` |
| **3** | `ct_lio` |
| **1** | `aloam`, `dlio`, `dlo`, `floam`, `lego_loam`, `mulls` |
| **0** | `clins` |

### 4.3 重要な差分

- **27 methods が CLI にいる**
- しかし **manifest があるのは 26 methods**
- **`clins` は bench バイナリに統合済みだが、manifest / result / docs 被覆はゼロ**

この差を忘れると、「27 methods が全部 experiments に入っている」と誤認する。

---

## 5. variants の成熟度

この repo の研究仮説は **variant-first** であり、本来は **各 method / problem に 3+ variants** が望ましい。  
しかし新規統合 handoff の現実として、まだそこまで到達していない。

### 5.1 3+ variants 未達の method 群

現時点で `<3 variants` の manifest を持つ method:

- `hdl_graph_slam` : 7/7 manifest が 1 variant

この turn で 3+ variants 化まで完了した method:

- `balm2` : 7/7 manifest が 3 variants
- `fast_lio2` : 7/7 manifest が 3 variants
- `fast_lio_slam` : 7/7 manifest が 3 variants
- `isc_loam` : 7/7 manifest が 3 variants
- `lins` : 7/7 manifest が 3 variants
- `lio_sam` : 7/7 manifest が 3 variants
- `loam_livox` : 7/7 manifest が 3 variants
- `point_lio` : 7/7 manifest が 3 variants
- `suma` : 7/7 manifest が 3 variants
- `vgicp_slam` : 7/7 manifest が 3 variants
- `xicp` : 7/7 manifest が 4 variants (`default / fast / dense / no_gt_seed`)

### 5.2 原因

原因は単純で、**残り 1 method** である `hdl_graph_slam` だけが、まだ manifest 側で `default` のみのまま止まっているため。

逆に、この turn で `balm2` / `fast_lio2` / `fast_lio_slam` / `isc_loam` / `lins` / `lio_sam` / `loam_livox` / `point_lio` / `suma` / `vgicp_slam` / `xicp` は CLI フラグ追加と manifest 拡張まで終わった。

補足:

- `lio_sam` の最初の dense profile は `0061_full` で重すぎたため、この turn で **`0.35 / 9000` -> `0.45 / 7000`** に戻して再実行した
- したがって今の `lio_sam` dense は「default より密だが、runtime が破綻しない」現実的な比較設定になっている
- `balm2` の最初の dense profile も full 系で重すぎたため、この turn で **`0.35 / 9000 / window=7 / outer=3 / ceres=8` -> `0.45 / 7000 / window=6 / outer=2 / ceres=6`** に戻して再実行した
- その結果、`balm2` dense は「精度寄りの比較設定」には留まりつつも、full 系を回し切れる帯に収まった

既存の old/core methods では:

- `gicp`
- `small_gicp`
- `voxel_gicp`
- `ndt`
- `kiss_icp`
- `ct_icp`
- `aloam`
- `floam`
- `lego_loam`
- `mulls`
- `dlo`
- `dlio`

などが fast / dense プロファイルを持つが、新規 method 群はそこまで到達していない。

### 5.3 次にやるなら何をすべきか

次の一手として最もまっとうなのは:

1. `hdl_graph_slam` の fast / dense manifest を作るかどうかを先に決める
2. 作るなら `0009_full` と `0061_full` で runtime guard を入れながら個別に回す
3. 無理なら `default only + skipped fast/dense` のような truth-preserving 設計は避け、現状の 1 variant のまま残す

理由:

- `fast_lio_slam` / `point_lio` / `balm2` は今回の turn で回し切れた
- `0061_full` では `fast_lio_slam` / `point_lio` / `balm2` / `isc_loam` / `lio_sam` / `loam_livox` / `vgicp_slam` のいずれも **`fast` が benchmark winner** になっており、新規群の比較軸は十分立った
- 一方 `hdl_graph_slam` は `0009_full` default だけで `2564s` 級なので、truthfulness を壊さずに進めるには最後に扱う方が安全

実作業の型は変わらない:

1. `pcd_dogfooding.cpp` に各 method の fast / dense profile を追加する
2. 対応する 7 manifest を `default / fast / dense` に広げる
3. method ごとに 1 本だけ先に sanity check する
4. 問題なければ 7 manifest を回し、最後に **必ず full refresh**

---

## 6. この turn で再確認したこと

この節は「前セッションの伝聞」ではなく、この turn で実行した結果。

### 6.1 CTest

以下を実行:

```bash
ctest --test-dir build --output-on-failure -j"$(nproc)"
```

結果:

- **38/38 tests passed**
- 実時間は約 3.28 秒

### 6.2 CI フィクスチャ smoke

以下を実行:

```bash
bash evaluation/scripts/smoke_ci_fixture.sh
```

結果:

- **成功**
- 現在の fixture には `imu.csv` がないため、常時走るのは **24 method**
- `ct_lio`, `fast_lio2`, `clins` は fixture に `imu.csv` がある場合のみ smoke に乗る

### 6.3 Worktree

- smoke 実行後も `git status --short` は clean
- `experiments/results/runs/ci_fixture_smoke/` は既存運用に吸収され、今回の作業で追加差分は出ていない

---

## 7. 重要な罠・齟齬

### 7.1 `skipped` は `blocked` とは別物

対象:

- manifest: [`experiments/hdl_graph_slam_kitti_raw_0061_full_matrix.json`](experiments/hdl_graph_slam_kitti_raw_0061_full_matrix.json)
- aggregate: [`experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`](experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json)

状態:

- manifest 上の `problem.state` は `ready`
- しかし唯一の variant は `SKIPPED`
- runner はこれを **aggregate / index では `status: "skipped"`** として出力する
- `current_default == null`

つまりこれは:

- **blocked**: そもそも契約上まだ bench できない
- **skipped**: bench 契約はあるが、その run では有効な結果が得られていない

この区別は重要。`skipped` を `ready` に混ぜると、また phantom ready が復活する。

次の作業者は、`hdl_graph_slam_kitti_raw_0061_full` を次のどちらかで処理すべき:

1. 実際に full sequence を最後まで走らせて真の `ready` にする
2. あるいは lighter profile / shorter slice を明示的に追加し、`default` とは別に比較可能な variant を作る

### 7.2 `--manifest` 部分実行の罠

`run_experiment_matrix.py --manifest ...` は便利だが、**その subset で `experiments/results/index.json` を上書きする**。  
したがって、manifest を個別に回した後は **コミット前に必ず**:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

を実行すること。

### 7.3 CT-LIO は「統合済み」だが main study 完走ではない

CT-LIO には:

- native-time reference-based な比較 1 件
- public ROS1 synthetic-time な比較 1 件
- blocked readiness 1 件

がある。  
**GT-backed public benchmark としては未解決**であり、main study の ready count に含めるべき対象ではない。

### 7.4 `dogfooding_results/` はコミットしない

大きい public / local データは `.gitignore` であり、コミット対象ではない。  
コミット対象は:

- manifest JSON
- aggregate JSON
- reference CSV
- 生成 docs
- 小さい fixture

のみ。

### 7.5 `README.md` を見て安心しない

`README.md` の benchmark 説明と method 数は古い。  
数字が必要なら必ず `index.json` と `pcd_dogfooding.cpp` を見ること。

---

## 8. blocked 問題の扱い

blocked は 1 件だけ:

- [`experiments/ct_lio_public_readiness_matrix.json`](experiments/ct_lio_public_readiness_matrix.json)
- [`experiments/results/ct_lio_public_readiness_matrix.json`](experiments/results/ct_lio_public_readiness_matrix.json)

内容:

- HDL-400 の LiDAR + IMU sequence はある
- しかし **独立に信頼できる GT CSV が repo 契約に沿って用意されていない**
- そのため CT-LIO は HDL-400 で **reference-based** / **public ROS1 synthetic-time** の補足比較に留め、main empirical study から外している

補足:

- [`experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)

この turn で public ROS1 HDL-400 bag を `evaluation/scripts/extract_ros1_lidar_imu.py --time-mode index` で復元した **synthetic per-point time bench** を separate problem として追加した。  
これにより:

- CT-LIO の public ROS1 synthetic-time default は `seed_only_fast` (`ATE 0.479317`)
- CT-ICP の public ROS1 synthetic-time default は `fast_window` (`ATE 2.464608`)

が明示的に再現できるようになった。  
ただし native-time reference result とは一致しないため、**exact reproduction の代用品ではなく、public-only の truth-preserving split** と捉えること。

次の作業者がこの blocked を触るべきか:

- **優先度は低い**
- 先にやるべきは **新規 method 群の 3+ variants 化** と **`clins` の manifest 化**

---

## 9. dataset family の見方

### 9.1 今ある 4 family

1. **Istanbul**
   - Autoware 系 window
   - 一部 core methods の比較土台
2. **HDL-400**
   - public native-time reference / public ROS1 synthetic-time / readiness 系
   - CT-LIO の blocked 論点が集中
3. **MCD**
   - `kth`, `ntu`, `tuhh`
   - Ouster 系 108 frames
   - CI はその 3 frame fixture を利用
4. **KITTI Raw**
   - short / full
   - 新規 12 method 群の主な収容先

### 9.2 物理データ配置の正規資料

外部データ配置は:

- [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md)
- [`evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md`](evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md)

を使う。

### 9.3 IMU 契約の重要点

この repo では **KITTI でなければいけないわけではない**。  
`DLIO`, `CT-LIO`, `FAST-LIO2`, `CLINS` など IMU 依存手法に必要なのは:

- `frame_timestamps.csv`
- それと整合した `imu.csv`

であり、データソース自体は任意。  
[`evaluation/scripts/smoke_dlio_imu_when_ready.sh`](evaluation/scripts/smoke_dlio_imu_when_ready.sh) にもその方針が明記されている。

---

## 10. Koide / AIST dataset メモ

前セッションの意図を汲むと、「次の dataset family を増やすなら Koide/AIST 系が候補ではないか」という論点があった。  
ここでは、その判断だけ短く固定しておく。

### 10.1 結論

**もし第 5 の public dataset family を増やすなら、AIST / Koide 系では `GLIM` の cross-sensor evaluation dataset を優先し、ICRA2024 の prior-map localization dataset は別トラック扱いにするのがよい。**

### 10.2 理由

公式ページによると:

- Koide の **GLIM cross-sensor evaluation dataset** は  
  **複数 LiDAR / depth sensor の LiDAR-IMU mapping test sequences** と  
  **groundtruth IMU trajectories** を提供する  
  <https://staff.aist.go.jp/k.koide/projects/glimsupp/versatile.html>
- 一方、Koide らの **ICRA2024 LiDAR-IMU Localization dataset** は  
  **3D prior map に対する localization** 用のデータである  
  <https://staff.aist.go.jp/k.koide/projects/icra2024_gl/>

この repo の現行契約は:

- `pcd_dogfooding <pcd_dir> <gt_csv> ...`
- sequence directory + GT trajectory CSV
- optional `imu.csv`

であり、これは **mapping / odometry / scan-to-map** ベンチの形に近い。  
したがって、**prior map を前提にする localization dataset をそのまま main bench に入れるのは契約がズレる**。

### 10.3 実務上の提案

次の作業者が dataset expansion をやるなら:

1. まず GLIM cross-sensor dataset を 1 つ選び、`pcd_dir + gt_csv + optional imu.csv` に変換する
2. core methods 数本で smoke する
3. それが安定したら family 化する
4. prior-map localization 系は別の benchmark track として設計する

### 10.4 これは推論である

上の「GLIM を先、ICRA2024 localization は別トラック」は、**AIST の公式 project pages と、この repo の現行 CLI 契約からの推論**である。  
今の repo に prior map benchmark 契約が無い以上、この切り分けは妥当。

---

## 11. 次にやるべきこと

ここから先は priority 順に書く。

### P0. 新規 method 群を 3+ variants 化する

この turn で完了:

- `balm2`
- `fast_lio2`
- `fast_lio_slam`
- `isc_loam`
- `lins`
- `lio_sam`
- `loam_livox`
- `point_lio`
- `suma`
- `vgicp_slam`
- `xicp`

残対象 method:

- `hdl_graph_slam`

やること:

1. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp) に profile flag を足す
2. 各 [`experiments/*_matrix.json`](experiments/) を `default / fast / dense` に広げる
3. method ごとに 1 manifest で sanity check
4. 全 manifest について results / docs を更新

理由:

- 今のままでは variant-first の論文主張に対して新規群だけ設計密度が低い
- ただし `hdl_graph_slam` 以外はこの段階を抜けたので、残る実質タスクは `hdl_graph_slam` の扱いだけでよい

### P1. `clins` の manifest を作る

状況:

- CLI 実装はある
- CTest も通る
- しかし experiments 層が無い

やること:

1. まず `KITTI Raw` または `imu.csv` 付きデータで 1 manifest 作成
2. その後 7 manifest へ広げるか判断

理由:

- 「27 methods 対応」と書けても、bench evidence は 26 methods 分しかない

### P2. README / paper docs の stale 数字を掃除する

対象:

- [`README.md`](README.md)
- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_captions.md`](docs/paper_captions.md)

理由:

- handoff ではなく public-facing な混乱源になっている
- 新規 contributor が最初に誤読する箇所

### P3. `hdl_graph_slam_kitti_raw_0061_full` を本当に `ready` にするか判断する

現状:

- [`experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`](experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json) は `status: "skipped"`
- blocker は `Skipped: computation exceeds 1 hour on KITTI Raw 0061 full (703 frames)`

やること:

1. そのまま long run を受け入れて本当に回す
2. もしくは fast profile / shorter full-like slice を導入する
3. どちらもやらないなら、`skipped` のまま stable に運用する

### P4. Istanbul / HDL の物理データ整備

やること:

- [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md) の想定パスに配置
- 必要なら GT / `imu.csv` を整理

理由:

- 現在の main bench は KITTI / MCD に重心が寄っている
- 4 family を均等に使いたいなら、Istanbul / HDL の再現性を上げる必要がある

### P5. dataset expansion をするなら GLIM 系を先に

これは P0-P4 より後。  
まずは今ある 4 family と **26 methods 分の manifest 被覆 + 半統合状態の `clins`** をきれいにする方が先。

---

## 12. 作業プレイブック

### 12.1 新規 profile flag を 1 method に追加するときの最短ルート

例: `hdl_graph_slam`

1. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp) に:
   - options struct のパラメータ
   - usage 文
   - `--hdl-graph-slam-fast-profile`
   - `--hdl-graph-slam-dense-profile`
   を追加
2. [`experiments/hdl_graph_slam_*_matrix.json`](experiments/) の `variants` を増やす
3. 個別確認:

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/hdl_graph_slam_kitti_raw_0009_matrix.json
```

4. 問題なければ full refresh:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 12.2 IMU 系 method を bench に載せるとき

確認事項:

- `frame_timestamps.csv` があるか
- `imu.csv` が同じ timeline 契約で置かれているか
- smoke は [`evaluation/scripts/smoke_dlio_imu_when_ready.sh`](evaluation/scripts/smoke_dlio_imu_when_ready.sh) を使う

### 12.3 full refresh 前にやること

最低限:

```bash
ctest --test-dir build --output-on-failure -j"$(nproc)"
bash evaluation/scripts/smoke_ci_fixture.sh
python3 evaluation/scripts/refresh_study_docs.py
```

### 12.4 full refresh 後に見るもの

1. [`experiments/results/index.json`](experiments/results/index.json)
2. 更新した method の aggregate JSON
3. [`docs/experiments.md`](docs/experiments.md)
4. `git status`

---

## 13. 便利ファイル一覧

### 13.1 bench / runner

- [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
- [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
- [`evaluation/scripts/refresh_study_docs.py`](evaluation/scripts/refresh_study_docs.py)

### 13.2 smoke / setup

- [`evaluation/scripts/smoke_ci_fixture.sh`](evaluation/scripts/smoke_ci_fixture.sh)
- [`evaluation/scripts/smoke_mcd_pcd_dogfooding.sh`](evaluation/scripts/smoke_mcd_pcd_dogfooding.sh)
- [`evaluation/scripts/smoke_dlio_imu_when_ready.sh`](evaluation/scripts/smoke_dlio_imu_when_ready.sh)
- [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md)
- [`evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md`](evaluation/scripts/VERIFY_KITTI_IMU_DLIO.md)

### 13.3 generated docs

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)

### 13.4 special topic

- [`papers/cube_lio_repro/README.md`](papers/cube_lio_repro/README.md)

`CUBE-LIO` は現状 **前処理 / cubemap / IGM デモ**まで。  
フル本体の再現実装として扱わないこと。

---

## 14. 最終 handoff

いまの repo は:

- **CLI とテストの土台は動いている**
- **167 manifests / 165 ready / 1 blocked / 1 skipped** という index はある
- **38/38 tests** と **CI fixture smoke** は earlier turn の最新成功記録があり、この turn では `extract_ros1_lidar_imu.py` の `py_compile` と manifest/docs 再生成までを再確認した

ただし、次の Codex が本当に向き合うべき未完は次の 4 点:

1. **`hdl_graph_slam` を 3+ variants 化するか、1 variant のまま truthfully 固定するか決めること**
2. **`clins` を evidence 層に載せること**
3. **stale docs の数字を掃除すること**
4. **native-time CT reference を exact reproduction したいなら、`hdl_400_ros2` 系 artifact の provenance を回収すること**

数字だけ見ると大きく前進しているが、構造的にはまだ **「統合は終わった、比較設計と provenance が一部未熟」** という段階にある。  
次の担当は、method を増やすより **variants・truthfulness・source provenance を整える**方が価値が高い。

---

*End of PLAN.md*
