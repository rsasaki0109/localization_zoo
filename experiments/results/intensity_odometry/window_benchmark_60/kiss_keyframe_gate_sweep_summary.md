# KISS Keyframe Correction Gate Sweep

Benchmark: `experiments/results/intensity_odometry/window_benchmark_60`

## Aggregate

| Method / gate | Mean ATE (m) | Median ATE (m) | Max ATE (m) | Mean step RMSE (m) | Improved vs stability |
| --- | ---: | ---: | ---: | ---: | ---: |
| KISS pair | 1.071 | 1.097 | 2.246 | 0.073 | - |
| Stability motion 0.0025 | 0.800 | 0.776 | 1.582 | 0.060 | - |
| Keyframe c<=0.3 | 0.888 | 0.784 | 1.780 | 0.079 | 5/12 |
| Keyframe c<=0.5 | 0.784 | 0.749 | 1.295 | 0.089 | 8/12 |
| Keyframe c<=0.7 | 0.720 | 0.520 | 1.537 | 0.109 | 7/12 |
| Keyframe c<=1.0 | 0.731 | 0.403 | 1.537 | 0.121 | 6/12 |

## Selection

- Best mean ATE gate: `0.7 m`.
- Best tail ATE gate: `0.5 m`.
- Product-risk recommendation: `0.5 m`, because it minimizes max ATE first.
