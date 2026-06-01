# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 15 / 29 | 0.400 | 0.053 | {'small_gicp': 20, 'kiss': 9} |
| kitti_seq_05 | 6 / 29 | 0.677 | 0.051 | {'kiss': 27, 'small_gicp': 2} |
| kitti_seq_08 | 11 / 29 | 0.824 | 0.078 | {'kiss': 28, 'small_gicp': 1} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | 0.003977 |
| geometry_score | 0.020527 |
| log_overlap | 0.024367 |
| motion_norm | 0.006919 |
| abs_yaw_deg | -0.007493 |
| shared_residual_score | -0.019218 |
| shared_residual_rmse | 0.013162 |
| shared_inlier_ratio | 0.010083 |
| shared_stability_score | 0.076913 |
| shared_stability_mean_delta | -0.050070 |
| shared_stability_min_delta | 0.010622 |
| is_kiss | -0.024486 |
| is_small_gicp | 0.024486 |
