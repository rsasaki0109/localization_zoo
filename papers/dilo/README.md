# DiLO

From-paper reimplementation of **DiLO: Direct light detection and ranging
odometry based on spherical range images for autonomous driving**
(Han et al., ETRI Journal, 2021, doi:10.4218/etrij.2021-0088).
著者による公開実装は無く、本実装は論文記述からの独自再構成。

## 動機

従来の ICP 系は明示的な最近傍探索 (kd-tree/voxel) に依存し、探索が計算コストの
支配項になる。回転式 LiDAR の生スキャンは球面レンジ画像 (SRI) に自然に対応する
ため、**投影によるデータ対応付け (projective data association)** で探索を省ける。

## アプローチ

本リポジトリの他手法 (明示的 NN + scan-to-map) と異なる **direct / frame-to-keyframe**
の front-end:

1. **球面レンジ画像 (SRI)** — 点群を (仰角→行, 方位角→列) で 2D 画像に投影。各画素
   は最近 range の 3D 点を保持し、画像ベースで面法線を計算する (隣接画素の range
   勾配の外積、センサ方向に向ける)。

2. **Projective data association** — 現在スキャン点 `p` を相対姿勢でキーフレーム座標
   `p' = T_rel·p` に移し、SRI へ投影した画素の点 `q`・法線 `n` を対応とする
   (最近傍探索なし)。

3. **Direct alignment** — point-to-plane 残差 `e = n·(p' − q)` を Gauss-Newton で
   最小化。各反復で再投影する (direct)。Cauchy ロバスト核併用。

4. **キーフレーム管理** — 相対運動 (並進/回転) がしきい値を超えると現在スキャンを
   新キーフレームとし、世界姿勢 = キーフレーム姿勢 × 相対姿勢で累積する。

IMU は使わず、初期相対姿勢は等速予測で与える。

## パラメータ

- `sri_height` / `sri_width` (既定 64 / 900): SRI 解像度。
- `fov_up_deg` / `fov_down_deg` (既定 2.0 / −24.8): 仰角 FOV (KITTI HDL-64E)。
- `initial_threshold` (既定 1.0 m): point-to-plane 残差の外れ値しきい値。
- `robust_scale` (既定 0.5): Cauchy 核スケール。
- `keyframe_translation` / `keyframe_rotation_deg` (既定 2.0 m / 10°): キーフレーム
  切替しきい値。

## テスト

`test_dilo` の 3 ケース:

1. `SphericalRangeImageProjectsAndNormals` — SRI 投影と法線推定が機能する。
2. `RecoversKnownTranslation` — projective association で既知並進を direct に回復。
3. `KeyframeUpdatesOnLargeMotion` — 相対運動がしきい値超でキーフレーム更新。

## KITTI full での所見

KITTI full (no GT seed) では SRI 投影による対応付けが毎フレーム機能し、最近傍探索
なしで direct alignment が走る。frame-to-keyframe 構成 (持続的グローバルマップを
持たない) のため、scan-to-map の voxel 手法より drift が出やすいのが honest な
トレードオフ。探索を投影に置換する direct 機構の正しさはユニットテストで担保し、
リーダーボードへは odometry RPE を honest に反映する。
