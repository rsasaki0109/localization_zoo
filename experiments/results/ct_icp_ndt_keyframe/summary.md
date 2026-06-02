# CT-ICP + NDT Keyframe Correction, KITTI 108-Frame Windows

`ct_icp_ndt_keyframe` uses CT-ICP no-GT odometry as the base trajectory and
attempts NDT T1 corrections only on keyframes. Corrections are accepted when the
NDT result remains close to the CT-ICP prior pose. These runs use
`--ct-icp-dense-profile --ct-icp-refinement-gate` and NDT T1
(`--ndt-resolution 0.5 --ndt-max-iterations 12`).

## Keyframe Interval Sweep

Correction gate: translation <= 0.5m, rotation <= 0.35rad.

| Sequence | Interval | ATE [m] | RPE trans [%] | FPS |
| --- | ---: | ---: | ---: | ---: |
| 02 | 1 | 8.385247 | 2.558069 | 10.473299 |
| 02 | 5 | 8.381008 | 2.538438 | 18.585126 |
| 02 | 10 | 8.380805 | 2.538403 | 20.410872 |
| 05 | 1 | 0.552665 | 1.284727 | 10.327055 |
| 05 | 5 | 0.577926 | 1.284696 | 17.738115 |
| 05 | 10 | 0.579660 | 1.298984 | 19.849673 |
| 08 | 1 | 11.631727 | 32.147097 | 11.914317 |
| 08 | 5 | 11.631411 | 32.111273 | 18.938881 |
| 08 | 10 | 11.631591 | 32.091451 | 21.082944 |

## Gate Width Sweep

Interval fixed at 1 frame, rotation gate fixed at 0.35rad.

| Sequence | Translation gate [m] | ATE [m] | FPS | Accepted |
| --- | ---: | ---: | ---: | ---: |
| 02 | 0.25 | 8.385247 | 10.478023 | 96/107 |
| 02 | 0.50 | 8.385247 | 10.473299 | 96/107 |
| 02 | 1.00 | 8.385247 | 10.553397 | 96/107 |
| 05 | 0.25 | 0.552665 | 10.077664 | 102/107 |
| 05 | 0.50 | 0.552665 | 10.327055 | 102/107 |
| 05 | 1.00 | 0.552665 | 9.875299 | 102/107 |
| 08 | 0.25 | 11.631727 | 11.778769 | 99/107 |
| 08 | 0.50 | 11.631727 | 11.914317 | 99/107 |
| 08 | 1.00 | 11.631727 | 11.544649 | 99/107 |

## Readout

Keyframe correction improves runtime when the interval is raised, but it does
not solve the no-seed accuracy gap. Gate width also has no meaningful effect:
accepted NDT corrections are already small relative to the CT-ICP prior.

This exposes a runner-level limitation for product localization experiments:
the current NDT path builds its map online from the estimated trajectory. If the
CT-ICP prior has global drift, the NDT map drifts with it, so NDT can improve
local consistency but cannot relocalize to a fixed world map. To test true
map-based correction, the next step should be a fixed-map NDT localization mode
that builds the target map from GT/reference poses or a saved map artifact, then
uses CT-ICP only as the live initial guess.
