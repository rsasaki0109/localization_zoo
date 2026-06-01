# NDT T1 KITTI 108-Frame Convergence Basin

Generated with `evaluation/scripts/run_convergence_basin_sweep.py` using
`--profile ndt_t1`, `--max-frames 108`, and success threshold `ATE <= 0.5m`.
Perturbations are applied in the local GT seed frame while evaluation uses the
unperturbed GT trajectory.

## Coarse Grid

| Sequence | Grid | Success | Best ATE [m] | Max successful x [m] | Max successful yaw [deg] |
| --- | --- | ---: | ---: | ---: | ---: |
| 02 | x={0,1,2}, yaw={0,5,10} | 3/9 | 0.042679 | 0.0 | 10.0 |
| 05 | x={0,1,2}, yaw={0,5,10} | 3/9 | 0.035833 | 0.0 | 10.0 |
| 08 | x={0,1,2}, yaw={0,5,10} | 3/9 | 0.228466 | 0.0 | 10.0 |

## Fine Axis Sweep

| Sequence | Axis | Successful values | Failure boundary |
| --- | --- | --- | --- |
| 02 | x [m] | 0.0, 0.25 | 0.5m gives 0.502m ATE |
| 02 | y [m] | 0.0, 0.25 | 0.5m gives 0.502m ATE |
| 02 | yaw [deg] | 0, 2, 5, 10, 15, 20 | not reached |
| 05 | x [m] | 0.0, 0.25 | 0.5m gives 0.510m ATE |
| 05 | y [m] | 0.0, 0.25 | 0.5m gives 0.529m ATE |
| 05 | yaw [deg] | 0, 2, 5, 10, 15, 20 | not reached |
| 08 | x [m] | 0.0, 0.25 | 0.5m gives 0.567m ATE |
| 08 | y [m] | 0.0, 0.25 | 0.5m gives 0.557m ATE |
| 08 | yaw [deg] | 0, 2, 5, 10, 15, 20 | not reached |

## Readout

NDT T1 is a strong seeded upper bound, but its translation convergence basin is
narrow under the current scan-to-map runner. Across KITTI seq 02/05/08 short
windows, 0.25m translation perturbations remain below the 0.5m ATE threshold,
while 0.5m perturbations are already outside threshold. Yaw-only perturbations
up to 20deg remain stable.

This points the next experiment at translational initialization rather than yaw
bootstrapping: CT-ICP or another odometry prior needs to get NDT within roughly
0.25m translation on these windows, or the local correction must be widened.
