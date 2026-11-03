# LF-GICP: sensor-centred Fisher direction on full KITTI 00-10

LF-GICP `paper_default` fails on KITTI 10 (5.616 % vs the paper's 0.837 %,
6.71x) while every other sequence is within 0.65-1.28x. This study locates the
cause and tests one opt-in correction with a single fixed setting on every
sequence. The [protocol](protocol.md) was committed (`bfe1d4d1`) before any
full-sequence run of the study; sequence 10 was the development sequence.

## Cause on sequence 10

The paper's Eq. (2) linearizes about the world origin (`J = [-[Rp + t]x, I]`,
`T <- exp(delta xi) T`), and Eq. (7) weights every correspondence by its
information along the weakest eigenvector of that Hessian. Written in that
twist, a rotation about the sensor needs a translation part `t x omega`, so its
twist norm grows with the distance `|t|` from the origin, and it becomes the
"weakest" direction once the vehicle has left the origin. The weights then
emphasize correspondences that constrain that rotation, not the weak
translation axis the paper targets.

Passive traces of the same binary (`--lf-gicp-trace-csv`,
[`development_seq10/`](development_seq10/), tabulated by `analyze_trace.py`):

| Frames | Gated | Mean distance from origin [m] | Paper-form weak direction is a rotation | Sensor-centred weak direction is a rotation | Max applied weight (paper / centred) | Weight share: half the weight on this fraction of correspondences (paper / centred) |
|---|---:|---:|---:|---:|---:|---:|
| 0-99 | 99 | 26 | 2 % | 0 % | 16.5 / 10.8 | 0.22 / 0.33 |
| 100-199 | 100 | 109 | 100 % | 0 % | 31.3 / 12.5 | 0.09 / 0.28 |
| 200-299 | 100 | 186 | 92 % | 17 % | 76.2 / 25.0 | 0.11 / 0.29 |
| 300-399 | 100 | 254 | 100 % | 0 % | 119.1 / 12.0 | 0.14 / 0.32 |
| 400-499 | 30 / 47 | 324 | 100 % | 0 % | 55.1 / 18.6 | 0.13 / 0.32 |

"Rotation" means a 10 m lever arm moves more under the rotation than under the
sensor-local translation of that eigenvector. The committed saved-pose
diagnostics of `paper_default` put the fastest drift in blocks 200-300 and
300-400 (path ratio 1.048 and 1.069), inside these gated frames.

The opt-in `--lf-gicp-sensor-centered-fisher` takes the weakest eigenvector of
`A^T H A`, `A = [[I, 0], [[t]x, I]]` (the same Hessian in a tangent centred on
the sensor), and maps it back to the world twist. Gate, field, normalization,
Huber, solver and every paper constant are unchanged. It is **not in the
paper** and is default off. Unit tests check the tangent change, the
zero-origin identity with Eq. (7), the origin dependence of the paper form and
the origin independence of the centred form, and odometry with the gate forced
on (`papers/lf_gicp/test/test_lf_gicp.cpp`).

## Full KITTI 00-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed, same binary for
all arms (SHA-256 `7cbd8e0d737d6a7cb6d0e326a5ccb16ae02d11286d4d34543e228f50a7e50d2c`).
Ratios are to the paper's Table I LF-GICP value. The paper's VGICP column is
its vanilla backend without degeneracy handling.

| Seq | Paper LF-GICP | `paper_default` | Sensor-centred | Candidate / reference | Gated frames (ref / cand) | No mitigation | Paper VGICP |
|---|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.692 | 0.664 (0.96x) | 0.765 (1.11x) | 1.152 | 33 / 32 of 4541 | 0.734 (1.06x) | 1.062 |
| 01 | 1.822 | 1.816 (1.00x) | 1.993 (1.09x) | 1.097 | 1100 / 1100 of 1101 | 1.923 (1.06x) | 2.390 |
| 02 | 1.084 | 1.075 (0.99x) | 1.059 (0.98x) | 0.984 | 628 / 634 of 4661 | 1.074 (0.99x) | 1.429 |
| 03 | 1.207 | 0.785 (0.65x) | 1.020 (0.85x) | 1.299 | 403 / 403 of 801 | 0.897 (0.74x) | 1.165 |
| 04 | 0.814 | 0.629 (0.77x) | 1.079 (1.33x) | 1.715 | 268 / 268 of 271 | 0.746 (0.92x) | 1.065 |
| 05 | 0.577 | 0.512 (0.89x) | 0.512 (0.89x) | 1.000 | 0 / 0 of 2761 | 0.512 (0.89x) | 0.841 |
| 06 | 0.546 | 0.543 (0.99x) | 0.544 (1.00x) | 1.002 | 3 / 3 of 1101 | 0.543 (0.99x) | 0.698 |
| 07 | 0.496 | 0.411 (0.83x) | 0.444 (0.90x) | 1.080 | 13 / 13 of 1101 | 0.483 (0.97x) | 0.596 |
| 08 | 0.878 | 1.005 (1.14x) | 0.889 (1.01x) | 0.885 | 787 / 746 of 4071 | 0.918 (1.05x) | 1.110 |
| 09 | 0.566 | 0.725 (1.28x) | 0.713 (1.26x) | 0.985 | 631 / 594 of 1591 | 0.669 (1.18x) | 1.268 |
| 10 | 0.837 | 5.616 (6.71x) | 0.993 (1.19x) | 0.177 | 429 / 446 of 1201 | 0.831 (0.99x) | 1.604 |

