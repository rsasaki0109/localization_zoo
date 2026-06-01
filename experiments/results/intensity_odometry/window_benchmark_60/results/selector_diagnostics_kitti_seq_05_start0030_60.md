# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_05_start0030_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 41, 'kiss': 18}
- Signal hits: {'geometry_score': 18, 'overlap': 18, 'shared_residual_score': 30, 'shared_residual_rmse': 21, 'shared_stability_score': 41, 'shared_stability_mean_delta': 41} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.602 | 0.087 |
| small_gicp | 0.321 | 0.044 |
| oracle_pair_error | 0.217 | 0.036 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.072 | 0.057 | 0.150 |
| small_gicp | 0.039 | 0.034 | 0.091 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
