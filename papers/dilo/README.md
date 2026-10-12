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
   (最近傍探索なし)。実データでは投影丸めと小さな初期値誤差で exact pixel の対応が
   落ちやすいため、既定では 1px 近傍だけを調べ、3D 距離 1.5m 以内の候補に限定する。

3. **Direct alignment** — point-to-plane 残差 `e = n·(p' − q)` を Gauss-Newton で
   最小化。各反復で再投影する (direct)。Cauchy ロバスト核併用。

4. **キーフレーム管理** — 相対運動 (並進/回転) がしきい値を超えると現在スキャンを
   新キーフレームとし、世界姿勢 = キーフレーム姿勢 × 相対姿勢で累積する。

IMU は使わず、初期相対姿勢は等速予測で与える。

## パラメータ

- `sri_height` / `sri_width` (既定 64 / 900): SRI 解像度。
- `fov_up_deg` / `fov_down_deg` (既定 2.0 / −24.8): 仰角 FOV (KITTI HDL-64E)。
- `lookup_radius` / `max_lookup_point_distance` (既定 1px / 1.5m): SRI 近傍
  projective association の上限。`lookup_radius=0` で exact-pixel のみ。
- `initial_threshold` (既定 1.0 m): point-to-plane 残差の外れ値しきい値。
- `robust_scale` (既定 0.5): Cauchy 核スケール。
- `keyframe_translation` / `keyframe_rotation_deg` (既定 2.0 m / 10°): キーフレーム
  切替しきい値。

## テスト

`test_dilo` の 3 ケース:

1. `SphericalRangeImageProjectsAndNormals` — SRI 投影と法線推定が機能する。
2. `RecoversKnownTranslation` — projective association で既知並進を direct に回復。
3. `TracksConsecutiveMotion` — 連続フレームの小さな並進を累積追従する。

## KITTI full での所見

KITTI full (no GT seed) では SRI 投影による対応付けが毎フレーム機能し、最近傍探索
なしで direct alignment が走る。bounded 1px lookup を入れた current default は
100 m RPE で seq00 **1.200%** (ATE 38.7m, 64.5 FPS) / seq07 **1.533%** (ATE 7.2m, 65.0 FPS)、
公式 KITTI RTE では 2.687 % / 1.713 %。
旧 exact-pixel lookup は seq00 18.305% / seq07 18.966% まで drift したため、実運用
では投影近傍探索が load-bearing。

frame-to-keyframe 構成 (持続的グローバルマップを持たない) のため、scan-to-map の
voxel 手法より drift が出やすいのが honest なトレードオフ。探索を投影に置換する
direct 機構の正しさはユニットテストで担保し、リーダーボードへは odometry RPE を
honest に反映する。

## 論文どおりのオドメトリ (`--dilo-paper-odometry`)

従来経路は frame-to-keyframe の 1 回の GN で、論文の主要な仕組み (距離画像の作り方、法線、
key point の選び方、Algorithm 1 のウィンドウ最適化) をほぼ持っていなかった。論文 Sec. 3 を追う
経路 (`DiloPaperOdometry`, `include/dilo/dilo_paper.h`) を別に用意した。

- **距離画像** (Sec. 3.2): 2048 列、行は sin(仰角) に比例 (同じ尺度, 192 行)。各画素は最も近い
  点。縦の隙間 (4 行まで) は、距離変化が緩やかなら補間し、そうでなければ遠い方の距離で埋める。
  補間は距離ではなく 3D 点で行う (距離の線形補間は地面をビーム間で曲げる)。計測画素は実際の
  点を保持する。
- **法線と平面度** (Sec. 3.3, 3.6): Farid-Simoncelli 5 タップフィルタ。法線は同じフィルタを
  3D 点画像にかけた 2 方向の接ベクトルの外積 (式 11 は画素中心の光線を仮定し、ビームとの
  ずれで地面の法線が傾く)。平面度 a = 1 − |dn/dq|²。
- **key point** (Sec. 3.6): a (x × n) の 6 リスト、a n_x, a n_y の 4 リスト、地面 a n_z の
  1 リスト、各上位 100 点。
- **残差** (式 18): (n_r · R n_s) n_r · (x_r − T x_s)、参照画像への射影で対応付け。
- **Algorithm 1**: 直近 N = 10 フレームのキュー。新フレームを全フレームに Huber (0.25) で
  合わせ、その後 5 回、ウィンドウ内の全ペアで Tukey (1.0) の 1 ステップを取り、各フレームの
  姿勢を相対姿勢の log 平均で更新する (最古フレーム固定)。出力はウィンドウを出たときの姿勢。
- 論文にない点: 予測は直前の相対運動の x, y, yaw だけを使う (垂直成分のノイズを持ち越すと
  KITTI 07 の最初のカーブで発散した)。初期推定が予測から 1 m / 0.1 rad 以上離れたら予測を
  採用し、ペアの 1 ステップが 0.5 m / 0.05 rad を超えたら捨てる (KITTI 00 の 4269-4278 で
  登録が連続して失敗し発散した)。
- 未実装: エッジ key point (Sec. 3.5)。比較相手は論文の平面のみ版 DiLO-plane。

### KITTI Odometry (公式 RTE, GT seed なし)

`experiments/dilo_kitti_seq_{00,07}_full_paper_comp_matrix.json`:

| variant | seq 00 | seq 07 | FPS |
|---|---:|---:|---:|
| 従来 (default) | 2.687 % | 1.713 % | 40 |
| 論文どおり (`paper_odometry`) | 1.308 % | 0.619 % | 4 |
| 論文どおり + 0.22° 仰角補正 (`paper_odometry_elevation`) | **0.797 %** | **0.579 %** | 4 |
| ウィンドウ更新なし (ablation) | NO_WINDOW_00 | 1.061 % | 5 |
| 論文 DiLO-plane (Table 1) | 0.84 % | 0.58 % | |
| 論文 DiLO (Table 1) | 0.64 % | 0.42 % | |

仰角補正付きは平面のみ版の論文値と同じ水準 (0.95x / 1.00x)、エッジ込みの論文値に対しては
1.25x / 1.38x。ウィンドウの姿勢平均を外すと大きく悪化するので、この手法の精度は Algorithm 1
が支えている。論文は仰角補正に触れていないが、補正なしでは 00 で 1.6 倍悪い。

