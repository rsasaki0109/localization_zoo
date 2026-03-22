# IMU Preintegration on Manifold

## 論文情報

- **著者**: Christian Forster, Luca Carlone, Frank Dellaert, Davide Scaramuzza
- **会場**: IEEE Transactions on Robotics (T-RO) 2017 / RSS 2015
- **概要**: IMU計測値をSO(3)多様体上で事前積分し、ファクターグラフ最適化に組み込む

## 手法概要

2つのキーフレーム間のIMU計測値を事前積分して相対変換を計算。
バイアス更新時に事前積分を再計算せず、1次近似で補正できる。

### 事前積分量

```
ΔR_ij = Π R(ω_k dt)           (相対回転)
Δv_ij = Σ ΔR_ik a_k dt        (相対速度)
Δp_ij = Σ ΔR_ik a_k dt² / 2 + Δv_ik dt  (相対位置)
```

### バイアス補正 (1次近似)

```
ΔR_ij(b_g) ≈ ΔR_ij(b̂_g) · Exp(J_r^ΔR δb_g)
Δv_ij(b) ≈ Δv_ij(b̂) + J_v^bg δb_g + J_v^ba δb_a
Δp_ij(b) ≈ Δp_ij(b̂) + J_p^bg δb_g + J_p^ba δb_a
```

## 用途

VINS-Mono, LIO-SAM, FAST-LIO 等のIMUファクターの基盤
