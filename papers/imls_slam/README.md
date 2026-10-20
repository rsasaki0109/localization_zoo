# IMLS-SLAM

Paper-path interfaces: [Python API and examples](../../python/README.md#stateful-paper-pipelines),
[ROS 2 nodes](../../ros2/localization_zoo_ros/README.md#paper-odometry-nodes).
[Native fixture checks](../../experiments/results/paper_pipeline_bindings/README.md).

The initial ROS interface trial observed 2/3 odometry messages before its
per-scan deadline with best-effort input. The recorded reliable-input trial
completed 3/3 and matched the native pipeline; commands and both trials are
linked above. This short interface check retains the benchmark evidence below.

暗黙的移動最小二乗 (Implicit Moving Least Squares, IMLS) 曲面への scan-to-model
マッチングによる LiDAR オドメトリ。**IMLS-SLAM: scan-to-model matching based on
3D data** (J.-E. Deschaud, ICRA 2018, arXiv:1802.08633) の from-paper 再構成。
著者は後発の **CT-ICP** は公開したが、IMLS-SLAM 本体の実装は未公開。本リポジトリ
向けオドメトリパイプラインとしての公開コードも無い。

## 動機

多くの LiDAR オドメトリは、新スキャンの各点を局所近傍の単一平面 (PCA 平面) に当てる
point-to-plane で解く。これは曲面を区分平面で近似するため、近傍点の取り方に敏感で、
スパースな点群では面の表現が粗い。IMLS は過去スキャンの**有向点群が定義する滑らかな
暗黙的曲面**に点を当てることで、複数の有向点を距離で重み付け補間した高次の局所曲面
表現を得る。単一最近傍平面 (mcc_lo / 既存の robust 系) や voxel-surfel
(CT-VoxelMap / R-VoxelMap) とは別系統の幾何当てはめ機構。

## アプローチ

過去 N スキャンの有向点群 `{p_i, n_i}` が暗黙的曲面

```
I^P(x) = Σ_i W_i(x) (x − p_i)·n_i / Σ_i W_i(x),   W_i(x) = exp(−||x − p_i||² / h²)
```

を定義する。`I^P(x)` は点 `x` の符号つき曲面距離で、零等位面が再構成曲面。新スキャン
のサンプル点 `x_j` を曲面へ投影 `y_j = x_j − I^P(x_j)·n_j` し、

```
min_{R,t} Σ_j |n_j·(R x_j + t − y_j)|²
```

を小角近似の Gauss-Newton で解く (point-to-implicit-surface)。`n_j` は最近傍モデル
点の法線。残差は曲面距離 `I^P` そのもので、法線方向の point-to-plane として線形化する。

### 観測性ベースのサンプリング

全点ではなく、**6-DoF を最もよく拘束する点**を選んで解く。各点の法線 `n` と位置 `p`
(センサ原点基準) から、並進拘束 `|n·e_a|` と回転拘束 `|(p×n)·e_a|` を 3 並進軸 +
3 回転軸の 6 リストで評価し、各リスト上位 `samples_per_axis` 点の和集合を採る。これに
より、ある方向に偏ったスキャンでも稀少な拘束点を取りこぼさず、効率と条件数を改善する
(原論文の 9-score サンプリングの近似)。

## パラメータ

- `imls_h` (既定 0.5 m): IMLS カーネルバンド幅 `h`。
- `imls_support_radius` (既定 0 → `3*imls_h`): 曲面を定義する支持半径。
- `max_correspondence_dist` (既定 1.0 m): 最近傍点ゲート `r`。
- `samples_per_axis` (既定 100): 各観測性リストから採る点数 (原論文 `s=100`)。
- `use_observability_sampling` (既定 true): false で全有効点を使用 (`--imls-slam-no-sampling`)。
- `voxel_size` (既定 1.0), `normal_radius` (既定 0 → `1.5*voxel_size`) ほか共通。

**密度に関する注意**: 原論文は生 HDL64 密度向けに `h=0.06` / `r=0.20` を用いるが、
本実装はマップをボクセルダウンサンプル (`voxel_size` 既定 1.0 m) するため、`h` と `r`
をマップ密度に合わせて拡大している。`--imls-slam-h` でアブレーション可能。

## テスト

`test_imls_slam` の 3 ケース:

1. `ImplicitSurfaceDistanceMatchesPlane` — 平面モデルへの IMLS 距離が高さに一致し、
   遠近の有向点を距離で重み付けする (Moving Least Squares の局所性)。
2. `ObservabilitySamplingCoversAllAxes` — 1 方向に偏ったスキャンでも各軸を拘束する
   稀少点を拾い、3 並進軸が代表される。
3. `RecoversForwardTranslation` — scan-to-implicit-model で前進並進を回復する。

## KITTI full での所見

KITTI full (no GT seed, `--imls-slam-dense-profile`):

| seq | RPE drift | ATE | FPS | 参照 KISS-ICP (同データ) |
|---|---:|---:|---:|---|
| 00 (4541 fr) | 1.000% | 17.6 m | 7.4 | 0.872% / 12.0 m |
| 07 (1101 fr) | 0.700% | 3.04 m | 9.3 | 0.618% / 1.8 m |

**正直な所見 (honest negative)**: IMLS-SLAM は両 seq で KISS-ICP を僅かに下回る。
整備済み・幾何リッチで CV 予測が効く KITTI では、ボクセルダウンサンプルした局所
マップ上の暗黙的 MLS 曲面 (`h=0.4`, voxel 0.8) は実質 point-to-plane に退化し、
KISS-ICP の適応 point-to-plane を上回らない。原論文の 0.4–0.7% drift は**生 HDL64
密度**の稠密曲面 (ボクセル化なし) + 900 点観測性サンプルによるもので、本パイプライン
の密度では再現しない。

**観測性サンプリングは有効**: seq07 で全点 (2023 点) を使っても RPE 0.701% / ATE
3.09 m と、サンプリング (807 点) の 0.700% / 3.04 m と同等の精度を保ちつつ FPS は
2 倍 (9.3 vs 4.3)。精度差は under-sampling 起因ではなく、6-DoF を拘束する点を選べて
いることを示す (`--imls-slam-no-sampling` でアブレーション可能)。

```
pcd_dogfooding <seq_pcd_dir> <seq_gt_csv> \
  --methods imls_slam --imls-slam-dense-profile --no-gt-seed \
  --summary-json results.json
```

## 論文どおりのオドメトリ (`--imls-slam-paper-odometry`)

上の「正直な所見」の推測どおり、差は手法ではなくハーネス側の運用にあった。論文 Sec. III-VI の
処理をそのまま追う経路 (`ImlsPaperOdometry`, `include/imls_slam/imls_paper_odometry.h`) を
別に用意した。

- **入力**: 生スキャン (ボクセル化・点数上限なし) に、論文と同じ **0.22° の仰角補正**
  (`--input-vertical-angle-correction-deg 0.22`)。
- **モデル**: 直近 n スキャンの生点をそのまま積んだスライディングウィンドウ (最古のスキャンを
  落とす)。r = 0.20 m のハッシュ格子でセル内点数の上限なし。従来経路は 0.8 m セルに最初の
  20 点だけを残し、以後の点を捨てていた。
- **IMLS**: h = 0.06 m、r = 0.20 m。法線は反復ごとにクエリ点の周囲 r 内のモデル点から PCA で
  求め、近傍全点に同じ法線を使う (論文 p.4)。
- **サンプリング**: スキャン点ごとの PCA 法線と平面度 a2D = (σ2 − σ3)/σ1 から、
  a2D² (x × n)·e (3 軸 × 符号) と a2D² |n·e| の 9 リストを作り、各反復で上から順に
  「モデルに r 以内の点がある」ものを 100 点ずつ採る (重複も残す、計 900 点)。
- **最適化**: y = x − I(x) n への点-平面最小二乗を小角近似で 20 回固定。予測は等速。
- 未実装: 小物体除去 (論文 Table II で 0.58 → 0.55 %)。

### KITTI Odometry (公式 RTE, GT seed なし)

`experiments/imls_slam_kitti_seq_{00,07}_full_paper_comp_matrix.json`:

| variant | seq 00 | seq 07 | FPS |
|---|---:|---:|---:|
| 従来 `--imls-slam-dense-profile` | 1.378 % | 0.846 % | 5-6 |
| 論文どおり, 窓 n = 100 (`paper_odometry_w100`) | **0.523 %** | **0.339 %** | 1.1 |
| 論文どおり, 窓 n = 10 (`paper_odometry_w10`) | 0.529 % | 0.378 % | 1.7 |
| 窓 n = 10, 仰角補正なし (ablation) | 1.327 % | 0.706 % | 1.7 |
| 論文 Table I | 0.50 % | 0.33 % | |

窓 100 は論文の 1.05x / 1.03x (幾何平均 1.04x)。仰角補正を外すと 2 倍以上悪化するので、
差の大半は「疎なボクセル地図」と「補正なし」の 2 点で説明できる。窓 10 → 100 の差は論文
Table IV (n = 10: 0.56 %, n = 100: 0.55 %) と同じく小さい。

### KITTI 00-10 (通常設定の選択は 07、粗い初期段の導入は 04)

窓 100 の設定を 00-10 で走らせると、04 だけが 11.93 % (36x) で崩れる。04 は最初のスキャンで
既に 1.3 m/scan で走っており、等速予測はゼロから始まる。サンプルはモデル点が r = 0.2 m 以内に
ある点だけが採られ、リストは s 点集まるまで下へ辿られるので、予測を直すはずの点
(進行方向を横切る面) が飛ばされ、推定はおよそ 60 スキャン短いままになる。最初の 1 回の
位置合わせだけ粗い段 (r = 1.0 m, h = 0.3 m, 10 反復) を先に入れると (論文にはない、
`--imls-slam-paper-coarse-frames 1`) 04 は 0.425 % になる (1 / 3 / 10 フレームで同じ)。
この初期段の実行順序・登録回数の制御は `common/initial_registration.h` に移し、
SuMa と共有した。IMLS の半径・帯域幅・反復数・対応付けと更新順序は維持し、
SuMa は自身のソルバーを使う。粗い段は論文にはなく、既定オフのままである。
通常設定は 07、粗い初期段の導入は 04 の失敗を見て決めたので、04 は取り置き評価ではない。
[共通化の監査と SuMa 転用実験](../../experiments/results/shared_coarse_start/README.md)
は別に記録し、以下の既存結果は更新しない。

<!-- SHARED_COARSE_AUDIT:START -->
00–10 の既存採用セルを IMLS-SLAM・SuMa 各 11 件、計 22 件照合し、RPE・ATE・
フレーム数・公式並進／回転 RTE の差はすべて 0 だった
([全監査集計](../../experiments/results/shared_coarse_start/report/summary.json))。
共通化前の固定バイナリとの full 04 比較も記録した
([比較](../../experiments/results/shared_coarse_start/sequences/04/frozen_comparison.json))。
監査対象は初期段ヘルパーを使うこの 2 手法であり、全手法の再実行ではない。
<!-- SHARED_COARSE_AUDIT:END -->
00 / 07 は変わらない。`experiments/imls_slam_kitti_seq_NN_full_paper_comp_matrix.json`、
公式 RTE [%]:

| Seq | Paper | `paper_odometry_w100` | `paper_odometry_w100_coarse_start` | Ratio |
|---|---:|---:|---:|---:|
| 00 | 0.50 | 0.523 | 0.523 | 1.05x |
| 01 | 0.82 | 0.888 | 0.842 | 1.03x |
| 02 | 0.53 | - | 0.522 | 0.98x |
| 03 | 0.68 | 0.771 | 0.770 | 1.13x |
| 04 | 0.33 | 11.928 | 0.425 | 1.29x |
| 05 | 0.32 | - | 0.292 | 0.91x |
| 06 | 0.33 | 0.404 | 0.360 | 1.09x |
| 07 (chosen) | 0.33 | 0.339 | 0.339 | 1.03x |
| 08 | 0.80 | - | 0.844 | 1.05x |
| 09 | 0.55 | - | 0.515 | 0.94x |
| 10 | 0.53 | - | 0.571 | 1.08x |

Geometric mean over 11 sequences: 1.05x.

## Optional execution acceleration

`--paper-pipeline-accelerated` enables repository implementation optimizations
for `--imls-slam-paper-odometry`. It is not a processing step from the paper and
is **off by default**. It reuses cell-neighbour topology and repeated model
queries within an iteration, and parallelizes normals, list sorting and queries
with OpenMP. Duplicate samples and the original list/point accumulation order
are retained; scan/window sizes and registration parameters are unchanged.

The initial candidate (`fe607199`) took 79.561 s against 160.513 s for the
frozen baseline on the first 120 KITTI 07 frames (**2.02x**). The changed
binary's default path took 194.906 s. Both saved trajectories match the control
exactly. This development-window attempt falls short of the speed target; it
is not a full-sequence claim. Raw evidence and exact commands were committed in
`8fe732a7` under
[`candidates/fe607199/`](../../experiments/results/paper_pipeline_speed/candidates/fe607199/).
Model-cell topology is now cached per list across iterations of an immutable
model, including absent cells and the original traversal order. Caches are
recreated for each coarse/fine registration call; they never retain projected
values across iterations or model changes.

Repeated development windows (first 120 KITTI 07 frames, four workers,
profiler disabled; every source and exact command is in the linked evidence):

| Source | Frozen control [s] | Changed default [s] | Opt-in [s] | Control / opt-in |
|---|---:|---:|---:|---:|
| `fe607199` | 160.513 | 194.906 | 79.561 | 2.02x |
| `fe03ed88` | 115.240 | 157.179 | 46.903 | 2.46x |
| `67867d0b` | 118.768 | 173.868 | 78.127 | 1.52x |

All saved poses agree with the frozen control at the saved precision (nine
significant digits). Execution time varies substantially, including between
unchanged method implementations. These windows do not establish the full
speed target. The primary full comparison uses fresh immediately preceding
controls. Source `67867d0b` raw screening is committed in `f3c6b9f3`.

[Execution protocol and CPU profiles](../../experiments/results/paper_pipeline_speed/README.md)
and [full comparison](../../experiments/results/paper_pipeline_speed/comparisons/f4755573.json).

Full KITTI 07 at measured candidate `f4755573` and control `134ead2f`,
identical selected registration arguments, four OpenMP workers, one method at a
time on the same recorded host; the profiler is disabled. These times belong
to those revisions and are not remeasured by the later shared-component integration:

| Execution | Time [s] | FPS | Official RTE [%] | ATE [m] |
|---|---:|---:|---:|---:|
| Paired frozen control | 1626.28 | 0.677 | 0.339406 | 0.693085 |
| Opt-in acceleration | 808.42 | 1.362 | 0.339406 | 0.693085 |

Measured speedup: **2.01x**. Maximum translation difference from the
paired frozen control: 0.000000000 m.
Pose differences use the saved KITTI matrices (nine significant digits).
Official RTE agrees to the third decimal.

**Negative result:** this attempt does not meet the roadmap speed target.
The option remains off by default.
The roadmap item is still open.

KITTI 07 guided implementation and is not held out. This comparison uses a
frozen-source full control run immediately before acceleration on the same host
and thread count. Historical
Table 6 results retain their original evidence and are not replaced.

## Acceleration integration with shared components

The current-main compatibility evidence, committed in `7619f198`, compares
frozen main, merged default and merged global-flag modes on the same 3 real
MCD scans with the historical fixed arguments. Both merged modes have zero
recorded accuracy/frame differences from main and byte-identical saved poses.
The existing coarse startup is retained through the shared policy; acceleration
stays opt-in and default off.
These short executions check compatibility; they do not establish long-segment
KITTI RTE or remeasure full-sequence throughput. The speed figures above remain
tied to their recorded sources. [Commands, binary/input hashes, comparison and
limits](../../experiments/results/paper_pipeline_speed/README.md#integration-with-merged-shared-components).

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored IMLS-SLAM full-sequence cells complete with native exits 0. 11 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
