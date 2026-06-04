# PCR-DAT

From-paper reimplementation of **PCR-DAT: a new point cloud registration method
for lidar inertial odometry via distance and Gauss distributed**
(Intelligent Service Robotics, 2024, DOI: 10.1007/s11370-024-00517-6).
著者による公開実装は無く、本実装は論文記述からの独自再構成。

## 動機

伝統的な ICP / NDT は、特徴が豊富な領域と疎な領域が混在するシーンで局所最適に
陥りやすい。NDT 系の分布ファクタは点が密な面では強力だが疎な点では分布が立たず、
逆に点間距離ファクタは疎な点で機能するが密な面では情報を活かしきれない。

## アプローチ

各ソース点について局所点密度を見て、ファクタを **per-point に切り替える**:

- **疎(sparse)な特徴** (近傍点が少ない) → **距離ファクタ**
  point-to-plane 残差 `n·(p_w − μ)` (法線が立たなければ point-to-point)。
- **豊富(rich)な特徴** (近傍点が多く面分布が立つ) → **分布ファクタ**
  Mahalanobis 残差 `(p_w − μ)ᵀ Σ⁻¹ (p_w − μ)` (GICP/NDT 系 point-to-distribution)。
  共分散 `Σ` は NDT 流に正則化 (最小固有値を λmax の比で下限化)。

両ファクタを 1 つの Gauss-Newton 系に統合する。論文の IMU 事前積分ファクタは
KITTI には IMU が無いため等速予測 prior (`prior_precision`) で代替する。

左摂動ヤコビアン `J = [−skew(p_w), I]` を共通に用い、分布ファクタは
`Jᵀ Σ⁻¹ J`、距離ファクタは `wᵢ Jrᵀ Jr` を蓄積する。Cauchy ロバスト核で外れ値を抑制。

## パラメータ

- `distribution_min_points` (既定 8): 近傍点数がこれ以上で rich → 分布ファクタ。
- `distribution_planarity` (既定 0.1): rich 判定の平面性下限。
- `distance_planarity` (既定 0.4): 距離ファクタで point-to-plane を使う平面性下限。
- `distance_sigma` / `distance_weight` / `distribution_weight`: 各ファクタの重み。
- `distribution_regularization` (既定 0.01): 共分散の最小固有値下限比。
- `robust_scale` (既定 1.0): Cauchy 核スケール。
- `prior_precision` (既定 0.0): 等速予測への prior (IMU 事前積分ファクタ代替)。

## テスト

`test_pcr_dat` の 3 ケース:

1. `RecoversKnownTranslation` — 良条件シーンで既知並進を回復。
2. `UsesBothFactorTypes` — 密な壁面=分布、孤立点=距離に振り分けられ、両方使われる。
3. `DistanceOnlyStillConverges` — rich 判定を無効化しても距離ファクタのみで収束。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では、距離/分布の使い分けは確かに動作し
両ファクタが毎フレーム使われる (`num_distribution` / `num_distance` で確認可能)。
一方で KITTI のような屋外・点密度が比較的均一なシーンでは、ファクタ切替による
RPE の改善幅は限定的で、本質的には密な面が支配的なため point-to-plane 主体の
標準パイプライン (≒KISS) に近い挙動に収束する。ファクタ切替機構の正しさは
ユニットテストで担保し、リーダーボードへは honest に odometry RPE を反映する。
