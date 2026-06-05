# GMM-LO

Gaussian-Mixture / EM ソフト割当 LiDAR オドメトリ。**Fast and Accurate Point
Cloud Registration using Trees of Gaussian Mixtures** (HGMR, Eckart et al., 2018,
arXiv:1807.02587) の GMM-EM 登録機構を論文記述から compact 再構成したもの。著者
公開コードは無い。

## 動機

ICP は最近傍 (hard NN) の 1:1 対応、NDT はボクセル単位 (hard voxel) の対応を取る。
どちらも離散的な割当で、初期誤差が大きいと誤対応に陥りやすい。GMM/EM は各ソース点を
近傍モデル点群が成す混合分布へ **ソフトに対応付け** (responsibility γ)、期待対数尤度を
最大化する。割当が滑らかなので収束域が広く、外れ値も responsibility で自然に減衰する。

## アプローチ

scan-to-map で EM を回す。

- **E-step**: 各ソース点 `p_i` (現姿勢 T で変換) に対し近傍モデル点 `q_j` の
  responsibility を `γ_ij ∝ exp(-‖Tp_i - q_j‖² / 2σ²)` で計算。一様アウトライア項を
  分母に加えて正規化し、ソフト重心 `m_i = Σ_j γ_ij q_j / Σ_j γ_ij` とインライア信頼度
  `w_i = W/(W+outlier)` (W=Σγ) を得る。
- **M-step**: `Σ_i w_i [n_i·(Tp_i - m_i)]²` を Gauss-Newton で最小化 (ソフト重心への
  point-to-plane、`n_i` は responsibility 重み付き近傍共分散の最小固有ベクトル)。
- **Deterministic annealing**: σ を反復で `sigma_init → sigma_final` に幾何級数で
  絞り、粗 (広い収束域・滑らかな割当) から密 (シャープな割当、ほぼ hard NN) へ移行。

完全な GMM tree (階層混合) は範囲外で、単一スケールのボクセル近傍 GMM に圧縮している。

## パラメータ

- `sigma_init` (既定 1.2 m): annealing 初期 σ (広い収束域)。
- `sigma_final` (既定 0.25 m): annealing 最終 σ (シャープ割当)。
- `outlier_weight` (既定 0.1): 一様アウトライア項 (外れ値の自動棄却)。
- `neighbor_radius_scale` (既定 3.0): 近傍探索半径 = scale*σ。
- `voxel_size` (既定 1.0), `planarity_threshold` (既定 0.5) ほか共通。

## テスト

`test_gmm_lo` の 3 ケース:

1. `SoftCentroidBlendsWithSigma` — σ 小で近クラスタ寄り、σ 大で両クラスタ中間へ。
2. `RecoversForwardTranslation` — 良条件で前進並進を回復。
3. `RobustToOutliers` — アウトライア混入下で EM ソフト割当が前進を回復。

## KITTI full での所見

KITTI full (no GT seed) では seq00 RPE **0.941%** / seq07 **0.657%**、ATE 14.4/1.4 m。
ATE は低めで健全だが RPE は KISS-ICP (0.872/0.618%) を僅かに下回る mid-pack。

**正直な所見**: GMM/EM ソフト割当は **annealing 後に near-hard へ収束**する
(平均 responsibility `mean_weight ≈ 0.98`)。整備済みで幾何リッチな KITTI では
ソフト割当の優位 (広い収束域・誤対応耐性) が出にくく、CV 予測で初期姿勢が既に良いため
front-end は実質 point-to-plane ICP に近づく ─ 本リポジトリ恒例の「整備済み KITTI で
機構が near-redundant」知見と整合。コストは速度 (~2.3 FPS: 対応ごとに responsibility と
重み付き共分散を計算)。ソフト割当の優位は初期誤差が大きい/重畳が少ない設定で出るため、
機構の正しさ (σ 依存ブレンド・アウトライア棄却) はユニットテストで担保している。
`--gmm-lo-sigma-init/-sigma-final/-outlier-weight` でアブレーション可能。
