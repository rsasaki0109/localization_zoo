# Recursive geometry consensus at the original estimated rotation

This outside-paper, default-off extension runs the same setting on every full
KITTI 00–10 sequence. It tests the mechanism that passed the preceding conditional
feasibility gate, now with its own changed mapping and prediction history. It is
not an independent held-out evaluation: original profile on 07, diagnosis on 00, inherited
DiLO 01 prior 0.03 and every previous KITTI observation are development.

`--inten-loam-paper-set s2s_geometry_consensus=1` keeps the original native joint
S2S estimate, computes the unchanged full-point translation-consensus profiles at
the original estimated input, corrects seed translation and facade support when
strict-majority/interior consensus is available, then actually re-solves translation
while retaining the original estimated rotation. No GT, hand region, threshold
sweep, per-sequence winner or parameter retuning enters the estimator.

All points, profile grid/trimming/admission-band/vote rules are inherited unchanged
from the committed conditional study. Every cluster can admit multiple shifts.
Cluster votes do not establish static-object truth. Invalid original solves,
unusable consensus or unsuccessful refinement retain the original result. The
native return rule requires at least 30 returned pairs and a finite pose; an
iteration-limit stop can still be accepted. Applied does not mean converged or
accurate. Native S2M, DOR/insertion and mapped prediction definitions stay fixed, but their realized
inputs and maps change. Fixing local S2S rotation does not fix global mapped
rotation or guarantee that the original rotation is correct.

Both arms keep mapped seed, prior 0.03, ground/facade PCA and acceleration on;
facade Cauchy, predicted DOR, row completion and plane spread off; range 120 m,
correction 0.17 deg, S2M plane check 0.05 m, no GT seed. The new Boolean is the only accuracy-setting
difference. Baseline OMP 1/candidate OMP 4 are fixed across sequences, with bitwise
cached-cost/estimated-solver agreement at both worker counts. Raw times remain
available, but unequal fixed worker counts on the shared host do not constitute
a standalone throughput benchmark.

Numerical evidence `b64bbf82291aee39308d757db5a1f4b40403e4ed` precedes this interpretation.

Geometric mean candidate/reference official RTE: **1.106697x**.
Lower: 04, 06, 08, 09. Higher: 00, 01, 02, 03, 05, 07, 10. Equal: none.
Every sequence and every adverse metric is retained. The option stays default off; no per-sequence default is adopted.

| Sequence | Official RTE ref → candidate [%] | Rotation ref → candidate [deg/100m] | ATE ref → candidate [m] | RPE ref → candidate [%] | Bad length ref → candidate | Bad vector ref → candidate |
|---|---:|---:|---:|---:|---:|---:|
| 00 | 0.683293 → 0.690181 | 0.317055 → 0.316955 | 17.051301 → 17.306201 | 0.794246 → 0.799513 | 13 → 13 | 42 → 40 |
| 01 | 4.481189 → 8.021836 | 0.289430 → 0.258104 | 88.786456 → 110.696528 | 3.760742 → 7.681170 | 44 → 69 | 44 → 69 |
| 02 | 0.756938 → 1.143779 | 0.262615 → 0.347104 | 40.602835 → 46.557283 | 0.813418 → 1.302706 | 2 → 30 | 8 → 34 |
| 03 | 0.884094 → 0.905656 | 0.632477 → 0.649018 | 6.251484 → 6.415721 | 0.586278 → 0.585667 | 1 → 0 | 3 → 3 |
| 04 | 0.692176 → 0.676220 | 0.295843 → 0.329046 | 1.563220 → 1.541896 | 0.715182 → 0.700470 | 0 → 0 | 0 → 0 |
| 05 | 0.477296 → 0.503103 | 0.302184 → 0.309691 | 9.639439 → 10.076314 | 0.424881 → 0.413982 | 0 → 0 | 0 → 0 |
| 06 | 0.350792 → 0.347652 | 0.143796 → 0.137419 | 1.952842 → 1.881950 | 0.506118 → 0.505452 | 0 → 0 | 0 → 0 |
| 07 | 0.348742 → 0.368622 | 0.254061 → 0.273701 | 1.391385 → 1.434512 | 0.432137 → 0.430667 | 0 → 0 | 0 → 0 |
| 08 | 0.955307 → 0.950314 | 0.342667 → 0.334579 | 29.422940 → 28.896896 | 1.301395 → 1.305430 | 19 → 19 | 99 → 98 |
| 09 | 0.672321 → 0.636656 | 0.261265 → 0.243284 | 11.508132 → 10.404056 | 0.690141 → 0.696794 | 0 → 0 | 0 → 0 |
| 10 | 1.093310 → 1.172497 | 0.352714 → 0.388468 | 8.851550 → 9.812784 | 0.825808 → 0.834861 | 0 → 0 | 0 → 0 |

