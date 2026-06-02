# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_08_start0060_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 13, 'kiss': 16}
- Signal hits: {'geometry_score': 16, 'overlap': 16, 'shared_residual_score': 11, 'shared_residual_rmse': 18, 'shared_stability_score': 16, 'shared_stability_mean_delta': 17} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.609 | 0.046 |
| small_gicp | 0.651 | 0.049 |
| oracle_pair_error | 0.563 | 0.040 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.056 | 0.046 | 0.118 |
| small_gicp | 0.055 | 0.057 | 0.096 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
