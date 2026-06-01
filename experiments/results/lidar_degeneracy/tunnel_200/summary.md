# LiDAR Degeneracy Diagnostics

- PCD sequence: `dogfooding_results/lidar_degeneracy_tunnel_200`
- Frames: `200`
- High-degeneracy frames (`score >= 0.90`): `0` (0.0%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 44182.1300 | 39779.5000 | 52147.3500 | 52432.0000 |
| `linearity` | 0.6144 | 0.6341 | 0.6451 | 0.6676 |
| `planarity` | 0.2907 | 0.2823 | 0.3344 | 0.3450 |
| `scattering` | 0.0949 | 0.0834 | 0.1236 | 0.1283 |
| `degeneracy_score` | 0.6144 | 0.6341 | 0.6451 | 0.6676 |
| `range_mean_m` | 6.3460 | 6.6346 | 6.6652 | 6.6797 |
| `range_p95_m` | 14.2654 | 14.7186 | 14.7948 | 14.8395 |
| `intensity_mean` | 1872.1332 | 1837.1203 | 2014.6765 | 2264.0089 |
| `intensity_std` | 1473.2825 | 1497.0512 | 1573.5021 | 1669.0247 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
