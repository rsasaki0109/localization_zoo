# MCGICP-LO

Multi-Channel Generalized-ICP LiDAR オドメトリ。**Multi-Channel Generalized-ICP:
A robust framework for multi-channel scan registration** (Servos & Waslander,
Robotics and Autonomous Systems, 2017) の機構を from-paper で再構成したもの。
著者公開コードは無い。

## 動機

標準 GICP は xyz のみで共分散を推定するため、幾何的に退化した領域では対応が曖昧になる。
MCGICP は **強度 (reflectance) チャンネルを 4D 共分散に統合**し、曲面接平面内の不確かさと
強度変動を同時にモデル化する。対応探索と最適化は 4D Mahalanobis 距離で行い、SE(3) は xyz
のみに作用する (強度は剛体変換に伴わない)。

## アプローチ

scan-to-map で frame ごとに MCGICP を解く。

- **4D 表現**: `v = [x, y, z, s·I]^T` (`s` = `intensity_scale`)。
- **共分散**: 各点の k 近傍から 4×4 共分散を推定し、最小固有値を法線方向 `normal_epsilon`、
  接平面方向を 1.0、強度方向を `max(λ, intensity_epsilon)` に GICP 正則化。
- **対応**: ローカルマップ上で 4D Mahalanobis が最小の近傍点を選択。
- **最適化**: `r^T (C_t + R C_s R^T)^{-1} r` を Gauss-Newton (SE(3)) で最小化。
  強度残差のヤコビアンはゼロ (強度は回転・並進に不変)。

`--mcgicp-no-intensity` で強度スケール 0 にし 3D GICP に退化 (アブレーション)。

## パラメータ

| Param | Meaning | Default |
|---|---|---|
| `intensity_scale` | 強度を距離単位へ写像 | 2.0 |
| `normal_epsilon` | 法線方向 GICP 正則化 | 1e-3 |
| `intensity_epsilon` | 強度分散下限 | 0.05 |
| `k_neighbors` | 共分散推定の近傍数 | 12 |
| `voxel_size` | ローカルマップ解像度 | 1.0 |

## テスト

`test_mcgicp` (3 ケース):

1. `IntensityIncreasesMahalanobis` — 同一 xyz で強度不一致は Mahalanobis が大きい。
2. `RecoversForwardTranslation` — 平面 + 強度で前進並進を回復。
3. `IntensityResolvesAmbiguity` — 幾何的に近い候補で強度一致が選ばれる。

## KITTI full での所見

KITTI full (no GT seed, `--mcgicp-dense-profile`) では seq00 RPE **0.940%** / ATE
19.6 m (1.8 FPS)、seq07 RPE **0.774%** / ATE 4.5 m (2.1 FPS)。

**正直な所見**: 4D 共分散 + 強度重み付き point-to-plane は **mid-pack competitive**
(GMM-LO / Student-T-LO 帯)。seq07 は KISS-ICP (0.618%) には僅差で劣るが ATE は低め。
強度を距離計量に直接入れるため I-LOAM の対応コスト方式ほどの positive ablation は
出にくいが、**near-redundant ではなく seq07 でわずかに効く**程度。機構の正しさ
(4D Mahalanobis・強度重み) はユニットテストで担保。`--mcgicp-no-intensity` で
3D 退化のアブレーション可。
