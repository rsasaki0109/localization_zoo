# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_08_start0090_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 21, 'kiss': 8}
- Signal hits: {'geometry_score': 8, 'overlap': 8, 'shared_residual_score': 14, 'shared_residual_rmse': 10, 'shared_stability_score': 18, 'shared_stability_mean_delta': 18} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 1.111 | 0.069 |
| small_gicp | 0.676 | 0.052 |
| oracle_pair_error | 0.652 | 0.048 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.071 | 0.074 | 0.117 |
| small_gicp | 0.055 | 0.052 | 0.102 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