Local bad labels use the inherited 20% threshold when GT translation is at least 0.1 m. They are distinct from official distance-segment RTE.

## Every adverse comparison

- 00: RTE [%] 0.683293 → 0.690181; ATE [m] 17.051301 → 17.306201; RPE [%] 0.794246 → 0.799513.
- 01: RTE [%] 4.481189 → 8.021836; ATE [m] 88.786456 → 110.696528; RPE [%] 3.760742 → 7.681170; bad length 44 → 69; bad vector 44 → 69.
- 02: RTE [%] 0.756938 → 1.143779; rotation [deg/100m] 0.262615 → 0.347104; ATE [m] 40.602835 → 46.557283; RPE [%] 0.813418 → 1.302706; bad length 2 → 30; bad vector 8 → 34.
- 03: RTE [%] 0.884094 → 0.905656; rotation [deg/100m] 0.632477 → 0.649018; ATE [m] 6.251484 → 6.415721.
- 04: rotation [deg/100m] 0.295843 → 0.329046.
- 05: RTE [%] 0.477296 → 0.503103; rotation [deg/100m] 0.302184 → 0.309691; ATE [m] 9.639439 → 10.076314.
- 07: RTE [%] 0.348742 → 0.368622; rotation [deg/100m] 0.254061 → 0.273701; ATE [m] 1.391385 → 1.434512.
- 08: RPE [%] 1.301395 → 1.305430.
- 09: RPE [%] 0.690141 → 0.696794.
- 10: RTE [%] 1.093310 → 1.172497; rotation [deg/100m] 0.352714 → 0.388468; ATE [m] 8.851550 → 9.812784; RPE [%] 0.825808 → 0.834861.

| Sequence | New bad length frames | Reference-only bad length | New bad vector frames | Reference-only bad vector |
|---|---:|---:|---:|---:|
| 00 | 0 | 0 | 0 | 2 |
| 01 | 69 | 44 | 69 | 44 |
| 02 | 29 | 1 | 28 | 2 |
| 03 | 0 | 1 | 1 | 1 |
| 04 | 0 | 0 | 0 | 0 |
| 05 | 0 | 0 | 0 | 0 |
| 06 | 0 | 0 | 0 | 0 |
| 07 | 0 | 0 | 0 | 0 |
| 08 | 0 | 0 | 0 | 1 |
| 09 | 0 | 0 | 0 | 0 |
| 10 | 0 | 0 | 0 | 0 |

Every affected frame index is retained in the numerical report. These are realized-trajectory comparisons, not matched-history causal interventions.

## Consensus use and remaining motion deficits

| Sequence | Evaluated | Usable | Applied | Original-result fallbacks | Excluded cluster decisions |
|---|---:|---:|---:|---:|---:|
| 00 | 4540 | 4539 | 4539 | 1 | 82799 |
| 01 | 1100 | 1094 | 1094 | 6 | 36942 |
| 02 | 4660 | 4655 | 4655 | 5 | 64672 |
| 03 | 800 | 799 | 799 | 1 | 13085 |
| 04 | 270 | 269 | 269 | 1 | 5496 |
| 05 | 2760 | 2759 | 2759 | 1 | 47112 |
| 06 | 1100 | 1099 | 1099 | 1 | 17455 |
| 07 | 1100 | 1099 | 1099 | 1 | 16216 |
| 08 | 4070 | 4069 | 4069 | 1 | 80100 |
| 09 | 1590 | 1582 | 1582 | 8 | 40049 |
| 10 | 1200 | 1199 | 1199 | 1 | 24491 |

Counts condition on this realized trajectory. The original joint trace remains in `s2s`; actual conditional refinement is in `s2s_consensus.refinement`. S2S error statistics use the accepted relative motion. First ground/facade counts and prior-information statistics describe the original joint solve; the refinement has separate complete equations and checks.

## Error before and after each applied refinement

