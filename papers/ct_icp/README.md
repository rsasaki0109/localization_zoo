# CT-ICP: Continuous-Time ICP for LiDAR Odometry

## 論文情報

- **著者**: Pierre Dellenbach, Jean-Emmanuel Deschaud, Bastien Jacquet, François Goulette
- **会場**: ICRA 2022
- **arXiv**: https://arxiv.org/abs/2109.12979
- **元リポジトリ**: https://github.com/jedeschaud/ct_icp (MIT License)

## 手法概要

従来のICPは1フレーム=1ポーズ(6DoF)だが、CT-ICPは1フレーム=2ポーズ(begin+end, 12DoF)で
スキャン取得中のセンサ運動を陽にモデル化。SLERP+線形補間でスキャン内の各点のタイムスタンプに
応じた連続時間ポーズを計算し、モーション歪みを最適化で直接解消する。

### 核心アルゴリズム

**連続時間補間**:
```
P(alpha) = slerp(q_begin, q_end, alpha), (1-alpha)*t_begin + alpha*t_end
alpha = (timestamp - t_begin) / (t_end - t_begin)
```

**コスト関数** (Point-to-Plane):
```
r = w * n^T * (p_ref - P(alpha) * p_raw)
```

**正則化**:
- 位置整合性: ||t_begin^k - t_end^{k-1}||
- 等速制約: ||t_end^k - t_begin^k - v_prev||

## 実装ノート

- ROS非依存の純粋C++実装
- Ceresの `AutoDiffCostFunction` + `EigenQuaternionParameterization`
- ボクセルハッシュマップによるO(1)近傍探索
