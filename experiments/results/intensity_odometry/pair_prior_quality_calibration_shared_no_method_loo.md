# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 17 / 29 | 0.390 | 0.051 | {'small_gicp': 22, 'kiss': 7} |
| kitti_seq_05 | 20 / 29 | 0.349 | 0.033 | {'kiss': 9, 'small_gicp': 20} |
| kitti_seq_08 | 23 / 29 | 0.499 | 0.059 | {'small_gicp': 23, 'kiss': 6} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | 0.004668 |
| geometry_score | 0.017559 |
| log_overlap | -0.021865 |
| motion_norm | 0.004792 |
| abs_yaw_deg | -0.007861 |
| shared_residual_score | -0.031980 |
| shared_residual_rmse | 0.010480 |
| shared_inlier_ratio | 0.018137 |
| shared_stability_score | 0.072160 |
| shared_stability_mean_delta | -0.044498 |
| shared_stability_min_delta | 0.011221 |
