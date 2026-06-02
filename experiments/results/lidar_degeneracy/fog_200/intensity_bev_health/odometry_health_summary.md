# LiDAR Degradation Odometry Health

| Window | Frames | Obscurant | Accepted | Converged | Score mean | Overlap mean | Used path m | Keyframes | Flags |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `baseline` 0-29 | 30 | 0.006 | 1.000 | 1.000 | 0.606 | 5852.6 | 7.250 | 0/0 | ok |
| `degraded` 170-199 | 30 | 0.911 | 1.000 | 1.000 | 0.797 | 5695.7 | 4.177 | 0/0 | low_motion_margin_dominant |
| `point_count_tail` 115-144 | 30 | 0.521 | 1.000 | 1.000 | 0.740 | 5971.3 | 0.000 | 0/0 | low_used_path, low_motion_margin_dominant |
