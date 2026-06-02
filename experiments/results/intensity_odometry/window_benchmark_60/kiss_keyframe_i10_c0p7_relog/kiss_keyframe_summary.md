# KISS Keyframe Window Benchmark

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`
Keyframe interval: `10` frames

## Aggregate

| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) |
| --- | ---: | ---: | ---: | ---: |
| KISS pair | 1.071 | 1.097 | 2.246 | - |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - |
| KISS keyframe i10 | 0.720 | 0.520 | 1.537 | 0.109 |

## Windows

| Window | KISS pair ATE | Stability motion ATE | Keyframe ATE | Delta vs stability | Step RMSE | Keyframes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| kitti_seq_02_start0000_60 | 1.026 | 0.756 | 0.311 | -0.445 | 0.083 | 5/5 |
| kitti_seq_02_start0030_60 | 1.169 | 0.796 | 0.442 | -0.355 | 0.094 | 5/5 |
| kitti_seq_02_start0060_60 | 1.212 | 1.173 | 0.925 | -0.248 | 0.112 | 4/5 |
| kitti_seq_02_start0090_60 | 0.643 | 0.442 | 0.521 | +0.079 | 0.121 | 3/5 |
| kitti_seq_05_start0000_60 | 0.619 | 0.170 | 0.519 | +0.348 | 0.080 | 3/5 |
| kitti_seq_05_start0030_60 | 0.601 | 0.302 | 0.313 | +0.011 | 0.136 | 5/5 |
| kitti_seq_05_start0060_60 | 0.798 | 0.488 | 0.342 | -0.146 | 0.128 | 5/5 |
| kitti_seq_05_start0090_60 | 0.453 | 0.285 | 0.294 | +0.009 | 0.125 | 5/5 |
| kitti_seq_08_start0000_60 | 1.310 | 1.161 | 0.847 | -0.314 | 0.130 | 3/5 |
| kitti_seq_08_start0030_60 | 1.237 | 1.137 | 1.537 | +0.400 | 0.122 | 4/5 |
| kitti_seq_08_start0060_60 | 1.542 | 1.309 | 1.289 | -0.020 | 0.078 | 5/5 |
| kitti_seq_08_start0090_60 | 2.246 | 1.582 | 1.304 | -0.278 | 0.096 | 5/5 |
