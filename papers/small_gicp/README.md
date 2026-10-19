# small_gicp

lightweight `small_gicp` 再現版です。voxel downsample した代表点に対して近傍共分散を
推定し、対応点数を capped にした compact GICP を回します。

この実装で入れている要素:

- voxel downsample した source / target
- downsample 後の kNN covariance 推定
- capped nearest-neighbor correspondence
- compact Gauss-Newton based GICP optimization

full original library の並列化や細かな実装最適化を完全移植したものではなく、この repo
の `GICP` / `Voxel-GICP` の間を埋める lightweight registration 版です。

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 5 selected stored Small-GICP full-sequence cells complete with native exits 0. 5 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
