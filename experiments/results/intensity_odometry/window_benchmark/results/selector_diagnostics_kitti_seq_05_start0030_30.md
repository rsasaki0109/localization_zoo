# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_05_start0030_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 22, 'kiss': 7}
- Signal hits: {'geometry_score': 7, 'overlap': 7, 'shared_residual_score': 14, 'shared_residual_rmse': 9, 'shared_stability_score': 20, 'shared_stability_mean_delta': 20} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.661 | 0.083 |
| small_gicp | 0.246 | 0.038 |
| oracle_pair_error | 0.263 | 0.036 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.071 | 0.061 | 0.148 |
| small_gicp | 0.037 | 0.036 | 0.080 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
