# S2S facade Cauchy influence: fixed full KITTI evaluation

`--inten-loam-paper-set s2s_facade_cauchy=1` is a repository extension,
**not in the paper and default off**. It replaces only S2S facade Huber IRLS
influence by Cauchy, at the same inherited native scale 0.1 m. Extraction,
correspondence gates, other feature-class loss definitions, S2M and map-insertion
definitions stay fixed. Adaptive S2S prior information changes immediately with
the data system; later reassociation, motion prediction and map feedback can
change every class along the realized trajectory.

For each accepted S2S facade point-to-plane residual `r` at native scale `h`,
the reference uses `w = 1 if abs(r) <= h else h/abs(r)` and the option uses
`w = 1/(1+(r/h)^2)`. Both accumulate `w*J*Jᵀ` and `w*J*r`. No residual is
discarded by this rule, and the stored weighted squared residual remains an
IRLS surrogate rather than a comparable robust loss energy across the arms.

One Boolean at the unchanged scale was selected after committed current01
terminal controls `f52b84a0`, before native implementation and full evaluation.
The positive204 and adverse247/rotation cases are retained below. No scale fit,
sweep, hand region, predicted-DOR source mask or GT seed/input enters the S2S
intervention. The inherited post-S2M DOR and map-insertion logic remains enabled
in both full arms. Original
profile selection on07,00 diagnosis, inherited DiLO01 prior tuning and all prior
KITTI exposure are disclosed. All sequences are development evidence; no
independent held-out or per-sequence adoption claim is made.

Both arms retain mapped-motion prediction, S2S prior0.03, ground/facade PCA,
acceleration,120m range,0.17deg correction and0.05m S2M plane check. Predicted DOR,
plane spread and row completion remain off; original post-S2M DOR remains on.
Both use the same new native binary;
the default-off arm must reproduce the immutable preceding facade-PCA candidate
(also PR162 OFF reference) in saved pose/GT bytes, accuracy metrics and numerical
trace bytes after only the declared scan-root substitution. Original traces are
archived unchanged. Raw paired CPU timing is preserved but is not a standalone
FPS measurement and never selects a default or proxy ranking.

The evaluated S2S facade Cauchy influence does not resolve the current 01 collapse: official RTE changes 4.481189% → 38.696732% and bad-length labels 44 → 317. It is not adopted as a remedy; the existing default remains off.

Geometric mean of option / reference official RTE: **1.231530x** across all sequences.
Lower RTE: 04, 06, 09. Higher RTE: 00, 01, 02, 03, 05, 07, 08, 10. Unchanged RTE: none.
This is a fixed experimental comparison; no per-sequence default is adopted.
Bad-length labels use the inherited local 20% threshold for GT motion at least 0.1 m,
separate from official KITTI 100–800 m RTE. Raw paired timing is not an FPS benchmark.

Numerical sources committed before prose: `c2775241564cfc41d669710bb7d3b706999f56bf`.

| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |
|---|---:|---:|---:|---:|---:|
| 00 | 0.683293 | 0.693417 | 17.051301 | 17.323192 | 13 / 13 |
| 01 | 4.481189 | 38.696732 | 88.786456 | 663.848572 | 44 / 317 |
| 02 | 0.756938 | 0.776016 | 40.602835 | 41.654381 | 2 / 2 |
| 03 | 0.884094 | 0.886578 | 6.251484 | 6.256227 | 1 / 0 |
| 04 | 0.692176 | 0.681660 | 1.563220 | 1.551919 | 0 / 0 |
| 05 | 0.477296 | 0.478345 | 9.639439 | 9.530742 | 0 / 0 |
| 06 | 0.350792 | 0.347947 | 1.952842 | 1.973790 | 0 / 0 |
| 07 | 0.348742 | 0.373458 | 1.391385 | 1.559166 | 0 / 0 |
| 08 | 0.955307 | 0.958029 | 29.422940 | 28.620505 | 19 / 19 |
| 09 | 0.672321 | 0.648465 | 11.508132 | 11.066699 | 0 / 0 |
| 10 | 1.093310 | 1.182742 | 8.851550 | 9.920418 | 0 / 0 |

Every adverse comparison (reference → option):

