# Keyframe Replay Gate Sweep

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`

| Run | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved | Regressions |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | - | - | - |
| c<=0.5, corr>=80 | 0.626 | 0.517 | 1.059 | 0.084 | 9/12 | 3/12 |
| c<=0.5, corr>=5000 | 0.620 | 0.528 | 1.059 | 0.077 | 8/12 | 2/12 |
| c<=0.5, corr>=6000 | 0.605 | 0.528 | 1.059 | 0.071 | 7/12 | 0/12 |
| c<=0.5, corr>=7000 | 0.639 | 0.558 | 1.119 | 0.070 | 6/12 | 1/12 |
| c<=0.7, corr>=80 | 0.668 | 0.615 | 1.437 | 0.107 | 8/12 | 4/12 |

## Selection

- Best mean ATE: `c<=0.5, corr>=6000`.
- Best max ATE: `c<=0.5, corr>=6000`.
- Product-risk recommendation: `c<=0.5, corr>=6000`.
