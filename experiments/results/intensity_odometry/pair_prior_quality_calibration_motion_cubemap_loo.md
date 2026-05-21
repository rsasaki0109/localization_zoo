# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 15 / 29 | 0.397 | 0.052 | {'small_gicp': 10, 'kiss': 19} |
| kitti_seq_05 | 17 / 29 | 0.477 | 0.039 | {'small_gicp': 13, 'kiss': 16} |
| kitti_seq_08 | 18 / 29 | 0.648 | 0.074 | {'small_gicp': 14, 'kiss': 15} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | -0.003145 |
| motion_norm | 0.000407 |
| abs_yaw_deg | -0.011663 |
