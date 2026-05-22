# Pair Prior Selector Diagnostics

- GT: `experiments/reference_data/kitti_seq_02_108_gt.csv`
- Selector: `experiments/results/intensity_odometry/selected_prior_small_margin001_kitti_seq_02_30.json`
- Selector pair accuracy: 16/29
- Selected counts: {'small_gicp': 21, 'kiss': 8}
- Oracle counts: {'kiss': 19, 'small_gicp': 10}
- Signal hits: {'cubemap_score': 21, 'geometry_score': 19, 'overlap': 19, 'shared_residual_score': 15, 'shared_residual_rmse': 16, 'shared_stability_score': 20, 'shared_stability_mean_delta': 20} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.425 | 0.040 |
| small_gicp | 0.566 | 0.062 |
| oracle_pair_error | 0.325 | 0.031 |
| selector | 0.353 | 0.054 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.040 | 0.035 | 0.077 |
| small_gicp | 0.056 | 0.045 | 0.109 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
| 1 | small_gicp | kiss | 0.115 | 0.036 |
| 4 | small_gicp | kiss | 0.039 | 0.024 |
| 5 | small_gicp | kiss | 0.081 | 0.045 |
| 6 | small_gicp | kiss | 0.033 | 0.030 |
| 7 | small_gicp | kiss | 0.080 | 0.030 |
| 9 | small_gicp | kiss | 0.086 | 0.032 |
| 10 | small_gicp | kiss | 0.032 | 0.026 |
| 20 | kiss | small_gicp | 0.062 | 0.035 |
| 21 | small_gicp | kiss | 0.057 | 0.056 |
| 23 | small_gicp | kiss | 0.097 | 0.052 |
| 24 | small_gicp | kiss | 0.087 | 0.072 |
| 26 | small_gicp | kiss | 0.150 | 0.080 |
| 28 | small_gicp | kiss | 0.075 | 0.053 |
