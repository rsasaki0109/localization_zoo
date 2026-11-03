# InTEn-LOAM KITTI 10 yaw bias: feature-class arms (negative)

The [gap triage](../gap_decomposition/README.md#inten-loam-heading-error-follow-up)
attributed InTEn-LOAM's KITTI 10 gap (2.57x) mainly to heading. The cause is a
systematic per-frame yaw bias: -0.0048 deg/frame at the S2S output and
-0.0024 deg/frame after S2M. This study tests whether one feature class carries
that bias. The [protocol](protocol.md), including the reading rule, was
committed (`269c6751`) before any run; the analysis script was committed in
`0043f9bb` before the results.

## Runs

There are five recursive full runs of KITTI 10 (development sequence). All use
the unchanged `paper_odometry_plane_check` arguments; each non-reference arm
adds one existing `--inten-loam-paper-set` key. Passive traces were on. The arms
ran concurrently with one OpenMP thread each, the same binary (SHA-256
`7cbd8e0d…`) and exit code 0. Commands and timings are in
[`run_record.json`](run_record.json). The traces (31-37 MB each) are not
committed: [`results/`](results/) keeps each arm's summary, log and a per-frame
yaw-error CSV. The KITTI 10 scans were deleted after the runs.

**Reference check passes.** `reference` reproduces the committed
`paper_odometry_plane_check` result exactly: official RTE 1.155652 %, 100 m RPE
0.843492 %, ATE 9.669713 m. Its S2S and final yaw statistics equal those of the
committed natural trace bit for bit.

## Results

Yaw values are per-frame relative-yaw error against GT. "Yaw RTE" is the
official RTE of a trajectory that takes only yaw from that stage.

| Arm | Official RTE [%] | ATE [m] | S2S bias [deg/frame] | S2S yaw RTE [%] | Final bias [deg/frame] | Final std | Final yaw RTE [%] | Final bias / reference |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `reference` | 1.1557 | 9.67 | -0.00481 | 1.068 | -0.00242 | 0.0394 | 0.909 | 1.00 |
| `no_edge` (`edges_per_sector=0`) | 1.4260 | 13.46 | -0.00879 | 3.612 | -0.00314 | 0.0473 | 1.239 | 1.30 |
| `no_facade` (`facades_per_sector=0`) | 1.1304 | 8.50 | -0.00450 | 2.121 | -0.00176 | 0.0551 | 0.789 | 0.73 |
| `no_intensity` (`use_intensity=0`) | 1.2108 | 9.68 | -0.00511 | 1.073 | -0.00253 | 0.0391 | 0.961 | 1.05 |
| `no_ground` (`max_ground=0`) | 1.8686 | 19.00 | -0.00527 | 1.288 | -0.00249 | 0.0409 | 0.980 | 1.03 |

**No class is implicated by the fixed rule.** The rule asked for the final bias
to at least halve with RTE no worse than twice the reference. The largest change
is `no_facade` at 0.73x, which also lowers official RTE slightly
(1.156 → 1.130 %). Removing edges increases the bias. Removing intensity or
ground leaves it nearly unchanged.

**The bias is not iteration truncation.** S2S stops at its 10-iteration limit on
843 of 1200 reference frames. Even so, S2S's net yaw correction relative to its
own prediction averages +0.000008 deg/frame, and the last applied yaw update
averages -0.00017 deg with no consistent sign. Every arm behaves the same way.
On average, the S2S optimum agrees with its prediction's biased yaw rate rather
than pulling it toward GT. So the bias sits in what S2S converges to, not in
stopping early, and a larger iteration budget is not expected to remove it.

## What this does and does not show

- InTEn-LOAM's KITTI 10 heading bias does not come from one feature class alone.
  The edge class reduces it and the facade class adds to it; the others are
  neutral.
- These are recursive runs, so each arm also changes map history and the S2M
  input. They are not fixed-history interventions.
- Most other methods share a bias of the same sign on 03/05/10, mostly smaller,
  so part of it may come from the data, the GT or the calibration. This study does not
  separate that component.
- The next step would be a fixed-history attribution: split each S2S system's
  yaw gradient by feature class at the GT pose. The passive trace records only
  combined normal equations, so this needs a native extension.

No estimator, default, stored result or Table 6 value changes. All KITTI is
project development data.

```bash
python3 experiments/results/inten_heading_classes/run_arms.py --velodyne <kitti>/dataset/sequences/10/velodyne --work-root <scratch> --record run_record.json
python3 experiments/results/inten_heading_classes/analyze_arms.py --work-root <scratch>
```
