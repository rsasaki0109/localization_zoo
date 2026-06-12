# LiTAMIN2: Ultra Light LiDAR-based SLAM using Geometric Approximation applied with KL-Divergence

## 論文情報

- **著者**: Masashi Yokozuka, Kenji Koide, Shuji Oishi, Atsuhiko Banno (産総研 AIST)
- **会場**: ICRA 2021
- **arXiv**: https://arxiv.org/abs/2103.00784
- **プロジェクトページ**: https://staff.aist.go.jp/shuji.oishi/assets/projects/LiTAMIN/index.html

## 手法概要

LiDAR点群をボクセルに分割し、各ボクセル内の点群を正規分布で近似。
正規分布間の対称KLダイバージェンスに基づくICPコストで高速かつ高精度な点群レジストレーションを実現。

### パイプライン

1. 入力点群をボクセルグリッドに投票
2. 各ボクセル内の点群を正規分布 (平均・共分散) で近似
3. source/target両方をボクセル化し、正規分布同士でレジストレーション

### コスト関数

ICPコスト (距離):
```
E_ICP = (q - (Rp+t))^T C_qp (q - (Rp+t))
C_qp = (C_q + R C_p R^T + λI)^{-1} / ||(C_q + R C_p R^T + λI)^{-1}||_F
```

共分散コスト (分布形状の差):
```
E_Cov = (Tr(R C_p^{-1} R^T C_q) + Tr(C_q^{-1} R C_p R^T) - 6)^2
```

ロバスト重み:
```
w_ICP = 1 - E_ICP / (E_ICP + σ_ICP^2)    (σ_ICP = 0.5)
w_Cov = 1 - E_Cov / (E_Cov + σ_Cov^2)    (σ_Cov = 3)
```

### パラメータ

| パラメータ | 値 | 説明 |
|---|---|---|
| voxel_size | 3.0 m | ボクセルサイズ (KITTI最適値) |
| λ | 1e-6 | Frobenius正規化パラメータ |
| σ_ICP | 0.5 | ICPロバスト重みの閾値 |
| σ_Cov | 3.0 | 共分散ロバスト重みの閾値 |
| correspondence_search_radius | 0 | 対応探索の近傍ボクセル半径 |
| max_correspondence_distance | 0.0 m | 対応点距離上限。0で無効 |

## 実装ノート

- この実装はSLAMフロントエンド (オドメトリ) の点群レジストレーション部分を再現
- ループクロージャ・グラフ最適化は含まない
- 公式実装は未公開。非公式実装 (https://github.com/bzdfzfer/litamin2) を参考
- `pcd_dogfooding` では `--litamin2-correspondence-search-radius` と
  `--litamin2-max-correspondence-distance` で対応探索の広さと距離gateを
  sweepできる。デフォルトは従来互換の同一ボクセル探索。
- KITTI seq02 の 200-frame no-GT smoke では、tuned profile
  (`--litamin2-voxel-resolution 1.0 --litamin2-max-iterations 12`) に
  `--litamin2-correspondence-search-radius 1 --litamin2-max-correspondence-distance 1.5`
  を追加すると ATE 22.522 m / drift 2.330 m/100m から
  ATE 1.042 m / drift 0.730 m/100m に改善した。一方 seq05/07/08 の
  short smoke では改善しないため、まだデフォルト昇格はしていない。
