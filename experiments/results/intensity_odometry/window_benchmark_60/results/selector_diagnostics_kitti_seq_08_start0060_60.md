# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_08_start0060_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 34, 'kiss': 25}
- Signal hits: {'geometry_score': 25, 'overlap': 25, 'shared_residual_score': 27, 'shared_residual_rmse': 32, 'shared_stability_score': 35, 'shared_stability_mean_delta': 36} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.539 | 0.059 |
| small_gicp | 1.360 | 0.050 |
| oracle_pair_error | 1.333 | 0.044 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.064 | 0.062 | 0.120 |
| small_gicp | 0.056 | 0.054 | 0.100 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
