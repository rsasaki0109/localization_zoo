# CT-ICP Diagnostic Watch Sweep

Comparison source: `experiments/results/lidar_degeneracy/method_health_comparison/method_health_comparison.json`

## Variants

| Variant | max iter | ceres iter | planarity | Rows | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear | Actions | Guard | Blockers |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- |
| `current` | 8 | 1 | 0.080 | 6 | 1.000 | 0.121 | 1.000 | 7.822 | 2.926 | 2.619 | 0 | fallback_required:5, optimizer_retry:1 | fallback_to_prior:5, retry_optimizer:1 | iterations_pinned:6, path_vs_all_high:3, path_vs_healthy_high:5 |
| `more_iter` | 16 | 1 | 0.080 | 6 | 1.000 | 0.213 | 1.000 | 14.483 | 3.048 | 2.746 | 1 | clear_candidate:1, fallback_required:5 | fallback_to_prior:5, use_refined:1 | iterations_pinned:2, path_vs_all_high:4, path_vs_healthy_high:5 |
| `more_ceres` | 8 | 3 | 0.080 | 6 | 0.994 | 0.155 | 0.994 | 7.799 | 2.988 | 2.598 | 0 | fallback_required:4, optimizer_retry:1, reject_or_retry:1 | fallback_to_prior:4, reject_or_retry:1, retry_optimizer:1 | accepted_below_one:1, iterations_pinned:6, path_vs_all_high:3, path_vs_healthy_high:4, refinement_gate_below_one:1 |
| `relaxed_planarity` | 8 | 1 | 0.050 | 6 | 1.000 | 0.161 | 1.000 | 7.776 | 3.018 | 2.670 | 0 | fallback_required:5, optimizer_retry:1 | fallback_to_prior:5, retry_optimizer:1 | iterations_pinned:6, path_vs_all_high:3, path_vs_healthy_high:5 |

## Rows

| Variant | Sequence | Window | Accepted | Converged | CT gate | CT iter | Path/healthy | Path/all | Clear? | Action | Guard decision | Blockers |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | --- | --- | --- | --- |
| `current` | `fog_200` | `baseline` 0-29 | 1.000 | 0.138 | 1.000 | 7.759 | 0.942 | 0.967 | no | `optimizer_retry` | `retry_optimizer` | iterations_pinned |
| `current` | `fog_200` | `point_count_tail` 115-144 | 1.000 | 0.000 | 1.000 | 8.000 | 2.750 | 2.750 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `current` | `tunnel_geom_2700_200` | `baseline` 5-34 | 1.000 | 0.138 | 1.000 | 8.000 | 3.415 | 1.549 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `current` | `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 1.000 | 0.103 | 1.000 | 7.793 | 3.683 | 3.683 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `current` | `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 1.000 | 0.138 | 1.000 | 7.724 | 3.752 | 3.752 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `current` | `tunnel_geom_2700_200` | `degraded` 170-199 | 1.000 | 0.207 | 1.000 | 7.655 | 3.015 | 3.015 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `more_iter` | `fog_200` | `baseline` 0-29 | 1.000 | 0.103 | 1.000 | 15.276 | 1.205 | 1.237 | yes | `clear_candidate` | `use_refined` | none |
| `more_iter` | `fog_200` | `point_count_tail` 115-144 | 1.000 | 0.069 | 1.000 | 15.586 | 3.063 | 3.063 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `more_iter` | `tunnel_geom_2700_200` | `baseline` 5-34 | 1.000 | 0.103 | 1.000 | 15.862 | 3.372 | 1.529 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `more_iter` | `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 1.000 | 0.345 | 1.000 | 13.448 | 3.733 | 3.733 | no | `fallback_required` | `fallback_to_prior` | path_vs_healthy_high, path_vs_all_high |
| `more_iter` | `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 1.000 | 0.345 | 1.000 | 13.552 | 3.618 | 3.618 | no | `fallback_required` | `fallback_to_prior` | path_vs_healthy_high, path_vs_all_high |
| `more_iter` | `tunnel_geom_2700_200` | `degraded` 170-199 | 1.000 | 0.310 | 1.000 | 13.172 | 3.293 | 3.293 | no | `fallback_required` | `fallback_to_prior` | path_vs_healthy_high, path_vs_all_high |
| `more_ceres` | `fog_200` | `baseline` 0-29 | 1.000 | 0.138 | 1.000 | 7.690 | 1.094 | 1.124 | no | `optimizer_retry` | `retry_optimizer` | iterations_pinned |
| `more_ceres` | `fog_200` | `point_count_tail` 115-144 | 0.966 | 0.000 | 0.966 | 8.000 | 1.850 | 1.850 | no | `reject_or_retry` | `reject_or_retry` | accepted_below_one, refinement_gate_below_one, iterations_pinned |
| `more_ceres` | `tunnel_geom_2700_200` | `baseline` 5-34 | 1.000 | 0.103 | 1.000 | 7.897 | 4.336 | 1.966 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `more_ceres` | `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 1.000 | 0.103 | 1.000 | 7.931 | 3.799 | 3.799 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `more_ceres` | `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 1.000 | 0.207 | 1.000 | 7.759 | 3.517 | 3.517 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `more_ceres` | `tunnel_geom_2700_200` | `degraded` 170-199 | 1.000 | 0.379 | 1.000 | 7.517 | 3.335 | 3.335 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `relaxed_planarity` | `fog_200` | `baseline` 0-29 | 1.000 | 0.138 | 1.000 | 7.793 | 1.031 | 1.059 | no | `optimizer_retry` | `retry_optimizer` | iterations_pinned |
| `relaxed_planarity` | `fog_200` | `point_count_tail` 115-144 | 1.000 | 0.000 | 1.000 | 8.000 | 2.962 | 2.962 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `relaxed_planarity` | `tunnel_geom_2700_200` | `baseline` 5-34 | 1.000 | 0.172 | 1.000 | 7.690 | 3.878 | 1.758 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high |
| `relaxed_planarity` | `tunnel_geom_2700_200` | `point_count_tail` 80-109 | 1.000 | 0.207 | 1.000 | 7.690 | 3.572 | 3.572 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `relaxed_planarity` | `tunnel_geom_2700_200` | `geometry_degeneracy` 90-119 | 1.000 | 0.207 | 1.000 | 7.862 | 3.504 | 3.504 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |
| `relaxed_planarity` | `tunnel_geom_2700_200` | `degraded` 170-199 | 1.000 | 0.241 | 1.000 | 7.621 | 3.163 | 3.163 | no | `fallback_required` | `fallback_to_prior` | iterations_pinned, path_vs_healthy_high, path_vs_all_high |

## Readout

- This sweep only probes the diagnostic-watch CT-ICP windows; it does not change the main policy gate.
- `iterations_pinned` means the mean CT-ICP iteration count remains within 0.5 of the configured max iterations.
- A row can be a clear candidate only when acceptance, refinement-gate rate, iteration headroom, and path-ratio checks all pass.
- `fallback_required` means CT-ICP is accepted but its trajectory path disagrees with the healthy/all-method path reference enough that a production component should reject, downweight, or fall back instead of silently trusting it.
- Guard decisions translate watch actions into production choices: `use_refined`, `retry_optimizer`, `fallback_to_prior`, or `reject_or_retry`.
