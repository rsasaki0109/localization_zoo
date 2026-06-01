# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_08_start0000_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 28, 'kiss': 31}
- Signal hits: {'geometry_score': 31, 'overlap': 31, 'shared_residual_score': 45, 'shared_residual_rmse': 45, 'shared_stability_score': 44, 'shared_stability_mean_delta': 43} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.309 | 0.077 |
| small_gicp | 1.825 | 0.073 |
| oracle_pair_error | 0.741 | 0.050 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.069 | 0.059 | 0.169 |
| small_gicp | 0.072 | 0.070 | 0.122 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
