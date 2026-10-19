# CT-ICP: Continuous-Time ICP for LiDAR Odometry

## 論文情報

- **著者**: Pierre Dellenbach, Jean-Emmanuel Deschaud, Bastien Jacquet, François Goulette
- **会場**: ICRA 2022
- **arXiv**: https://arxiv.org/abs/2109.12979
- **元リポジトリ**: https://github.com/jedeschaud/ct_icp (MIT License)

## 手法概要

従来のICPは1フレーム=1ポーズ(6DoF)だが、CT-ICPは1フレーム=2ポーズ(begin+end, 12DoF)で
スキャン取得中のセンサ運動を陽にモデル化。SLERP+線形補間でスキャン内の各点のタイムスタンプに
応じた連続時間ポーズを計算し、モーション歪みを最適化で直接解消する。

### 核心アルゴリズム

**連続時間補間**:
```
P(alpha) = slerp(q_begin, q_end, alpha), (1-alpha)*t_begin + alpha*t_end
alpha = (timestamp - t_begin) / (t_end - t_begin)
```

**コスト関数** (Point-to-Plane):
```
r = w * n^T * (p_ref - P(alpha) * p_raw)
```

**正則化**:
- 位置整合性: ||t_begin^k - t_end^{k-1}||
- 等速制約: ||t_end^k - t_begin^k - v_prev||

## 実装ノート

- ROS非依存の純粋C++実装
- Ceresの `AutoDiffCostFunction` + `EigenQuaternionParameterization`
- ボクセルハッシュマップによるO(1)近傍探索

## 上流実装との比較と `--ct-icp-upstream-mechanics` (2026-10-11)

公式実装 (jedeschaud/ct_icp, `config/odometry/driving_config.yaml`) を同じ KITTI
スキャンで動かすと KITTI 07 の公式 RTE は 0.308 % で、論文 Table I の 0.31 % と
一致した (`evaluation/scripts/run_official_ct_icp_baseline.py`)。従来の Table 6
の 4.03x の差は公式コードと論文の差ではなく、この実装側にあった。

`pcd_dogfooding --ct-icp-upstream-mechanics` は上流のパイプラインをそのままなぞる
専用ランナー (`runCTICPUpstream`) を使う。既定の `runCTICP` と CT-ICP ライブラリの
既定値は変えていないので、保存済みの結果と CI の golden はそのまま再現する。

| 項目 | 上流 driving_config | 既定の `runCTICP` | upstream mechanics |
|---|---|---|---|
| 点ごとの時刻 | 掃引角 (公式 KITTI PLY) | 点の格納順 j/N (KITTI .bin では仰角の順) | 掃引角 0.5 (1 - atan2(y, x) / π) |
| 入力の間引き | 0.5 m ボクセルからランダムに 1 点 (最初の 20 フレームは 0.2 m)、点数上限なし | 0.6 m ボクセル重心、700 点まで | 上流と同じ |
| 初期値 | begin / end とも等速外挿 | begin = end = 前フレームの end | 上流と同じ |
| 最初の 2 フレーム | 剛体 (全点を end 時刻に) | 連続時間 | 上流と同じ |
| マップ | 0.8 m ボクセル、30 点、点間 0.1 m、100 m より遠いボクセルを削除、永続 | 1.2 m ボクセル、20 点、直近 20 フレーム | 上流と同じ |
| 挿入 | フレーム内回転 3° 超ならスキップ (連続 5 回まで) | 毎フレーム | 上流と同じ |
| 近傍 | 半径 0.75 m 内の最近傍 20 点が必須 | 27 ボクセルで k = 5 | 上流と同じ |
| 平面上の点 | 20 点のうち最遠点 (max-heap を先頭から取り出すため) | 近傍の平均 | 上流と同じ |
| 平面性 a2D | 標準偏差 (√固有値) で計算、足切りなし | 固有値で計算、0.06 未満を棄却 | 上流と同じ |
| 重み | (0.9 a2D² + 0.1 exp(-距離 / 6 m)) / 1.0 | a2D | 上流と同じ |
| 残差数 | キーポイント順で先頭 900 個 | 全部 | 上流と同じ |
| 反復 / 停止 | 5 × 5 (立ち上がり 20 フレームは外側 15 回)、並進変化の和 < 0.01 m かつ回転変化の和 < 0.1° | 10 × 6、並進変化 < 0.001 m | 上流と同じ |
| Cauchy σ / 正則化 | 0.1 / 位置 0.001 + 等速 0.001 (√(残差数 β)) | 0.5 / 位置・姿勢・等速 0.001 | 上流と同じ |
| 出力ポーズ | スキャン中央 (alpha 0.5) | end | 上流と同じ |