| Arm | Geometric mean ratio to paper | Mean RTE [%] | Sequences above 2x |
|---|---:|---:|---|
| `paper_default` | 1.119x | 1.253 | 10 |
| `sensor_centered_fisher` | 1.043x | 0.910 | none |
| `no_mitigation` | 0.979x | 0.848 | none |
| paper LF-GICP / VGICP | - | 0.865 / 1.203 | - |

- The fresh `paper_default` equals the committed historical result on all 11
  sequences (official RTE, 100 m RPE, ATE, frame count).
- Candidate against the fresh reference: **4 lower (02, 08, 09, 10), 6 higher
  (00, 01, 03, 04, 06, 07), 1 equal (05, gate never on)**; geometric mean of
  the per-sequence ratio 0.932x, almost entirely from 10.
- Adverse sequences: 04 0.629 → 1.079 % (+72 %, ATE 1.72 → 3.47 m),
  03 0.785 → 1.020 % (+30 %), 00 0.664 → 0.765 % (+15 %, only 32 gated frames),
  01 1.816 → 1.993 % (+10 %), 07 0.411 → 0.444 % (+8 %), 06 0.543 → 0.544 %.
- Sequence 10 with the candidate: 8 moving steps with length error above 20 %
  (all in frames 1-8, where the car starts in motion) instead of 26; 12 instead
  of 34 with local vector error above 20 %; ATE 50.98 → 13.43 m.
- Against `no_mitigation`, the candidate is lower on 3 sequences (02, 07, 08),
  higher on 7 (00, 01, 03, 04, 06, 09, 10) and equal on 05.

Every per-sequence value, the historical comparison and the step diagnostics of
all 33 runs are in [`summary.json`](summary.json) and
`experiments/results/runs/lf_gicp_kitti_seq_NN_full_sensor_fisher_matrix/<arm>/`.

## What this shows and what it does not

- The 6.71x failure on 10 is caused by the world-origin Fisher direction acting
  in a long gated stretch; with the direction taken about the sensor, 10 is at
  1.19x and no sequence exceeds twice the paper.
- The correction is **not a general improvement**: it is worse than the
  reference on six sequences. Where the gate is on for most of a short
  sequence (04, 03, 01), weighting the true weak translation axis hurts, which
  matches the paper's statement that weighting under dilution degrades accuracy.
- The gate fires far more often here than in the paper. The paper reports at
  most 8 % of KITTI open-road frames; here 01 is gated 100 %, 04 99 %, 03 50 %,
  09 37-40 %, 10 36-37 %, 08 18-19 % and 02 13-14 %; 00, 05, 06 and 07 stay
  at or below 1.2 %. This is the remaining discrepancy; it
  comes from the beta_f calibration documented in `papers/lf_gicp/README.md`.
- Without mitigation the backend scores 0.979x the paper's LF-GICP numbers and
  0.70x (geometric mean) its VGICP column. This repository's backend is
  therefore much closer to the paper's LF-GICP than to its VGICP baseline, so
  the paper's 28 % gain over VGICP is not reproduced as a gain from the
  mitigation here. If the paper's gate is as silent on KITTI as it states, its
  KITTI numbers are mostly backend numbers, which is consistent with, but not
  proven by, the no-mitigation arm.
- All KITTI sequences are project development data; sequence 10 motivated the
  candidate. No numeric value was chosen: the candidate is one Boolean.

Decisions fixed in the protocol: defaults are unchanged and Table 6 stays on
`paper_default` (1.12x). Promoting the opt-in, or replacing the beta_f
calibration, is left to the owner. A calibration that reproduces the paper's
gate behaviour by its own rule needs its second calibration trace (GEODE
Tunnel01), which is not available here.

## Commands

Build: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target pcd_dogfooding test_lf_gicp`.

One arm by hand (NN = sequence):

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods lf_gicp --summary-json out.json --lf-gicp-sensor-centered-fisher
```

Study (fetch, three concurrent arms with one OpenMP thread and their own
working directory each, staging, step diagnostics, hash re-check, scan
deletion), import and tables:

```bash
python3 experiments/results/lf_gicp_sensor_centered_fisher/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [--sequence ...]
python3 experiments/results/lf_gicp_sensor_centered_fisher/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/lf_gicp_sensor_centered_fisher/summarize_study.py
python3 experiments/results/lf_gicp_sensor_centered_fisher/analyze_trace.py development_seq10/default_trace.csv.gz
```

Then `refresh_study_docs.py`, `generate_full_variant_table.py` and
`generate_paper_ratio_table.py`. The regenerated PNG figures in
`docs/assets/paper/` were not committed (environment rendering churn, about
8 MB); every text and CSV asset was.

## Execution record

[`records/`](records/) holds every command with exit codes and timings:
04 (controller check, `run_record_04.json`), 10/01/03/06/07/09/05
(`run_record_main.json`) and 00/02/08 (`run_record_resume.json`), plus the
import. All 33 arms and 33 step diagnostics exit 0. For each of the 11 sequences
the scan hashes match before and after its runs, and the scans were deleted
afterwards.

- The first fetch of 00 failed with `ConnectionResetError` after 4281 of 4541
  scans ([log](records/fetch_00_attempt1_connection_reset.log)). The resumed
  fetch reused complete frames by their archive size and re-extracted 260 with
  CRC checks; the full sequence hash was then recorded before running.
- The resumed controller ran at commit `e50e4c10` (study scripts added only);
  the binary SHA-256 is identical in all records.
- Three arms ran concurrently on a 4-core host, so FPS values are not
  comparable to the historical single-run FPS.
