# Keyframe-Corrected Stability Prior Benchmark

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`
Anchor dir: `kiss_keyframe_i10_c0p5_relog`

## Aggregate

| Method | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved windows |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - | - |
| Stability + keyframe corrections | 0.639 | 0.558 | 1.119 | 0.070 | 6/12 |

## Windows

| Window | Stability ATE | Corrected ATE | Delta | Step RMSE | Corrections |
| --- | ---: | ---: | ---: | ---: | --- |
| kitti_seq_02_start0000_60 | 0.756 | 0.757 | +0.001 | 0.083 | 3 accepted |
| kitti_seq_02_start0030_60 | 0.796 | 0.467 | -0.329 | 0.108 | 4 accepted |
| kitti_seq_02_start0060_60 | 1.173 | 1.119 | -0.054 | 0.116 | 1 accepted |
| kitti_seq_02_start0090_60 | 0.442 | 0.442 | +0.000 | 0.085 | 0 accepted |
| kitti_seq_05_start0000_60 | 0.170 | 0.170 | +0.000 | 0.029 | 0 accepted |
| kitti_seq_05_start0030_60 | 0.302 | 0.302 | +0.000 | 0.044 | 0 accepted |
| kitti_seq_05_start0060_60 | 0.488 | 0.488 | +0.000 | 0.047 | 0 accepted |
| kitti_seq_05_start0090_60 | 0.285 | 0.285 | +0.000 | 0.044 | 0 accepted |
| kitti_seq_08_start0000_60 | 1.161 | 0.966 | -0.195 | 0.067 | 1 accepted |
| kitti_seq_08_start0030_60 | 1.137 | 0.988 | -0.148 | 0.058 | 3 accepted |
| kitti_seq_08_start0060_60 | 1.309 | 1.059 | -0.251 | 0.083 | 5 accepted |
| kitti_seq_08_start0090_60 | 1.582 | 0.628 | -0.954 | 0.073 | 4 accepted |
