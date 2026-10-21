# Predicted-DOR facade gating: fixed full KITTI evaluation

This evaluates one opt-in repository extension, **not in the paper** and default
off: `--inten-loam-paper-set s2s_predicted_dor_facade=1`. Before normal S2S,
existing DOR evaluates the extracted frame at the initial motion prediction,
using the prior local map and previous full scan. Only source facade queries
whose cluster is marked dynamic are rejected. Extraction, other feature classes,
S2M queries and original post-S2M map insertion retain their existing rules at
the same history. Later reassociation/map/prediction feedback can change all
classes. The existing adaptive S2S prior also changes immediately with facade H.
Mapping and DOR must be enabled; forced-pose diagnostics remain unchanged.

Existing DOR thresholds 0.1/0.5 are reused without fitting. The Boolean candidate
was chosen on current KITTI 01 after committed full-sequence detector evidence
`11cc0aa3` and all matched-seed terminal controls `fe932b7f`. Original profile
selection on 07,00 diagnosis, inherited DiLO 01 prior tuning and all earlier KITTI
exposure are disclosed. No sequence is independent held out. One setting is
frozen across every full 00–10 run; no per-sequence winner is adopted.

Both arms retain mapped-motion prediction, S2S prior 0.03, ground/facade PCA,
acceleration, 120 m range, 0.17 degree elevation and 0.05 m S2M plane check. Plane spread
and row completion remain off. Both use the same freshly compiled native runner;
the default-off arm must reproduce the preceding immutable facade-PCA candidate's
saved pose/GT bytes, metrics and numeric trace body after declared scan-root-only
substitution. The actual native traces are archived unchanged.

The evaluated predicted-DOR facade gate does not resolve the current 01 collapse: official RTE changes 4.481189% → 16.682628% and bad-length labels 44 → 140. It is not adopted as a remedy; the existing default remains off.

Geometric mean of option / reference official RTE: **1.142753x** across all sequences.
Lower RTE: 03, 04, 06. Higher RTE: 00, 01, 02, 05, 07, 08, 09, 10. Unchanged RTE: none.
This is a fixed experimental comparison; no per-sequence default is adopted.
Bad-length labels use the inherited local 20% threshold for GT motion at least 0.1 m,
separate from official KITTI 100–800 m RTE. Raw paired timing is not an FPS benchmark.

Numerical sources committed before prose: `993b539fc1fae6cdea663a406a3b11ec7c4d357c`.

| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |
|---|---:|---:|---:|---:|---:|
| 00 | 0.683293 | 0.692676 | 17.051301 | 17.125431 | 13 / 13 |
| 01 | 4.481189 | 16.682628 | 88.786456 | 276.606530 | 44 / 140 |
| 02 | 0.756938 | 0.768594 | 40.602835 | 43.011557 | 2 / 1 |
| 03 | 0.884094 | 0.873913 | 6.251484 | 6.206519 | 1 / 1 |
| 04 | 0.692176 | 0.691799 | 1.563220 | 1.565672 | 0 / 0 |
| 05 | 0.477296 | 0.483211 | 9.639439 | 9.812135 | 0 / 0 |
| 06 | 0.350792 | 0.344722 | 1.952842 | 1.930538 | 0 / 0 |
| 07 | 0.348742 | 0.355733 | 1.391385 | 1.458557 | 0 / 0 |
| 08 | 0.955307 | 0.974813 | 29.422940 | 28.344228 | 19 / 19 |
| 09 | 0.672321 | 0.673126 | 11.508132 | 10.886310 | 0 / 0 |
| 10 | 1.093310 | 1.208837 | 8.851550 | 9.861123 | 0 / 0 |

Every adverse comparison (reference → option):

- 00: official RTE [%] 0.683293 → 0.692676; ATE [m] 17.051301 → 17.125431; RPE [%] 0.794246 → 0.797955.
- 01: official RTE [%] 4.481189 → 16.682628; rotation [deg/100 m] 0.289430 → 0.402297; ATE [m] 88.786456 → 276.606530; RPE [%] 3.760742 → 13.763909; bad-length steps 44 → 140; bad-vector steps 44 → 140.
- 02: official RTE [%] 0.756938 → 0.768594; rotation [deg/100 m] 0.262615 → 0.271366; ATE [m] 40.602835 → 43.011557.
- 04: rotation [deg/100 m] 0.295843 → 0.308928; ATE [m] 1.563220 → 1.565672; RPE [%] 0.715182 → 0.716071.
- 05: official RTE [%] 0.477296 → 0.483211; rotation [deg/100 m] 0.302184 → 0.314142; ATE [m] 9.639439 → 9.812135; RPE [%] 0.424881 → 0.436951.
- 07: official RTE [%] 0.348742 → 0.355733; rotation [deg/100 m] 0.254061 → 0.266232; ATE [m] 1.391385 → 1.458557.
- 08: official RTE [%] 0.955307 → 0.974813; RPE [%] 1.301395 → 1.315598; bad-vector steps 99 → 100.
- 09: official RTE [%] 0.672321 → 0.673126; RPE [%] 0.690141 → 0.697834.
- 10: official RTE [%] 1.093310 → 1.208837; rotation [deg/100 m] 0.352714 → 0.406639; ATE [m] 8.851550 → 9.861123; RPE [%] 0.825808 → 0.862694.

