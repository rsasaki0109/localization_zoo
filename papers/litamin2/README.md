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
| min_cov_eigenvalue | 1e-3 | ボクセル共分散の最小固有値 |
| optimize_covariance_cost | false | 共分散形状コストの回転勾配を更新式へ入れるか |
| covariance_gradient_weight | 1.0 | 共分散形状コスト更新項の追加重み |
| enable_line_search | false | 更新後コストでステップ幅を縮小するか |
| scan_to_scan | false | 直前scanをtargetにする非公式実装風の相対pose積算 |
| coarse_to_fine_voxels | unset | coarse-to-fine voxel schedule。例: 3.0,2.0,1.0 |
| coarse_to_fine_iterations | unset | coarse-to-fine 各段の反復数。短い場合は最後の値を再利用 |
| refresh_interval | 3 | target map 再構築間隔 |
| map_radius | 45.0 m | map に保持する近傍半径 |
| max_seed_translation_delta | 2.0 m | 初期値からの refinement acceptance gate |
| max_seed_rotation_delta | 0.25 rad | 初期値からの refinement acceptance gate |
| max_motion_translation_delta | disabled | frame-to-frame motion consistency gate |
| max_motion_rotation_delta | disabled | frame-to-frame motion consistency gate |
| correspondence_search_radius | 0 | 対応探索の近傍ボクセル半径 |
| max_correspondence_distance | 0.0 m | 対応点距離上限。0で無効 |

## 実装ノート

- この実装はSLAMフロントエンド (オドメトリ) の点群レジストレーション部分を再現
- ループクロージャ・グラフ最適化は含まない
- 公式実装は未公開。非公式実装 (https://github.com/bzdfzfer/litamin2) を参考
- 非公式実装のKITTI例は、current scan を source、直前scanを target とする
  scan-to-scan registration を行い、`swapSourceAndTarget()` しながら相対poseを積算する。
  `pcd_dogfooding` のLiTAMIN2はscan-to-map + GT/velocity seed + local map refresh で
  評価しているため、論文/非公式実装との差分には registration 式だけでなく
  フロントエンド運用差が含まれる。
  これを確認するため `--litamin2-scan-to-scan` を opt-in で追加した。
  KITTI seq02 108-frame では ATE 1.395 m / RPE 2.568 % (GT relative seed)、
  ATE 1.474 m / RPE 2.774 % (no-GT, previous relative seed) まで動くが、
  full seq02 では ATE 445 m / RPE 21.8-24.4 % までdriftするため、
  long-trajectory default にはしない。
- 非公式 point-to-voxel 実装では covariance shape cost の最適化項はコメントアウトされ、
  ICP側のKL Mahalanobis costが主に使われている。これは本実装の
  `optimize_covariance_cost=false` default と整合する。
- `pcd_dogfooding` では `--litamin2-correspondence-search-radius` と
  `--litamin2-max-correspondence-distance` で対応探索の広さと距離gateを
  sweepできる。デフォルトは従来互換の同一ボクセル探索。
- KITTI seq02 の no-GT sweep では、tuned profile
  (`--litamin2-voxel-resolution 1.0 --litamin2-max-iterations 12`) に
  `--litamin2-correspondence-search-radius 1 --litamin2-max-correspondence-distance 1.5`
  を追加すると、200-frame smoke で ATE 22.522 m / drift 2.330 m/100m から
  ATE 1.042 m / drift 0.730 m/100m に改善した。full sequence でも
  ATE 50.622 m / RPE 0.975 % から ATE 44.005 m / RPE 0.936 % に
  改善したが、FPS は 67.7 から 39.3 に落ちる。full-sequence
  横展開では seq05 も ATE/RPE 改善、seq07 は悪化、seq08 は ATE 改善
  だが RPE 悪化だった。seq02/05/07/08 の幾何平均 RPE は baseline
  0.806 % に対して gate付きが 0.810 % で微悪化するため、まだ
  デフォルト昇格はしていない。
- `--litamin2-min-cov-eigenvalue 1e-4` も sweep可能。seq02/05/07/08
  full の幾何平均 RPE は baseline 0.806 % に対して 0.807 % と
  横ばいだが、FPS は改善しやすい。論文再現の精度 default はまだ
  従来値 1e-3 のまま。
- `--litamin2-covariance-gradient` は、論文式の共分散形状コストを
  回転更新へ入れる opt-in 実装。108-frame smoke では seq02/05 に
  改善傾向があったが、full sequence では seq02 が ATE 384.474 m /
  RPE 5.679 %、seq05 が ATE 9.654 m / RPE 0.684 %、seq07 が
  ATE 5.237 m / RPE 0.845 %、seq08 が ATE 18.694 m / RPE 1.351 %
  で、seq02/05/07/08 の幾何平均 RPE は 1.451 % に悪化した。
  `--litamin2-covariance-gradient-weight 0.1 --litamin2-line-search` まで
  弱めると full の幾何平均 RPE は 0.806 % まで戻るが、seq02 ATE が
  50.622 m から 81.961 m に悪化する。重み設計とステップ制御がまだ
  trade-off を持つため default にはしない。
- `--litamin2-coarse-to-fine-voxels 3.0,2.0,1.0` は、3段の
  voxel schedule を coarse から fine へ順に適用する opt-in 実装。
  108-frame smoke では seq02 RPE が 11.969 % から 0.670 % へ大きく
  改善したが、full sequence では seq02/08 が悪化し、seq02/05/07/08
  の幾何平均 RPE は baseline 0.806 % に対して 1.185 % だった。
  `--litamin2-refresh-interval 1` と tighter seed gate
  (`--litamin2-max-seed-translation-delta 1.0 --litamin2-max-seed-rotation-delta 0.15`)
  も full seq02 で確認したが、それぞれ RPE 4.412 % / 2.306 % で
  改善しなかった。`--litamin2-coarse-to-fine-iterations 3,3,6` は full seq02 を
  ATE 48.589 m / RPE 1.224 % / 89.5 FPS まで戻し、`2,2,8` は
  RPE 0.976 % と baseline 並みに戻ったが、ATE は 55.516 m で悪化した。
  `2,2,8` を seq05/07/08 full に横展開すると ATE は seq05 7.350 -> 1.454 m、
  seq07 1.964 -> 0.625 m、seq08 22.875 -> 1.244 m と大きく下がる一方、
  RPE は seq05 0.626 -> 0.630 %、seq07 0.535 -> 0.782 %、
  seq08 1.295 -> 1.386 % に悪化し、seq02/05/07/08 の幾何平均 RPE も
  0.806 -> 0.903 % に悪化した。
  schedule 自体と段ごとの iteration sweep は再現可能になったが、長距離では
  粗段の over-iteration が drift を誘発しやすく、ATE/RPE trade-off も大きいため
  全体 default にはしない。
- `--litamin2-max-motion-translation-delta` /
  `--litamin2-max-motion-rotation-delta` は、candidate の frame-to-frame
  motion を GT relative motion (GT-seeded) または直前推定motion (no-GT) と比較する
  opt-in gate。`0.25 / 0.05` を coarse-to-fine default に足すと、
  seq02/05/07/08 full の ATE は 0.957 / 0.957 / 0.714 / 0.978 m まで
  下がるが、RPE は 1.352 / 1.321 / 1.111 / 1.359 % となり、幾何平均 RPE は
  baseline 0.806 % に対して 1.282 % へ悪化する。ATE重視の診断ノブとしては
  有用だが、RPE主張の default にはしない。
