# Voxel-GICP

fast_gicp 系の発想に寄せた、ボクセル代表点ベースの GICP 実装。

## 核心

- 点群をボクセル化して各ボクセルの重心と共分散を計算
- ソース・ターゲットともに代表点で GICP を解く
- 点単位 GICP より対応点数を減らして高速化する

## 実装メモ

- 共分散はボクセル内 PCA から構築
- 対応点探索はターゲット代表点への最近傍探索
- 最適化は SE(3) 上の Gauss-Newton

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 5 selected stored Voxel-GICP full-sequence cells complete with native exits 0. 5 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
