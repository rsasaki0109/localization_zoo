# TrICP-LO

Trimmed / Fractional ICP (最小トリム二乗, Least Trimmed Squares) point-to-plane
LiDAR オドメトリ。**The Trimmed Iterative Closest Point Algorithm** (D. Chetverikov
et al., ICPR 2002) / **Robust Euclidean alignment of 3D point sets: the trimmed ICP
algorithm** (Image Vis. Comput. 2005) と **Outlier Robust ICP for Minimizing
Fractional RMSD** (J. Phillips et al., 3DIM 2007) のロバスト推定機構を scan-to-map
LiDAR オドメトリへ適用した from-paper 再構成。原論文は対点群レジストレーションのみで、
本リポジトリ向けオドメトリパイプラインとしての公開実装は無い。

## 動機

M 推定子 (Student-T / Cauchy / MCC) や GNC は残差の**大きさ**に応じて連続的に重みを
下げる (magnitude-based)。対して TrICP は残差を**順位**でソートし、最良 ξ 割合のみを
最小二乗に用いる **最小トリム二乗 (LTS)** — 高破壊点 (high-breakdown) ロバスト統計の
別系統。さらにオーバーラップ率 ξ をデータから**自動推定**できるため、連続スキャン間で
必ず生じる非オーバーラップ領域 (新出/消失・動物体) を明示的に棄却できる。

## アプローチ

scan-to-map point-to-plane を **LTS** で解く。

- 各反復で全有効対応の point-to-plane 残差 `r = n·(Tp − anchor)` を計算。
- 二乗残差を昇順ソートし、最小 `⌊ξN⌋` 個のみを最小二乗に採用 (順位トリミング)。
- オーバーラップ率 ξ は **FRMSD 最小化** (FICP) で自動推定:
  `FRMSD(ξ) = sqrt(e(ξ)) / ξ^λ`、`e(ξ)` は最小 `⌊ξN⌋` 個の二乗残差平均。
  ξ を縮めれば `e` は下がるが `ξ^λ` ペナルティが増すため、外れ値を含み始める直前の
  ξ で最小化される。`min_overlap` を下限とする。
- 採用した inlier 集合のみで Gauss-Newton 更新。

## パラメータ

- `auto_overlap` (既定 true): FRMSD で ξ を自動推定。false で固定 ξ。
- `overlap_ratio` (既定 0.9): 固定モード時の ξ。
- `min_overlap` (既定 0.4): 自動推定の下限 (拘束点不足を防ぐ)。
- `frmsd_lambda` (既定 1.5): FRMSD ペナルティ指数 λ。
- `voxel_size` (既定 1.0), `planarity_threshold` (既定 0.5) ほか共通。

CLI: `--tricp-lo-{fast,dense}-profile`, `--tricp-lo-overlap <ξ>` (固定 ξ),
`--tricp-lo-fixed-overlap` (自動推定を切る), `--tricp-lo-lambda <λ>`。

## テスト

`test_tricp_lo` の 3 ケース:

1. `FrmsdEstimatesOverlapFraction` — 70% inlier + 30% outlier で推定 ξ ≈ 0.7。
2. `TrimmedMeanIgnoresOutliers` — トリム平均が順位で外れ値を無視する。
3. `TrimmingRejectsOutliers` — 既整合 + 外れ値ブロックで、トリミングが外れ値を棄却し
   姿勢が引きずられない (トリミング無し ξ=1 より明確に頑健)。

> 注: TrICP/FICP は**粗整合を前提**とする手法。大きな初期ずれ + 退化幾何 (軸平行な箱
> など) では、整合済みの 0 残差点だけが残り並進信号を持つ点を過剰トリムしうる。本
> パイプラインでは CV 予測が良好な初期姿勢を供給するため、この退化は実データで生じない。

## KITTI full での所見

KITTI full (no GT seed, `--tricp-lo-dense-profile`):

| seq | RPE drift | ATE | FPS | mean ξ | 参照 KISS-ICP (同データ) |
|---|---:|---:|---:|---:|---|
| 00 (4541 fr) | 0.931% | 10.04 m | 5.6 | 0.800 | 0.872% / 12.0 m |
| 07 (1101 fr) | 0.662% | 2.00 m | 6.2 | 0.800 | 0.618% / 1.8 m |

**所見**: TrICP-LO は KISS-ICP に RPE で僅差で劣るが (0.931 vs 0.872 / 0.662 vs 0.618)、
**seq00 ATE は 10.04 m と KISS-ICP の 12.0 m を下回り**、IMLS-SLAM(29) や robust 群の
多くより良好 — competitive。

**FRMSD 自動推定の所見 (honest)**: 推定オーバーラップ ξ は両 seq とも下限 0.800 に
張り付いた (mean ξ=0.800)。整備済み KITTI ではセンサノイズで残差が**連続分布**するため、
FRMSD は「ノイズで残差大の良点」を「非オーバーラップ」と誤認し ξ を縮め続ける (FICP を
高オーバーラップ odometry に適用した際の既知の弱点)。結果として TrICP は実質「最良 80%
分位を採る固定トリム = ロバスト分位 point-to-plane」として動作し、KISS-ICP に肉薄する。
LTS の順位トリミング機構そのものは有効 (ユニットテスト参照)。`--tricp-lo-overlap` /
`--tricp-lo-lambda` でアブレーション可能。

```
pcd_dogfooding <seq_pcd_dir> <seq_gt_csv> \
  --methods tricp_lo --tricp-lo-dense-profile --no-gt-seed \
  --summary-json results.json
```
