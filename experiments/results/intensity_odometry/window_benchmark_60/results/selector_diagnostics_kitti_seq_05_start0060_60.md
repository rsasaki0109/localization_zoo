# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_05_start0060_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 17, 'small_gicp': 42}
- Signal hits: {'geometry_score': 17, 'overlap': 17, 'shared_residual_score': 25, 'shared_residual_rmse': 22, 'shared_stability_score': 37, 'shared_stability_mean_delta': 36} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.796 | 0.081 |
| small_gicp | 0.552 | 0.046 |
| oracle_pair_error | 0.262 | 0.036 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.069 | 0.061 | 0.146 |
| small_gicp | 0.039 | 0.030 | 0.099 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
