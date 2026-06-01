# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 13 / 29 | 0.504 | 0.057 | {'small_gicp': 26, 'kiss': 3} |
| kitti_seq_05 | 17 / 29 | 0.520 | 0.041 | {'kiss': 14, 'small_gicp': 15} |
| kitti_seq_08 | 16 / 29 | 0.638 | 0.072 | {'small_gicp': 24, 'kiss': 5} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | -0.002120 |
| geometry_score | 0.018271 |
| log_overlap | -0.022289 |
| motion_norm | 0.004262 |
| abs_yaw_deg | -0.009849 |
