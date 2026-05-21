# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_02_start0030_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 46, 'small_gicp': 13}
- Signal hits: {'geometry_score': 46, 'overlap': 46, 'shared_residual_score': 40, 'shared_residual_rmse': 42, 'shared_stability_score': 41, 'shared_stability_mean_delta': 41} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.163 | 0.068 |
| small_gicp | 0.855 | 0.109 |
| oracle_pair_error | 1.097 | 0.058 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.071 | 0.058 | 0.177 |
| small_gicp | 0.123 | 0.096 | 0.341 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
