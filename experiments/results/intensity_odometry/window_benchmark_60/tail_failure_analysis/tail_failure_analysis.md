# Pair Prior Tail Failure Analysis

- Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`
- Windows: `kitti_seq_08_start0090_60`, `kitti_seq_08_start0060_60`

## kitti_seq_08_start0090_60

- Best ATE policy: `small_gicp`
- Best max-frame policy: `small_gicp`

| Policy | ATE [m] | Step RMSE [m] | Max frame err [m] | Max frame | Pair hits | Positive regret sum [m] | Selected counts |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| KISS | 2.246 | 0.059 | 3.931 | 59 | 17/59 | 1.156 | `{'kiss': 59}` |
| SmallGICP | 1.560 | 0.041 | 2.725 | 59 | 42/59 | 0.290 | `{'small_gicp': 59}` |
| Stability delta | 1.789 | 0.048 | 3.161 | 59 | 38/59 | 0.497 | `{'kiss': 24, 'small_gicp': 35}` |
| Stability margin 0.001 | 1.644 | 0.044 | 2.820 | 59 | 39/59 | 0.430 | `{'small_gicp': 54, 'kiss': 5}` |
| Stability margin 0.0025 | 1.582 | 0.042 | 2.783 | 59 | 41/59 | 0.303 | `{'small_gicp': 58, 'kiss': 1}` |
| Stability motion 0.0025 | 1.582 | 0.042 | 2.783 | 59 | 41/59 | 0.303 | `{'small_gicp': 58, 'kiss': 1}` |
| Calibrated full | 1.609 | 0.042 | 2.802 | 59 | 41/59 | 0.315 | `{'small_gicp': 50, 'kiss': 9}` |

### Top Regret Pairs

| Policy | Pair | Selected | Oracle | Regret [m] | Selected err [m] | Oracle err [m] | GT step [m] |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |
| KISS | 27 | kiss | small_gicp | 0.069 | 0.083 | 0.014 | 1.124 |
| KISS | 53 | kiss | small_gicp | 0.067 | 0.084 | 0.017 | 1.196 |
| KISS | 10 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| KISS | 42 | kiss | small_gicp | 0.061 | 0.092 | 0.030 | 1.145 |
| KISS | 21 | kiss | small_gicp | 0.058 | 0.081 | 0.023 | 1.106 |
| SmallGICP | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| SmallGICP | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| SmallGICP | 49 | small_gicp | kiss | 0.027 | 0.039 | 0.012 | 1.216 |
| SmallGICP | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| SmallGICP | 41 | small_gicp | kiss | 0.020 | 0.058 | 0.038 | 1.140 |
| Stability delta | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability delta | 10 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| Stability delta | 42 | kiss | small_gicp | 0.061 | 0.092 | 0.030 | 1.145 |
| Stability delta | 12 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability delta | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability margin 0.001 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability margin 0.001 | 42 | kiss | small_gicp | 0.061 | 0.092 | 0.030 | 1.145 |
| Stability margin 0.001 | 12 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability margin 0.001 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability margin 0.001 | 49 | small_gicp | kiss | 0.027 | 0.039 | 0.012 | 1.216 |
| Stability margin 0.0025 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability margin 0.0025 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability margin 0.0025 | 49 | small_gicp | kiss | 0.027 | 0.039 | 0.012 | 1.216 |
| Stability margin 0.0025 | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| Stability margin 0.0025 | 41 | small_gicp | kiss | 0.020 | 0.058 | 0.038 | 1.140 |
| Stability motion 0.0025 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability motion 0.0025 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability motion 0.0025 | 49 | small_gicp | kiss | 0.027 | 0.039 | 0.012 | 1.216 |
| Stability motion 0.0025 | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| Stability motion 0.0025 | 41 | small_gicp | kiss | 0.020 | 0.058 | 0.038 | 1.140 |
| Calibrated full | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Calibrated full | 42 | kiss | small_gicp | 0.061 | 0.092 | 0.030 | 1.145 |
| Calibrated full | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Calibrated full | 49 | small_gicp | kiss | 0.027 | 0.039 | 0.012 | 1.216 |
| Calibrated full | 48 | kiss | small_gicp | 0.018 | 0.038 | 0.020 | 1.202 |

## kitti_seq_08_start0060_60

- Best ATE policy: `stability_margin_0p0025`
- Best max-frame policy: `stability_margin_0p0025`

| Policy | ATE [m] | Step RMSE [m] | Max frame err [m] | Max frame | Pair hits | Positive regret sum [m] | Selected counts |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| KISS | 1.542 | 0.059 | 3.094 | 59 | 25/59 | 1.050 | `{'kiss': 59}` |
| SmallGICP | 1.366 | 0.050 | 2.623 | 59 | 34/59 | 0.599 | `{'small_gicp': 59}` |
| Stability delta | 1.426 | 0.053 | 2.736 | 59 | 36/59 | 0.627 | `{'small_gicp': 35, 'kiss': 24}` |
| Stability margin 0.001 | 1.399 | 0.050 | 2.620 | 59 | 36/59 | 0.610 | `{'small_gicp': 51, 'kiss': 8}` |
| Stability margin 0.0025 | 1.309 | 0.051 | 2.468 | 59 | 35/59 | 0.634 | `{'small_gicp': 56, 'kiss': 3}` |
| Stability motion 0.0025 | 1.309 | 0.051 | 2.468 | 59 | 35/59 | 0.634 | `{'small_gicp': 56, 'kiss': 3}` |
| Calibrated full | 1.363 | 0.050 | 2.552 | 59 | 40/59 | 0.540 | `{'small_gicp': 51, 'kiss': 8}` |

### Top Regret Pairs

| Policy | Pair | Selected | Oracle | Regret [m] | Selected err [m] | Oracle err [m] | GT step [m] |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |
| KISS | 16 | kiss | small_gicp | 0.081 | 0.135 | 0.053 | 0.644 |
| KISS | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| KISS | 57 | kiss | small_gicp | 0.069 | 0.083 | 0.014 | 1.124 |
| KISS | 40 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| KISS | 51 | kiss | small_gicp | 0.057 | 0.082 | 0.024 | 1.106 |
| SmallGICP | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| SmallGICP | 22 | small_gicp | kiss | 0.045 | 0.080 | 0.034 | 0.799 |
| SmallGICP | 18 | small_gicp | kiss | 0.045 | 0.081 | 0.036 | 0.684 |
| SmallGICP | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
| SmallGICP | 17 | small_gicp | kiss | 0.036 | 0.042 | 0.006 | 0.645 |
| Stability delta | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| Stability delta | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| Stability delta | 40 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| Stability delta | 42 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability delta | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
| Stability margin 0.001 | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| Stability margin 0.001 | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| Stability margin 0.001 | 42 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability margin 0.001 | 18 | small_gicp | kiss | 0.045 | 0.081 | 0.036 | 0.684 |
| Stability margin 0.001 | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
| Stability margin 0.0025 | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| Stability margin 0.0025 | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| Stability margin 0.0025 | 22 | small_gicp | kiss | 0.045 | 0.080 | 0.034 | 0.799 |
| Stability margin 0.0025 | 18 | small_gicp | kiss | 0.045 | 0.081 | 0.036 | 0.684 |
| Stability margin 0.0025 | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
| Stability motion 0.0025 | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| Stability motion 0.0025 | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| Stability motion 0.0025 | 22 | small_gicp | kiss | 0.045 | 0.080 | 0.034 | 0.799 |
| Stability motion 0.0025 | 18 | small_gicp | kiss | 0.045 | 0.081 | 0.036 | 0.684 |
| Stability motion 0.0025 | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
| Calibrated full | 8 | kiss | small_gicp | 0.077 | 0.107 | 0.031 | 0.444 |
| Calibrated full | 49 | small_gicp | kiss | 0.071 | 0.099 | 0.029 | 1.133 |
| Calibrated full | 22 | small_gicp | kiss | 0.045 | 0.080 | 0.034 | 0.799 |
| Calibrated full | 18 | small_gicp | kiss | 0.045 | 0.081 | 0.036 | 0.684 |
| Calibrated full | 53 | small_gicp | kiss | 0.041 | 0.071 | 0.030 | 1.108 |