- 00: official RTE [%] 0.683293 → 0.693417; ATE [m] 17.051301 → 17.323192; RPE [%] 0.794246 → 0.797602.
- 01: official RTE [%] 4.481189 → 38.696732; rotation [deg/100 m] 0.289430 → 0.545806; ATE [m] 88.786456 → 663.848572; RPE [%] 3.760742 → 31.124457; bad-length steps 44 → 317; bad-vector steps 44 → 317.
- 02: official RTE [%] 0.756938 → 0.776016; rotation [deg/100 m] 0.262615 → 0.267973; ATE [m] 40.602835 → 41.654381.
- 03: official RTE [%] 0.884094 → 0.886578; rotation [deg/100 m] 0.632477 → 0.636698; ATE [m] 6.251484 → 6.256227.
- 04: rotation [deg/100 m] 0.295843 → 0.320329.
- 05: official RTE [%] 0.477296 → 0.478345; RPE [%] 0.424881 → 0.426088.
- 06: ATE [m] 1.952842 → 1.973790.
- 07: official RTE [%] 0.348742 → 0.373458; rotation [deg/100 m] 0.254061 → 0.284914; ATE [m] 1.391385 → 1.559166.
- 08: official RTE [%] 0.955307 → 0.958029; RPE [%] 1.301395 → 1.313902.
- 10: official RTE [%] 1.093310 → 1.182742; rotation [deg/100 m] 0.352714 → 0.388060; ATE [m] 8.851550 → 9.920418; RPE [%] 0.825808 → 0.851294.

New local bad labels in the candidate relative to the reference (all frames):

| Sequence | Newly bad length steps | Reference-only bad length steps | Newly bad vector steps | Reference-only bad vector steps |
|---|---:|---:|---:|---:|
| 00 | 0 | 0 | 0 | 1 |
| 01 | 273 | 0 | 273 | 0 |
| 02 | 1 | 1 | 0 | 1 |
| 03 | 0 | 1 | 0 | 2 |
| 04 | 0 | 0 | 0 | 0 |
| 05 | 0 | 0 | 0 | 0 |
| 06 | 0 | 0 | 0 | 0 |
| 07 | 0 | 0 | 0 | 0 |
| 08 | 0 | 0 | 1 | 1 |
| 09 | 0 | 0 | 0 | 0 |
| 10 | 0 | 0 | 0 | 0 |

Each pair uses the same input frame and GT labels but different realized
trajectories. These are outcome differences, not matched-seed interventions.
Every affected frame index is retained in the numerical report.

Failure-sequence observations along each realized trajectory:

- 01: final bad-length frames 204–247 → 202–518; raw S2S bad-length outputs 48 → 317; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.
- 02: final bad-length frames 1787, 3308 → 3308–3309; raw S2S bad-length outputs 2 → 3; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.
- 10: final bad-length frames none → none; raw S2S bad-length outputs 0 → 0; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.

Candidate sequences exceeding twice the cited paper RTE:

| Sequence | Option RTE [%] | Paper RTE [%] | Option / paper | Largest position-error growth block | Position error start → end [m] | Local block error [%] |
|---|---:|---:|---:|---|---:|---:|
| 01 | 38.696732 | 0.640000 | 60.463644 | 400–500 | 506.462623 → 794.731143 | 109.630871 |
| 01 | 38.696732 | 0.640000 | 60.463644 | 300–400 | 232.493919 → 506.462623 | 103.116440 |
| 01 | 38.696732 | 0.640000 | 60.463644 | 200–300 | 2.199079 → 232.493919 | 86.403641 |
| 10 | 1.182742 | 0.450000 | 2.628316 | 700–800 | 9.482345 → 15.021287 | 0.916655 |
| 10 | 1.182742 | 0.450000 | 2.628316 | 600–700 | 6.921797 → 9.482345 | 2.067136 |
| 10 | 1.182742 | 0.450000 | 2.628316 | 800–900 | 15.021287 → 17.501691 | 0.465098 |

These fixed-size motion blocks locate error growth; their relative errors are
not official distance-segment RTE and do not establish the solver-level cause.
Paper values are cited context; this extension is not the paper algorithm.

These are natural full-run observations, not matched-seed causal interventions.
The complete report retains every step and all motion-error blocks. The option
remains outside the paper and default off; earlier results are preserved.

## Matched-seed loss controls before full evaluation

Every one of 2200 terminal calls completes; all 1100 unchanged outputs and first data-class systems match the archived reference. Every baseline solver iteration and state field also matches, with only the explicitly checked forced diagnostic first-count convention separated from the normal caller last count.
The Cauchy arm keeps every first association/count and non-facade first data equation. Later reassociation and the adaptive S2S prior can change. GT appears only in offline errors/directions. No GT seed, hand region, DOR mask or fitted scale enters either terminal arm. These are terminal solves from identical prefixes, without future map/prediction feedback.
Terminal bad labels refer to the S2S estimate. Full-run table labels refer to the final mapped increment, so their baseline counts need not agree.

