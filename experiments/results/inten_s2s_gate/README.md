# InTEn-LOAM: 5 m S2S gate with ground without forward on full KITTI 00-10

KITTI 01 is InTEn-LOAM's largest gap: 16.614 % against 0.64 % (25.96x).

- With a GT seed, S2S returns a good step on all but one of 155 inspected
  pairs ([seed interventions](../inten_seed_intervention/README.md)).
- With the natural seed, the step shrinks from frame to frame once it is a few
  percent short. At frame 215, a seed 0.953 of GT gives an output of 0.793.
- The correct solution therefore has a narrow basin along the forward axis.

S2S keeps a nearest neighbour only within 1 m (`s2s_gate_sq = 1`), and the
paper leaves this value open. At highway speed, about 2.5 m per scan, a short
seed moves many points out of that gate.

This study tests the LOAM-family gate of 5 m (A-LOAM
`DISTANCE_SQ_THRESHOLD = 25`). It is combined with the merged
`s2s_ground_no_forward` ([study](../inten_ground_forward/README.md)), and one
fixed setting runs on every sequence with a paper value. The
[protocol](protocol.md) was committed (`6be3e060`) before any full run of the
candidate on 00 or 03-10. 01 and 02 were the development sequences.

## Candidate (not in the paper, opt-in)

```
--inten-loam-paper-set s2s_gate_sq=25 --inten-loam-paper-set s2s_ground_no_forward=1
```

The other arguments are those of Table 6's `paper_odometry_plane_check`. No code
changes: `s2s_gate_sq` was already a runner key.

## Development (01 and 02)

Records are in [`development/`](development/). Official KITTI RTE:

