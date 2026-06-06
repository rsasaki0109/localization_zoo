# IMLS-SLAM

暗黙的移動最小二乗 (Implicit Moving Least Squares, IMLS) 曲面への scan-to-model
マッチングによる LiDAR オドメトリ。**IMLS-SLAM: scan-to-model matching based on
3D data** (J.-E. Deschaud, ICRA 2018, arXiv:1802.08633) の from-paper 再構成。
著者は後発の **CT-ICP** は公開したが、IMLS-SLAM 本体の実装は未公開。本リポジトリ
向けオドメトリパイプラインとしての公開コードも無い。

## 動機

多くの LiDAR オドメトリは、新スキャンの各点を局所近傍の単一平面 (PCA 平面) に当てる
point-to-plane で解く。これは曲面を区分平面で近似するため、近傍点の取り方に敏感で、
スパースな点群では面の表現が粗い。IMLS は過去スキャンの**有向点群が定義する滑らかな
暗黙的曲面**に点を当てることで、複数の有向点を距離で重み付け補間した高次の局所曲面
表現を得る。単一最近傍平面 (mcc_lo / 既存の robust 系) や voxel-surfel
(CT-VoxelMap / R-VoxelMap) とは別系統の幾何当てはめ機構。

## アプローチ

過去 N スキャンの有向点群 `{p_i, n_i}` が暗黙的曲面

```
I^P(x) = Σ_i W_i(x) (x − p_i)·n_i / Σ_i W_i(x),   W_i(x) = exp(−||x − p_i||² / h²)
```

を定義する。`I^P(x)` は点 `x` の符号つき曲面距離で、零等位面が再構成曲面。新スキャン
のサンプル点 `x_j` を曲面へ投影 `y_j = x_j − I^P(x_j)·n_j` し、

```
min_{R,t} Σ_j |n_j·(R x_j + t − y_j)|²
```

を小角近似の Gauss-Newton で解く (point-to-implicit-surface)。`n_j` は最近傍モデル
点の法線。残差は曲面距離 `I^P` そのもので、法線方向の point-to-plane として線形化する。

### 観測性ベースのサンプリング

全点ではなく、**6-DoF を最もよく拘束する点**を選んで解く。各点の法線 `n` と位置 `p`
(センサ原点基準) から、並進拘束 `|n·e_a|` と回転拘束 `|(p×n)·e_a|` を 3 並進軸 +
3 回転軸の 6 リストで評価し、各リスト上位 `samples_per_axis` 点の和集合を採る。これに
より、ある方向に偏ったスキャンでも稀少な拘束点を取りこぼさず、効率と条件数を改善する
(原論文の 9-score サンプリングの近似)。

## パラメータ

- `imls_h` (既定 0.5 m): IMLS カーネルバンド幅 `h`。
- `imls_support_radius` (既定 0 → `3*imls_h`): 曲面を定義する支持半径。
- `max_correspondence_dist` (既定 1.0 m): 最近傍点ゲート `r`。
- `samples_per_axis` (既定 100): 各観測性リストから採る点数 (原論文 `s=100`)。
- `use_observability_sampling` (既定 true): false で全有効点を使用 (`--imls-slam-no-sampling`)。
- `voxel_size` (既定 1.0), `normal_radius` (既定 0 → `1.5*voxel_size`) ほか共通。

**密度に関する注意**: 原論文は生 HDL64 密度向けに `h=0.06` / `r=0.20` を用いるが、
本実装はマップをボクセルダウンサンプル (`voxel_size` 既定 1.0 m) するため、`h` と `r`
をマップ密度に合わせて拡大している。`--imls-slam-h` でアブレーション可能。

## テスト

`test_imls_slam` の 3 ケース:

1. `ImplicitSurfaceDistanceMatchesPlane` — 平面モデルへの IMLS 距離が高さに一致し、
   遠近の有向点を距離で重み付けする (Moving Least Squares の局所性)。
2. `ObservabilitySamplingCoversAllAxes` — 1 方向に偏ったスキャンでも各軸を拘束する
   稀少点を拾い、3 並進軸が代表される。
3. `RecoversForwardTranslation` — scan-to-implicit-model で前進並進を回復する。

## KITTI full での所見

> KITTI full (no GT seed) での seq00/seq07 評価は、評価マシン (KITTI 点群 +
> Ceres ビルド環境) で実行して反映する。本アーカイブ環境には点群データが無いため
> 未測定。実行コマンドは下記。

```
pcd_dogfooding <seq_pcd_dir> <seq_gt_csv> \
  --methods imls_slam --imls-slam-dense-profile --no-gt-seed \
  --summary-json results.json
```
