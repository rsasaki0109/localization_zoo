# LiDAR Degeneracy Diagnostics

- PCD sequence: `dogfooding_results/lidar_degeneracy_tunnel_stride5`
- Frames: `649`
- High-degeneracy frames (`score >= 0.90`): `0` (0.0%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 49929.4946 | 51091.0000 | 51444.8000 | 52385.0000 |
| `linearity` | 0.7136 | 0.7288 | 0.7549 | 0.7732 |
| `planarity` | 0.1751 | 0.1534 | 0.2836 | 0.3450 |
| `scattering` | 0.1113 | 0.1135 | 0.1265 | 0.1413 |
| `degeneracy_score` | 0.7136 | 0.7288 | 0.7549 | 0.7732 |
| `range_mean_m` | 5.8475 | 5.5638 | 7.9341 | 8.4562 |
| `range_p95_m` | 13.0466 | 12.3903 | 18.2785 | 19.2357 |
| `intensity_mean` | 2039.6512 | 2107.7625 | 2188.9903 | 2244.5276 |
| `intensity_std` | 1403.8557 | 1411.6846 | 1531.2613 | 1738.5228 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
