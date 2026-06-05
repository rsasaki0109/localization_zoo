# GNC-LO

Graduated Non-Convexity (GNC) ロバスト point-to-plane LiDAR オドメトリ。
**Graduated Non-Convexity for Robust Spatial Perception** (Yang, Antonante,
Tzoumas, Carlone, IEEE RA-L 2020, arXiv:1909.08605) の GNC-TLS ロバスト推定機構を
LiDAR オドメトリに適用した from-paper 再構成。著者の参照実装は認識系ライブラリ内に
あり、本リポジトリ向け scan-to-map オドメトリとしての公開実装は無い。

## 動機

ロバスト化には M 推定子 (Cauchy/Huber/Student-t) のように **固定の** ロバストカーネルを
使う方法があるが、強い非凸カーネルは局所解に陥りやすい。GNC は **代理関数の非凸性
そのものを連続変化** させる継続法で、凸に近い問題から始めて徐々に真の非凸コスト
(Truncated Least Squares) へ近づけることで、広い収束域と高い外れ値耐性を両立する。

## アプローチ

scan-to-map point-to-plane を GNC-TLS で解く。

- Truncated Least Squares (TLS) を Black-Rangarajan 双対で重み付き最小二乗化。
- 制御変数 μ を凸寄りの初期値 (最大残差から `μ⁰ = c̄² / (2 r_max² − c̄²)`) から始め、
  反復ごとに `μ ← gnc_factor·μ` で非凸 TLS へ近づける。
- 各 μ で GNC-TLS 重み
  - `w = 1`                          (`r² ≤ μ/(μ+1) c̄²`,  インライア)
  - `w = 0`                          (`r² ≥ (μ+1)/μ c̄²`,  アウトライア)
  - `w = (c̄/|r|)√(μ(μ+1)) − μ`        (その間, ソフト)

  を計算し point-to-plane を Gauss-Newton で解く。μ が大きくなると外れ値は重み 0 で
  **hard 棄却** される。

`c̄` (truncation) はインライア残差の上限 [m]。

## パラメータ

- `gnc_truncation` (既定 0.3 m): 切断しきい値 c̄。
- `gnc_factor` (既定 1.4): 継続係数 (μ ← factor·μ)。
- `gnc_mu_init` (既定 0=自動): μ 初期値 (≤0 で最大残差から凸スタートに自動設定)。
- `voxel_size` (既定 1.0), `planarity_threshold` (既定 0.5) ほか共通。

## テスト

`test_gnc_lo` の 3 ケース:

1. `WeightGraduatesFromConvexToTls` — μ→∞ で hard 判定、μ 小で大残差に寛容。
2. `RecoversForwardTranslation` — 良条件で前進並進を回復。
3. `RobustToHighOutlierRatio` — 高外れ値比でも GNC が前進を回復。

## KITTI full での所見

KITTI full (no GT seed) では seq00 RPE **0.986%** / seq07 **0.722%**、ATE 17.5/2.3 m、
2.6 FPS。mid-pack で発散せず健全。

**正直な所見**: GNC の真価 (高い外れ値比・大きな初期誤差での広い収束域) は KITTI では
発揮されにくい。CV 予測で初期姿勢が良好かつ整備済みの幾何リッチなスキャンでは外れ値が
少なく、`mean_inlier_ratio ≈ 0.86` (約 14% を hard 棄却) はやや過剰で、有効な対応まで
切り落として精度を僅かに損なう (Student-T-LO 0.952 / GMM-LO 0.941 をわずかに下回る)。
継続法と TLS の正しさ (μ による凸→非凸の重み変化・hard 棄却) はユニットテストで担保。
GNC は外れ値が支配的/初期誤差が大きい設定でこそ有利で、その意味で本結果は「整備済み
KITTI で robust 機構が near-redundant〜やや過剰」という恒例知見と整合する honest な結果。
`--gnc-lo-truncation / --gnc-lo-factor` でアブレーション可能。
