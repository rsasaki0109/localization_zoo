# CT-ICP Prior + NDT Hybrid, KITTI 108-Frame Windows

`ct_icp_ndt` runs CT-ICP without GT seed, then uses that pose stream as the
scan-to-map initial guess for NDT T1 (`--ndt-resolution 0.5
--ndt-max-iterations 12`). Runtime is reported as CT-ICP plus NDT combined.

## Baselines

| Sequence | NDT no-GT ATE [m] | CT-ICP ATE [m] | CT-ICP+NDT ATE [m] | NDT GT-seeded ATE [m] |
| --- | ---: | ---: | ---: | ---: |
| 02 | 64.699570 | 8.381005 | 8.385247 | 0.052461 |
| 05 | 50.913863 | 0.582715 | 0.552665 | 0.065936 |
| 08 | 26.597689 | 11.631729 | 11.631727 | 0.228466 |

## CT-ICP Prior Variants

| Sequence | Variant | CT-ICP+NDT ATE [m] | RPE trans [%] | FPS | Status |
| --- | --- | ---: | ---: | ---: | --- |
| 02 | dense gate | 8.385247 | 2.558069 | 10.712523 | ok |
| 02 | dense gate native seed | null | null | 42.271411 | invalid_metric |
| 02 | dense paper arch | 1.108092 | 1.311784 | 10.041992 | ok |
| 02 | dense paper arch native seed | 1.366547 | 1.436887 | 10.060170 | ok |
| 05 | dense gate | 0.552665 | 1.284727 | 10.429807 | ok |
| 05 | dense gate native seed | 0.507058 | 1.403969 | 10.114234 | ok |
| 05 | dense paper arch | 0.639161 | 1.933416 | 9.821875 | ok |
| 05 | dense paper arch native seed | 0.675713 | 1.886196 | 9.730112 | ok |
| 08 | dense gate | 11.631727 | 32.147097 | 12.413501 | ok |
| 08 | dense gate native seed | 15.882008 | 105.361202 | 43.467570 | ok |
| 08 | dense paper arch | 11.780471 | 34.398773 | 10.908845 | ok |
| 08 | dense paper arch native seed | 11.759380 | 34.174193 | 11.403273 | ok |

## Readout

The hybrid implementation works, but CT-ICP is not yet a universal initializer
for NDT T1. The convergence-basin sweep showed NDT needs roughly <=0.25m
translation error to recover cleanly. The CT-ICP priors here are still outside
that basin on seq02 and seq08, so NDT mostly preserves the CT-ICP trajectory
rather than correcting it. Seq05 is close enough to show a small improvement
from 0.582715m to 0.552665m, but still far from the GT-seeded NDT upper bound
of 0.065936m.

Next useful experiment: switch from frame-by-frame CT-ICP pose seeding to
keyframe correction with rejection. That should let CT-ICP provide continuity
while NDT only accepts corrections when score/error gates are favorable.
