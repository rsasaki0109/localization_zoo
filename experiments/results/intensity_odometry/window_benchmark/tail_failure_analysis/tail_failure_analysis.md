# Pair Prior Tail Failure Analysis

- Benchmark: `experiments/results/intensity_odometry/window_benchmark`
- Windows: `kitti_seq_08_start0030_30`, `kitti_seq_08_start0090_30`

## kitti_seq_08_start0030_30

- Best ATE policy: `stability_motion_0p0025`
- Best max-frame policy: `stability_motion_0p0025`

| Policy | ATE [m] | Step RMSE [m] | Max frame err [m] | Max frame | Pair hits | Positive regret sum [m] | Selected counts |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| KISS | 0.439 | 0.073 | 0.927 | 29 | 20/29 | 0.353 | `{'kiss': 29}` |
| SmallGICP | 1.270 | 0.077 | 2.128 | 29 | 9/29 | 0.957 | `{'small_gicp': 29}` |
| Stability delta | 0.451 | 0.058 | 0.924 | 29 | 22/29 | 0.231 | `{'kiss': 17, 'small_gicp': 12}` |
| Stability margin 0.001 | 0.659 | 0.065 | 1.277 | 29 | 16/29 | 0.445 | `{'kiss': 11, 'small_gicp': 18}` |
| Stability margin 0.0025 | 0.907 | 0.068 | 1.640 | 29 | 15/29 | 0.528 | `{'small_gicp': 23, 'kiss': 6}` |
| Stability motion 0.0025 | 0.365 | 0.054 | 0.773 | 29 | 22/29 | 0.122 | `{'small_gicp': 12, 'kiss': 17}` |
| Calibrated full | 0.786 | 0.067 | 1.466 | 29 | 16/29 | 0.477 | `{'small_gicp': 20, 'kiss': 9}` |

### Top Regret Pairs

| Policy | Pair | Selected | Oracle | Regret [m] | Selected err [m] | Oracle err [m] | GT step [m] |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |
| KISS | 23 | kiss | small_gicp | 0.089 | 0.175 | 0.086 | 0.218 |
| KISS | 22 | kiss | small_gicp | 0.071 | 0.193 | 0.121 | 0.197 |
| KISS | 21 | kiss | small_gicp | 0.051 | 0.151 | 0.100 | 0.159 |
| KISS | 20 | kiss | small_gicp | 0.045 | 0.153 | 0.108 | 0.157 |
| KISS | 26 | kiss | small_gicp | 0.026 | 0.089 | 0.063 | 0.271 |
| SmallGICP | 28 | small_gicp | kiss | 0.155 | 0.178 | 0.022 | 0.306 |
| SmallGICP | 2 | small_gicp | kiss | 0.093 | 0.114 | 0.022 | 0.022 |
| SmallGICP | 3 | small_gicp | kiss | 0.090 | 0.109 | 0.019 | 0.038 |
| SmallGICP | 17 | small_gicp | kiss | 0.085 | 0.093 | 0.009 | 0.074 |
| SmallGICP | 8 | small_gicp | kiss | 0.075 | 0.092 | 0.016 | 0.028 |
| Stability delta | 17 | small_gicp | kiss | 0.085 | 0.093 | 0.009 | 0.074 |
| Stability delta | 10 | small_gicp | kiss | 0.046 | 0.069 | 0.023 | 0.053 |
| Stability delta | 24 | small_gicp | kiss | 0.045 | 0.061 | 0.016 | 0.216 |
| Stability delta | 16 | kiss | small_gicp | 0.025 | 0.068 | 0.043 | 0.093 |
| Stability delta | 15 | small_gicp | kiss | 0.019 | 0.077 | 0.058 | 0.063 |
| Stability margin 0.001 | 17 | small_gicp | kiss | 0.085 | 0.093 | 0.009 | 0.074 |
| Stability margin 0.001 | 8 | small_gicp | kiss | 0.075 | 0.092 | 0.016 | 0.028 |
| Stability margin 0.001 | 13 | small_gicp | kiss | 0.052 | 0.081 | 0.029 | 0.032 |
| Stability margin 0.001 | 9 | small_gicp | kiss | 0.048 | 0.066 | 0.018 | 0.047 |
| Stability margin 0.001 | 10 | small_gicp | kiss | 0.046 | 0.069 | 0.023 | 0.053 |
| Stability margin 0.0025 | 17 | small_gicp | kiss | 0.085 | 0.093 | 0.009 | 0.074 |
| Stability margin 0.0025 | 8 | small_gicp | kiss | 0.075 | 0.092 | 0.016 | 0.028 |
| Stability margin 0.0025 | 4 | small_gicp | kiss | 0.071 | 0.087 | 0.015 | 0.021 |
| Stability margin 0.0025 | 13 | small_gicp | kiss | 0.052 | 0.081 | 0.029 | 0.032 |
| Stability margin 0.0025 | 9 | small_gicp | kiss | 0.048 | 0.066 | 0.018 | 0.047 |
| Stability motion 0.0025 | 24 | small_gicp | kiss | 0.045 | 0.061 | 0.016 | 0.216 |
| Stability motion 0.0025 | 16 | kiss | small_gicp | 0.025 | 0.068 | 0.043 | 0.093 |
| Stability motion 0.0025 | 1 | small_gicp | kiss | 0.016 | 0.048 | 0.032 | 0.060 |
| Stability motion 0.0025 | 27 | small_gicp | kiss | 0.013 | 0.076 | 0.062 | 0.271 |
| Stability motion 0.0025 | 18 | small_gicp | kiss | 0.011 | 0.090 | 0.079 | 0.108 |
| Calibrated full | 17 | small_gicp | kiss | 0.085 | 0.093 | 0.009 | 0.074 |
| Calibrated full | 8 | small_gicp | kiss | 0.075 | 0.092 | 0.016 | 0.028 |
| Calibrated full | 13 | small_gicp | kiss | 0.052 | 0.081 | 0.029 | 0.032 |
| Calibrated full | 9 | small_gicp | kiss | 0.048 | 0.066 | 0.018 | 0.047 |
| Calibrated full | 10 | small_gicp | kiss | 0.046 | 0.069 | 0.023 | 0.053 |

