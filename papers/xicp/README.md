# X-ICP: Localizability-Aware LiDAR Registration for Robust Localization in Extreme Environments

## 論文情報

- **著者**: Turcan Tuna, Julian Nubert, Yoshua Nava, Shehryar Khattak, Marco Hutter (ETH Zurich)
- **会場**: IEEE Transactions on Robotics (T-RO) 2024
- **arXiv**: https://arxiv.org/abs/2211.16335

## 手法概要

Point-to-plane ICPのヘシアンを固有値分解し、各方向のローカライズ可能性を
3段階 (full/partial/none) で分類。退化方向への更新をLagrange乗数法で制約。

### コア数式

**ヘシアン**: `A' = Σ [p×n; n][p×n; n]^T` (6x6)

**ローカライズ可能性スコア**:
```
I_t = |F_t · V_t|  (各対応点の法線をeigenvector方向に射影)
L_c(j) = Σ I'_c(i,j)  (閾値以上の寄与を集計)
```

**分類**: L_c >= κ₁(250) → full, >= κ₂(180) → partial, else → none

**制約付きICP**: min ||A'x - b'||² s.t. Cx = d