上流 CT-ICP の KITTI ローダは仰角を +0.205° 補正する
([`docs/kitti_elevation_correction.md`](../../docs/kitti_elevation_correction.md))。
論文の値はこの補正後のスキャン (KITTI-corrected) のものなので、比較には
`--input-vertical-angle-correction-deg 0.205` を併用する。

### KITTI 00-10 (公式 KITTI RTE [%])

| Seq | この実装 補正なし | この実装 補正あり | 公式 CT-ICP 補正あり | 論文 Table I |
|---|---:|---:|---:|---:|
| 00 | 1.092 | 0.525 | 0.538 | 0.49 |
| 01 | 2.408 | 0.716 | 0.671 | 0.76 |
| 02 | 1.394 | 0.549 | 0.541 | 0.52 |
| 03 | 1.081 | 0.586 | 0.595 | 0.72 |
| 04 | 1.241 | 0.717 | 0.684 | 0.39 |
| 05 | 0.795 | 0.315 | 0.310 | 0.25 |
| 06 | 0.783 | 0.284 | 0.273 | 0.27 |
| 07 | 0.742 | 0.315 | 0.308 | 0.31 |
| 08 | 1.193 | 0.902 | 0.904 | 0.81 |
| 09 | 1.261 | 0.629 | 0.636 | 0.49 |
| 10 | 1.442 | 0.742 | 0.722 | 0.48 |
| 00-10 平均 | 1.210 | **0.587** | **0.585** | 0.531 |

平均は KITTI devkit と同じく全系列の 100-800 m 区間をまとめた平均 (各系列を区間数で
重み付け)。論文列は論文の系列ごとの値を同じ区間数で重み付けしたもの。この実装は
すべての系列で公式コードとの差が 0.05 ポイント以内で、00-10 平均は公式コードと一致する
(0.587 % と 0.585 %)。論文との差 (平均で 1.10x、04/05/09/10 で大きい) は公式コードにも
同じだけあるので、論文と公開コードの間の差であってこの実装の差ではない。補正なしの
スキャンでは 1.21 % と倍になり、CT-ICP は KISS-ICP (0.564 % → 1.053 %) と同じく補正に
強く依存する。

### 何が効いているか (KITTI 07、補正あり)

全部入りから 1 つずつ外した結果 (`--ct-icp-upstream-disable NAME`):

| 外した要素 | RTE [%] |
|---|---:|
| なし (upstream mechanics) | 0.317 |
| 等速初期値 (`cv-init`) | 43.6 (発散) |
| スキャン中央の出力ポーズ (`mid-pose`) | 0.418 |
| 掃引角の時刻 (`azimuth-time`、格納順に戻す) | 0.397 |
| 永続マップ (`persistent-map`、20 フレーム窓に戻す) | 0.382 |
| 平面性の標準偏差化 (`std-a2d`) | 0.357 |
| その他 (startup, rigid-start, radius, farthest-ref, nb-weight, residual-cap, stop, prune, insert-gate) | 0.305-0.344 |

公式コードの補正角に 1e-6° から 1e-5° を足して動かすと 0.296-0.317 % に散るので、
±0.02 ポイント程度の違いは誤差の範囲。決定的に効くのは等速初期値と出力ポーズの
定義で、ほかは個々には小さい。公式コードも格納順の時刻にすると 0.398 % になり、
この実装の `azimuth-time` を外した値 (0.397 %) と一致する。

### 再現性 (ホスト依存の原因)

KITTI 00 の Table 6 バリアントは、i5-1145G7 のホストで 1.664 %、このコンテナでは
ビルドによって 3.446 % と 2.041 % と大きく違っていた。同じホスト・同じバイナリでは完全に決定的 (繰り返し、OMP スレッド数、
`MALLOC_PERTURB_`、PCD と .bin の入力で同一、valgrind でもエラーなし) なので、
未初期化メモリやスレッドの競合ではない。原因は旧パイプラインの数値的な不安定さで、
入力に 1e-6° (80 m 先で約 1.4 µm) の仰角の摂動を加えるだけで結果が大きく動く:

