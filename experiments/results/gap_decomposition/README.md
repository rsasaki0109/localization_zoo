# Table 6 gap triage by error channel

Which kind of error makes each remaining paper gap? `decompose_table6.py`
takes the exact Table 6 variant of every method/sequence and checks that its
committed poses reproduce the Table 6 value to three decimals. It then scores
trajectories in which only one channel keeps the estimate's error. All scores
use the official KITTI RTE (100-800 m) against the full GT, which comes from
the committed reference CSV normalized to the first pose.

| Channel | Estimate keeps | GT supplies |
|---|---|---|
| horizontal | x, y, yaw | z, pitch, roll (world frame) |
| vertical | z, pitch, roll | x, y, yaw |
| rotation | relative rotations | relative translations |
| yaw | the yaw of each relative rotation | relative pitch/roll and translations |
| tilt | the pitch/roll of each relative rotation | relative yaw and translations |
| translation | relative translations | relative rotations |
| scale | each step's length | relative rotations and step directions |

These are attributions, not parts of a sum: RTE averages segment-end error
norms, so channels overlap or cancel. A channel ratio above 1 means that this
error alone already exceeds the paper's total. It does not say why the
estimator made the error.

```bash
python3 experiments/results/gap_decomposition/decompose_table6.py   # ~3 min, writes decomposition.json
```

## Coverage

Of 144 Table 6 cells, 117 were analysed. In 10 cells the committed poses do not
reproduce the Table 6 value: A-LOAM 00/02/05/07/08, CT-ICP 02/04/07/10 and
Mesh-LOAM 00. These come from the historical replay differences documented in
`experiments/results/shared_near_range/`. 17 cells have no committed poses.
Every cell and its status is in [`decomposition.json`](decomposition.json).

## Cells at or above 1.3x the paper (ratio to paper)

| Method | Seq | Total | Horizontal | Vertical | Rotation | Yaw | Tilt | Translation | Scale |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| InTEn-LOAM | 01 | 25.96 | 25.81 | 1.17 | 1.99 | 1.75 | 0.75 | 26.04 | **22.16** |
| LF-GICP | 10 | 6.71 | 6.56 | 0.54 | 6.14 | **5.99** | 0.72 | 1.52 | 1.44 |
| InTEn-LOAM | 02 | 5.97 | 5.76 | 1.32 | 4.61 | 4.51 | 0.68 | 6.22 | **5.45** |
| InTEn-LOAM | 10 | 2.57 | 2.40 | 0.70 | 2.18 | **2.02** | 0.54 | 1.19 | 0.80 |
| L-LO | 09 | 2.18 | 0.77 | 1.85 | 2.49 | 0.88 | **2.09** | 0.78 | 0.15 |
| L-LO | 10 | 2.16 | 0.81 | 1.87 | 2.07 | 0.73 | **1.80** | 0.71 | 0.07 |
| SuMa | 01 | 2.09 | 1.99 | 0.34 | 0.42 | 0.22 | 0.32 | 1.94 | **1.68** |
| Mesh-LOAM | 04 | 2.00 | 1.73 | 0.72 | 1.26 | 0.72 | 0.71 | 1.58 | 0.94 |
| L-LO | 06 | 1.99 | 0.81 | 1.72 | 2.68 | 0.79 | 2.42 | 2.19 | 0.27 |
| L-LO | 02 | 1.91 | 1.30 | 1.14 | 1.86 | 1.10 | 1.30 | 1.17 | 0.63 |
| InTEn-LOAM | 04 | 1.83 | 1.75 | 0.47 | 0.43 | 0.17 | 0.37 | 1.80 | **1.47** |
| L-LO | 05 | 1.80 | 1.09 | 1.18 | 1.82 | 1.16 | 1.21 | 1.82 | 0.23 |
| L-LO | 00 | 1.63 | 0.97 | 1.08 | 1.80 | 1.05 | 1.27 | 1.19 | 0.23 |
| A-LOAM | 01 | 1.53 | 0.99 | 1.49 | 1.35 | 0.25 | 1.27 | 0.58 | 0.09 |
| Mesh-LOAM | 10 | 1.51 | 1.14 | 0.92 | 1.28 | 0.94 | 0.78 | 0.63 | 0.21 |
| InTEn-LOAM | 06 | 1.49 | 1.29 | 0.63 | 1.16 | 0.84 | 0.72 | 1.46 | 0.42 |
| Mesh-LOAM | 03 | 1.47 | 0.73 | 1.20 | 1.30 | 1.14 | 0.57 | 1.38 | 0.52 |
| ELO | 02 | 1.43 | 1.28 | 0.68 | 1.24 | 1.17 | 0.27 | 0.97 | 0.34 |
| ELO | 01 | 1.42 | 1.38 | 0.35 | 0.62 | 0.38 | 0.45 | 1.54 | 0.37 |
| DiLO | 01 | 1.42 | 1.20 | 0.59 | 1.01 | 0.69 | 0.59 | 0.89 | 0.32 |
| InTEn-LOAM | 03 | 1.41 | 1.22 | 0.59 | 1.94 | **1.91** | 0.17 | 1.27 | 0.29 |
| ELO | 09 | 1.41 | 1.19 | 0.65 | 0.74 | 0.57 | 0.42 | 1.17 | 0.34 |
| MULLS | 04 | 1.40 | 1.34 | 0.34 | 0.23 | 0.18 | 0.13 | 1.32 | 0.86 |
| InTEn-LOAM | 05 | 1.38 | 1.26 | 0.42 | 1.78 | **1.63** | 0.49 | 0.97 | 0.37 |
| InTEn-LOAM | 09 | 1.38 | 1.21 | 0.52 | 1.14 | 1.07 | 0.25 | 1.22 | 0.54 |
| ELO | 06 | 1.37 | 1.12 | 0.59 | 1.56 | 1.27 | 0.69 | 1.13 | 0.43 |
| ELO | 03 | 1.37 | 1.12 | 0.69 | 1.73 | **1.70** | 0.19 | 1.37 | 0.50 |
| ELO | 08 | 1.37 | 1.08 | 0.68 | 0.95 | 0.85 | 0.30 | 0.96 | 0.34 |
| Mesh-LOAM | 09 | 1.34 | 0.85 | 0.93 | 1.03 | 0.76 | 0.57 | 1.11 | 0.29 |
| InTEn-LOAM | 00 | 1.34 | 1.19 | 0.49 | 1.27 | 1.15 | 0.33 | 1.04 | 0.46 |
| Mesh-LOAM | 01 | 1.34 | 1.10 | 0.67 | 0.84 | 0.38 | 0.72 | 1.42 | 0.35 |
| InTEn-LOAM | 08 | 1.32 | 1.00 | 0.72 | 0.97 | 0.91 | 0.24 | 1.04 | 0.35 |