New local bad labels in the candidate relative to the reference (all frames):

| Sequence | Newly bad length steps | Reference-only bad length steps | Newly bad vector steps | Reference-only bad vector steps |
|---|---:|---:|---:|---:|
| 00 | 0 | 0 | 0 | 1 |
| 01 | 101 | 5 | 101 | 5 |
| 02 | 0 | 1 | 0 | 2 |
| 03 | 0 | 0 | 0 | 1 |
| 04 | 0 | 0 | 0 | 0 |
| 05 | 0 | 0 | 0 | 0 |
| 06 | 0 | 0 | 0 | 0 |
| 07 | 0 | 0 | 0 | 0 |
| 08 | 0 | 0 | 3 | 2 |
| 09 | 0 | 0 | 0 | 0 |
| 10 | 0 | 0 | 0 | 0 |

Each pair uses the same input frame and GT labels but different realized
trajectories. These are outcome differences, not matched-seed interventions.
Every affected frame index is retained in the numerical report.

Failure-sequence observations along each realized trajectory:

- 01: final bad-length frames 204–247 → 209–252, 424–519; raw S2S bad-length outputs 48 → 139; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.
- 02: final bad-length frames 1787, 3308 → 3308; raw S2S bad-length outputs 2 → 2; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.
- 10: final bad-length frames none → none; raw S2S bad-length outputs 0 → 0; S2S fallbacks 0 → 0; final rejected S2M 0 → 0.

Candidate sequences exceeding twice the cited paper RTE:

| Sequence | Option RTE [%] | Paper RTE [%] | Option / paper | Largest position-error growth block | Position error start → end [m] | Local block error [%] |
|---|---:|---:|---:|---|---:|---:|
| 01 | 16.682628 | 0.640000 | 26.066606 | 400–500 | 94.940296 → 312.408711 | 83.331503 |
| 01 | 16.682628 | 0.640000 | 26.066606 | 200–300 | 2.243831 → 93.170017 | 34.407945 |
| 01 | 16.682628 | 0.640000 | 26.066606 | 500–600 | 312.408711 → 361.425592 | 19.320268 |
| 10 | 1.208837 | 0.450000 | 2.686304 | 700–800 | 9.274950 → 15.002735 | 1.041882 |
| 10 | 1.208837 | 0.450000 | 2.686304 | 600–700 | 6.691982 → 9.274950 | 2.120652 |
| 10 | 1.208837 | 0.450000 | 2.686304 | 800–900 | 15.002735 → 17.532306 | 0.501119 |

These fixed-size motion blocks locate error growth; their relative errors are
not official distance-segment RTE and do not establish the solver-level cause.
Paper values are cited context; this extension is not the paper algorithm.

These are natural full-run observations, not matched-seed causal interventions.
The complete report retains every step and all motion-error blocks. The option
remains outside the paper and default off; earlier results are preserved.

## Detector and terminal controls

The unmodified reference traverses all 1101 frames. Every unanchored estimator/solver/count/DOR field matches; the display-anchor and scan-path conventions are declared separately. Detailed range samples independently reproduce native covered/flagged counts.

| Endpoint | First accepted facades | Source prediction mask | Source final mask | Any previous-final-masked target support |
|---|---:|---:|---:|---:|
| 190 | 296 | 0 | 0 | 5 |
| 200 | 465 | 4 | 4 | 20 |
| 202 | 499 | 4 | 5 | 22 |
| 203 | 579 | 6 | 7 | 31 |
| 204 | 548 | 11 | 10 | 42 |
| 205 | 614 | 2 | 0 | 7 |
| 247 | 371 | 11 | 5 | 12 |
| 433 | 352 | 0 | 0 | 39 |