| | KITTI 07 | KITTI 00 |
|---|---|---|
| 既定の `runCTICP` (Table 6 バリアント) | 1.03-1.18 % (±7 %) | 1.64-2.04 % (±11 %) |
| upstream mechanics (補正あり) | 0.314-0.335 % (±3 %) | - |
| 公式 CT-ICP (補正あり) | 0.296-0.317 % (±3.5 %) | - |
| 参考: KISS-ICP | 0.79-0.82 % (±2 %) | - |

ビルドやホストの違いで生じる浮動小数点の差がこの増幅で数十 % の差になる。保存値
1.664 % は摂動での散らばりの範囲に入っている。upstream mechanics は公式コードと
同程度に安定で、長い系列の CT-ICP の数値はこちらを使うべきである。

<!-- SHARED_NEAR_RANGE_AUDIT -->
## Shared-loader replay: historical KITTI 04 difference

After extracting the opt-in near-range filter, the full 04 stored-cell replay
(`upstream_mechanics_elevation`, original recorded arguments, cut still off)
has 100 m RPE **0.733247%** and ATE **2.191953 m**, versus stored **0.713006%**
and **2.184636 m**. All 271 frames complete. This is a negative reproduction
result: the historical audit exits 1 and the stored result is preserved.

The frozen pre-extraction executable produces the same replayed RPE/ATE,
frame count, official translation/rotation RTE and byte-identical saved poses.
Thus the difference predates this filter extraction; its historical cause
is still unresolved. This does not establish a new CT-ICP accuracy result,
attribute the difference to a particular compiler, or reselect a setting.

[Exact audit commands, exits, summaries and logs](../../experiments/results/shared_near_range/sequences/04/audit/),
[frozen replay](../../experiments/results/shared_near_range/sequences/04/frozen_baseline/),
[comparison](../../experiments/results/shared_near_range/sequences/04/frozen_comparison.json)
were committed at `7458c24d16096cc305889ed0864a5ef8bfb6bdf4`.
The [whole-zoo audit](../../experiments/results/shared_near_range/README.md)
is incomplete; the opt-in cut remains off by default.
<!-- /SHARED_NEAR_RANGE_AUDIT -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader replay: KITTI 01 frozen comparison complete

The full stored `upstream_mechanics_elevation` replay completes all 1101 frames with
native exit 0. Historical 100 m RPE is **0.826792%**, versus
**0.830115%** in this replay; ATE is **58.190972 m**
versus **57.923334 m**. RPE worsens; ATE
improves. The historical audit exits **1**, preserving the difference.

The frozen pre-extraction replay also completes with native exit 0. RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. This
historical difference predates extraction; its original cause remains unresolved.
The frozen historical audit also exits 1. No setting is reselected, and the
near-range cut remains off.

[Commands, exits, hashes and candidate results](../../experiments/results/shared_near_range/sequences/01/audit/)
were committed at `9555f01fda68a0294fc7dd5a828a85fac207f68f`. The
[study and cleaned checkpoint scope](../../experiments/results/shared_near_range/README.md)
now include the full [frozen comparison](../../experiments/results/shared_near_range/sequences/01/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/01/cleanup.json),
committed at `6188b71d8e48933a4dffce54afdde7a01382f4b0`. The cleaned
[04/00/01 checkpoint](../../experiments/results/shared_near_range/checkpoints/01/summary.json)
remains partial whole-zoo coverage.
<!-- /SHARED_NEAR_RANGE_REPLAY -->

<!-- SHARED_NEAR_RANGE_01_DIAGNOSTICS -->
### Saved KITTI 01 path diagnosis

The current path's largest position-error increase in the existing fixed
blocks is frames **700-800**:
**61.594297 to 81.754537 m**
against evaluated GT. This locates current error growth, not the historical
metric difference.

The referenced historical per-run summary is absent in the inspected tree
and was not found in the reachable history of this checkout. There is no
paired historical path here, so this cannot locate where that path differs.

