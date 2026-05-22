# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_02_start0030_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 23, 'small_gicp': 6}
- Signal hits: {'geometry_score': 23, 'overlap': 23, 'shared_residual_score': 21, 'shared_residual_rmse': 21, 'shared_stability_score': 22, 'shared_stability_mean_delta': 22} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.513 | 0.041 |
| small_gicp | 0.287 | 0.063 |
| oracle_pair_error | 0.441 | 0.034 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.047 | 0.040 | 0.104 |
| small_gicp | 0.083 | 0.079 | 0.187 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
