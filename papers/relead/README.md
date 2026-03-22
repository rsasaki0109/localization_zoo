# RELEAD: Resilient Localization with Enhanced LiDAR Odometry in Adverse Environments

## 論文情報

- **著者**: Zhiqiang Chen, Hongbo Chen, Yuhua Qi, Shipeng Zhong, Dapeng Feng, Jin Wu, Weisong Wen, Ming Liu
- **会場**: ICRA 2024
- **arXiv**: https://arxiv.org/abs/2402.18934

## 手法概要

LiDARスキャンマッチングの退化を検知し、制約付きESIKF更新で退化方向のドリフトを防ぐ。

### コアアルゴリズム

1. **退化検知**: ヘシアン A' = A^T A をSVDで分解、各方向のローカライズ可能性を分類
2. **制約付きESIKF**: 退化方向への更新をゼロに制約
3. **制約射影**: G = ΣC^T(CΣC^T)^{-1}, Δx̄ = Δx - G(CΔx - d)

## 実装ノート

- フロントエンド (CESIKF + 退化検知) のみ実装
- バックエンド (GNC + rIFL) は含まない
- 公式実装は未公開
