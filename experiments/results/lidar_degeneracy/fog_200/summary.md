# LiDAR Degeneracy Diagnostics

- PCD sequence: `dogfooding_results/lidar_degeneracy_fog_200`
- Frames: `200`
- High-degeneracy frames (`score >= 0.90`): `0` (0.0%)

| Metric | Mean | Median | P95 | Max |
|---|---:|---:|---:|---:|
| `points` | 31639.0600 | 32090.0000 | 42968.4000 | 43126.0000 |
| `linearity` | 0.6135 | 0.6284 | 0.6923 | 0.7010 |
| `planarity` | 0.1887 | 0.1788 | 0.2837 | 0.3111 |
| `scattering` | 0.1978 | 0.1701 | 0.3037 | 0.3482 |
| `degeneracy_score` | 0.6135 | 0.6284 | 0.6923 | 0.7010 |
| `range_mean_m` | 1.7602 | 1.8098 | 1.8973 | 1.9095 |
| `range_p95_m` | 3.9951 | 4.1522 | 4.3061 | 4.3258 |
| `intensity_mean` | 3016.2858 | 3163.4847 | 3585.9382 | 3681.2978 |
| `intensity_std` | 2530.3023 | 2760.5239 | 2921.0037 | 2927.7346 |

Interpretation:

- `linearity` near 1 means points are mostly distributed along one dominant axis.
- `planarity` near 1 means a dominant plane with weak orthogonal structure.
- `scattering` near 0 with high `linearity` or `planarity` is a degeneracy warning.
- This is a GT-free diagnostic; use it before odometry metrics to locate difficult windows.
