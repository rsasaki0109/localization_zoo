# LiDAR Degeneracy Diagnostics

- PCD sequence: `/mnt/e/datasets/loc_zoo/dogfooding_results/lidar_degeneracy_fog_full`
- Frames: `1729`
- High-degeneracy frames (`score >= 0.90`): `44` (2.5%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 29118.0920 | 23346.0000 | 50317.0000 | 53173.0000 |
| `linearity` | 0.5215 | 0.5078 | 0.8265 | 0.9239 |
| `planarity` | 0.2213 | 0.2076 | 0.4174 | 0.6174 |
| `scattering` | 0.2572 | 0.2604 | 0.4498 | 0.5339 |
| `degeneracy_score` | 0.5398 | 0.5171 | 0.8265 | 0.9239 |
| `range_mean_m` | 2.9114 | 1.6029 | 7.7675 | 7.9508 |
| `range_p95_m` | 7.2216 | 3.3464 | 21.6783 | 23.2570 |
| `intensity_mean` | 1891.9295 | 1778.4151 | 3459.6720 | 4349.2752 |
| `intensity_std` | 1649.5563 | 1527.2993 | 2847.3072 | 2971.6230 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
