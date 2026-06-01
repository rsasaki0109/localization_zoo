# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_05_start0060_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 11, 'small_gicp': 18}
- Signal hits: {'geometry_score': 11, 'overlap': 11, 'shared_residual_score': 16, 'shared_residual_rmse': 12, 'shared_stability_score': 21, 'shared_stability_mean_delta': 21} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.776 | 0.091 |
| small_gicp | 0.366 | 0.050 |
| oracle_pair_error | 0.102 | 0.036 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.075 | 0.049 | 0.153 |
| small_gicp | 0.042 | 0.026 | 0.102 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
