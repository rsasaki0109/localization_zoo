# NHC-LIO

From-paper reimplementation of **NHC-LIO: A Novel Vehicle Lidar-Inertial Odometry
(LIO) With Reliable Nonholonomic Constraint (NHC) Factor** (Chen et al., IEEE
Sensors Journal, 2023, doi:10.1109/JSEN.2023.3320150).
著者による公開実装は無く、本実装は論文記述からの独自再構成。

## 動機

車両は **非ホロノミック (nonholonomic)** な運動をする: body 座標系で横滑り
(lateral) と上下動 (vertical) の並進速度がほぼ 0 で、前進 (+ヨー旋回) のみ動く。
この事前知識を odometry に拘束として加えると、特に横方向の累積 drift を抑えられる。

## アプローチ

scan-to-map の point-to-plane オドメトリに **NHC ファクタ** を soft 拘束として加える:

- 直前 body 系での運動 `(t_cur − t_prev)` を `R_prevᵀ` で body 系に戻し、前進軸
  以外の 2 成分 (横 y / 上下 z) を選択行列 `S` で取り出した残差
  `r_nhc = S · R_prevᵀ (t_cur − t_prev)` を 0 へ引く。
- ヤコビアン `J_nhc = S R_prevᵀ [−skew(t_cur), I]` を Gauss-Newton 系に蓄積する。

### Adaptive 重み (論文の reliable NHC の近似)

論文は 9 軸 IMU から CNN で reliable な NHC 擬似観測を予測し情報行列を adaptive に
する。KITTI は IMU 無しのため、**運動状態 (yaw rate) ベースの adaptive 重み**で
近似する: `w = nhc_weight / (1 + (yaw_rate/yaw_ref)²)`。直進時は強く拘束し、急旋回時
(横滑りが増える) は減衰させる。CNN/IMU 部分は IMU-free KITTI では範囲外。

## パラメータ

- `enable_nhc` (既定 true): NHC ファクタの有効化。
- `nhc_weight` (既定 5.0): NHC 基本情報重み。
- `nhc_yaw_ref` (既定 0.03 rad/frame): 旋回時 adaptive 減衰スケール。
- `forward_axis` (既定 0=x): 車両前進軸 (KITTI Velodyne は x)。

## テスト

`test_nhc_lio` の 3 ケース:

1. `RecoversForwardTranslation` — 良条件で前進並進を回復 (NHC が前進を妨げない)。
2. `AdaptiveWeightDecaysOnTurn` — 急旋回の次フレームで NHC 重みが減衰する。
3. `NhcReducesLateralDrift` — 横が弱拘束なシーンで NHC が横 drift を抑える。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では NHC ファクタが毎フレーム働き
(adaptive 重みは `nhc_weight_used` で確認可能)、車両の前進主体の運動に対し横方向の
弱拘束を補う。直進が多い系列で横 drift 抑制に寄与し、急旋回区間では adaptive に
減衰して過拘束を避ける。論文の CNN/IMU による reliable NHC 予測は IMU-free KITTI
では範囲外で、運動状態ベースの近似に置換している点を honest に明記する。
