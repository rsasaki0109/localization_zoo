# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_08_start0030_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 37, 'small_gicp': 22}
- Signal hits: {'geometry_score': 37, 'overlap': 37, 'shared_residual_score': 37, 'shared_residual_rmse': 42, 'shared_stability_score': 37, 'shared_stability_mean_delta': 38} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.238 | 0.061 |
| small_gicp | 2.178 | 0.064 |
| oracle_pair_error | 1.063 | 0.047 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.059 | 0.045 | 0.151 |
| small_gicp | 0.068 | 0.063 | 0.112 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
