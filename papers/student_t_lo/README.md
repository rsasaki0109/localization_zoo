# Student-T-LO

Heavy-tail ロバスト LiDAR オドメトリ。**Research on Student's T-Distribution
Point Cloud Registration Algorithm Based on Local Features** (2024,
PMC11314997) の機構を論文記述から独自再構成したもの。著者による公開実装は無い。

## 動機

LiDAR の残差は laser speckle・データ欠損・動的物体により **重尾 (heavy-tail)
ノイズ** を含む。Gaussian を仮定する標準 ICP / NDT / GICP は、二乗誤差が外れ値に
過敏で系統誤差を生む。残差分布を **Student's-t 分布** に置き換えると、重尾により
外れ値を滑らかに減衰でき、hard クリップ無しで頑健化できる。

## アプローチ

scan-to-map の point-to-plane 残差 `r = nᵀ(p − q)` を Student's-t 分布でモデル化し、
**IRLS (反復再重み付け最小二乗)** で解く。

- **E-step**: 各対応に重み
  `w = (ν + 1) / (ν + (r/σ)²)` を与える (正規化して r=0 で w=1)。
- **M-step**: スケールを `σ² = Σ wᵢ rᵢ² / N` で EM 更新し、フレームの残差分布に
  追従させる (adaptive scale)。

自由度 `ν` は Cauchy カーネル (ν=1) を一般化し、`ν → ∞` で Gaussian (重み ≈ 1、
減衰しない最小二乗) に近づく。M 推定子としては **ν が小さいほど大残差を強く減衰
= 頑健** になる (`ν=1` の Cauchy が最も頑健、Gaussian は減衰なし)。

## パラメータ

- `enable_student_t` (既定 true): false で従来 Gaussian カーネルに退化 (アブレーション)。
- `student_t_dof` (既定 5.0): 自由度 ν。小さいほど重尾でアウトライアに頑健。
- `scale_init` (既定 0.5 m): 残差スケール σ の初期値。
- `estimate_scale` (既定 true): σ を EM で各反復更新する。
- `scale_floor` (既定 0.05 m): σ の下限 (0 への崩壊を防ぐ)。

## テスト

`test_student_t_lo` の 3 ケース:

1. `WeightDownweightsOutliers` — r=0 で w=1、単調減少、ν 小 ほど大残差を強く減衰。
2. `RecoversForwardTranslation` — 良条件で前進並進を回復。
3. `RobustToOutliers` — アウトライア混入下で重尾重み付けが前進並進を頑健に回復。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では seq00 RPE 0.952% / seq07 0.696% で
**mid-pack (KISS-ICP 0.872%/0.618% を僅かに下回る)**、発散せず健全。

**正直な所見**: Student-t ロバスト化は KITTI では **near-redundant**。voxel
(0.5/1.0 m) + range gating + `|e|>kernel` クリップを通った後の残差には重尾外れ値が
ほとんど残らず、平均重み `mean_weight ≈ 0.80` (約 20% の減衰のみ)、推定スケール
`σ ≈ 0.12 m` に収束する。整備済みの幾何リッチな KITTI では front-end が実質
KISS-ICP 相当の point-to-plane に落ちる、という本リポジトリ恒例の知見と整合する。
機構自体の正しさ (重尾減衰・EM スケール) はユニットテストで担保している。CLI の
`--student-t-lo-dof / --student-t-lo-gaussian / --student-t-lo-fixed-scale` で
自由度・Gaussian 退化・固定スケールのアブレーションが可能。
