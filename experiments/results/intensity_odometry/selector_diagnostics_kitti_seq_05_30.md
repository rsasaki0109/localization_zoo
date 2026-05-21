# Pair Prior Selector Diagnostics

- GT: `experiments/reference_data/kitti_seq_05_108_gt.csv`
- Selector: `experiments/results/intensity_odometry/selected_prior_small_margin001_kitti_seq_05_30.json`
- Selector pair accuracy: 20/29
- Selected counts: {'small_gicp': 20, 'kiss': 9}
- Oracle counts: {'small_gicp': 25, 'kiss': 4}
- Signal hits: {'cubemap_score': 16, 'geometry_score': 4, 'overlap': 4, 'shared_residual_score': 15, 'shared_residual_rmse': 13, 'shared_stability_score': 19, 'shared_stability_mean_delta': 21} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.765 | 0.055 |
| small_gicp | 0.191 | 0.017 |
| oracle_pair_error | 0.188 | 0.015 |
| selector | 0.378 | 0.033 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.051 | 0.047 | 0.106 |
| small_gicp | 0.019 | 0.019 | 0.035 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
| 2 | kiss | small_gicp | 0.082 | 0.019 |
| 6 | kiss | small_gicp | 0.062 | 0.023 |
| 9 | kiss | small_gicp | 0.082 | 0.034 |
| 11 | kiss | small_gicp | 0.080 | 0.011 |
| 13 | kiss | small_gicp | 0.041 | 0.024 |
| 17 | small_gicp | kiss | 0.013 | 0.005 |
| 20 | small_gicp | kiss | 0.020 | 0.002 |
| 23 | kiss | small_gicp | 0.031 | 0.012 |
| 29 | kiss | small_gicp | 0.060 | 0.013 |