| Sequence | Moving applied frames | Vector error lower / higher / equal | Absolute length error lower / higher / equal |
|---|---:|---|---|
| 00 | 4513 | 2531 / 1982 / 0 | 2350 / 2163 / 0 |
| 01 | 1094 | 568 / 526 / 0 | 562 / 532 / 0 |
| 02 | 4655 | 2491 / 2164 / 0 | 2425 / 2230 / 0 |
| 03 | 784 | 430 / 354 / 0 | 407 / 377 / 0 |
| 04 | 269 | 142 / 127 / 0 | 136 / 133 / 0 |
| 05 | 2682 | 1609 / 1073 / 0 | 1421 / 1261 / 0 |
| 06 | 1099 | 595 / 504 / 0 | 540 / 559 / 0 |
| 07 | 1009 | 615 / 394 / 0 | 589 / 420 / 0 |
| 08 | 4039 | 2302 / 1737 / 0 | 2097 / 1942 / 0 |
| 09 | 1582 | 902 / 680 / 0 | 799 / 783 / 0 |
| 10 | 1152 | 688 / 464 / 0 | 661 / 491 / 0 |

These compare the valid original joint estimate with the actually accepted refinement at the same frame and realized history, restricted to applied refinements and GT steps of at least 0.1 m. Rotation is held at the original estimate. GT is used only for this diagnostic. The comparison measures the immediate local change; it does not reconstruct a counterfactual full baseline or attribute later map drift to one cause. Exact joint/accepted errors and every frame remain in the step CSVs.

- 01: final bad-length intervals 204–247 → 426–457, 811–834, 838–850; accepted S2S bad-length steps 48 → 69; original S2S fallbacks 0 → 0; rejected S2M 0 → 0.
- 02: final bad-length intervals 1787, 3308 → 1890, 1892–1896, 1899–1900, 3308–3309, 4473–4492; accepted S2S bad-length steps 2 → 30; original S2S fallbacks 0 → 0; rejected S2M 0 → 0.
- 10: final bad-length intervals none → none; accepted S2S bad-length steps 0 → 0; original S2S fallbacks 0 → 0; rejected S2M 0 → 0.

## Error growth above twice paper

| Sequence | Candidate RTE [%] | Paper RTE [%] | Ratio | Growth block | Position error start → end [m] | Block translation error [%] |
|---|---:|---:|---:|---|---:|---:|
| 01 | 8.021836 | 0.640000 | 12.534119 | 400–500 | 6.776122 → 98.089424 | 35.950948 |
| 01 | 8.021836 | 0.640000 | 12.534119 | 800–900 | 103.560733 → 185.965371 | 42.406730 |
| 01 | 8.021836 | 0.640000 | 12.534119 | 900–1000 | 185.965371 → 193.034094 | 0.314913 |
| 02 | 1.143779 | 0.540000 | 2.118109 | 4400–4500 | 48.546976 → 84.199441 | 18.306659 |
| 02 | 1.143779 | 0.540000 | 2.118109 | 4600–4660 | 100.727345 → 117.904186 | 1.027465 |
| 02 | 1.143779 | 0.540000 | 2.118109 | 4500–4600 | 84.199441 → 100.727345 | 0.623028 |
| 10 | 1.172497 | 0.450000 | 2.605549 | 700–800 | 9.309239 → 14.859270 | 0.981912 |
| 10 | 1.172497 | 0.450000 | 2.605549 | 600–700 | 6.770395 → 9.309239 | 2.044933 |
| 10 | 1.172497 | 0.450000 | 2.605549 | 800–900 | 14.859270 → 17.360337 | 0.503191 |

These blocks locate error growth, including gradual drift without a local label crossing. They do not establish a solver-level physical cause. Block errors are not official distance-segment RTE; paper values contextualize an outside-paper extension. All steps, boundary observations and error blocks remain in the complete report.

![All official RTE outcomes](report/official_rte.png)

![KITTI 01 error growth](report/seq_01_error_growth.png)

![KITTI 02 error growth](report/seq_02_error_growth.png)

![KITTI 10 error growth](report/seq_10_error_growth.png)

## Reproduction and preservation

Sources/rules/manifests and binary/library hashes were frozen before full runs.
Every unchanged full baseline reproduces the historical accepted reference's
metrics, pose/GT bytes and full numerical trace after the declared scan-root-only
substitution. Cached controls reproduce every original full-point profile and
estimated solver trace at both worker counts, including the prior adverse
endpoints and declared majority/boundary fallback. No GT is read by the estimator.

