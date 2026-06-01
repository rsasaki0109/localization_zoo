# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_02_start0060_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 7, 'kiss': 22}
- Signal hits: {'geometry_score': 22, 'overlap': 22, 'shared_residual_score': 19, 'shared_residual_rmse': 20, 'shared_stability_score': 18, 'shared_stability_mean_delta': 18} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.529 | 0.089 |
| small_gicp | 0.670 | 0.139 |
| oracle_pair_error | 0.472 | 0.076 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.096 | 0.084 | 0.192 |
| small_gicp | 0.162 | 0.120 | 0.346 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
