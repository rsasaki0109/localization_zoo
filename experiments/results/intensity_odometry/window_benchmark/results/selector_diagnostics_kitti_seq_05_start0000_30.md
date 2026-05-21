# Pair Prior Selector Diagnostics

- GT: `experiments/results/intensity_odometry/window_benchmark/windows/kitti_seq_05_start0000_30/gt.csv`
- Selector: `None`
- Selector pair accuracy: n/a
- Selected counts: {}
- Oracle counts: {'small_gicp': 25, 'kiss': 4}
- Signal hits: {'geometry_score': 4, 'overlap': 4, 'shared_residual_score': 15, 'shared_residual_rmse': 13, 'shared_stability_score': 19, 'shared_stability_mean_delta': 21} / 29

## Trajectory Metrics

| Method | ATE xy [m] | Step RMSE [m] |
| --- | ---: | ---: |
| kiss | 0.765 | 0.055 |
| small_gicp | 0.191 | 0.017 |
| oracle_pair_error | 0.188 | 0.015 |

## Pair Translation Error

| Method | Mean [m] | Median [m] | P95 [m] |
| --- | ---: | ---: | ---: |
| kiss | 0.051 | 0.047 | 0.106 |
| small_gicp | 0.019 | 0.019 | 0.035 |

## Selector Misses

| Pair | Selected | Oracle | Selected err [m] | Oracle err [m] |
| ---: | --- | --- | ---: | ---: |