**Checks against the two causes already established.** LF-GICP 10 attributes to
yaw (5.99x). Its world-origin Fisher direction turns into a rotation about the
sensor ([study](../lf_gicp_sensor_centered_fisher/README.md)). L-LO 09/10
attributes to tilt and the vertical channel, matching the
[vertical-channel study](../l_lo_vertical_decomposition/README.md).

## What the triage shows

- **Step-length collapse** (scale channel alone above the paper):
  - InTEn-LOAM 01 (22.16x), 02 (5.45x) and 04 (1.47x);
  - SuMa 01 (1.68x).
  These are straight, forward-degenerate stretches where the estimated motion
  is too short. The InTEn 01/02 collapse was already traced to S2S output
  ([natural traces](../inten_natural_traces/README.md)). 04 and SuMa 01 belong
  to the same class.
- **Heading error.** InTEn-LOAM 10 (yaw 2.02x, scale 0.80x), 03 (1.91x), 05 (1.63x)
  and ELO 03 (1.70x). The InTEn 10 "gradual drift" that the earlier saved-pose
  diagnostics could not place is heading error, not step length.
- **Distributed gaps.** ELO and Mesh-LOAM mostly have no single channel that
  dominates: most channels lie between 0.7x and 1.4x.
- **Origin dependence (negative).** A follow-up check binned the per-step
  absolute yaw error by the sensor's distance from the world origin. On
  00/03/05/08/10 InTEn-LOAM is noisier than CT-ICP, MULLS, DiLO, IMLS-SLAM and
  A-LOAM: 3.1-6.6 deg/100 m against 1.4-5.6.
  Its error does not grow with distance from the origin any more than theirs
  does; the 400-800 m bin of 00 is larger for every method. This crude metric
  is dominated by per-step noise and does not support an LF-GICP-like origin
  dependence for InTEn.

All KITTI is project development data. These attributions locate error types on
saved trajectories; they change no estimator, default, stored result or Table 6
value.

## InTEn-LOAM heading error (follow-up)

`inten_heading.py` reads the committed passive trace of the unchanged
`paper_odometry_plane_check` run on KITTI 10
(`experiments/results/inten_natural_traces/sequences/10/on`). It compares
per-frame relative yaw at three stages with GT: the S2S prediction, the S2S
output, and the final pose after S2M. Results are in
[`inten_heading.json`](inten_heading.json).

| Stage (KITTI 10) | Mean yaw error [deg/frame] | Std | Sum over 1200 steps [deg] | Yaw-only official RTE |
|---|---:|---:|---:|---:|
| S2S prediction | -0.0048 | 0.098 | -5.78 | 1.456 % |
| S2S output | -0.0048 | 0.044 | -5.77 | 1.068 % |
| Final (after S2M) | -0.0024 | 0.039 | -2.90 | 0.909 % |

The paper's total on 10 is 0.45 %.

**S2S leaves the bias in; S2M halves it.** S2S removes most of the prediction's
noise but none of its bias. S2M halves the bias, and the remainder alone is
twice the paper's total error.

**The bias drives the heading error, not the noise.** On 03/05/10 almost every
method has a small negative yaw bias, which may be a common GT/calibration
component. InTEn-LOAM's bias is the largest on all three sequences, apart from
LF-GICP's sign-flipped failure on 10:

| Seq | InTEn-LOAM bias | Other methods' bias |
|---|---:|---|
| 10 | -0.0024 | -0.0001 to -0.0018 |
| 03 | -0.0043 | -0.0006 to -0.0039 |
| 05 | -0.0020 | -0.0005 to -0.0029 (L-LO -0.0029, the rest at most -0.0013) |

Quadric-LO is noisier (std 0.048-0.051) but nearly unbiased, and its yaw channel
stays at 0.21-0.29x the paper. InTEn-LOAM's yaw channel reaches 1.6-2.0x.

**The bias does not follow turning or speed.** A per-frame regression on GT
turn rate and step length explains at most 1.5 % of the variance at any stage.
On straight frames the S2S bias is largest at walking speed (step below 0.3 m:
-0.0155 deg/frame over 124 frames). After S2M the bias is -0.002 to
-0.0066 deg/frame at every speed. Slow frames contribute only about 0.3 deg of
the 2.9 deg total.

These are observations on saved traces of one development sequence (10),
compared across methods on 03/05/10. They locate the heading error in a
systematic per-frame bias that S2S does not correct and S2M only halves. They do
not identify which association or residual produces it. Finding that needs a
native intervention, for example S2S with selected feature classes removed at
fixed history, as the earlier InTEn studies did for step length.

```bash
python3 experiments/results/gap_decomposition/inten_heading.py   # needs decomposition.json; writes inten_heading.json
```