| Seq | Reference | 5 m gate | 5 m gate + ground | Ground only (#187) |
|---|---:|---:|---:|---:|
| 01 | 16.614 % | 11.040 % | 9.384 % | 16.636 % |
| 02 | 3.221 % | 3.083 % | 0.811 % | 0.823 % |

After this, gates of 10 m and 20 m (with ground) were run on 01 only, to see
whether the effect saturates. They give 9.084 % and 9.049 %, so it does. The
candidate stays at the 5 m value fixed beforehand.

## Full KITTI 00-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed.

- Both arms ran concurrently on the same binary (SHA-256 `d6351e4e…`), which was
  also used for the development runs.
- "Ground only" is the candidate of `../inten_ground_forward/`: the same
  arguments without the gate, run there on another binary.

| Seq | Paper | Reference | Ground only | + 5 m gate + ground | Candidate / reference | Bad-length steps (ref → cand) | Scale-only (ref → cand) | ATE ref / cand [m] |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.52 | 0.697 (1.34x) | 0.701 | 0.685 (1.32x) | 0.983 | 13 → 13 | 0.239 → 0.233 | 17.64 / 17.27 |
| 01 (dev) | 0.64 | 16.614 (25.96x) | 16.636 | **9.384 (14.66x)** | 0.565 | 136 → 94 | 14.181 → 8.403 | 275.59 / 137.34 |
| 02 (dev) | 0.54 | 3.221 (5.97x) | 0.823 | **0.811 (1.50x)** | 0.252 | 133 → 18 | 2.945 → 0.269 | 122.84 / 41.41 |
| 03 | 0.63 | 0.891 (1.41x) | 0.894 | 0.888 (1.41x) | 0.997 | 2 → 1 | 0.185 → 0.187 | 6.26 / 6.27 |
| 04 | 0.37 | 0.676 (1.83x) | 0.683 | 0.687 (1.86x) | 1.016 | 0 → 0 | 0.543 → 0.552 | 1.54 / 1.56 |
| 05 | 0.36 | 0.497 (1.38x) | 0.504 | 0.478 (1.33x) | 0.961 | 0 → 0 | 0.133 → 0.146 | 9.98 / 9.82 |
| 06 | 0.24 | 0.359 (1.49x) | 0.352 | 0.357 (1.49x) | 0.994 | 0 → 0 | 0.100 → 0.100 | 1.92 / 1.94 |
| 07 | 0.34 | 0.364 (1.07x) | 0.347 | 0.356 (1.05x) | 0.978 | 0 → 0 | 0.157 → 0.158 | 1.46 / 1.31 |
| 08 | 0.71 | 0.938 (1.32x) | 0.953 | 0.978 (1.38x) | 1.042 | 20 → 19 | 0.252 → 0.248 | 27.67 / 27.56 |
| 09 | 0.48 | 0.661 (1.38x) | 0.662 | 0.643 (1.34x) | 0.972 | 0 → 0 | 0.259 → 0.254 | 11.00 / 10.07 |
| 10 | 0.45 | 1.156 (2.57x) | 1.120 | 1.108 (2.46x) | 0.958 | 0 → 0 | 0.358 → 0.345 | 9.67 / 8.68 |

| Arm | Geometric mean vs paper | Sequences above 2x |
|---|---:|---|
| `paper_odometry_plane_check` (Table 6) | 2.189x | 01, 02, 10 |
| + ground without forward (#187) | 1.926x | 01, 10 |
| + 5 m gate + ground without forward | **1.817x** | 01, 10 |

**Reference check.** The fresh reference equals the stored Table 6 result on all
11 sequences.

**01 (development).**
- RTE falls from 16.614 % to 9.384 %, still 14.66x the paper.
- For context, earlier studies used a separate stack of opt-ins (ground PCA,
  mapped-motion seed, soft prior). That stack reaches 5.28 % on 01, and the
  KISS-ICP hybrid reaches 0.97 % ([method README](../../../papers/inten_loam/README.md)).
  Neither is the Table 6 setting.
- ATE halves (275.6 → 137.3 m), and the path length rises from 0.872 to 0.922
  of GT.
- The two sustained collapses change. 217-253 becomes 201-209 and 211-220;
  420-518 becomes 417-427 and 477-513, with a recovery in between.
- A collapse at 823-844 is new.
- Neither the gate (it saturates beyond 5 m) nor the ground flag (no effect on
  01 alone) explains the remaining failures.

**02 (development).** 0.811 % (1.50x), the same as the ground flag alone.

**Held-out sequences (00, 03-10).**
- Candidate/reference geometric mean: 0.989x. 7 of 9 are lower: 00, 03, 05, 06,
  07, 09 and 10 (10: 1.156 → 1.108 %).
- Higher: 04 (+1.6 %) and 08 (+4.2 %). No sequence rises by more than 5 %, so
  none is adverse under the protocol.
- Against ground only, the held-out geometric mean is 0.994x.
- ATE is lower on 00, 05, 07, 08, 09 and 10, and higher on 03, 04 and 06.

## What this does and does not show

- The 1 m S2S gate limits recovery on the 01 highway: with 5 m, 01 nearly
  halves its error. The gate is a value the paper leaves open, and 5 m is the
  LOAM-family value.
- On the held-out sequences, the change is neutral to slightly positive (7 of 9
  lower, none above +5 %).
- 01 remains far from the paper (14.66x), and 10 stays above 2x (2.46x).
- Defaults do not change, and Table 6 / `repo_variants` stay on
  `paper_odometry_plane_check`. Promoting the setting is the owner's decision.
- Every KITTI sequence is project development data, and 01/02 chose the
  setting.

## Commands and execution record

```bash
python3 experiments/results/inten_s2s_gate/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [...]
python3 experiments/results/inten_s2s_gate/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/inten_s2s_gate/summarize_study.py
```

One arm by hand:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods inten_loam --no-gt-seed --inten-loam-paper-odometry --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 --inten-loam-paper-set s2m_plane_max_dist=0.05 --inten-loam-paper-set s2s_gate_sq=25 --inten-loam-paper-set s2s_ground_no_forward=1 --summary-json out.json
```

- The [`records/`](records/) directory holds every command with exit codes,
  input hashes and timings for part A (02, 00, 08) and part B (01, 09, 05, 10,
  03, 04, 06, 07).
- Every arm and its step diagnostics exit 0, and every sequence's input hashes
  match before and after its runs.
- The scans were deleted after each sequence.
- FPS is not comparable to the historical values.
