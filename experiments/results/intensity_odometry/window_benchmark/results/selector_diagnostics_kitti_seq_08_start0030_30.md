# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_08_start0030_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 20, 'small_gicp': 9}
- Signal hits: {'geometry_score': 20, 'overlap': 20, 'shared_residual_score': 26, 'shared_residual_rmse': 24, 'shared_stability_score': 22, 'shared_stability_mean_delta': 22} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.440 | 0.073 |
| small_gicp | 1.270 | 0.077 |
| oracle_pair_error | 0.366 | 0.052 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.059 | 0.044 | 0.166 |
| small_gicp | 0.080 | 0.077 | 0.119 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
