# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark_smoke/windows/kitti_seq_02_start0090_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 12, 'small_gicp': 17}
- Signal hits: {'geometry_score': 12, 'overlap': 12, 'shared_residual_score': 14, 'shared_residual_rmse': 13, 'shared_stability_score': 11, 'shared_stability_mean_delta': 11} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.448 | 0.100 |
| small_gicp | 0.386 | 0.104 |
| oracle_pair_error | 0.163 | 0.068 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.088 | 0.100 | 0.156 |
| small_gicp | 0.079 | 0.041 | 0.211 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
