# Pair Prior Quality Calibration

Ridge model target: negative GT pair translation error.

## Leave-One-Sequence-Out

| Held-out | Hits | ATE xy [m] | Step RMSE [m] | Selected counts |
| --- | ---: | ---: | ---: | --- |
| kitti_seq_02 | 8 / 29 | 0.351 | 0.059 | {'small_gicp': 15, 'kiss': 14} |
| kitti_seq_05 | 13 / 29 | 0.496 | 0.040 | {'kiss': 14, 'small_gicp': 15} |
| kitti_seq_08 | 12 / 29 | 0.817 | 0.082 | {'kiss': 13, 'small_gicp': 16} |

## Full-Model Weights

| Feature | Weight |
| --- | ---: |
| bias | -0.051298 |
| cubemap_score | -0.008750 |
