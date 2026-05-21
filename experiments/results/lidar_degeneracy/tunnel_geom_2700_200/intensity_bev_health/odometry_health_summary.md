# LiDAR Degradation Odometry Health

| Window | Frames | Obscurant | Accepted | Converged | Score mean | Overlap mean | Used path m | Keyframes | Flags |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `baseline` 5-34 | 30 | 0.002 | 1.000 | 1.000 | 0.535 | 5522.3 | 7.975 | 0/0 | ok |
| `degraded` 170-199 | 30 | 0.227 | 1.000 | 1.000 | 0.648 | 5534.3 | 7.561 | 0/0 | motion_margin_dominant |
| `point_count_tail` 80-109 | 30 | 0.035 | 1.000 | 1.000 | 0.540 | 5374.6 | 9.875 | 0/0 | ok |
| `geometry_degeneracy` 90-119 | 30 | 0.045 | 1.000 | 1.000 | 0.521 | 5114.1 | 12.642 | 0/0 | ok |
