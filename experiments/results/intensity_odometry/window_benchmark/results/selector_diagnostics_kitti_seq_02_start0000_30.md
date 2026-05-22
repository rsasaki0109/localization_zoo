# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_02_start0000_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'kiss': 19, 'small_gicp': 10}
- Signal hits: {'geometry_score': 19, 'overlap': 19, 'shared_residual_score': 15, 'shared_residual_rmse': 16, 'shared_stability_score': 20, 'shared_stability_mean_delta': 20} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.425 | 0.040 |
| small_gicp | 0.566 | 0.062 |
| oracle_pair_error | 0.325 | 0.031 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.040 | 0.035 | 0.077 |
| small_gicp | 0.056 | 0.045 | 0.109 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
