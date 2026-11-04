# L-LO: ground-height-map vertical mode on full KITTI 00/02-10

L-LO's remaining paper gap is vertical. The
[channel decomposition](../l_lo_vertical_decomposition/README.md) shows that on
09/10 the horizontal error alone is below the paper. The paper's frame-to-frame
vertical model and the existing opt-in plane modes either ignore height or let
it drift. This study tests one opt-in vertical estimator with a single fixed
setting on every sequence with a paper value. The [protocol](protocol.md) was
committed (`aa057fab`) before any full-sequence run; sequence 10 was the
development sequence.

## What `pitch_mode=3` does

It is **not in the paper**, opt-in and off by default. The code is in
`papers/l_lo/src/l_lo.cpp` (`solveGroundMap`, `insertGround`).

- Each frame's ground points in the existing 5-25 m band are thinned to one mean
  point per 0.5 m cell.
- They are stored in world coordinates in a height map of the last 30 frames
  (mean height per cell).
- Height, pitch and roll come from Gauss-Newton iterations (5, Huber 0.1 m, at
  least 50 residuals) that match the current ground points' world heights to the
  map. With `R = Rz(yaw) Ry(pitch) Rx(roll)`, a point's world height does not
  depend on yaw.
- x, y and yaw stay those of L-LO's horizontal module. The horizontal step is
  applied in the previous sensor's x-y plane.
- All constants were fixed in the code before any KITTI run, and none was chosen
  on an evaluated error.

Mechanism tests (`papers/l_lo/test/test_l_lo.cpp`, synthetic streets):

- body roll and pitch are recovered on flat ground, where the paper mode never
  estimates roll;
- a 4 % grade change is tracked to 5 cm, where the paper mode loses part of the
  climb;
- horizontal motion is unchanged on flat ground.

## Full KITTI 00/02-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed. The `seq07_tuned`
arguments are unchanged in both arms, which ran concurrently on the same binary
(SHA-256 `f11f6032…`). Sequence 01 is NA in the paper. The horizontal-only and
vertical-only channels keep only that part of each arm's error (see
`../gap_decomposition/`).

| Seq | Paper | `seq07_tuned` | + ground map | Candidate / reference | Vertical-only (ref → cand) | Horizontal-only (ref → cand) | ATE ref / cand [m] |
|---|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.91 | 1.479 (1.63x) | 1.577 (1.73x) | 1.066 | 0.982 → 1.194 | 0.887 → 0.884 | 15.7 / 19.9 |
| 02 | 2.35 | 4.478 (1.91x) | 4.091 (1.74x) | 0.914 | 2.681 → 1.962 | 3.045 → 3.144 | 142.0 / 143.6 |
| 03 | 2.92 | 3.066 (1.05x) | 2.336 (0.80x) | 0.762 | 2.093 → 1.304 | 1.927 → 1.899 | 24.1 / 13.9 |
| 04 | 2.28 | 2.135 (0.94x) | 2.087 (0.92x) | 0.977 | 1.484 → 1.491 | 1.454 → 1.424 | 5.3 / 7.1 |
| 05 | 0.82 | 1.473 (1.80x) | 1.308 (1.59x) | 0.888 | 0.967 → 0.783 | 0.892 → 0.881 | 15.5 / 14.1 |
| 06 | 0.66 | 1.315 (1.99x) | 1.051 (1.59x) | 0.800 | 1.137 → 0.819 | 0.535 → 0.521 | 5.8 / 6.2 |
| 07 | 0.69 | 1.004 (1.46x) | 0.950 (1.38x) | 0.946 | 0.699 → 0.706 | 0.577 → 0.552 | 2.7 / 3.0 |
| 08 | 1.37 | 1.661 (1.21x) | 1.493 (1.09x) | 0.899 | 1.094 → 0.893 | 1.068 → 1.062 | 36.4 / 38.1 |
| 09 | 1.89 | 4.122 (2.18x) | 2.304 (1.22x) | 0.559 | 3.505 → 1.599 | 1.458 → 1.502 | 36.4 / 34.8 |
| 10 | 1.90 | 4.105 (2.16x) | 2.345 (1.23x) | 0.571 | 3.558 → 1.641 | 1.536 → 1.603 | 8.6 / 16.9 |

| Arm | Geometric mean vs paper | Sequences above 2x |
|---|---:|---|
| `seq07_tuned` (Table 6) | 1.569x | 09, 10 |
| `seq07_tuned` + `pitch_mode=3` | **1.289x** | none |

- **Reference check.** The fresh `seq07_tuned` equals the stored Table 6 result
  on all 10 sequences (official RTE, 100 m RPE, ATE, frame count).
- **Candidate against the reference.** 9 sequences lower, 1 higher; geometric
  mean of the per-sequence ratio 0.821x. 09 and 10 fall from 2.18x/2.16x to
  1.22x/1.23x, which is where the vertical channel was largest.
- **The adverse sequence.** 00 rises from 1.479 to 1.577 % (+6.6 %), with its
  vertical channel 0.982 → 1.194 %. Its height range is 20 m and its vertical
  channel was already near the horizontal one.
- **Horizontal channel unchanged.** It moves by at most ±0.1 points on every
  sequence: the mode changes the vertical part of the pose as intended.
- **ATE.** ATE is lower on 03, 05 and 09 and higher on 00, 02, 04, 06, 07, 08
  and 10 (10: 8.6 → 16.9 m). The map follows the estimate, so absolute height
  can still drift over a sequence: on 10 the estimated height range is 33.6 m
  against 23.8 m in GT. The official RTE is relative over 100-800 m and benefits
  from the bounded local tilt error. Absolute drift remains.

## What follows

Defaults do not change. Table 6 and `repo_variants` stay on `seq07_tuned`
(1.57x): the mode is outside the paper's frame-to-frame method, so promoting it
is the owner's decision. The study shows that L-LO's horizontal module plus a
scan-to-map vertical estimate comes within 1.29x of the paper, with no sequence
above 2x. Every KITTI sequence is project development data, and 10 motivated
the mode.

## Commands and execution record

```bash
python3 experiments/results/l_lo_ground_map/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [...]
python3 experiments/results/l_lo_ground_map/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/l_lo_ground_map/summarize_study.py
```

One arm by hand:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods l_lo --l-lo-set cluster_cell=0.25 --l-lo-set ground_tolerance=0.15 --l-lo-set max_range=80 --l-lo-set pitch_mode=3 --summary-json out.json
```

The [`records/`](records/) directory holds every command with exit codes, input
hashes and timings.

- `run_record_part1.json` covers 10/04/03/06/07/09.
- The container was then restarted while sequence 05 was being fetched (484 of
  2761 scans on disk). Nothing for 05 had run, and no record entry was written.
- `run_record_part2.json` covers 05/00/02/08. Complete frames were reused by
  archive size and the rest re-fetched with CRC checks. Every sequence's scan
  hashes were then recorded before running.
- Both parts use the same binary SHA-256. All 20 arms and their step diagnostics
  exit 0, and every sequence's input hashes match before and after its runs.
  The scans were deleted after each sequence.
- Both arms of a sequence ran concurrently, so FPS is not comparable to the
  historical values.
