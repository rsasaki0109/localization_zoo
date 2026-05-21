# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_02_start0000_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 42, 'small_gicp': 17}
- Signal hits: {'geometry_score': 42, 'overlap': 42, 'shared_residual_score': 37, 'shared_residual_rmse': 37, 'shared_stability_score': 42, 'shared_stability_mean_delta': 42} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.023 | 0.040 |
| small_gicp | 1.094 | 0.062 |
| oracle_pair_error | 0.819 | 0.032 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.043 | 0.036 | 0.082 |
| small_gicp | 0.068 | 0.057 | 0.151 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
