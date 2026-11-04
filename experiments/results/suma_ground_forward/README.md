# SuMa: ground pixels without forward information on full KITTI 00-10

With its official defaults on raw scans (`paper_odometry`), SuMaPaper is close to
the paper on nine KITTI sequences but fails on two:

- 01: 13.793 % against 1.7 % (8.11x);
- 02: 3.512 % against 1.1 % (3.19x).

The [method README](../../../papers/suma/README.md#seq-02-forward-drift-in-corridors-and-the-recommended-kitti-setting)
diagnoses 02. In corridors the forward direction is nearly unconstrained, and
73 % of the forward gradient at the GT increment comes from ground pixels whose
normals tilt about 1.3 deg towards the sensor. Table 6 therefore selects a
second variant on 01/02 (normalised lookup plus a 0.22 deg correction), which
costs on the other sequences. With one setting for every sequence, SuMa stands
at 1.25x. InTEn-LOAM's 02 collapse had the same cause
([study](../inten_ground_forward/README.md)).

This study tests one opt-in change, with one fixed setting on every sequence
with a paper value. The [protocol](protocol.md) was committed (`44604b9b`)
before any full run of the candidate on 00 or 03-10. 01 and 02 were the
development sequences.

## What `ground_no_forward_vertical_cos=0.866` does

It is **not in the paper**, opt-in and off by default.

- Pixels whose model normal lies within 30 deg of the sensor's vertical axis
  give no information along the sensor's forward axis.
- The forward component of their translation Jacobian is removed in
  sensor-centred coordinates (`SuMaPaper::accumulate`), the same construction
  as InTEn-LOAM's `s2s_ground_no_forward`.
- They keep height, roll, pitch, lateral and yaw information. All other pixels
  are unchanged, as are the official lookup, the gates and the map update.
- Scans stay raw, with no vertical-angle correction.
- The 30 deg bound was fixed before any run, and no other value was tried.

Unit tests (`papers/suma/test/test_suma_paper.cpp`):

- at the identity, only the forward row and column change, by the ground
  pixels' share;
- the known-motion alignment test still passes with the option on.

All 11 SuMaPaper tests pass.

## Full KITTI 00-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed.

- Both arms ran concurrently on the same binary (SHA-256 `d6351e4e…`), which is
  also the development binary.
- "Table 6" is the current table: for each sequence it takes the better of
  `paper_odometry` and `paper_odometry_clean_lookup_elevation`.
- The paper reports one decimal.

| Seq | Paper | Table 6 (best of two) | `paper_odometry` | + ground without forward | Candidate / reference | Bad-length steps (ref → cand) | Scale-only (ref → cand) | ATE ref / cand [m] |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.7 | 0.697 (1.00x) | 0.697 (1.00x) | 0.689 (0.98x) | 0.988 | 13 → 13 | 0.214 → 0.220 | 8.20 / 7.80 |
| 01 (dev) | 1.7 | 3.550 (2.09x) | 13.793 (8.11x) | **1.629 (0.96x)** | 0.118 | 134 → 7 | 13.579 → 0.553 | 175.83 / 75.75 |
| 02 (dev) | 1.1 | 0.784 (0.71x) | 3.512 (3.19x) | **1.558 (1.42x)** | 0.444 | 168 → 82 | 3.314 → 1.039 | 70.31 / 38.79 |
| 03 | 0.7 | 0.651 (0.93x) | 0.651 (0.93x) | 0.630 (0.90x) | 0.967 | 1 → 1 | 0.321 → 0.292 | 1.45 / 1.46 |
| 04 | 0.4 | 0.417 (1.04x) | 0.417 (1.04x) | 0.307 (0.77x) | 0.736 | 0 → 0 | 0.294 → 0.128 | 0.97 / 0.95 |
| 05 | 0.5 | 0.379 (0.76x) | 0.379 (0.76x) | 0.392 (0.78x) | 1.033 | 0 → 0 | 0.086 → 0.100 | 3.42 / 3.50 |
| 06 | 0.4 | 0.286 (0.71x) | 0.286 (0.71x) | 0.275 (0.69x) | 0.963 | 0 → 0 | 0.104 → 0.083 | 2.96 / 3.33 |
| 07 | 0.4 | 0.331 (0.83x) | 0.331 (0.83x) | 0.339 (0.85x) | 1.023 | 0 → 0 | 0.134 → 0.150 | 1.06 / 1.11 |
| 08 | 1.0 | 1.060 (1.06x) | 1.060 (1.06x) | 1.052 (1.05x) | 0.992 | 23 → 20 | 0.223 → 0.225 | 24.05 / 23.47 |
| 09 | 0.5 | 0.510 (1.02x) | 0.510 (1.02x) | 0.546 (1.09x) | 1.071 | 0 → 0 | 0.187 → 0.217 | 6.47 / 6.42 |
| 10 | 0.7 | 0.698 (1.00x) | 0.698 (1.00x) | 0.745 (1.06x) | 1.068 | 0 → 1 | 0.276 → 0.308 | 5.25 / 6.60 |

| Arm | Geometric mean vs paper | Sequences above 2x |
|---|---:|---|
| Table 6, best of two variants per sequence | 0.967x | 01 |
| `paper_odometry`, one setting | 1.254x | 01, 02 |
| + ground without forward, one setting | **0.941x** | none |

**Reference check.** The fresh `paper_odometry` equals the stored result on all
11 sequences (official RTE, 100 m RPE, ATE, frame count).

**01 and 02 (development).**
- 01 falls from 13.793 % to 1.629 %, below the paper's 1.7 %. Bad-length steps
  fall from 134 to 7, and the path length from 1.136 to 1.005 of GT.
- 02 falls from 3.512 % to 1.558 %, with bad-length steps 168 → 82. That is
  worse than Table 6's clean-lookup variant on 02 (0.784 %).
- Both are dominated by the scale channel, which falls from 13.579 % to 0.553 %
  (01) and from 3.314 % to 1.039 % (02).

**Held-out sequences (00, 03-10).**
- The candidate/reference geometric mean is 0.977x.
- 5 sequences are lower: 00, 03, 04, 06 and 08. On 04, RTE falls from 0.417 %
  to 0.307 % and the scale channel halves.
- 4 are higher: 05 (+3.3 %), 07 (+2.3 %), 09 (+7.1 %) and 10 (+6.8 %).
- **Adverse under the protocol (more than +5 %): 09 (0.510 → 0.546 %) and 10
  (0.698 → 0.745 %).** On 10 one bad-length step appears and ATE rises from
  5.25 m to 6.60 m.
- ATE is also higher on 03, 05, 06 and 07, and lower on 00, 04, 08 and 09.

**One setting for SuMa.** With the option on every sequence, SuMa's geometric
mean against the paper is 0.941x, and no sequence is above 2x. The current
Table 6 picks between two variants per sequence and reaches 0.967x with 01 at
2.09x. The one setting is therefore lower than best-of-two on 01, 00, 03, 04, 06
and 08, and higher on 02, 05, 07, 09 and 10.

## What this does and does not show

- The 01 and 02 failures of the official setting come mainly from the ground
  pixels' forward term. Removing it fixes 01 and more than halves 02. On the
  held-out sequences RTE changes by -26 % to +7 %.
- SuMa can now run with **one setting** that is below the paper on average.
  That setting is outside the paper. It worsens 09 and 10 by about 7 %, and on
  02 it is worse than the clean-lookup variant.
- Defaults do not change, and Table 6 keeps its best-of-two selection. The
  candidate is an ablation variant, so Table 6 cannot select it. Pinning SuMa
  to this one setting is the owner's decision.
- Every KITTI sequence is project development data, and 01/02 motivated the
  change.

## Commands and execution record

```bash
python3 experiments/results/suma_ground_forward/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [...]
python3 experiments/results/suma_ground_forward/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/suma_ground_forward/summarize_study.py
```

One arm by hand:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods suma --no-gt-seed --suma-paper --suma-paper-set ground_no_forward_vertical_cos=0.866 --summary-json out.json
```

The [`records/`](records/) directory holds every command with exit codes, input
hashes and timings.

- Two runners ran in parallel: part A (02, 00, 08) and part B (01, 09, 05, 10,
  03, 04, 06, 07).
- Every arm and its step diagnostics exit 0, and every sequence's input hashes
  match before and after its runs. The scans were deleted after each sequence.
- Both arms of a sequence ran concurrently, so FPS is not comparable to the
  historical values.
