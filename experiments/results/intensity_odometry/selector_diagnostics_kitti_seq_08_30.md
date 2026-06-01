# Pair Prior Selector Diagnostics

- GT: `experiments/reference_data/kitti_seq_08_108_gt.csv`
- Selector: `experiments/results/intensity_odometry/selected_prior_small_margin001_kitti_seq_08_30.json`
- Selector pair accuracy: 18/29
- Selected counts: {'small_gicp': 20, 'kiss': 9}
- Oracle counts: {'small_gicp': 19, 'kiss': 10}
- Signal hits: {'cubemap_score': 17, 'geometry_score': 10, 'overlap': 10, 'shared_residual_score': 18, 'shared_residual_rmse': 20, 'shared_stability_score': 21, 'shared_stability_mean_delta': 20} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.877 | 0.082 |
| small_gicp | 0.530 | 0.067 |
| oracle_pair_error | 0.396 | 0.048 |
| selector | 0.560 | 0.065 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.079 | 0.071 | 0.156 |
| small_gicp | 0.064 | 0.061 | 0.125 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
| 2 | small_gicp | kiss | 0.083 | 0.046 |
| 3 | kiss | small_gicp | 0.067 | 0.038 |
| 5 | kiss | small_gicp | 0.051 | 0.009 |
| 6 | small_gicp | kiss | 0.054 | 0.040 |
| 11 | small_gicp | kiss | 0.044 | 0.007 |
| 12 | kiss | small_gicp | 0.088 | 0.053 |
| 18 | kiss | small_gicp | 0.168 | 0.071 |
| 21 | small_gicp | kiss | 0.048 | 0.042 |
| 22 | kiss | small_gicp | 0.045 | 0.028 |
| 25 | small_gicp | kiss | 0.077 | 0.044 |
| 26 | small_gicp | kiss | 0.111 | 0.071 |
