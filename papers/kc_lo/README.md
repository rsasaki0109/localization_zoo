# KC-LO

Kernel Correlation (対応点を取らない / correspondence-free) LiDAR オドメトリ。
**A Correlation-Based Approach to Robust Point Set Registration** (Y. Tsin &
T. Kanade, ECCV 2004) のカーネル相関機構を scan-to-map LiDAR オドメトリへ適用した
from-paper 再構成。原論文は対点群レジストレーションのみで、本リポジトリ向け
オドメトリパイプラインとしての公開実装は無い。

## 動機

ICP は最近傍**対応点**を取り、その誤差を最小化する (硬い割当)。GMM-EM は潜在割当の
事後確率 (soft assignment)、NDT はボクセル Gaussian への当てはめ。カーネル相関は
これらと異なり、**対応点を一切取らず**、2 点群を確率密度 (KDE) とみなしてその
**相関 (= Renyi 二次エントロピー)** を最大化する。割当の曖昧さに起因する局所解を避け、
σ を粗→細にアニーリングすることで広い収束域を得る。

## アプローチ

変換後スキャン X とモデル Y の総親和度 (カーネル相関) を最大化する:

```
C(θ) = Σ_i Σ_j exp(−||T(θ)x_i − y_j||² / 2σ²)
```

これは結合点群密度の Renyi 二次エントロピー最小化に等しい。各反復で各スキャン点 x_i を
近傍モデル点の**親和度重み平均** `ȳ_i = Σ_j w_j y_j / Σ_j w_j` (w_j=exp(−d²/2σ²)) へ
引き寄せる soft な point-to-point 更新を Gauss-Newton で解く (総親和度 Σw_j で重み付け、
低密度点の寄与を下げる)。σ は `σ_init → σ_min` に毎反復アニーリング (coarse-to-fine)。

## パラメータ

- `kc_sigma` (既定 0.5): 最終 (最小) カーネルスケール σ_min [m]。
- `kc_sigma_init` (既定 2.0): 初期 (粗) カーネルスケール σ_init [m]。
- `kc_anneal` (既定 0.7): 毎反復 σ ← max(σ*anneal, σ_min)。
- `kc_radius_factor` (既定 3.0): 探索半径 = factor*σ。
- `voxel_size` (既定 1.0) ほか共通。

CLI: `--kc-lo-{fast,dense}-profile`, `--kc-lo-sigma <σ_min>`,
`--kc-lo-sigma-init <σ_init>`, `--kc-lo-no-anneal` (σ_init=σ_min 固定)。

## テスト

`test_kc_lo` の 3 ケース:

1. `KernelWeightDecaysWithDistance` — ガウス親和度が距離 0 で 1、指数減衰、σ で寛容。
2. `WeightedMeanPulledToNearbyDensity` — 親和度重み平均が近傍密度に支配される。
3. `RecoversForwardTranslation` — 対応点を取らず前進並進を回復する。

## KITTI full での所見

KITTI full (no GT seed, `--kc-lo-dense-profile`):

| seq | RPE drift | ATE | FPS | 参照 KISS-ICP (同データ) |
|---|---:|---:|---:|---|
| 00 (4541 fr) | **0.837%** | 13.40 m | 2.6 | 0.872% / 12.0 m |
| 07 (1101 fr) | **0.510%** | 0.86 m | 3.1 | 0.618% / 1.8 m |

**所見 (positive)**: KC-LO は **両 seq で KISS-ICP の drift (RPE) を下回り**、
**seq07 RPE 0.510% は leaderboard 全体の新トップ** (これまでの Adaptive-ICP 0.569% を
上回る)。seq07 ATE 0.83 m も KISS-ICP 1.8 m を大きく下回る。seq00 は RPE↓ だが ATE は
13.4 m と KISS-ICP 12.0 m をやや上回る (honest な RPE↓/ATE↑ split)。

「整備済み KITTI では全機構が point-to-plane に退化する」という本キャンペーンの恒例
パターンを破る数少ない例。対応点を取らない密度相関は、離散最近傍対応のノイズ・誤対応に
頑健で、結果的に低 drift を達成する。σ schedule は下の ablation で分離する。

**代償は速度**: カーネル相関は各点で多数の近傍モデル点の親和度を総和するため低速。
ただし KITTI full の constant-velocity 予測下では、coarse-to-fine annealing を切って
σ=0.4 m 固定にしても RPE は 1% 未満の差で、速度は約 1.9-2.2 倍になる。

### Sigma schedule ablation

| seq | Annealed σ 1.5→0.4 | Fixed σ=0.4 (`--kc-lo-no-anneal`) | Δ RPE | Speed-up |
|---|---:|---:|---:|---:|
| 00 | 0.841902% / 1.39 FPS | **0.837450% / 2.65 FPS** | -0.53% rel | 1.90× |
| 07 | 0.514327% / 1.39 FPS | **0.509921% / 3.12 FPS** | -0.86% rel | 2.24× |

Honest interpretation: the paper's correspondence-free kernel-correlation
mechanism is the positive signal on KITTI; σ annealing is a convergence-safety
knob. With strong frame-to-frame velocity prediction, the fixed fine kernel is
already inside the basin and is preferable for this protocol.

Raw artifacts:

- [`seq00_kc_lo.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_kc_lo.json)
  and [`seq00_kc_lo_no_anneal.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_kc_lo_no_anneal.json)
- [`seq07_kc_lo.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_kc_lo.json)
  and [`seq07_kc_lo_no_anneal.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_kc_lo_no_anneal.json)
- Paired summary:
  [`kc_lo_sigma_schedule_ablation.json`](../../docs/benchmarks/kitti_full_new_methods/kc_lo_sigma_schedule_ablation.json)

```
pcd_dogfooding <seq_pcd_dir> <seq_gt_csv> \
  --methods kc_lo --kc-lo-dense-profile --no-gt-seed \
  --summary-json results.json

# fixed-sigma ablation; put --kc-lo-no-anneal after --kc-lo-dense-profile
pcd_dogfooding <seq_pcd_dir> <seq_gt_csv> \
  --methods kc_lo --kc-lo-dense-profile --kc-lo-no-anneal --no-gt-seed \
  --summary-json results_no_anneal.json
```
