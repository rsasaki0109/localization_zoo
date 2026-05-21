# Keyframe-Corrected Stability Prior Benchmark

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`
Anchor dir: `kiss_keyframe_i10_c0p7_relog`

## Aggregate

| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved windows |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - | - |
| Stability + keyframe corrections | 0.668 | 0.615 | 1.437 | 0.107 | 8/12 |

## Windows

| Window | Stability ATE | Corrected ATE | Delta | Step RMSE | Corrections |
| --- | ---: | ---: | ---: | ---: | --- |
| kitti_seq_02_start0000_60 | 0.756 | 0.631 | -0.125 | 0.093 | 5 accepted |
| kitti_seq_02_start0030_60 | 0.796 | 0.425 | -0.372 | 0.117 | 5 accepted |
| kitti_seq_02_start0060_60 | 1.173 | 0.958 | -0.215 | 0.130 | 4 accepted |
| kitti_seq_02_start0090_60 | 0.442 | 0.426 | -0.016 | 0.099 | 3 accepted |
| kitti_seq_05_start0000_60 | 0.170 | 0.294 | +0.124 | 0.056 | 3 accepted |
| kitti_seq_05_start0030_60 | 0.302 | 0.607 | +0.305 | 0.123 | 5 accepted |
| kitti_seq_05_start0060_60 | 0.488 | 0.311 | -0.177 | 0.118 | 5 accepted |
| kitti_seq_05_start0090_60 | 0.285 | 0.492 | +0.207 | 0.131 | 5 accepted |
| kitti_seq_08_start0000_60 | 1.161 | 0.754 | -0.407 | 0.120 | 3 accepted |
| kitti_seq_08_start0030_60 | 1.137 | 1.437 | +0.301 | 0.114 | 4 accepted |
| kitti_seq_08_start0060_60 | 1.309 | 1.059 | -0.251 | 0.083 | 5 accepted |
| kitti_seq_08_start0090_60 | 1.582 | 0.622 | -0.959 | 0.098 | 5 accepted |
