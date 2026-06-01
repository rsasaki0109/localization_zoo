# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_60/windows/kitti_seq_08_start0090_60/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 42, 'kiss': 17}
- Signal hits: {'geometry_score': 17, 'overlap': 17, 'shared_residual_score': 32, 'shared_residual_rmse': 26, 'shared_stability_score': 37, 'shared_stability_mean_delta': 38} / 59

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 2.247 | 0.059 |
| small_gicp | 1.554 | 0.041 |
| oracle_pair_error | 1.574 | 0.037 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.066 | 0.068 | 0.109 |
| small_gicp | 0.051 | 0.049 | 0.098 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
