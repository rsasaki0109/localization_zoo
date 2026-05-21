# LiDAR Degeneracy Diagnostics

- PCD sequence: `dogfooding_results/lidar_degeneracy_tunnel_geom_2700_200`
- Frames: `200`
- High-degeneracy frames (`score >= 0.90`): `0` (0.0%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 50919.9050 | 50845.5000 | 51378.1000 | 51422.0000 |
| `linearity` | 0.7191 | 0.7390 | 0.7687 | 0.7781 |
| `planarity` | 0.1752 | 0.1511 | 0.2689 | 0.2809 |
| `scattering` | 0.1057 | 0.1060 | 0.1157 | 0.1182 |
| `degeneracy_score` | 0.7191 | 0.7390 | 0.7687 | 0.7781 |
| `range_mean_m` | 6.2242 | 5.9961 | 7.3772 | 7.5050 |
| `range_p95_m` | 13.7578 | 14.4493 | 15.3092 | 15.7131 |
| `intensity_mean` | 1976.7126 | 2050.9972 | 2149.1430 | 2153.8696 |
| `intensity_std` | 1303.6287 | 1353.8066 | 1410.9191 | 1420.7789 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
