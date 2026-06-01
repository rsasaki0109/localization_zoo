# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_02_start0060_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 26, 'kiss': 33}
- Signal hits: {'geometry_score': 33, 'overlap': 33, 'shared_residual_score': 33, 'shared_residual_rmse': 34, 'shared_stability_score': 31, 'shared_stability_mean_delta': 31} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.234 | 0.095 |
| small_gicp | 1.308 | 0.122 |
| oracle_pair_error | 1.123 | 0.072 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.093 | 0.096 | 0.177 |
| small_gicp | 0.119 | 0.095 | 0.342 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
