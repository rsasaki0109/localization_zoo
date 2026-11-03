# L-LO: where the KITTI 09/10 gap is

L-LO `seq07_tuned` scores 4.122 % / 4.105 % official KITTI RTE on 09 / 10,
2.18x / 2.16x the paper's 1.89 % / 1.90 %. The saved-pose diagnostics found few
bad steps and path lengths within 0.1 % of GT, so they located the drift without
explaining it. This study splits the error by channel using only committed poses.

## Method

`decompose.py` scores trajectories with the official KITTI RTE (100-800 m)
against the full 6-DoF GT as the runner evaluates it: the committed reference
CSV with the first pose's inverse applied. Where a run saved its evaluated GT,
the two agree within 5e-7 (print precision). A pose is split as
`R = Rz(yaw) Ry(pitch) Rx(roll)`, `t = (x, y, z)` in the sensor frame:

- **horizontal-only error**: estimate x, y, yaw with GT z, pitch, roll;
- **vertical-only error**: GT x, y, yaw with estimate z, pitch, roll;
- **GT planar**: GT x, y, yaw with z = 0 and no pitch/roll;
- **GT roll-free**: GT position and yaw/pitch, roll = 0.

RTE is a mean of segment-end error norms, so the two channel-isolated values are
attributions, not parts of a sum. The GT roll-free values reproduce the
"GT positions, attitude without roll" column already in `papers/l_lo/README.md`
for 00/02/05/07/08 exactly (1.055 / 0.955 / 0.685 / 0.909 / 1.419 %). The GT planar
values are within 0.003 points of the "x, y, yaw only" column there.

```bash
python3 experiments/results/l_lo_vertical_decomposition/decompose.py
```

## Result

Official RTE [%]; "-" where no saved L-LO poses are committed (00/02/05/07/08
predate per-run pose export).

| Seq | Paper | Estimate | Horizontal-only error | Vertical-only error | GT planar | GT roll-free | GT height range [m] |
|---|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.91 | - | - | - | 0.974 | 1.055 | 20.0 |
| 02 | 2.35 | - | - | - | 2.801 | 0.955 | 49.1 |
| 03 | 2.92 | 3.066 | 1.927 | 2.093 | 2.212 | 0.924 | 40.5 |
| 04 | 2.28 | 2.135 | 1.454 | 1.484 | 1.541 | 0.010 | 5.2 |
| 05 | 0.82 | - | - | - | 0.901 | 0.685 | 13.5 |
| 06 | 0.66 | 1.315 | 0.535 | 1.137 | 0.912 | 0.295 | 6.6 |
| 07 | 0.69 | - | - | - | 0.757 | 0.909 | 3.3 |
| 08 | 1.37 | - | - | - | 1.095 | 1.419 | 37.0 |
| 09 | 1.89 | 4.122 | **1.458** | 3.505 | 3.537 | 1.226 | 36.0 |
| 10 | 1.90 | 4.105 | **1.536** | 3.558 | 3.843 | 0.666 | 23.8 |

- On 09 and 10 the horizontal module is **below** the paper's total error (1.458
  vs 1.89 %, 1.536 vs 1.90 %). The whole gap is in the vertical channel.
- The vertical-only error (3.505 / 3.558 %) is close to a GT trajectory flattened
  to z = 0 without pitch (3.537 / 3.843 %). The default `pitch_mode=0` sets each
  step's z to zero and does not follow road grade, so on these hilly sequences it
  behaves like a flat trajectory.
- Even GT positions and heading with a flat vertical channel (GT planar) score
  3.5-3.8 % on 09/10, about twice the paper. A frame-to-frame estimate that does
  not track the vertical channel cannot reach the paper's values there.
- On 06 the vertical channel also dominates (1.137 vs 0.535 % horizontal); on 03
  and 04 both channels contribute comparably.

## Existing vertical modes on sequence 10 (development check, negative)

Before planning a full-sequence run, the two existing opt-in vertical modes were
run once on sequence 10 with otherwise unchanged `seq07_tuned` arguments
(`--l-lo-set cluster_cell=0.25 --l-lo-set ground_tolerance=0.15 --l-lo-set max_range=80`).
The 0.4 deg value is the one with the smallest per-frame pitch bias in the README's
GT-horizontal prototype on 07, not an RTE choice.

| Arm (`--l-lo-set ...`) | Official RTE [%] | Horizontal-only | Vertical-only | Estimated height range [m] | Final height est / GT [m] |
|---|---:|---:|---:|---:|---:|
| stored `seq07_tuned` (`pitch_mode=0`) | 4.105 | 1.536 | 3.558 | - | - |
| `pitch_mode=1 vertical_angle_correction_deg=0.4` | 4.540 | 1.353 | 4.397 | 69.9 (GT 23.8) | -40.9 / 11.2 |
| `pitch_mode=1` | 10.149 | 4.510 | 10.357 | 173.4 (GT 23.8) | -110.2 / 11.2 |

The same-patch plane fit integrates frame-to-frame pitch and height changes; their
bias and noise accumulate, so the estimated altitude drifts by tens of metres and
the vertical channel ends up worse than not modelling it. Both modes are
rejected; no full-sequence run was made and defaults are unchanged. Summaries,
logs and compressed trajectories are in [`development_seq10/`](development_seq10/).
They used `build/evaluation/pcd_dogfooding` SHA-256
`7cbd8e0d737d6a7cb6d0e326a5ccb16ae02d11286d4d34543e228f50a7e50d2c`. That binary was
built from the LF-GICP study branch (`bfe1d4d1`), whose L-LO code and L-LO runner
path are identical to `main`.

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_10_full experiments/reference_data/kitti_seq_10_full_gt.csv 0 --methods l_lo --l-lo-set cluster_cell=0.25 --l-lo-set ground_tolerance=0.15 --l-lo-set max_range=80 --l-lo-set pitch_mode=1 [--l-lo-set vertical_angle_correction_deg=0.4] --summary-json summary.json
python3 experiments/results/l_lo_vertical_decomposition/decompose.py --extra "label:<trajectory>:<evaluated_gt>"
```

## What this does not show

The decomposition attributes error to channels of the saved poses; it does not
show what vertical estimator the paper used. Sequence 10 was the development
sequence for the negative check, and all KITTI sequences are project development
data. Closing the gap needs a vertical estimate that does not drift frame to frame
(for example against a local ground map), which is outside the paper's described
frame-to-frame method and would be opt-in.
