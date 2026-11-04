# InTEn-LOAM: S2S ground without forward information on full KITTI 00-10

InTEn-LOAM's KITTI 02 gap (3.221 % vs 0.54 %, 5.97x) lies in two runs of steps
that are too short: frames 1878-1943 and 4460-4524. Both are already short at
the S2S output ([natural traces](../inten_natural_traces/README.md)). At their
entry frames the GT-seeded S2S system has no edge support, and its ground class
gives a backward first-order correction of 3.7-4.3 % of the GT step
([seed interventions](../inten_seed_intervention/README.md)).

This study tests one opt-in change aimed at that term, with one fixed setting
on every sequence with a paper value. The [protocol](protocol.md) was committed
(`340c9111`) before any full run of the candidate on 00 or 03-10. 01 and 02
were the development sequences.

## What `s2s_ground_no_forward=1` does

It is **not in the paper**, opt-in and off by default.

- In S2S only, the translation Jacobian of each ground residual loses its
  component along the sensor's forward axis.
- The change is made in sensor-centred tangent coordinates, so the rotation
  columns do not reintroduce it (`System::add` in
  `papers/inten_loam/src/inten_loam_paper.cpp`).
- Ground still constrains height, roll, pitch, lateral motion and yaw.
- Edges, facades, reflectors, S2M, the gates and the iteration budgets are
  unchanged.

The reasoning: flat ground carries little true forward information, but its
many residuals can add a small biased forward term. Where nothing else
constrains the forward direction, that term sets the step length. ELO has the
same opt-in (`ground_constrains_forward`), and LOAM-style two-step solvers
leave forward motion to non-ground features.

A companion flag `s2m_ground_no_forward` does the same in S2M. It was only
used in the development check below.

Unit tests (`papers/inten_loam/test/test_inten_loam_paper.cpp`):

- at a fixed initial pose only the ground class's forward row and column change;
- with both flags on, the street fixture is still tracked 500 m from the origin
  at a 30 deg heading.

All 61 InTEn-LOAM paper tests pass.

## Development: 01 and 02

Full runs on the study binary, official KITTI RTE ([records](development/)):

| Seq | Reference | S2S only | S2S + S2M |
|---|---:|---:|---:|
| 01 | 16.614 % | 16.636 % | 16.708 % |
| 02 | 3.221 % | 0.823 % | 0.855 % |

S2S only was fixed for the study: it is lower on both sequences and is the
smaller change. No other value was tried.

## Full KITTI 00-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed.

- The reference arm is the unchanged Table 6 setting `paper_odometry_plane_check`.
- Both arms ran concurrently on the same binary (SHA-256 `463043f3…`).
- Bad-length steps: moving steps whose length is off by more than 20 %
  (`diagnose_kitti_steps.py`).
- Scale-only and yaw-only keep only that part of each arm's error (see
  `../gap_decomposition/`).

| Seq | Paper | Reference | + S2S ground without forward | Candidate / reference | Bad-length steps (ref → cand) | Scale-only (ref → cand) | Yaw-only (ref → cand) | ATE ref / cand [m] |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.52 | 0.697 (1.34x) | 0.701 (1.35x) | 1.006 | 13 → 13 | 0.239 → 0.240 | 0.599 → 0.622 | 17.6 / 17.6 |
| 01 (dev) | 0.64 | 16.614 (25.96x) | 16.636 (25.99x) | 1.001 | 136 → 136 | 14.181 → 14.168 | 1.120 → 1.096 | 275.6 / 275.8 |
| 02 (dev) | 0.54 | 3.221 (5.97x) | 0.823 (1.52x) | 0.255 | 133 → 20 | 2.945 → 0.291 | 2.435 → 0.630 | 122.8 / 40.2 |
| 03 | 0.63 | 0.891 (1.41x) | 0.894 (1.42x) | 1.004 | 2 → 0 | 0.185 → 0.187 | 1.206 → 1.208 | 6.3 / 6.3 |
| 04 | 0.37 | 0.676 (1.83x) | 0.683 (1.85x) | 1.010 | 0 → 0 | 0.543 → 0.550 | 0.065 → 0.063 | 1.5 / 1.6 |
| 05 | 0.36 | 0.497 (1.38x) | 0.504 (1.40x) | 1.013 | 0 → 0 | 0.133 → 0.139 | 0.588 → 0.604 | 10.0 / 10.1 |
| 06 | 0.24 | 0.359 (1.49x) | 0.352 (1.47x) | 0.982 | 0 → 0 | 0.100 → 0.100 | 0.201 → 0.192 | 1.9 / 1.9 |
| 07 | 0.34 | 0.364 (1.07x) | 0.347 (1.02x) | 0.953 | 0 → 0 | 0.157 → 0.153 | 0.268 → 0.260 | 1.5 / 1.4 |
| 08 | 0.71 | 0.938 (1.32x) | 0.953 (1.34x) | 1.016 | 20 → 19 | 0.252 → 0.249 | 0.648 → 0.692 | 27.7 / 29.4 |
| 09 | 0.48 | 0.661 (1.38x) | 0.662 (1.38x) | 1.001 | 0 → 0 | 0.259 → 0.262 | 0.514 → 0.531 | 11.0 / 10.7 |
| 10 | 0.45 | 1.156 (2.57x) | 1.120 (2.49x) | 0.969 | 0 → 0 | 0.358 → 0.354 | 0.909 → 0.860 | 9.7 / 8.7 |

