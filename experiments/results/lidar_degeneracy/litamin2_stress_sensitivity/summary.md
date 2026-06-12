# LiTAMIN2 Stress Sensitivity

GT-free short-window health check on NTNU LiDAR degeneracy selected windows. These are robustness probes, not accuracy claims.

| Sequence | Variant | Windows | Mean accepted | Mean converged | Flagged | Max used path m |
|---|---|---:|---:|---:|---:|---:|
| `fog_200` | `default` | 3 | 1.000 | 1.000 | 0 | 1.036 |
| `fog_200` | `icp_only` | 3 | 1.000 | 1.000 | 0 | 1.036 |
| `fog_200` | `radius1` | 3 | 1.000 | 1.000 | 1 | 1.337 |
| `fog_200` | `tight_seed` | 3 | 1.000 | 1.000 | 0 | 1.036 |
| `fog_200` | `tight_step` | 3 | 1.000 | 1.000 | 0 | 1.036 |
| `tunnel_geom_2700_200` | `default` | 4 | 1.000 | 1.000 | 0 | 3.035 |
| `tunnel_geom_2700_200` | `icp_only` | 4 | 1.000 | 1.000 | 0 | 3.035 |
| `tunnel_geom_2700_200` | `radius1` | 4 | 1.000 | 1.000 | 0 | 2.810 |
| `tunnel_geom_2700_200` | `tight_seed` | 4 | 1.000 | 1.000 | 0 | 3.035 |
| `tunnel_geom_2700_200` | `tight_step` | 4 | 1.000 | 1.000 | 0 | 3.035 |

## Finding

- ICP-only, tight seed gate, and tight step gate match the default on both sequences: all selected windows stay accepted/converged with no health flags.
- Radius-1 correspondence search changes path length, but only creates one `fog_200` nominal `low_used_path` flag; it does not expose a useful stress-only failure signal.
- Treat this as GT-free robustness coverage, not a paper-level accuracy result.
