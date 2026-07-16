# LiDAR Degeneracy Diagnostics

- PCD sequence: `dogfooding_results/lidar_degeneracy_tunnel_full`
- Frames: `3241`
- High-degeneracy frames (`score >= 0.90`): `0` (0.0%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 49943.1922 | 51088.0000 | 51422.0000 | 52432.0000 |
| `linearity` | 0.7137 | 0.7287 | 0.7545 | 0.7781 |
| `planarity` | 0.1750 | 0.1534 | 0.2845 | 0.3450 |
| `scattering` | 0.1113 | 0.1133 | 0.1264 | 0.1423 |
| `degeneracy_score` | 0.7137 | 0.7287 | 0.7545 | 0.7781 |
| `range_mean_m` | 5.8463 | 5.5645 | 7.9375 | 8.4978 |
| `range_p95_m` | 13.0435 | 12.3914 | 18.3220 | 19.2549 |
| `intensity_mean` | 2039.7344 | 2105.9996 | 2189.7874 | 2264.0089 |
| `intensity_std` | 1403.5308 | 1411.8322 | 1536.3672 | 1743.4613 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
