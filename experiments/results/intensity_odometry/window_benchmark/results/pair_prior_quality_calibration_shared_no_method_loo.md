# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02_start0000 | 18 / 29 | 0.371 | 0.043 | {'kiss': 20, 'small_gicp': 9} |
| kitti_seq_02_start0030 | 16 / 29 | 0.398 | 0.045 | {'small_gicp': 13, 'kiss': 16} |
| kitti_seq_02_start0060 | 14 / 29 | 0.462 | 0.115 | {'kiss': 15, 'small_gicp': 14} |
| kitti_seq_02_start0090 | 13 / 29 | 0.676 | 0.101 | {'kiss': 16, 'small_gicp': 13} |
| kitti_seq_05_start0000 | 21 / 29 | 0.298 | 0.031 | {'kiss': 8, 'small_gicp': 21} |
| kitti_seq_05_start0030 | 14 / 29 | 0.362 | 0.072 | {'small_gicp': 11, 'kiss': 18} |
| kitti_seq_05_start0060 | 15 / 29 | 0.498 | 0.078 | {'kiss': 21, 'small_gicp': 8} |
| kitti_seq_05_start0090 | 15 / 29 | 0.366 | 0.056 | {'kiss': 16, 'small_gicp': 13} |
| kitti_seq_08_start0000 | 22 / 29 | 0.509 | 0.063 | {'small_gicp': 24, 'kiss': 5} |
| kitti_seq_08_start0030 | 12 / 29 | 1.086 | 0.072 | {'small_gicp': 26, 'kiss': 3} |
| kitti_seq_08_start0060 | 18 / 29 | 0.710 | 0.047 | {'small_gicp': 22, 'kiss': 7} |
| kitti_seq_08_start0090 | 21 / 29 | 0.755 | 0.052 | {'kiss': 4, 'small_gicp': 25} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.065204 |
| cubemap_score | 0.000000 |
| geometry_score | 0.030304 |
| log_overlap | -0.027670 |
| motion_norm | 0.002887 |
| abs_yaw_deg | -0.002815 |
| shared_residual_score | 0.020269 |
| shared_residual_rmse | 0.008632 |
| shared_inlier_ratio | -0.014939 |
| shared_stability_score | -0.069977 |
| shared_stability_mean_delta | 0.074630 |
| shared_stability_min_delta | 0.010048 |
