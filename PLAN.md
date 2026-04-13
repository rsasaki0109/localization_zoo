# Localization Zoo - GitHub Copilot 向け引き継ぎ PLAN

> **最終更新: 2026-04-11**
>
> この文書は、次の GitHub Copilot / Copilot Chat / Codex が
> 「いまの repo の真実はどこにあるか」
> 「last committed state と current dirty worktree がどうズレているか」
> 「どの benchmark 数字を public に言ってよく、どれを exact reproduction と呼んではいけないか」
> を短時間で掴むための handoff である。
>
> **2026-04-11 update:** この PLAN の前半には、`e4d9af4` 上の large dirty worktree を観察していた時点の記述が履歴として残っている。だが現在の実作業はすでに `wip/profile-expansion-refresh` に commit/push 済みで、draft PR #1 (`https://github.com/rsasaki0109/localization_zoo/pull/1`) に積まれている。**いまの真実は §1, §3, §9, §12, §13 を優先して読むこと。**
>
> 最初に本ファイルを読み、その後に次を見るのが最短:
>
> 1. [`experiments/results/index.json`](experiments/results/index.json)
> 2. [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
> 3. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 4. `git status --short`

---

## 0. この文書の目的

- これは wishlist ではなく、**現在地の正確な説明**である。
- 特にこの repo は、**commit 済みの状態**と、履歴として参照される**過去の dirty worktree 状態**を分けて読む必要がある。
- さらに HDL-400 の CT 系は、**native per-point `time` を前提にした reference**と、**public ROS1 bag + synthetic time を使う public-only split**が別物である。
- ここを曖昧にすると、Copilot は「exact reproduction ができた」と誤認しやすい。
- 現在の作業ブランチは clean で push 済みだが、PLAN 中盤の dirty-worktree 記述は provenance メモとして残している。

---

## 1. いまの HEAD と作業ツリー

| 項目 | 値 |
|------|----|
| Branch | `wip/profile-expansion-refresh` |
| HEAD | `c4f367fcf3d107a58fc5db56604b663b7a000d11` (`c4f367f`) |
| HEAD message | `Document HDL Graph SLAM full-sequence policy` |
| Worktree | **clean** |
| Push 状態 | `origin/wip/profile-expansion-refresh` へ push 済み |
| PR | Draft PR #1 — `https://github.com/rsasaki0109/localization_zoo/pull/1` |
| 最新 PR CI | `build-and-test = success`, `ros2-colcon-build = success` |

### 1.1 HEAD `e4d9af4` が意味するもの

`e4d9af4` は、少なくとも次を **commit 済み**として持っている:

- public ROS1 HDL-400 を native-time benchmark とは別 manifest に分離
- [`evaluation/scripts/extract_ros1_lidar_imu.py`](evaluation/scripts/extract_ros1_lidar_imu.py) に `--time-mode preserve|index|azimuth` を追加
- [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md) を public ROS1 synthetic-time 前提に更新
- [`experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json) を追加
- [`experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json) を追加
- [`experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json) を追加
- [`experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json) を追加

これは **clean boundary** としてまだ重要である。  
ただし、Claude が実際に作業を継ぐべきブランチは `e4d9af4` そのものではなく、**その上に 4 commits 積んだ `wip/profile-expansion-refresh`** である。

### 1.2 現在この branch に積まれている差分

`e4d9af4` の上には、現在少なくとも次の 4 commit が積まれている:

1. `8a6c625 Expand experiment profiles and refresh study docs`
   - broad profile expansion
   - generated docs / paper assets の refresh
   - `hdl_graph_slam` の短尺 ready set を 3 variants 化
2. `6d655d0 Add CLINS public benchmark evidence`
   - `clins` CLI evidence を public ROS1 HDL-400 synthetic-time に追加
   - `--clins-fast-profile` / `--clins-dense-profile` を追加
   - `experiments/clins_hdl_400_public_ros1_synthtime_matrix.json` と aggregate を追加
3. `1f3db45 Refresh public benchmark narrative`
   - `README.md` と `docs/paper_*` の stale public narrative を更新
   - public ROS1 synthetic-time と reference/native-time-style の区別を明文化
4. `c4f367f Document HDL Graph SLAM full-sequence policy`
   - `hdl_graph_slam` の KITTI Raw full-sequence policy を固定
   - `kitti_raw_0009_full = default-only`
   - `kitti_raw_0061_full = skipped`

つまり、**この PLAN に残っている “dirty worktree” の多くはもう commit/push 済み**である。  
以降の dirty-worktree 記述は、「なぜその commit stack になったか」を理解するための provenance として読むこと。

---

## 2. commit 済みの真実と worktree 上の真実は分けて扱う

これが一番重要。

### 2.1 last committed truth (`e4d9af4`)

`e4d9af4` で言えること:

- public ROS1 HDL-400 synthetic-time split は **正式に commit 済み**
- public ROS1 CT-LIO default は `seed_only_fast`
- public ROS1 CT-ICP default は `fast_window`
- native-time reference 側の aggregate は、まだ次の数字を持っている:
  - CT-LIO reference `seed_only_fast = 0.411644`
  - CT-ICP reference `fast_window = 1.513295`

### 2.2 current working truth (dirty worktree)

いまの worktree で言えること:

- public ROS1 CT-LIO default はそのまま `seed_only_fast = 0.479317`
- public ROS1 CT-ICP default は **性能優先採択**に切り替わり
  `dense_window = 1.254043`
- 一方で native-time reference 側の aggregate は、dirty refresh の結果として今は:
  - CT-LIO reference `seed_only_fast = 0.488244`
  - CT-ICP reference `fast_window = 2.582133`
 になっている

### 2.3 なぜこれが危険か

この repo の current worktree では、reference manifest が参照する local PCD window と aggregate の provenance が、もはや「昔の native-time artifact と完全一致している」とは限らない。

つまり:

- **public ROS1 split** は意図通りの separate bench
- **reference aggregate の current dirty values** は、history 的な canonical native-time truth ではなく、
  **現在の local data / refresh 結果**に引っ張られている可能性が高い

Copilot が最初にやるべき判断は:

1. `e4d9af4` を基準に話すのか
2. current dirty worktree を基準に話すのか

この二者択一である。

---

## 3. 数で見る現在地

以下は **2026-04-11 に current branch / current generated files から取り直した値**。

### 3.1 ベンチ全体

| 指標 | 値 | 備考 |
|------|----|------|
| manifest 数 | **168** | `experiments/*_matrix.json` |
| aggregate 数 | **168** | `experiments/results/*_matrix.json` |
| stored summary 数 | **454** | `experiments/results/runs/**/summary.json` |
| ready | **166** | `experiments/results/index.json` |
| blocked | **1** | `ct_lio_public_gt_readiness` |
| skipped | **1** | `hdl_graph_slam_kitti_raw_0061_full` |
| dataset surfaces | **5** | Istanbul / HDL-400 reference-native-style / HDL-400 public ROS1 synthetic-time / MCD / KITTI Raw |
| CLI method families | **27** | `pcd_dogfooding` 実装済み |
| manifest を持つ method families | **27** | `clins` も manifest 済み |

### 3.2 HDL-400 の内訳

HDL-400 系 manifest は **15** 件ある:

- reference/native-time-style 系: **11**
  - `gicp`, `kiss_icp`, `litamin2`, `ndt` の `reference / reference_b`
  - `ct_icp_hdl_400_reference / reference_b`
  - `ct_lio_reference_profile`
- public ROS1 synthetic-time 系: **3**
  - `ct_lio_hdl_400_public_ros1_synthtime`
  - `ct_icp_hdl_400_public_ros1_synthtime`
  - `clins_hdl_400_public_ros1_synthtime`
- readiness/blocker 系: **1**
  - `ct_lio_public_readiness`

### 3.3 CT 系 manifest counts

| method | manifest 数 |
|--------|-------------|
| `ct_icp` | **14** |
| `ct_lio` | **3** |
| `clins` | **1** |

### 3.4 27 CLI methods と 27 manifest methods は一致した

CLI 実装済み 27 method:

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

現時点では **manifest 未接続の CLI method は無い**。  
`clins` はこの handoff 期間中に evidence 層へ接続済み。

つまり public-facing に「27 methods 対応」と言うだけでなく、**bench evidence 側も 27 methods 分まで到達した**。  
ただし window coverage は method ごとにまだ uneven なので、**“27 methods が同じ grid で完全比較できる” とはまだ言わない**こと。

---

## 4. 真実源泉の優先順位

### 4.1 最上位

1. [`experiments/results/index.json`](experiments/results/index.json)
2. 個別の [`experiments/results/*_matrix.json`](experiments/results)
3. [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
4. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
5. [`experiments/*_matrix.json`](experiments/)

### 4.2 二次資料

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)
- [`docs/paper_assets.md`](docs/paper_assets.md)
- [`docs/paper_tracks.md`](docs/paper_tracks.md)

### 4.3 stale と見なすべきもの

- [`README.md`](README.md)
- 一部 `docs/paper_*` の narrative 文

特に `README.md` の method 数や ready count は古い前提が残っている可能性が高い。

---

## 5. HDL-400 / CT 系の現在地

### 5.1 public ROS1 synthetic-time は separate bench としては成立している

**CT-LIO public ROS1 synthetic-time**

- manifest: [`experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- aggregate: [`experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- current default: `seed_only_fast`
- current ATE: `0.479317`
- current FPS: `19.561344`

**CT-ICP public ROS1 synthetic-time**

- manifest: [`experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- aggregate: [`experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- current dirty default: `dense_window`
- current ATE: `1.254043`
- current FPS: `16.612127`

### 5.2 last committed `e4d9af4` での CT synthetic-time defaults

`e4d9af4` に限ると:

| problem | default | ATE | FPS |
|---------|---------|-----|-----|
| CT-LIO public ROS1 synthetic-time | `seed_only_fast` | `0.479317` | `19.561344` |
| CT-ICP public ROS1 synthetic-time | `fast_window` | `2.464608` | `68.524052` |

つまり、**CT-ICP public ROS1 の性能優先 default 変更はまだ未コミット**である。

### 5.3 native-time reference 側の数字は二層ある

**`e4d9af4` 時点**

| problem | default | ATE | FPS |
|---------|---------|-----|-----|
| CT-LIO reference | `seed_only_fast` | `0.411644` | `0.443120` |
| CT-ICP reference | `fast_window` | `1.513295` | `2.414365` |

**current dirty worktree**

| problem | default | ATE | FPS |
|---------|---------|-----|-----|
| CT-LIO reference | `seed_only_fast` | `0.488244` | `17.477206` |
| CT-ICP reference | `fast_window` | `2.582133` | `72.912540` |

この差は、Copilot にとって最も危険な罠である。  
current worktree の reference aggregate は、昔の native-time canonical result として読んではいけない。

### 5.4 synthetic time は使ってよいか

結論:

- **使ってよい**
- ただし **exact reproduction と呼んではいけない**

OK:

- public ROS1 bag しか無い状況で、別 benchmark track として使う
- 同じ synthetic-time 契約の中で variant 比較をする
- public-only benchmark として regression を見る

NG:

- native per-point `time` 付き benchmark と同一物として扱う
- “original native-time benchmark を再現できた” と主張する

---

## 6. current dirty worktree の中身

いまの `git status --short` は、単なる数ファイル変更ではない。  
大きく 3 バッチある。

### 6.1 バッチ A: CT public ROS1 performance-priority

主ファイル:

- [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
- [`experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/index.json`](experiments/results/index.json)
- generated docs 群

意味:

- manifest-level `benchmark_weights` を導入
- public ROS1 CT-ICP だけ ATE 優先で default を選ぶ

### 6.2 バッチ B: 新規 method 群の profile expansion

主ファイル:

- [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
- `balm2 / fast_lio2 / fast_lio_slam / hdl_graph_slam / isc_loam / lins / lio_sam / loam_livox / point_lio / suma / vgicp_slam / xicp`
  の manifest 群
- 対応する aggregate / docs

意味:

- 新規群の `default / fast / dense` 拡張
- `hdl_graph_slam` は ready な短尺 problem では 3 variants 化済み
- ただし full-sequence 側は `kitti_raw_0009_full = default only` / `kitti_raw_0061_full = skipped` のまま

### 6.3 バッチ C: generated paper assets / docs

主ファイル:

- `docs/assets/paper/*`
- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_assets.md`](docs/paper_assets.md)
- [`docs/paper_comparison.md`](docs/paper_comparison.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)

意味:

- full refresh 後の再生成
- 単独で手編集すべきではない

### 6.4 Copilot への重要指示

この dirty worktree を **一括で revert しないこと**。  
少なくとも current owner の意図は複数バッチに分かれており、`git checkout -- .` のような破壊的操作は危険。

---

## 7. 重要な罠

### 7.1 `--manifest` 部分実行は `index.json` を壊す

`python3 evaluation/scripts/run_experiment_matrix.py --manifest ...`
は便利だが、その subset で [`experiments/results/index.json`](experiments/results/index.json) を上書きする。

対策:

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

を最後に必ず実行すること。

### 7.2 `skipped` と `blocked` は別

- blocked: bench 契約上まだ解けていない
- skipped: manifest は ready だが、その run で有効 result が無い

current:

- blocked = [`experiments/results/ct_lio_public_readiness_matrix.json`](experiments/results/ct_lio_public_readiness_matrix.json)
- skipped = [`experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`](experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json)

### 7.3 `paper_tracks.md` の FPS 列は default FPS ではない

[`docs/paper_tracks.md`](docs/paper_tracks.md) の table header は:

- `Best ATE [m]`
- `Best FPS`

であり、**default の FPS ではない**。

そのため、public ROS1 CT-ICP が `dense_window` default になっていても、
table には `68.5` のような別 variant 由来の `Best FPS` が出る。

default の実 FPS を見るべき場所は:

- [`docs/paper_assets.md`](docs/paper_assets.md)
- 個別 aggregate JSON

### 7.4 `dogfooding_results/` は commit 対象ではない

大きい raw data / local reconstructed data は `.gitignore` 前提。

commit 対象は:

- manifest JSON
- aggregate JSON
- generator / runner
- generated docs
- 小さい fixture

のみ。

### 7.5 current worktree の reference JSON を canonical native-time truth と見なすな

これは再度強調する。  
current dirty worktree の:

- [`experiments/results/ct_lio_reference_profile_matrix.json`](experiments/results/ct_lio_reference_profile_matrix.json)
- [`experiments/results/ct_icp_hdl_400_reference_matrix.json`](experiments/results/ct_icp_hdl_400_reference_matrix.json)

は、**昔の native-time artifact 数字そのもの**としては扱わないこと。

---

## 8. 何を public に言ってよいか

### 8.1 言ってよい

- public ROS1 HDL-400 synthetic-time split を separate bench として追加した
- CT-LIO public ROS1 synthetic-time は `seed_only_fast` で `ATE 0.479317`
- CT-ICP public ROS1 synthetic-time は current dirty worktree では `dense_window` で `ATE 1.254043`
- synthetic time は public-only benchmark 契約としては使える

### 8.2 言ってはいけない

- synthetic-time 結果で original native-time benchmark を exact reproduction した
- current dirty reference aggregate が canonical native-time numbers だ
- `paper_tracks.md` の FPS が default の FPS だ

---

## 9. 次にやるべきこと

### P0. dirty worktree の保全

これは **対応済み**。dirty worktree ベースだった差分は `wip/profile-expansion-refresh` へ commit/push され、draft PR #1 上で継続開発する形に移した。

### P1. `hdl_graph_slam` の full-sequence policy を決める

**決定:** full-sequence 2 本は truthfully 例外扱いで維持する。

現状と採択:

- manifest は 7 件ある
- `kitti_raw_0009 / kitti_raw_0061 / mcd_* x3` は `default / fast / dense` を持つ
- 現在の採択 default は
  - `kitti_raw_0009 = fast`
  - `kitti_raw_0061 = fast`
- `mcd_kth_day_06 = fast`
- `mcd_ntu_day_02 = dense`
- `mcd_tuhh_night_09 = dense`
- `hdl_graph_slam_kitti_raw_0009_full` は runtime の重さから **`default` のみ維持**
- `hdl_graph_slam_kitti_raw_0061_full` は計算量から **`skipped` 維持**

理由:

1. `0009_full` の fast/dense を無理に追加すると、短尺 ready set と同じ cadence で回せない
2. `0061_full` は現状でも 1 時間超級で、`skipped` を崩すだけの semantically equivalent な軽量 profile がまだ無い
3. したがって、**full-sequence だけは truthful outlier として残す** 方が、artifact の意味を壊さない

### P2. `clins` を evidence 層に載せる

これは **対応済み**。

現状:

- CLI 実装あり
- `experiments/clins_hdl_400_public_ros1_synthtime_matrix.json` を追加済み
- public ROS1 HDL-400 synthetic-time window で aggregate/result を生成済み
- 現在の採択 default は `dense`
- observed trade-off は
  - `default`: ATE `484.064284`, FPS `61.271053`
  - `fast`: ATE `350.051541`, FPS `100.269128`
  - `dense`: ATE `1.473223`, FPS `12.200311`

最初の 1 本は **IMU 付き public ROS1 HDL-400** で成立した。  
次にやるなら、KITTI Raw のような別 IMU dataset へ横展開するか、`clins` の profile 面をさらに増やす。

### P3. exact native-time provenance を回収する

もし本当に exact reproduction をやるなら必要なのは:

- `hdl_400_ros2` 系 source artifact
- あるいは同等の native per-point `time` を保持した元点群

synthetic time ではここは埋まらない。

### P4. stale public docs を掃除する

これは **対応済み**。対象だった narrative docs は current artifact に合わせて更新済み。

対象:

- [`README.md`](README.md)
- narrative 部分の `docs/paper_*`

---

## 10. 作業プレイブック

### 10.1 まず current snapshot を保存する

Copilot に引き継ぐ前に最小限やるなら:

```bash
git status --short
git diff --stat
```

### 10.2 full refresh

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 10.3 最低限の script 検証

```bash
python3 -m py_compile evaluation/scripts/run_experiment_matrix.py
python3 -m py_compile evaluation/scripts/extract_ros1_lidar_imu.py
```

### 10.4 full verification をやるなら

この turn では未実施。  
last known success は earlier turn。

```bash
ctest --test-dir build --output-on-failure -j"$(nproc)"
bash evaluation/scripts/smoke_ci_fixture.sh
```

---

## 11. 最初に見るべきファイル

### 11.1 bench / runner

- [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
- [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
- [`evaluation/scripts/refresh_study_docs.py`](evaluation/scripts/refresh_study_docs.py)

### 11.2 HDL-400 / CT 関係

- [`evaluation/scripts/extract_ros1_lidar_imu.py`](evaluation/scripts/extract_ros1_lidar_imu.py)
- [`evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md)
- [`experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_lio_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/ct_icp_hdl_400_public_ros1_synthtime_matrix.json)
- [`experiments/results/ct_lio_reference_profile_matrix.json`](experiments/results/ct_lio_reference_profile_matrix.json)
- [`experiments/results/ct_icp_hdl_400_reference_matrix.json`](experiments/results/ct_icp_hdl_400_reference_matrix.json)
- [`experiments/ct_lio_public_readiness_matrix.json`](experiments/ct_lio_public_readiness_matrix.json)

### 11.3 generated docs

- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/paper_tracks.md`](docs/paper_tracks.md)
- [`docs/paper_assets.md`](docs/paper_assets.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)

---

## 12. verification メモ

### 12.1 current turn で確認したこと

- `git status --short`
- `git log --oneline -6`
- `git rev-parse HEAD`
- current aggregate / index / manifest / summary counts
- Draft PR #1 の最新 check runs
  - `build-and-test = success`
  - `ros2-colcon-build = success`

### 12.2 earlier turn の最新成功記録

- `cmake --build build -j8`: **success**
- earlier turn でも `ctest --test-dir build --output-on-failure -j"$(nproc)"`: **38/38 passed**
- earlier turn でも `bash evaluation/scripts/smoke_ci_fixture.sh`: **success**
- `python3 evaluation/scripts/refresh_study_docs.py`: **success**
- `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing --manifest experiments/clins_hdl_400_public_ros1_synthtime_matrix.json`: **success**
- `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing --manifest experiments/hdl_graph_slam_kitti_raw_0009_matrix.json --manifest experiments/hdl_graph_slam_kitti_raw_0061_matrix.json --manifest experiments/hdl_graph_slam_mcd_kth_day_06_matrix.json --manifest experiments/hdl_graph_slam_mcd_ntu_day_02_matrix.json --manifest experiments/hdl_graph_slam_mcd_tuhh_night_09_matrix.json`: **success**

---

## 13. 最終 handoff

いまの repo は、ざっくり言うと次の状態にある。

- **統合はかなり進んでいる**
- **168 manifests / 166 ready / 1 blocked / 1 skipped** の bench index がある
- **public ROS1 synthetic-time** は separate benchmark として成立している
- しかし **exact native-time CT provenance** はまだ unresolved
- `hdl_graph_slam` full-sequence は **truthful exception policy** で固定した
- draft PR #1 (`wip/profile-expansion-refresh`) 上に、ここまでの更新が積まれている
- 現在の branch は **clean / pushed / CI green** で、Claude がそのまま引き継げる

GitHub Copilot に一番伝えるべきことはこれ:

1. **`e4d9af4` を clean boundary として覚えること**
2. **public ROS1 synthetic-time と exact native-time provenance は別問題であること**
3. **current reference aggregates を canonical native-time truth と見なさないこと**
4. **`paper_tracks.md` の FPS を default FPS と誤読しないこと**
5. **次の構造的な未完は、exact native-time provenance 回収であること**

### 13.1 Claude 向けの一段深い要約

Claude がこの repo を引き継ぐとき、まず理解すべきことは「何がもう片付いていて、何がまだ repo 内では片付かないか」である。

**もう片付いたこと**

1. `hdl_graph_slam` の短尺 ready set は 3 variants 化済み
2. `hdl_graph_slam` の KITTI full-sequence は policy 決着済み
   - `0009_full = default-only`
   - `0061_full = skipped`
3. `clins` は public ROS1 HDL-400 synthetic-time で evidence 層に載った
4. public-facing narrative docs の stale counts / stale wording は掃除済み
5. draft PR #1 は green で、作業ツリーも clean

**まだ片付いていないこと**

1. exact native-time provenance は repo 単独では回収できない
2. public ROS1 synthetic-time の前進は、native-time exact reproduction の代替にならない
3. current reference aggregates は benchmark artifact としては読めても、昔の canonical native-time numbers と同一視してはいけない

**Claude が次に読むべき具体ファイル**

1. [`experiments/results/index.json`](experiments/results/index.json)
2. [`docs/interfaces.md`](docs/interfaces.md)
3. [`docs/decisions.md`](docs/decisions.md)
4. [`experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json`](experiments/results/clins_hdl_400_public_ros1_synthtime_matrix.json)
5. [`experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json`](experiments/results/hdl_graph_slam_kitti_raw_0009_full_matrix.json)
6. [`experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json`](experiments/results/hdl_graph_slam_kitti_raw_0061_full_matrix.json)

**Claude が次にやるなら現実的な一手**

- repo 内で完結する作業だけ選ぶなら、`P3` の前段として「exact native-time provenance を主張するために最低限必要な source artifact 条件」を docs に明文化する
- repo 外 artifact まで追えるなら、`hdl_400_ros2` 系 source / native per-point `time` を保持した元点群の所在を確定する

**Claude がやってはいけないこと**

1. public ROS1 synthetic-time を exact native-time reproduction と言う
2. `paper_tracks.md` の FPS を default FPS と読む
3. subset `--manifest` 実行のあとに `refresh_study_docs.py` を省略する
4. `hdl_graph_slam` full-sequence に、意味の変わる lightweight profile を後付けして “3 variants あります” と見せる

もし Claude が次に 1 手だけ打つなら、最優先は **exact native-time provenance の source artifact 条件を明文化すること** である。  
synthetic-time 側が前進しても、ここは別に埋まらない。

---

*End of PLAN.md*