The query implementation preserves the original float coordinates, exact k=1
nearest-neighbor definition, squared distances, point order, sort order and serial
sum order. It reuses the native exact FeatureTree and balances independent
(cluster, shift) cells; each cell still reduces in the original serial order.
The earlier implementation and both distinct accelerated builds/binaries remain
preserved. Both cached acceleration stages reproduce the original costs, native
poses and complete solver traces at the fixed worker counts; their receipts are
in `checks/cached_parity/` and `checks/cached_parity_balanced/`.

The original full 01 negative result is retained under
`../inten_recursive_consensus/sequences/01/`. Its complete numerical traces,
pose/GT bytes and accuracy metrics must match both new full 01 arms exactly.
The original slow 07 candidate was deliberately interrupted after the exact cached
controls passed. Its actual signal exit, wrapper failure, all completed baseline
bytes and partial candidate bytes were archived, roundtripped, committed and
pushed under `../inten_recursive_consensus/collection_interruption/`; it is not
reported as a complete run. The new candidate starts at frame zero and must match
every complete old candidate prefix record. The owned official 07 scans are reused
without another acquisition, then removed only after the new complete pair is
archived and pushed. Complete-sequence equivalence is recorded by the actual
`implementation_equivalence.py` command and receipt for every sequence.

Every full frame retains the actual admission bands, votes, excluded clusters,
seed/original estimate and refinement equations. Independent validation reconstructs
all votes/masks/fallbacks, verifies bitwise original rotation at every projected
step, additive translation, damped Htt backward error and accepted-pose flow.
Full original joint/S2M equations, adaptive prior and mapped prediction are also
checked. The completed CLI trace-on/off control preserves metrics and pose bytes.

The initial synthetic test confused the minimum-cost shift with the nearest-zero
admissible consensus; its failed actual CTest and build are retained. Only its
expectation changed; the helper/rules were preserved. The corrected native build
uses a distinct directory and every native check passes. Original log bytes are
losslessly archived, including CMake whitespace. A clean-tree preflight rejected
an unstaged artifact set before any replay; that actual rejected start is retained.
The first accelerated link driver used a stale library path after successful
configure/build/CTest; its failure and a link-only continuation are retained.
The first cached-control setup failed before native execution because of a stale
module name; the failure and corrected distinct staging paths remain preserved.
The balanced build and both cached equivalence gates have actual successful
command exits. These engineering failures did not trigger accuracy retuning.
No failed attempt or negative outcome is silently dropped.

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/build_balanced.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/parity_balanced.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/run_collection.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/run_postprocessing.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_consensus_query_acceleration/finalize_docs.py
```

Use the matching frozen source commits and fresh private build/staging paths;
existing evidence and binaries must not be overwritten. Exact actual commands,
cwd, source/environment, input/binary/static-library hashes and exit codes are in
`checks/`, `sequences/`, `collection_execution/` and postprocessing receipts.
The original external tool session became unavailable after a turn interruption;
its OS exit is explicitly unknown. The durable `subprocess.run` receipt records
the actual collector exit as zero, alongside every native/validator exit; the
completed wrapper archive commit and recovery observations are preserved in
`checks/outer_execution_completion.json`. No success is inferred from log text.
Official KITTI acquisition uses `fetch_kitti_odometry_sequences.py`, one owned
sequence at a time. Every scan/GT hash matches committed official inputs. Each
sequence was losslessly archived, roundtrip-checked, committed and normally pushed
before scan deletion; cleanup was committed/pushed before the next acquisition.

Disk space is conserved only by byte/hash/mode/owner/device-identical sharing
of immutable evidence and feature caches. Every logical path and file byte is
preserved. Closed-worktree coalescing excludes both producer worktrees and every
raw/source/build/object/library/binary/index/top-level aggregate path; the exact
source, command and observed successful OS exit remain in
`checks/closed_evidence_storage/`. Earlier cache-sharing receipts are retained too.

All historical aggregates/results/defaults and primary paper publications are
preserved. New complete rows are experimental, nondefault and unscored for paired
timing. No per-sequence winner is adopted. Scientific checks use actual exit codes;
full Native and ROS2 CI on the final commit are required before review.
