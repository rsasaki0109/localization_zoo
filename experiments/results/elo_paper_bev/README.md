# ELO with the paper's 0.1 m single-vertex ground BEV on full KITTI 00-10

ELO's Table 6 setting, `dense_elevation_paper_normals_unbiased`, reaches 1.22x
the paper over 00-10, with the gap spread over the sequences.

One remaining difference from the paper is the ground bird's-eye-view model:

- the dense profile keeps a running mean per 0.2 m cell;
- the paper keeps one vertex, the closest, per 0.1 m cell.

Before the three forward-bias fixes, the paper's BEV hurt on 07. With the fixes,
it no longer did on the 07 windows (0.383 → 0.373 %, 0.345 → 0.332 %;
[ELO README](../../../papers/elo/README.md#forward-drift-biases-of-the-projective-model)),
but it had not been run on full sequences.

This study runs it on every sequence with a paper value. The
[protocol](protocol.md) was committed (`9fb66ecd`) before any full run.
Development sequence: 07. Held out: 00-06 and 08-10.

## Candidate

Table 6's arguments plus `--elo-bev-resolution 0.1 --elo-bev-keep-closest`.

- This is the paper's BEV, not a new mechanism.
- No code changes: both flags already exist.
- It is marked as an ablation variant, so Table 6 cannot select it on its own.

## Full KITTI 00-10, one fixed setting

Official KITTI translational RTE [%] (100-800 m), no GT seed. Both arms ran
concurrently, with one OpenMP thread each, on the same binary (SHA-256
`d6351e4e…`).

| Seq | Paper | Reference | + paper BEV | Candidate / reference | Bad-length steps (ref → cand) | Yaw-only (ref → cand) | ATE ref / cand [m] |
|---|---:|---:|---:|---:|---:|---:|---:|
| 00 | 0.54 | 0.586 (1.09x) | 0.608 (1.13x) | 1.037 | 12 → 12 | 0.539 → 0.503 | 14.24 / 14.58 |
| 01 | 0.61 | 0.869 (1.42x) | 0.754 (1.24x) | 0.868 | 0 → 0 | 0.229 → 0.270 | 18.00 / 8.60 |
| 02 | 0.54 | 0.770 (1.43x) | 0.727 (1.35x) | 0.944 | 3 → 2 | 0.632 → 0.574 | 29.83 / 28.74 |
| 03 | 0.65 | 0.890 (1.37x) | 0.826 (1.27x) | 0.928 | 1 → 1 | 1.106 → 0.993 | 5.24 / 5.03 |
| 04 | 0.32 | 0.268 (0.84x) | 0.305 (0.95x) | 1.134 | 0 → 0 | 0.096 → 0.052 | 0.56 / 0.60 |
| 05 | 0.33 | 0.399 (1.21x) | 0.411 (1.25x) | 1.031 | 0 → 0 | 0.370 → 0.423 | 3.80 / 5.81 |
| 06 | 0.30 | 0.411 (1.37x) | 0.382 (1.27x) | 0.928 | 0 → 0 | 0.380 → 0.388 | 3.92 / 4.08 |
| 07 (dev) | 0.31 | 0.391 (1.26x) | 0.373 (1.20x) | 0.954 | 0 → 0 | 0.305 → 0.289 | 1.52 / 1.58 |
| 08 | 0.79 | 1.079 (1.37x) | 0.922 (1.17x) | 0.855 | 21 → 20 | 0.670 → 0.570 | 23.81 / 23.42 |
| 09 | 0.48 | 0.678 (1.41x) | 0.630 (1.31x) | 0.929 | 0 → 0 | 0.276 → 0.192 | 3.11 / 3.11 |
| 10 | 0.59 | 0.504 (0.85x) | 0.552 (0.94x) | 1.095 | 0 → 1 | 0.257 → 0.343 | 1.53 / 3.64 |

| Arm | Geometric mean vs paper | Sequences above 2x |
|---|---:|---|
| `dense_elevation_paper_normals_unbiased` (Table 6) | 1.218x | none |
| + paper BEV | **1.180x** | none |

**Reference check.** The fresh reference equals the stored Table 6 result on all
11 sequences (official RTE, 100 m RPE, ATE, frame count).

**Held-out sequences (00-06, 08-10).**

The candidate/reference geometric mean is 0.971x. The gain is on the sequences
where the dense profile is furthest from the paper (1.37-1.43x).

Lower on six sequences:

| Seq | Reference RTE | + paper BEV | Change | Note |
|---|---:|---:|---:|---|
| 08 | 1.079 % | 0.922 % | -14.5 % | |
| 01 | 0.869 % | 0.754 % | -13.2 % | ATE halves, 18.00 → 8.60 m |
| 03 | 0.890 % | 0.826 % | -7.2 % | |
| 06 | 0.411 % | 0.382 % | -7.2 % | |
| 09 | 0.678 % | 0.630 % | -7.1 % | |
| 02 | 0.770 % | 0.727 % | -5.6 % | |

Higher on four sequences: 00 (+3.7 %), 05 (+3.1 %; ATE 3.80 → 5.81 m), 04
(+13.4 %) and 10 (+9.5 %).

**Adverse under the protocol (more than +5 %): 04 (0.268 → 0.305 %) and 10
(0.504 → 0.552 %; ATE 1.53 → 3.64 m, yaw-only 0.257 → 0.343 %).** Both are
sequences where the reference was already below the paper (0.84x / 0.85x);
with the paper's BEV they are 0.95x / 0.94x.

**07 (development).** 0.391 → 0.373 %, consistent with the window checks.

## What this does and does not show

- The paper's BEV moves ELO closer to the paper in mechanism and in numbers:
  1.22x → 1.18x.
- It is not uniformly better: it is lower on 6 of 10 held-out sequences and
  adverse on 04 and 10.
- No sequence is above twice the paper in either arm.
- Defaults and Table 6 (unpinned for ELO) do not change. The candidate is an
  ablation variant, so Table 6 does not pick it. Adopting the paper's BEV for
  ELO's Table 6 row is the owner's decision.
- Every KITTI sequence is project development data.

## Commands and execution record

```bash
python3 experiments/results/elo_paper_bev/run_study.py --kitti-root <kitti> --work-root <scratch>/work --stage-root <scratch>/stage --record <record.json> --sequence NN [...]
python3 experiments/results/elo_paper_bev/import_study.py --stage-root <scratch>/stage --record <import.json>
python3 experiments/results/elo_paper_bev/summarize_study.py
```

One arm by hand:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods elo --no-gt-seed --elo-dense-profile --input-vertical-angle-correction-deg 0.195 --elo-normal-full-patch --elo-sri-fuse-same-surface --elo-ground-no-forward --elo-curvature-threshold 0.03 --elo-bev-resolution 0.1 --elo-bev-keep-closest --summary-json out.json
```

- The [`records/`](records/) directory holds every command with exit codes,
  input hashes and timings. Part A ran 02, 00 and 08; part B ran 04, 01, 09,
  05, 10, 03, 06 and 07.
- Every arm and its step diagnostics exit 0. Every sequence's input hashes
  match before and after its runs, and the scans were deleted after each
  sequence.
- FPS is not comparable to the historical values.