| Reference group | Frames | Length error: lower / higher / equal | Vector error: lower / higher / equal | Rotation error: lower / higher / equal | Remaining bad length / vector | Newly bad length / vector |
|---|---:|---:|---:|---:|---:|---:|
| all_moving | 1100 | 500 / 600 / 0 | 527 / 573 / 0 | 584 / 516 / 0 | 44 / 44 | 0 / 0 |
| baseline_bad_length | 48 | 12 / 36 / 0 | 12 / 36 / 0 | 24 / 24 / 0 | 44 / 44 | 0 / 0 |
| other_moving | 1052 | 488 / 564 / 0 | 515 / 537 / 0 | 560 / 492 / 0 | 0 / 0 | 0 / 0 |

| Endpoint | Reference length / GT | Cauchy length / GT | Reference vector error / GT | Cauchy vector error / GT | Reference rotation error [mrad] | Cauchy rotation error [mrad] |
|---|---:|---:|---:|---:|---:|---:|
| 190 | 0.990767 | 0.992208 | 0.014508 | 0.010651 | 2.320873 | 2.276540 |
| 200 | 0.975241 | 0.989548 | 0.025618 | 0.012604 | 1.717553 | 1.163835 |
| 202 | 0.748598 | 0.753723 | 0.251434 | 0.246340 | 3.270653 | 1.821137 |
| 203 | 0.918730 | 0.930111 | 0.081779 | 0.070957 | 2.359487 | 1.654489 |
| 204 | 0.607064 | 0.992481 | 0.392957 | 0.010956 | 1.716533 | 3.142477 |
| 205 | 0.372446 | 0.372472 | 0.627582 | 0.627542 | 2.419936 | 1.904926 |
| 247 | 0.522727 | 0.046692 | 0.477580 | 0.955662 | 8.113092 | 6.873531 |
| 433 | 0.926498 | 0.962924 | 0.074390 | 0.037970 | 1.232359 | 0.575256 |

The unchanged first-query set contains 395497 accepted facades; 329263 are inside the native Huber scale. Median absolute residual is 0.036666 m. Median Huber/Cauchy influence is 1.000000/0.881494. Cauchy changes weights inside the Huber inlier region too; residual magnitude is not a static/moving or correct-correspondence label.
Independent reconstruction of every first facade H/g has maximum relative error 4.168e-16; maximum recorded native linear-solve backward error is 7.192e-17. Weighted squared residual is the native IRLS surrogate, not a robust loss energy comparable across the two loss definitions.

| Current01 frame204 cluster | Accepted pairs | Reference joint first forward contribution [% of GT step] | Cauchy weights through fixed reference system | Cauchy weights through its own joint system |
|---|---:|---:|---:|---:|
| 600 | 19 | -1.195503 | -0.248013 | -0.371743 |
| 249 | 27 | -0.087402 | -0.033865 | -0.048261 |
| 506 | 3 | -0.498831 | -0.061329 | -0.092141 |

These are inherited onset examples, not selected detector rules. The fixed reference inverse isolates the algebraic weight change; the alternative inverse also includes changed information and adaptive prior. Conditional first-system contributions are not independent nonlinear interventions or physical-motion truth. Every cluster and every pair, including adverse and neutral outcomes, is retained. The improved204 and adverse247 controls motivated an unchanged-scale recursive trial; they did not establish recovery.

## Full-run error boundaries

| Sequence | Arm | Bad-interval start | S2S input / GT | S2S output / GT | Final step / GT | First accepted facades |
|---|---|---:|---:|---:|---:|---:|
| 01 | baseline | 204 | 0.915358 | 0.607064 | 0.422811 | 548 |
| 01 | candidate | 202 | 0.916279 | 0.649875 | 0.406279 | 520 |
| 02 | baseline | 1787 | 1.177768 | 1.190148 | 1.216228 | 296 |
| 02 | baseline | 3308 | 1.363143 | 1.271465 | 1.305844 | 276 |
| 02 | candidate | 3308 | 1.379070 | 1.272026 | 1.260654 | 274 |

Every bad-interval boundary and adjacent frame is retained in the numerical report; every native step is in its CSV. These full-run stages use different realized histories and are not matched-history causal interventions. Fixed-size error-growth blocks above also locate gradual drift without a local threshold crossing.

![Full official RTE comparison](report/official_rte.png)

![KITTI 01 full position error and final step length](report/seq_01_error_growth.png)

![KITTI 10 full position error and final step length](report/seq_10_error_growth.png)

Position-error curves normalize the first pose and are distinct from globally aligned ATE. Step ratios include only the declared moving frames; dashed lines show the same local20% label boundaries, not official KITTI distance-segment RTE.

## Commands and phase-specific reproduction

Run from the worktree root. These are recorded stage entry points, not a single
fresh-clone replay recipe for a completed checkout; scripts refuse existing
outputs. Full argv/cwd/source/environment and actual exit codes accompany each
native execution, build, acquisition, analysis, backup and check.

