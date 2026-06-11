# Quadric-LO

From-paper reimplementation of **Quadric Representations for LiDAR Odometry,
Mapping and Localization** (Xia et al., IEEE Robotics and Automation Letters
2023, arXiv:2304.14190). 公開実装は確認できず、本実装は論文記述からの独自再構成。

## 動機

点群や平面はシーンの曲面構造 (木の幹・柱=円柱、丸い構造物=球/楕円体) を忠実に
表せない。二次曲面 (quadric) は平面を退化として含むより一般的な表現で、曲面を
少ないパラメータでコンパクトに記述できる。

## アプローチ

シーンを二次曲面の implicit function で表現する:

`q(x) = xᵀ A x + bᵀ x + c`  (A は対称 3×3)

1. **Quadric 当てはめ** — マップの局所近傍 (中心化座標 `y = x − μ`) に対し、
   単項式 `m(y) = [y_x², y_y², y_z², 2y_xy_y, 2y_xy_z, 2y_yy_z, y_x, y_y, y_z, 1]`
   を使い `M = Σ m(y) m(y)ᵀ` (10×10) を作り、その **最小固有ベクトル** を係数
   `θ = [A, b, c]` とする (代数当てはめ; 当てはめ残差最小)。

2. **Point-to-quadric 拘束** — ソース点 `p_w` の **Taubin 距離**
   `d = q(y) / ‖∇q(y)‖`, `∇q = 2A y + b` を残差として最小化する。単位勾配
   `g = ∇q/‖∇q‖` が局所面法線となり、ヤコビアン `Jr = gᵀ [−skew(p_w), I]`。

3. **平面フォールバック** — 近傍が `quadric_min_neighbors` 未満で quadric が
   安定に立たない場合は局所 PCA の point-to-plane に退避 (平面は quadric の退化)。

Cauchy ロバスト核併用。IMU 事前積分は KITTI には無いため等速予測 prior で代替。

## パラメータ

- `quadric_min_neighbors` (既定 14): quadric 当てはめに要する近傍点数。
- `planarity_threshold` (既定 0.4): 平面フォールバックの平面性下限。
- `--quadric-lo-no-plane-fallback` (dogfooding CLI): quadric が立たない対応を
  point-to-plane に退避せず、最適化から外す ablation。
- `quadric_weight` (既定 1.0): quadric 拘束の相対重み。
- `min_grad_norm` (既定 1e-3): Taubin 距離の勾配ノルム下限 (退化 quadric の弾き)。
- `robust_scale` (既定 1.0): Cauchy 核スケール。
- `prior_precision` (既定 0.0): 等速予測への prior。

## テスト

`test_quadric_lo` の 3 ケース:

1. `MonomialMatrixRoundtrip` — 単項式 q=m·θ と行列形 yᵀAy+bᵀy+c の一致。
2. `RecoversKnownTranslation` — 平面+円柱シーンで既知並進を回復。
3. `UsesQuadricConstraints` — 円柱の曲面領域で quadric 拘束が使われる。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では quadric 当てはめが大半の対応で成立し、
point-to-quadric 拘束が使われる (`num_quadric` / `num_plane` で確認可能)。
既定の fallback-on では seq00 RPE 0.867%、seq07 RPE 0.598%。平面フォールバックは
seq00 で約 16/2779 corr/frame (0.58%)、seq07 で約 14/2757 corr/frame (0.51%) と少ない。

Plane-fallback off ablation (`--quadric-lo-no-plane-fallback`) では、quadric が立たない対応を
最適化から外す。seq00 は RPE 0.880%、ATE 12.9m、FPS 1.13、seq07 は RPE 0.590%、
ATE 2.15m、FPS 1.06。RPE は fallback-on から約 ±1.5% 以内で、速度は 1.6-1.8倍に上がる。
したがって KITTI evidence は plane fallback ではなく point-to-quadric path が支配的。
T0 昇格には、曲面物体が多い dataset または non-urban dataset での確認が残る。

Raw paired artifact:
[`quadric_lo_plane_fallback_ablation.json`](../../docs/benchmarks/kitti_full_new_methods/quadric_lo_plane_fallback_ablation.json).
