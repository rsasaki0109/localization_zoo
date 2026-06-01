# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_05_start0000_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 48, 'kiss': 11}
- Signal hits: {'geometry_score': 11, 'overlap': 11, 'shared_residual_score': 30, 'shared_residual_rmse': 23, 'shared_stability_score': 40, 'shared_stability_mean_delta': 42} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.620 | 0.071 |
| small_gicp | 0.172 | 0.030 |
| oracle_pair_error | 0.157 | 0.028 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.062 | 0.057 | 0.147 |
| small_gicp | 0.029 | 0.026 | 0.060 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