Use the recorded host analysis venv (on the measured machine,
`/workspace/localization-env/host-venv/bin/python`) with its directory first on
`PATH` and `PYTHONDONTWRITEBYTECODE=1`; keep OMP/BLAS threads at one. The
`analysis_runtime` record pins Python, NumPy and Matplotlib separately from the
native container runtime. Do not substitute a different plotting environment
when checking historical generated figures.

```bash
python experiments/results/inten_facade_loss/resume.py
python experiments/results/inten_facade_loss/seal.py
python experiments/results/inten_facade_loss/build_candidate.py
python experiments/results/inten_facade_loss/run_collection.py
python experiments/results/inten_facade_loss/recovery_collection_backups.py
python experiments/results/inten_facade_loss/report.py --output-dir <fresh-report-dir>
python experiments/results/inten_facade_loss/import_results.py
python experiments/results/inten_facade_loss/run_postprocessing.py
python experiments/results/inten_facade_loss/finalize_docs.py
```

The diagnostic clone and corrected probe use source `0686c89f`, with corrected
analysis `98b9bd5a`. These precede the native parameter ABI extension: reconstruct
those stages with their matching headers and unchanged pinned dependencies,
never compile the old clone against the final changed headers. The terminal
producer receives only official scans and the original estimated relative seed.
Mapping is disabled to isolate S2S; the seed is supplied through the forced
diagnostic interface and is never GT. Its internal observer keeps the public
passive/forced-trace guard intact. The forced first-data count convention is
checked explicitly and separated from the normal caller last count; all other
baseline solver state and iterations must match exactly.

The native implementation is frozen at `bf0d44cc`; full collection source is
`243b14a3` with runtime capture `8923e50d`, before its result directories exist.
Compiler/link flags, actual library/binary hashes and focused CTest exits are in
`binaries.json` and `checks/candidate_build/`. The recorded Ubuntu22.04/GCC11.4/
Eigen3.4/PCL1.12 container reuses byte-pinned unchanged dependencies. Rebuilding
dependencies or paths creates new hashes and requires a new committed protocol;
do not overwrite these pins or stored results. Host NumPy/Matplotlib versions and
the analysis venv/PATH prefix are recorded; native OMP/BLAS threads stay fixed.

`run_postprocessing.py` owns the report/import transaction and commits and pushes
its numerical outputs before `finalize_docs.py` writes the interpretation; do not
run both the individual import and its owner on the same checkout.

For analysis without downloading scans or importing again, run only `report.py`
to a fresh directory. It restores the committed full traces and recomputes all
tables and the figure. Its source-commit provenance field can differ on a later
checkout. `postprocessing_protocol.json` pins the raw collection snapshot and
analysis sources. Import runs once before its new aggregate files exist.

## Failures and preservation

The first official01 acquisition ended early and is retained alongside the
successful retry; all final scan/GT hashes match committed official inputs.
Initial diagnostic source `8f2600a8` rejected forced pose plus public passive
trace before producing any terminal output. The failure is committed before the
corrected internal observer; original binary/object remain unchanged and a
distinct corrected producer is compiled. The first analysis missed the optional
omission of an all-zero prior-information field; its failure and empty partial
outputs are retained before the parser correction. Analysis reuses verified
native output and does not rerun the producer or refetch scans.

After every full native arm and the collector complete, the original collection
wrapper rejects an already existing diagnostic backup archive as its final
storage target. Its actual failure, complete stdout/stderr and original driver
are retained under `collection_wrapper_failure/`. The committed
`recovery_collection_backups.py` archives complete backup receipts to a fresh
`full_backup_execution/`, with its actual exit and byte-roundtrip proof retained.
`backup_execution/` stays byte/hash/mode-identical. This recovery does not rerun
acquisition, native processing, validation or cleanup; collector and wrapper
exit codes are recorded separately.

Official scans use `evaluation/scripts/fetch_kitti_odometry_sequences.py`, one
owned full sequence at a time. Every raw/GT SHA is checked, all raw evidence is
losslessly roundtripped and committed/pushed before scan deletion, and cleanup
is committed/pushed before the next acquisition. Ordered32MiB uncompressed gzip
shards preserve large traces. Original CMake logs, including trailing spaces,
remain byte-exact gzip with private originals and roundtrip hashes.

Immutable historical storage sharing proves every byte/hash/mode and retains all
logical paths; old source/builds/binaries, aggregates, defaults and canonical
publication values remain unchanged. Every new full result is experimental,
nondefault and unscored, with aggregate timing unavailable. Numerical evidence
is committed before prose, the required derived files are regenerated, and full
Native plus ROS2 CI must pass at the exact final head before Ready.