These are native detector labels on accepted first-iteration queries, not motion precision/recall or semantic-object truth. Stage0 prediction/prior-local support is available before S2S. Stage1 uses the same prediction with the later current local map and is retrospective only. Stage2 uses the final pose/current local map. Map and pose effects must not be conflated.

| Reference frame group | Frames | Accepted first-iteration facades | Marked at prediction/prior map | Extracted facades in clusters with insufficient render coverage |
|---|---:|---:|---:|---:|
| all_moving | 1100 | 395497 | 1583 | 154882 |
| s2s_bad_length | 48 | 21795 | 459 | 5831 |
| final_bad_length | 44 | 20204 | 351 | 5318 |
| other_moving | 1052 | 373702 | 1124 | 149051 |

The last column counts extracted features over frames, not accepted pairs or unique physical points. Because native DOR divides flagged points by all cluster points, render coverage below the existing object threshold prevents that cluster from being marked. Adequate coverage alone does not ensure the radial threshold flags most points.

| Current01 frame 204 cluster | Accepted facade pairs | Conditional joint first forward update [% of GT step] | Prediction/prior map: total / covered / flagged / dynamic | Same prediction/current map | Final pose/same current map |
|---|---:|---:|---|---|---|
| 600 | 19 | -1.195503 | 341 / 299 / 67 / 0 | 341 / 299 / 67 / 0 | 341 / 258 / 21 / 0 |
| 249 | 27 | -0.087402 | 1657 / 449 / 137 / 0 | 1657 / 452 / 137 / 0 | 1657 / 926 / 19 / 0 |
| 506 | 3 | -0.498831 | 47 / 40 / 35 / 1 | 47 / 40 / 35 / 1 | 47 / 33 / 2 / 0 |

These committed onset examples distinguish inadequate coverage, weak radial-majority response despite coverage, and pose-feedback changes. The conditional update uses the unchanged joint damped first system; it is not an isolated re-solve or a motion label. Comparing the last two columns holds the rendered local map fixed. No coverage denominator or detector threshold change is selected from these observations.

| Endpoint | Baseline terminal S2S length / GT | Source prediction gate | Previous-final target filter | Baseline vector error / GT | Source gate vector error / GT |
|---|---:|---:|---:|---:|---:|
| 190 | 0.990767 | 0.990767 | 0.990170 | 0.014508 | 0.014508 |
| 200 | 0.975241 | 0.982577 | 0.972703 | 0.025618 | 0.018657 |
| 202 | 0.748598 | 0.805956 | 0.875927 | 0.251434 | 0.194065 |
| 203 | 0.918730 | 0.935184 | 0.919566 | 0.081779 | 0.065452 |
| 204 | 0.607064 | 0.921199 | 0.814394 | 0.392957 | 0.079145 |
| 205 | 0.372446 | 0.372427 | 0.373116 | 0.627582 | 0.627601 |
| 247 | 0.522727 | 0.556361 | 0.594366 | 0.477580 | 0.443942 |
| 433 | 0.926498 | 0.926769 | 0.927609 | 0.074390 | 0.074162 |

Every one of 3300 terminal calls completed successfully. All unchanged natural-seed outputs and non-facade initial data equations match. Source/target masks are the original detector decisions from identical prefixes; no GT or hand box enters either arm.

| Group | Arm | Length error: lower / higher / equal | Vector error: lower / higher / equal |
|---|---|---:|---:|
| all_moving | Source prediction gate | 194 / 199 / 707 | 185 / 208 / 707 |
| all_moving | Previous-final target filter | 229 / 221 / 650 | 234 / 215 / 651 |
| baseline_bad_length | Source prediction gate | 17 / 30 / 1 | 18 / 29 / 1 |
| baseline_bad_length | Previous-final target filter | 17 / 31 / 0 | 17 / 31 / 0 |
| other_moving | Source prediction gate | 177 / 169 / 706 | 167 / 179 / 706 |
| other_moving | Previous-final target filter | 212 / 190 / 650 | 217 / 184 / 651 |

All adverse and neutral terminal cases are retained. The source gate was selected after these committed development controls, before all full runs. A terminal repair does not include future map/prediction feedback. Target filtering is a diagnostic comparator and is not the implemented option. Ground truth appears only in offline errors.

## Where full-run errors grow

| Sequence | Arm | Boundary frame | S2S input / GT | S2S output / GT | Final step / GT | Raw masked facade features |
|---|---|---:|---:|---:|---:|---:|
| 01 | baseline | 204 | 0.915358 | 0.607064 | 0.422811 | 0 |
| 01 | candidate | 209 | 0.915602 | 0.814828 | 0.747275 | 95 |
| 01 | candidate | 424 | 0.875388 | 0.510718 | 0.347111 | 33 |
| 02 | baseline | 1787 | 1.177768 | 1.190148 | 1.216228 | 0 |
| 02 | baseline | 3308 | 1.363143 | 1.271465 | 1.305844 | 0 |
| 02 | candidate | 3308 | 1.366155 | 1.271412 | 1.276677 | 0 |

