# MCC-LO

Maximum Correntropy Criterion (MCC) ロバスト point-to-plane LiDAR オドメトリ。
**Robust 3D point cloud registration based on bidirectional Maximum Correntropy
Criterion** (He et al., PLOS ONE 2018, doi:10.1371/journal.pone.0197542) および
**Correntropy based scale ICP algorithm for robust point set registration**
(Pattern Recognition 2019) の相関エントロピーロバスト推定機構を scan-to-map
LiDAR オドメトリへ適用した from-paper 再構成。原論文は対点群レジストレーション
のみで、本リポジトリ向けオドメトリパイプラインとしての公開実装は無い。

## 動機

二乗誤差 (MSE) 最小化はガウス雑音には最適だが、非ガウス雑音や大きな外れ値に弱い。
相関エントロピー (correntropy) は情報理論的な局所相似尺度で、残差が大きいほど
寄与が指数的に減衰するため、外れ値に頑健。M 推定子 (Cauchy/Huber/Student-t) の
重尾カーネルや GNC の継続 TLS とは別系統の robust 化で、カーネルバンド幅 σ を
データから適応推定できる点が特徴。

## アプローチ

scan-to-map point-to-plane を **MCC** で解く。

- 二乗誤差 Σr² を最小化する代わりに、相関エントロピー
  `V = Σ Gσ(r) = Σ exp(−r²/2σ²)` を **最大化** する。
- 半二次最適化 (half-quadratic) で重み付き最小二乗に帰着し、IRLS 重み
  `w = exp(−r²/2σ²)` (Welsch/Gauss カーネル) を得る。外れ値は指数的に減衰。
- カーネルバンド幅 σ を毎反復 **Silverman の経験則** `σ = 1.06·std(r)·N^(−1/5)`
  で残差分布から適応推定する (`mcc_adaptive_sigma=true`)。

`r` は point-to-plane 残差 `n·(Tp − anchor)`。

## パラメータ

- `mcc_sigma` (既定 0.3 m): 固定モード (`--mcc-lo-fixed-sigma`) での σ。
- `mcc_adaptive_sigma` (既定 true): Silverman 経験則で σ を適応推定。
- `mcc_sigma_floor` (既定 0.3 m): 適応 σ の下限。整備済みスキャンで残差が小さく
  なると Silverman σ→0 となりカーネルが収束信号 (未整合点の残差) まで棄却して
  **過小収束** する (annealing 無し correntropy-ICP の既知の弱点) ため floor で抑止。
- `voxel_size` (既定 1.0), `planarity_threshold` (既定 0.5) ほか共通。

## テスト

`test_mcc_lo` の 3 ケース:

1. `CorrentropyWeightDownweightsOutliers` — 残差 0 で重み 1、外れ値で指数減衰、
   σ を広げると同残差でも寛容。
2. `RecoversForwardTranslation` — 良条件で前進並進を回復。
3. `RobustToHighOutlierRatio` — 高外れ値比でも前進を回復。

## KITTI full での所見

KITTI full (no GT seed) では seq07 RPE **0.611%** / ATE 2.15 m、2.6 FPS。
`mean_weight ≈ 0.90`、適応 σ は floor 0.3 に張り付く。

**正直な所見**: CV 予測で初期姿勢が良好かつ整備済みの幾何リッチなスキャンでは
残差が小さく外れ値も少ないため、Silverman の適応バンド幅は floor まで収縮し、
相関エントロピーは実質ガウス重み (mean_weight≈0.90) に退化する。robust 機構が
near-redundant になるという恒例知見と整合する一方、Welsch カーネルの穏やかな
減衰は GNC の hard 棄却より過剰棄却が少なく、seq07 では robust 群中で良好。
`--mcc-lo-sigma / --mcc-lo-fixed-sigma` でアブレーション可能。