## kitti_seq_08_start0090_30

- Best ATE policy: `stability_margin_0p0025`
- Best max-frame policy: `stability_margin_0p0025`

| Policy | ATE [m] | Step RMSE [m] | Max frame err [m] | Max frame | Pair hits | Positive regret sum [m] | Selected counts |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| KISS | 1.111 | 0.069 | 1.635 | 26 | 8/29 | 0.631 | `{'kiss': 29}` |
| SmallGICP | 0.679 | 0.052 | 1.099 | 29 | 21/29 | 0.165 | `{'small_gicp': 29}` |
| Stability delta | 0.842 | 0.060 | 1.302 | 29 | 18/29 | 0.312 | `{'kiss': 11, 'small_gicp': 18}` |
| Stability margin 0.001 | 0.770 | 0.053 | 1.204 | 29 | 19/29 | 0.234 | `{'small_gicp': 27, 'kiss': 2}` |
| Stability margin 0.0025 | 0.676 | 0.052 | 1.092 | 29 | 21/29 | 0.165 | `{'small_gicp': 29}` |
| Stability motion 0.0025 | 0.676 | 0.052 | 1.092 | 29 | 21/29 | 0.165 | `{'small_gicp': 29}` |
| Calibrated full | 0.755 | 0.052 | 1.166 | 29 | 21/29 | 0.126 | `{'kiss': 4, 'small_gicp': 25}` |

### Top Regret Pairs

| Policy | Pair | Selected | Oracle | Regret [m] | Selected err [m] | Oracle err [m] | GT step [m] |
| --- | ---: | --- | --- | ---: | ---: | ---: | ---: |
| KISS | 27 | kiss | small_gicp | 0.069 | 0.083 | 0.014 | 1.124 |
| KISS | 10 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| KISS | 21 | kiss | small_gicp | 0.058 | 0.081 | 0.023 | 1.106 |
| KISS | 18 | kiss | small_gicp | 0.049 | 0.060 | 0.011 | 1.117 |
| KISS | 29 | kiss | small_gicp | 0.048 | 0.074 | 0.025 | 1.119 |
| SmallGICP | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| SmallGICP | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| SmallGICP | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| SmallGICP | 25 | small_gicp | kiss | 0.016 | 0.042 | 0.027 | 1.115 |
| SmallGICP | 3 | small_gicp | kiss | 0.005 | 0.079 | 0.074 | 0.968 |
| Stability delta | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability delta | 10 | kiss | small_gicp | 0.064 | 0.116 | 0.052 | 1.119 |
| Stability delta | 12 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability delta | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability delta | 13 | kiss | small_gicp | 0.040 | 0.119 | 0.078 | 1.115 |
| Stability margin 0.001 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability margin 0.001 | 12 | kiss | small_gicp | 0.048 | 0.092 | 0.044 | 1.088 |
| Stability margin 0.001 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability margin 0.001 | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| Stability margin 0.001 | 8 | kiss | small_gicp | 0.021 | 0.076 | 0.054 | 1.037 |
| Stability margin 0.0025 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability margin 0.0025 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability margin 0.0025 | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| Stability margin 0.0025 | 25 | small_gicp | kiss | 0.016 | 0.042 | 0.027 | 1.115 |
| Stability margin 0.0025 | 3 | small_gicp | kiss | 0.005 | 0.079 | 0.074 | 0.968 |
| Stability motion 0.0025 | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Stability motion 0.0025 | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Stability motion 0.0025 | 2 | small_gicp | kiss | 0.027 | 0.052 | 0.026 | 0.947 |
| Stability motion 0.0025 | 25 | small_gicp | kiss | 0.016 | 0.042 | 0.027 | 1.115 |
| Stability motion 0.0025 | 3 | small_gicp | kiss | 0.005 | 0.079 | 0.074 | 0.968 |
| Calibrated full | 19 | small_gicp | kiss | 0.071 | 0.098 | 0.028 | 1.133 |
| Calibrated full | 23 | small_gicp | kiss | 0.041 | 0.071 | 0.029 | 1.108 |
| Calibrated full | 3 | small_gicp | kiss | 0.005 | 0.079 | 0.074 | 0.968 |
| Calibrated full | 1 | kiss | small_gicp | 0.003 | 0.038 | 0.035 | 0.924 |
| Calibrated full | 17 | small_gicp | kiss | 0.002 | 0.105 | 0.103 | 1.137 |
