# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_05_start0090_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 23, 'kiss': 6}
- Signal hits: {'geometry_score': 6, 'overlap': 6, 'shared_residual_score': 9, 'shared_residual_rmse': 10, 'shared_stability_score': 16, 'shared_stability_mean_delta': 15} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.402 | 0.067 |
| small_gicp | 0.231 | 0.042 |
| oracle_pair_error | 0.205 | 0.038 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.061 | 0.067 | 0.102 |
| small_gicp | 0.038 | 0.030 | 0.082 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
