# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02_start0000 | 19 / 29 | 0.328 | 0.042 | {'kiss': 19, 'small_gicp': 10} |
| kitti_seq_02_start0030 | 8 / 29 | 0.285 | 0.051 | {'small_gicp': 25, 'kiss': 4} |
| kitti_seq_05_start0000 | 16 / 29 | 0.337 | 0.034 | {'kiss': 15, 'small_gicp': 14} |
| kitti_seq_05_start0030 | 13 / 29 | 0.499 | 0.065 | {'small_gicp': 10, 'kiss': 19} |
| kitti_seq_08_start0000 | 21 / 29 | 0.594 | 0.063 | {'small_gicp': 15, 'kiss': 14} |
| kitti_seq_08_start0030 | 15 / 29 | 0.907 | 0.068 | {'small_gicp': 23, 'kiss': 6} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.057142 |
| cubemap_score | 0.000000 |
| geometry_score | 0.011525 |
| log_overlap | -0.011115 |
| motion_norm | 0.000548 |
| abs_yaw_deg | -0.005920 |
| shared_residual_score | 0.007248 |
| shared_residual_rmse | 0.019695 |
| shared_inlier_ratio | -0.009166 |
| shared_stability_score | 0.013815 |
| shared_stability_mean_delta | 0.000966 |
| shared_stability_min_delta | 0.013257 |