| Arm | Geometric mean vs paper | Sequences above 2x |
|---|---:|---|
| `paper_odometry_plane_check` (Table 6) | 2.189x | 01, 02, 10 |
| + `s2s_ground_no_forward=1` | **1.926x** | 01, 10 |

**Reference check.** The fresh reference equals the stored Table 6 result on all
11 sequences (official RTE, 100 m RPE, ATE, frame count).

**02 (development).** RTE falls from 3.221 % to 0.823 %, from 5.97x to 1.52x the
paper. Bad-length steps fall from 133 to 20. The two sustained runs, 1878-1943
(66 steps) and 4460-4524 (65 steps), break up into isolated steps:

- 1878-1881, 1924;
- 4464-4468, 4475-4476, 4493-4494, 4503, 4510.

The scale-only channel falls from 2.945 % to 0.291 %. The yaw-only channel also
falls, from 2.435 % to 0.630 %: the collapse runs carried heading error as
well.

**01 (development).** No change (16.614 → 16.636 %, 136 bad-length steps in both
arms). This matches the earlier evidence: at 01 frame 420 the backward
correction comes from edges and facades, not from the ground.

**Held-out sequences (00, 03-10).**

- The candidate/reference geometric mean is 0.995x.
- 3 sequences are lower: 06, 07 and 10 (10: 1.156 → 1.120 %).
- 6 are higher, by 0.06-1.6 %: 00, 03, 04, 05, 08 and 09.
- No sequence rises by more than 5 %, so none is adverse under the protocol.
- Bad-length steps do not increase anywhere.
- ATE is higher on 03, 04, 05, 06 and 08 (largest: 08, 27.67 → 29.39 m) and
  lower on 00, 07, 09 and 10 (10: 9.67 → 8.75 m).

**Over all 11 sequences.** The geometric mean against the paper falls from
2.189x to 1.926x. Almost all of that comes from 02, which chose the setting.

## What this does and does not show

- The 02 collapse comes mainly from S2S's ground forward term: removing that
  term removes most of it. On the held-out sequences RTE changes by -4.7 % to
  +1.6 %.
- It is not a general accuracy gain: on the held-out sequences the change is
  neutral.
- 01 (25.96x) and 10 (2.57x) remain above twice the paper. They need different
  explanations: edge/facade pull on 01, heading bias on 10
  ([heading arms](../inten_heading_classes/README.md)).
- Defaults do not change. Table 6 and `repo_variants` stay on
  `paper_odometry_plane_check` (2.19x). The flag is outside the paper, so
  promoting it is the owner's decision.
- Every KITTI sequence is project development data, and 01/02 chose the
  setting.

## Commands and execution record

```bash
python3 experiments/results/inten_ground_forward/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [...]
python3 experiments/results/inten_ground_forward/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/inten_ground_forward/summarize_study.py
```

One arm by hand:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods inten_loam --no-gt-seed --inten-loam-paper-odometry --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 --inten-loam-paper-set s2m_plane_max_dist=0.05 --inten-loam-paper-set s2s_ground_no_forward=1 --summary-json out.json
```

The [`records/`](records/) directory holds every command with exit codes, input
hashes and timings.

- Two runners ran in parallel: part A (02, 00, 08) and part B (01, 09, 05, 10,
  03, 04, 06, 07).
- Part A's 08 fetch failed with a connection reset (`part_a_08_fetch_failure.log`,
  3660 of 4071 scans on disk). Nothing for 08 had run.
- 08 was then run alone (`run_record_part_c_08_retry.json`). It reused the
  complete scans and re-fetched the remaining 411 with CRC checks.
- All parts use the same binary SHA-256 as the development runs. Every arm and
  its step diagnostics exit 0.
- Every sequence's input hashes match before and after its runs, and the scans
  were deleted after each sequence.
- Both arms of a sequence ran concurrently, so FPS is not comparable to the
  historical values.
