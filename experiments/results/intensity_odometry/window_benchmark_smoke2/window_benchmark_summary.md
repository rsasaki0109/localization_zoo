# Pair Prior Window Benchmark

- Feature set: `shared_no_method`
- Calibration: `experiments/results/intensity_odometry/window_benchmark_smoke2/results/pair_prior_quality_calibration_shared_no_method_loo.json`

| Window | KISS | SmallGICP | Margin | Stability delta | Calibrated full | Calibrated holdout |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| kitti_seq_02_start0000_30 | 0.426 / 0.040 | 0.565 / 0.062 | 0.353 / 0.054 | 0.325 / 0.041 | 0.359 / 0.044 | 0.323 / 0.042 |
| kitti_seq_02_start0030_30 | 0.514 / 0.041 | 0.287 / 0.063 | 0.329 / 0.061 | 0.516 / 0.043 | 0.434 / 0.047 | 0.297 / 0.054 |
| kitti_seq_05_start0000_30 | 0.765 / 0.055 | 0.190 / 0.017 | 0.378 / 0.033 | 0.185 / 0.021 | 0.391 / 0.035 | 0.392 / 0.035 |
| kitti_seq_05_start0030_30 | 0.661 / 0.083 | 0.246 / 0.038 | 0.279 / 0.052 | 0.498 / 0.059 | 0.457 / 0.062 | 0.571 / 0.066 |
| kitti_seq_08_start0000_30 | 0.878 / 0.082 | 0.531 / 0.067 | 0.560 / 0.065 | 0.624 / 0.063 | 0.537 / 0.059 | 0.728 / 0.071 |
| kitti_seq_08_start0030_30 | 0.439 / 0.073 | 1.270 / 0.077 | 1.092 / 0.074 | 0.451 / 0.058 | 0.450 / 0.057 | 0.742 / 0.066 |
