# Small-but-Mighty

From-paper reimplementation of **Small but Mighty: A Lightweight Feature
Enhancement Strategy for LiDAR Odometry in Challenging Environments**
(Remote Sensing, 2025, 17(15):2656). 著者による公開実装は無く、本実装は論文記述
からの独自再構成。

## 動機

LiDAR オドメトリは複雑環境で精度・頑健性が劣化する。固定しきい値の LOAM 系特徴
抽出は、シーンごとに特徴品質がばらつくため低品質な特徴を拾いやすい。本手法は
高品質 (安定) な特徴を最適化全体で強化することで、低コストに性能を底上げする。

## アプローチ

軽量 (kd-tree 不使用、ボクセル局所 PCA のみ) を意図した 2 段構成:

1. **Stability-aware feature selection** — ソース走査の局所 smoothness を局所 PCA の
   平面性 `(λ1−λ0)/λ2` / 線形性 `(λ2−λ1)/λ2` で測り、その **統計分布 (μ, σ)** を
   求める。固定しきい値でなく分布に基づいて面特徴 / 線特徴を分類する。

2. **Contribution weighting** — 分布平均より上の安定な特徴ほど大きな重みを与える:
   `w = 1 + gain·clamp((val−μ)/σ, 0, cap)`。これを point-to-plane (面) /
   point-to-line (線) 拘束に掛け、低品質残差の影響を抑制する。

レジストレーションは面特徴 = point-to-plane `n·(p−μ)`、線特徴 = point-to-line
`(I − d dᵀ)(p−μ)` (最大固有ベクトル `d` が線方向) を 1 つの Gauss-Newton 系に統合。
map 側の平面性 / 線形性も重みに掛けて品質を二重に反映。Cauchy ロバスト核併用。
IMU 事前積分は KITTI には無いため等速予測 prior (`prior_precision`) で代替。

## パラメータ

- `planar_min` (既定 0.4) / `edge_min` (既定 0.5): 面/線特徴とみなす品質下限。
- `contribution_gain` (既定 1.0) / `contribution_cap` (既定 3.0): 安定特徴の強化量。
- `plane_weight` (既定 1.0) / `edge_weight` (既定 0.5): 面/線の全体相対重み。
- `robust_scale` (既定 1.0): Cauchy 核スケール。
- `prior_precision` (既定 0.0): 等速予測への prior。

## テスト

`test_small_mighty` の 3 ケース:

1. `RecoversKnownTranslation` — 良条件シーンで既知並進を回復。
2. `StableFeaturesGetHigherWeight` — 全特徴の重み ≥ 1、安定特徴で 1 超、面/線両方選択。
3. `UniformWeightStillConverges` — `contribution_gain=0` (アブレーション) で一様重みでも収束。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では、stability-aware selection と
contribution weighting は毎フレーム動作し、面/線特徴が分布に応じて選ばれる
(`num_planar` / `num_edge` で確認可能)。軽量な特徴強化のため標準 LOAM 系の
point-to-plane / point-to-line パイプラインに近い挙動となる。リーダーボードへは
honest に odometry RPE を反映する。
