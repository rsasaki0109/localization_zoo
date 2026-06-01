# KISS Keyframe Window Benchmark

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`
Keyframe interval: `10` frames

## Aggregate

| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) |
| --- | ---: | ---: | ---: | ---: |
| KISS pair | 1.071 | 1.097 | 2.246 | - |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - |
| KISS keyframe i10 | 0.888 | 0.784 | 1.780 | 0.079 |

## Windows

| Window | KISS pair ATE | Stability motion ATE | Keyframe ATE | Delta vs stability | Step RMSE | Keyframes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| kitti_seq_02_start0000_60 | 1.026 | 0.756 | 0.754 | -0.002 | 0.042 | 2/5 |
| kitti_seq_02_start0030_60 | 1.169 | 0.796 | 0.644 | -0.152 | 0.082 | 3/5 |
| kitti_seq_02_start0060_60 | 1.212 | 1.173 | 0.925 | -0.248 | 0.112 | 4/5 |
| kitti_seq_02_start0090_60 | 0.643 | 0.442 | 0.648 | +0.206 | 0.117 | 2/5 |
| kitti_seq_05_start0000_60 | 0.619 | 0.170 | 0.591 | +0.421 | 0.078 | 2/5 |
| kitti_seq_05_start0030_60 | 0.601 | 0.302 | 0.552 | +0.249 | 0.089 | 2/5 |
| kitti_seq_05_start0060_60 | 0.798 | 0.488 | 0.814 | +0.326 | 0.079 | 1/5 |
| kitti_seq_05_start0090_60 | 0.453 | 0.285 | 0.421 | +0.136 | 0.068 | 1/5 |
| kitti_seq_08_start0000_60 | 1.310 | 1.161 | 1.081 | -0.080 | 0.079 | 1/5 |
| kitti_seq_08_start0030_60 | 1.237 | 1.137 | 1.084 | -0.052 | 0.066 | 3/5 |
| kitti_seq_08_start0060_60 | 1.542 | 1.309 | 1.361 | +0.051 | 0.074 | 4/5 |
| kitti_seq_08_start0090_60 | 2.246 | 1.582 | 1.780 | +0.198 | 0.064 | 2/5 |
