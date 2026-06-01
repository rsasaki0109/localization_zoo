# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02_start0000_60 | 32 / 59 | 0.897 | 0.046 | {'small_gicp': 28, 'kiss': 31} |
| kitti_seq_02_start0030_60 | 28 / 59 | 0.978 | 0.082 | {'small_gicp': 32, 'kiss': 27} |
| kitti_seq_02_start0060_60 | 33 / 59 | 0.932 | 0.106 | {'kiss': 27, 'small_gicp': 32} |
| kitti_seq_02_start0090_60 | 28 / 59 | 0.743 | 0.094 | {'kiss': 29, 'small_gicp': 30} |
| kitti_seq_05_start0000_60 | 41 / 59 | 0.232 | 0.048 | {'kiss': 19, 'small_gicp': 40} |
| kitti_seq_05_start0030_60 | 32 / 59 | 0.334 | 0.068 | {'small_gicp': 26, 'kiss': 33} |
| kitti_seq_05_start0060_60 | 33 / 59 | 0.581 | 0.065 | {'kiss': 31, 'small_gicp': 28} |
| kitti_seq_05_start0090_60 | 37 / 59 | 0.237 | 0.047 | {'kiss': 20, 'small_gicp': 39} |
| kitti_seq_08_start0000_60 | 33 / 59 | 1.587 | 0.068 | {'small_gicp': 54, 'kiss': 5} |
| kitti_seq_08_start0030_60 | 28 / 59 | 1.999 | 0.061 | {'small_gicp': 51, 'kiss': 8} |
| kitti_seq_08_start0060_60 | 40 / 59 | 1.363 | 0.050 | {'small_gicp': 51, 'kiss': 8} |
| kitti_seq_08_start0090_60 | 41 / 59 | 1.609 | 0.042 | {'small_gicp': 50, 'kiss': 9} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.066836 |
| cubemap_score | 0.000000 |
| geometry_score | 0.029844 |
| log_overlap | -0.028854 |
| motion_norm | 0.001709 |
| abs_yaw_deg | -0.002877 |
| shared_residual_score | 0.023687 |
| shared_residual_rmse | 0.009006 |
| shared_inlier_ratio | -0.019174 |
| shared_stability_score | -0.066951 |
| shared_stability_mean_delta | 0.071923 |
| shared_stability_min_delta | 0.008304 |
