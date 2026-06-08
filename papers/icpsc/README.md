# ICPSC-LO

Intensity Cylindrical-Projection Shape Context LiDAR オドメトリ。
**A LiDAR-intensity SLAM and loop closure detection method using an intensity
cylindrical-projection shape context descriptor** (Zhang et al., JAG 2023,
Int. J. Applied Earth Observation and Geoinformation) の機構を from-paper で
再構成。著者公開コードは無い。

## 動機

幾何特徴が乏しい環境では純幾何 LO/ICP が退化する。ICPSC は LiDAR 強度を
**円筒投影 shape-context 記述子 (ICPSC)** として符号化し、幾何マッチングと
強度マッチングを **適応重み融合** する。

## アプローチ (本 port)

- **円筒投影**: range + intensity + per-bin 点密度。
- **ICPSC 記述子**: ring×sector 上の平均強度 + log 密度 (shape-context 風)。
- **特徴抽出**: range 画像から edge/surf、intensity 画像から高勾配点。
- **適応融合**: `w_geom = N_geom / (N_geom + α·N_int)` で幾何/強度残差を重み付け。
- **scan-to-map**: 強度類似度 `exp(−ΔI²/2σ²)` で point-to-plane を重み付け。

ループクロージャ用 ICPSC 近傍探索・因子グラフ最適化は **本 port では後回し**
(odometry ベンチに集中)。

## テスト

`test_icpsc` 3 ケース:

1. `DescriptorDistanceSeparatesIntensity` — 強度パターン差を ICPSC 距離が検出。
2. `AdaptiveFusionPrefersGeometry` — 幾何特徴が多いとき w_geom が大きい。
3. `RecoversForwardTranslation` — 平面 + 円筒特徴で前進並進を回復。

## KITTI full での所見

`--icpsc-dense-profile` (source_voxel 0.35, cyl 1024×64, map voxel 0.8):

| Seq | Drift | ATE | FPS | mean w_geom |
|-----|-------|-----|-----|-------------|
| seq00 | **0.912%** | 19.2 m | 1.3 | 0.947 |
| seq07 | **0.660%** | 3.7 m | 1.5 | 0.946 |

両シーケンスで MCGICP-LO (seq00 0.940% / seq07 0.774%) を上回る mid-pack 帯。
幾何特徴が豊富な区間では `w_geom≈0.95` と強度は補助的に働く。
ループクロージャ無しの純オドメトリ評価。
