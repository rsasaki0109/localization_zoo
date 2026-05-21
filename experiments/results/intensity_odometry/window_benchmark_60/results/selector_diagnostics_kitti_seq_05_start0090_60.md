# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_05_start0090_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 43, 'kiss': 16}
- Signal hits: {'geometry_score': 16, 'overlap': 16, 'shared_residual_score': 30, 'shared_residual_rmse': 27, 'shared_stability_score': 43, 'shared_stability_mean_delta': 42} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.455 | 0.064 |
| small_gicp | 0.404 | 0.044 |
| oracle_pair_error | 0.331 | 0.036 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.066 | 0.066 | 0.124 |
| small_gicp | 0.042 | 0.034 | 0.094 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
