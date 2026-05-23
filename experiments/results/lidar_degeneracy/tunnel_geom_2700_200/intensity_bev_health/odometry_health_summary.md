# LiDAR Degradation Odometry Health

| Window | Frames | Obscurant | Accepted | Converged | Score mean | Overlap mean | Used path m | Keyframes | Flags |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `baseline` 5-34 | 30 | 0.002 | 1.000 | 1.000 | 0.535 | 5522.3 | 7.975 | 0/0 | ok |
| `degraded` 170-199 | 30 | 0.227 | 1.000 | 1.000 | 0.654 | 5632.7 | 0.000 | 0/0 | low_used_path, low_motion_margin_dominant |
| `point_count_tail` 80-109 | 30 | 0.035 | 1.000 | 1.000 | 0.538 | 5675.9 | 0.000 | 0/0 | low_used_path, low_motion_margin_dominant |
| `geometry_degeneracy` 90-119 | 30 | 0.045 | 1.000 | 1.000 | 0.515 | 5641.0 | 0.750 | 0/0 | low_motion_margin_dominant |