Every bad-interval boundary and adjacent frame is retained in the report, and every native step is retained in the CSVs. These stage errors are along different realized trajectories, not matched-history class ablations. The fixed-size growth blocks above also cover gradual drift without a local threshold crossing.

![Full official RTE comparison](report/official_rte.png)

## Reproduce and audit

Run from the repository worktree root. The commands below are the recorded stage
entry points, with complete argv and actual exits in the adjacent receipts.
They refuse existing outputs and are not a single replay command for a completed
checkout. The native producer source, compiler/
link metadata, immutable library/binary SHA256 values, command/cwd/environment
and actual exits are in `full_protocol.json`, `binaries.json`, `checks/` and each
sequence's receipts. Local changed-library tests use actual CTest exits; full
Native and ROS2 CI are required before review.

```bash
python experiments/results/inten_prediction_dor/build_candidate.py
python experiments/results/inten_prediction_dor/run_collection.py
python experiments/results/inten_prediction_dor/report.py --output-dir <fresh-report-dir>
python experiments/results/inten_prediction_dor/import_results.py
```

The native build phase uses source `8ab36c35`; full collection is frozen at
`29bb1fa9`, before its result directories exist. It follows the completed passive
and terminal stages and revalidates their owned 01 acquisition, rather than
silently downloading another copy. The full collector expects their local seal
receipts. Reconstruct those recorded prerequisites in an isolated workspace
before replaying native collection. Rebuilding creates new binary/library hashes
and therefore requires a new committed protocol; do not overwrite these pins or
stored results. The postprocessing protocol records the complete raw snapshot and
the analysis source hashes. Import runs once before its aggregate files exist.

For analysis of the committed results without downloading scans or modifying
aggregates, run only `report.py --output-dir <fresh-report-dir>`. It restores the
lossless traces, verifies the fixed profiles and solver records, and recomputes
all comparison tables and the figure. `summary.json` records the current analysis
source commit, so that provenance field can differ on a later checkout.

The scripts use the recorded managed Ubuntu22.04/GCC11.4/Eigen3.4/PCL1.12
container and byte-pinned unchanged native dependencies; rebuild those matching
dependencies when reconstructing this environment. Host analysis uses the recorded
NumPy/Matplotlib venv, with its PATH prefix and PYTHONDONTWRITEBYTECODE=1. Native
OMP/BLAS threads are fixed. Paired CPU timings are retained raw but never treated
as standalone FPS or used for proxy/default ranking.

Diagnostics predate the new native ABI. Reproduce the passive stage from source
`99b681d4` (corrected collector and analysis), and terminal stage from `782d6b06`,
using their matching headers and original native dependency metadata. Do not
compile those frozen source copies against the final changed private signature.
Their protocols, binaries, native outputs, input seeds/masks and phase hashes are
retained; diagnostic copies never supply full-sequence accuracy. The passive
producer never reads GT. Its `anchored_pose` is sensor-relative; the harness uses
a GT-derived output display anchor. That declared output-coordinate difference
and validated scan-path convention are excluded from passive parity; every
unanchored estimator/solver/count/DOR field matches exactly. GT and the inherited
hand region are offline labels only and never feed either automatic arm.

Official scans are fetched by `evaluation/scripts/fetch_kitti_odometry_sequences.py`
one owned full sequence at a time. Every raw/GT hash is checked against committed
inputs. Lossless archives are byte-roundtrip verified, committed and normally
pushed before deleting owned scans; cleanup is committed/pushed before the next
fetch. Initial metadata-parser exit 1 before native execution, acquisition retries
and CMake-log whitespace gate exit 2 remain recorded. Compilation/acquisition were
not silently rerun after the parser failure; an explicit verified continuation
ran the preserved native producer. CMake stdout is byte-exact gzip plus retained
originals, never sanitized. All native mechanism checks then exited 0.

All historical results, benchmarks, default behavior and canonical publication
numbers are preserved. Immutable historical evidence storage sharing has complete
byte/hash/mode proof; every logical path is retained and old builds/binaries are
untouched. New full variants remain experimental/nondefault/unscored with blank
aggregate timing. Numerical results are committed before this interpretation;
required derived files are regenerated after import.
