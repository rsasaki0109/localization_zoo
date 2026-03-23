# NDT: Normal Distributions Transform

## 論文情報
- **著者**: Peter Biber, Wolfgang Strasser (2003, 2D) / Martin Magnusson (2009, 3D)
- **会場**: IROS 2003 / PhD Thesis 2009
- **概要**: Autowareのlocalizationの基盤。点群を正規分布でモデル化してマッチング

## 手法概要
1. ターゲット点群をボクセルに分割、各ボクセル内で正規分布(平均+共分散)を計算
2. ソース点をターゲットの正規分布に対して尤度最大化
3. コスト: -Σ exp(-0.5 (p-μ)^T Σ^{-1} (p-μ))
4. Newton法で最適化
