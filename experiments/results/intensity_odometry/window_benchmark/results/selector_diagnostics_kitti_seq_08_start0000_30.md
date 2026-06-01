# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_08_start0000_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 19, 'kiss': 10}
- Signal hits: {'geometry_score': 10, 'overlap': 10, 'shared_residual_score': 18, 'shared_residual_rmse': 20, 'shared_stability_score': 21, 'shared_stability_mean_delta': 20} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.877 | 0.082 |
| small_gicp | 0.530 | 0.067 |
| oracle_pair_error | 0.396 | 0.048 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.079 | 0.071 | 0.156 |
| small_gicp | 0.064 | 0.061 | 0.125 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
