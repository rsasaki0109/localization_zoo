# CT-LIO

簡易な **Continuous-Time LiDAR-Inertial Odometry** 実装です。  
この実装は **CT-ICP の2ポーズ補間** と **IMU Preintegration** を組み合わせて、
1スキャン内のモーション歪みを連続時間で扱いながら IMU で拘束します。

## 位置づけ

- 着想元:
  - **CLINS: Continuous-Time Trajectory Estimation for LiDAR-Inertial System**
  - **CT-ICP**
  - **IMU Preintegration on Manifold**
- このコードは論文完全再現ではなく、`localization_zoo` の既存部品で組んだ
  **小さな連続時間 LIO プロトタイプ** です

## モデル

- 軌跡表現: 1フレームを `begin/end` の2ポーズで表現
- LiDAR残差: 連続時間 point-to-plane
- IMU残差:
  - 回転事前積分 `R_j = R_i * ΔR`
  - 並進事前積分 `p_j = p_i + v_i Δt + 1/2 g Δt² + R_i Δp`
- オプション:
  - 事前積分ヤコビアンを使った 1 次の gyro / accel bias 補正
  - 前フレーム bias を中心にした `dt` 依存の random walk prior
  - 直近数フレームの state を使う小さい fixed-lag window prior
- 速度状態: フレーム開始速度 `v_i`

## いまの実装範囲

- ボクセルマップに対する scan-to-map
- IMU 事前積分を使った終端ポーズの初期化
- Ceres による LiDAR + IMU の同時最適化
- `estimate_imu_bias=true` で有効になる optional bias 推定
- bias state の multi-frame 持ち回り
- configurable な fixed-lag smoother relinearization 回数
- スライディングウィンドウ地図

## 制限

- B-spline や高次連続時間軌跡は未実装
- bias は random walk 付き smoother ではなく、1 フレーム内の 1 次補正のみ
- bias 推定は長い実データ列ではまだ安定に効いておらず、デフォルトでは無効
- iEKF / fixed-lag smoother ではなく、軽量なバッチ最適化です
