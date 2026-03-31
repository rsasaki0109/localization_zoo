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
