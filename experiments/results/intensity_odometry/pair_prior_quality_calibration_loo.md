# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 13 / 29 | 0.504 | 0.057 | {'small_gicp': 26, 'kiss': 3} |
| kitti_seq_05 | 4 / 29 | 0.765 | 0.055 | {'kiss': 29} |
| kitti_seq_08 | 10 / 29 | 0.877 | 0.082 | {'kiss': 29} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | -0.002547 |
| geometry_score | 0.019274 |
| log_overlap | -0.012142 |
| motion_norm | 0.004335 |
| abs_yaw_deg | -0.009851 |
| is_kiss | -0.005543 |
| is_small_gicp | 0.005543 |
