# GICP

Generalized Iterative Closest Point (Segal et al., RSS 2009) の簡易実装。

## 核心

- 各点の局所近傍から平面共分散を推定
- 対応点ごとに `C_t + R C_s R^T` のマハラノビス距離で誤差を評価
- Gauss-Newton で SE(3) を反復更新

## 実装メモ

- 共分散は PCA から法線方向の分散を小さくした平面モデルで近似
- 対応点はターゲット点群への最近傍探索で更新
- 依存は Eigen + PCL のみ