The [diagnostic table, plot, CSVs, source pins and exact command](../../experiments/results/shared_near_range/diagnostics/01/README.md)
come from committed evidence `1426a851c9f021cbffae239e814b6379a5788b25`. First-pose normalization
is used without fitted alignment or rescaling. No estimator is rerun and no
published accuracy result is replaced.
<!-- /SHARED_NEAR_RANGE_01_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_02_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI02 difference and saved-path diagnosis

| Method | Seq | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| CT-ICP | 02 | 0.810338 | 0.813023 | 20.606805 | 19.687922 |

The selected `upstream_mechanics_elevation` arguments are unchanged throughout this full
sequence. The historical audit retains exit **1**; native executions exit **0**.
Frozen pre-extraction controls have zero recorded metric differences and
byte-identical poses. This historical difference predates extraction; its
original cause remains unresolved. Input hashes were verified before owned
scan/alias removal. The near-range cut stays disabled.

The largest current position-error increase among the inherited fixed blocks
is frames **4400-4500**:
**19.888348 to 30.148291 m**.

The referenced historical per-run path is unavailable in the inspected tree/history.
Current error growth cannot locate a gap from that unavailable trajectory.

The [protocol, all steps/CSVs and plot](../../experiments/results/shared_near_range/diagnostics/02/README.md)
come from committed evidence `8f5841e3946995cedfa9c0e2fc484dbd2c02373e`. First-pose normalization uses no fitted
alignment or rescaling. These observations locate errors without assigning a
cause or replacing published accuracy. The
[cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/03/summary.json)
retains all historical differences and audit exits.
<!-- /SHARED_NEAR_RANGE_02_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_04_DIAGNOSTICS -->
## Saved KITTI04 historical difference

| Method | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---:|---:|---:|---:|
| CT-ICP | 0.713006 | 0.733247 | 2.184636 | 2.191953 |

Both metrics worsen. The historical audit retains exit **1**. The frozen
pre-extraction executable has zero recorded metric differences and byte-identical
saved poses, so this difference predates extraction; its original cause remains
open. Owned scans and alias were removed after input hash verification.

The largest current GT position-error increase in the inherited fixed blocks
is frames **0-100**:
**0.000000 to 1.537809 m**.

The referenced historical per-run path is unavailable in the inspected tree and
reachable-history probe. This current error-growth interval cannot locate the
difference from that missing path.

[Protocol, every step/block, plots and actual commands](../../experiments/results/shared_near_range/diagnostics/04/README.md)
come from raw evidence `4cb636eedcd614cb15e2b63121bb39d5966e55bb`, committed before this prose.
The diagnostic parameters transfer unchanged from 02/01. Native settings and
published results are preserved; the near-range cut remains disabled.
<!-- /SHARED_NEAR_RANGE_04_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_05_DIAGNOSTICS -->
## Shared-loader replay: committed KITTI05 difference

The selected historical variant `upstream_mechanics_elevation` completes **2761**
frames with native exit **0**, using its original arguments unchanged.

| Metric | Historical | Replay |
|---|---:|---:|
| 100 m RPE [%] | 0.469581 | 0.469523 |
| ATE [m] | 4.264453 | 4.079794 |

RPE improves; ATE improves. The historical audit retains exit **1**. The frozen
pre-extraction control has **0** differences in RPE, ATE, frames and official
translation/rotation RTE, with byte-identical saved poses. The difference
therefore predates extraction; its original cause remains unresolved.

The current first-pose-normalized position error grows most in inherited block
**2600-2700**, from **3.264133 to 7.011568 m**.
This locates current error growth against GT, not its cause or a change in
historical accuracy. It does not replace official KITTI RTE.

The referenced historical summary/path is absent in the inspected tree and was
not found by its exact-path reachable-history probe. No paired historical path
is available here; the current trace cannot explain the historical difference.

[Raw replay, receipts and verified cleanup](../../experiments/results/shared_near_range/sequences/05/),
[all diagnostic blocks, traces, commands and limits](../../experiments/results/shared_near_range/diagnostics/05/README.md),
and the [cleaned checkpoint](../../experiments/results/shared_near_range/checkpoints/05/summary.json)
retain all values and actual exits. Native source remains `0b079dbb`; current-main
acceleration/passive tracing is outside this frozen replay's scope. No estimator,
registration setting, default or published result changes. Raw diagnosis
`6285c60b` precedes this prose; active06 scans and executables are untouched.
<!-- /SHARED_NEAR_RANGE_05_DIAGNOSTICS -->
