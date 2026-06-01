# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_02_start0090_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 12, 'small_gicp': 47}
- Signal hits: {'geometry_score': 12, 'overlap': 12, 'shared_residual_score': 29, 'shared_residual_rmse': 14, 'shared_stability_score': 28, 'shared_stability_mean_delta': 29} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.645 | 0.113 |
| small_gicp | 0.366 | 0.078 |
| oracle_pair_error | 0.205 | 0.054 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.105 | 0.114 | 0.161 |
| small_gicp | 0.058 | 0.034 | 0.196 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
