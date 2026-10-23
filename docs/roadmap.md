# Roadmap and handoff

Long-term plan and development handoff for Localization Zoo.
It is written for the next agent or contributor taking over: where things
stand, how work is done here, what to do next and how to tell it is done.

The goal is a usable implementation zoo with numbers people can trust: every
method runs from one harness, every reported number can be re-run, and every
gap to a paper is either closed or explained. Writing a paper is not a goal.

## 1. Where things stand


### Current development decision (2026-10-24)

The user requested closing and merging this development increment. The fixed
trial is paused, not complete; the default-off hybrid remains experimental.
The [saved 00 prefix](../experiments/results/inten_geometric_prior/paused_attempts/merge_requested_00/prefix_audit.json)
(`44c8b1c5`) independently verifies 3072 frames with exact reference parity.
Full 01 remains complete; full 00 and 02–10 and the prospective retention gate
remain future work. Resume only after checking the recorded process identities;
if native state is gone, restart 00 from zero with the frozen settings. The
following acquisition/recovery notes describe the history before this pause.

The default-off hybrid geometric initializer/prior is now implemented; see
[the fixed full-sequence trial](../experiments/results/inten_geometric_prior/README.md).
Focused native tests, normal CLI build and disabled-path fixture parity pass.
The first full fixed hybrid sequence, KITTI 01, now completes (`c99beb6a`).
Against the frozen improved InTEn reference, official RTE is 4.481189% → 0.967180%
and ATE is 88.786456 m → 20.265326 m. The independent audit (`11902445`)
reconstructs all diagnostics and verifies exact reference parity across all 1101
frames. The successful run acquired and verified every input before starting;
retain all preceding interruptions. Normal Git publication has also recovered.

This hypothesis was selected on 01; every KITTI sequence is development data.
00 and 02–10 remain pending and are run with the same frozen binary/settings.
The 00 acquisition failure is a bounded-range extractor defect: local ZIP extra
fields are 28 bytes while the central directory reports 24 (`73ca5f08`).
The extractor now reads the final local header; CRC and frozen input SHA checks
remain mandatory. The estimator and settings are unchanged. Preserve completed
01; restarted controllers must select only remaining sequences and explicitly
use the corrected fetch script. The corrected 00 attempt then processed 384
frames with exact reference parity before renewed HTTP 503 acquisition failures
exhausted its retries (`1049b182`). The response names `cloudflare_https_tunnel`;
this is distinct from the repaired ZIP-length defect. Its native exit -9 records
controller termination after failed acquisition. All interrupted evidence and
its independent audit are retained; no full 00 metric or gate verdict follows.
Normal push subsequently publishes the retained follow-up through `c3f95819`.
A [new launch](../experiments/results/inten_geometric_prior/extended_acquisition_launch.json)
selects only remaining sequences with `--fetch-attempts 120`. The default remains
20; this opt-in changes acquisition retries only, so a short outage can leave the
same native process waiting instead of discarding its map/history. The original
binary hash and all estimator settings remain fixed. Initial small probes recover,
but the real scan preflight again returns HTTP 503. The current launch is not a
completed result. Inspect its live command receipt/process before starting any
other controller; do not run duplicate sequence jobs. Preserve the completed 01.
After the environment returned, the original controller/native process were
still alive. The [recovery observation](../experiments/results/inten_geometric_prior/recovered_runtime_observation/observation.json)
(`e5d385e6`) preserves 512 processed input records with frozen input hashes and
zero recorded reference matrix difference. It is a live progress snapshot, not
a full-sequence accuracy result or independent native audit. Scan acquisition
and normal Git push still fail with HTTP 503; preserve the running native state.
[PR #174](https://github.com/rsasaki0109/localization_zoo/pull/174) may merge at
the user's explicit request with this incomplete trial documented. The fixed
all-sequence requirement and prospective retention gate still govern any claim
of an accepted improvement; merging does not satisfy either requirement.

Bad-length steps decrease from 44 to 7, but failures remain at 209–210, 214–215,
432–433 and 436. The last three are new relative to the reference. At 432,
S2S length / GT is 1.005920 but accepted mapping ends at 0.734083. The
[post-hoc complete stage report](../experiments/results/inten_geometric_prior/posthoc_bad_steps_01.json)
retains every remaining bad step. The predeclared 215 negative also remains:
1.003769 at S2S input, 0.780277 after S2S, and 0.373955 after mapping. These
local negatives coexist with the full 01 improvement. Do not tune from them.
After the frozen trial, investigate the accepted mapping correction with the
same map history. No Table 6, paper eligibility or SOTA claim follows.

A [fresh complete Python discovery](../experiments/results/inten_geometric_prior/publication_final_discovery/)
now succeeds: 448 tests, five skipped, exit 0 (`a9f2991b`). The adapter restores
archived production sources against unchanged hashes, runs the original workers
serially, and binds nested recovery checks. Worker outputs and the final global
audit pass in the same execution; regenerated historical tables are byte-identical.
Earlier failures are retained. Full native/ROS2 CI is not claimed. A controlled
same-version comparison shows that the earlier numerical witness depends on the
NumPy installation/build: the system build fails and a PyPI wheel passes with
the same Python/NumPy/SciPy versions. Preserve build provenance and tolerances;
this runtime control is not a trajectory improvement.



The independent initialization comparison is complete; see
[the executed study](../experiments/results/inten_independent_onset/README.md).
On full KITTI 01 with the frozen InTEn input policy, upstream KISS-ICP reaches
0.949387% RTE versus the fixed opt-in InTEn profile's 4.481189%. In 73 isolated
S2S pairs, independent current/previous initial poses reduce >20% step errors
from 46 to 2/3, with exact native replay controls. These poses also set the
weak translation-prior target; the intervention does not separate optimizer
initialization from that prior. This is diagnostic evidence,
not an improved full InTEn run. Good seeds still fail at 215 and 420; retain
those adverse controls and the near-threshold case at 202.

The GT-free independent initializer is now implemented as the default-off
hybrid above. The next milestone is completion of its fixed all-sequence trial.
Do not continue scalar/mask searches without a new causal reason. Keep the
fixed improved profile as reference and retain every previous failed gate.
No Table 6 or paper-number change follows from this single-sequence study.


### Paper-number check (official KITTI RTE, 100-800 m segments)

Repo value over paper value, geometric mean over the sequences run
([Table 6](assets/paper/paper_ratio_table.csv),
`evaluation/scripts/generate_paper_ratio_table.py`). A-LOAM, LF-GICP,
L-LO, Quadric-LO, Mesh-LOAM and InTEn-LOAM are restricted to one fixed
variant across every sequence with a paper value. Other methods may select
among allowed variants per sequence. "One setting" is the ratio when one
variant runs on every sequence.

| Method | Ratio | Sequences run | Paper values not run yet | Notes |
|---|---:|---|---|---|
| LiTAMIN2 | 1.01x | 00-10 | - | one setting 1.05x |
| A-LOAM | 1.07x | 00-10 | - | one setting selected on 07; vs secondary-source LOAM values; no sequence above 2x |
| LF-GICP | 1.12x | 00-10 | - | one setting; calibrated on 00 after earlier 02/08 observations; above 2x: 10 (6.71x) |
| KISS-ICP | 1.04x | 00/04 | - | 00-10 average equals the official code's |
| MULLS | 1.12x | 00-10 | - | 00-10 average equals the official code's |
| CT-ICP | 1.15x | 00-10 | - | 00-10 average equals the official code's |
| SuMa | 0.97x | 00-10 | - | 01 (highway) 2.09x; one setting 1.25x |
| Quadric-LO | 0.85x | 00-10 | - | one setting selected using 00/07; scan-to-submap reference; no sequence above 2x |
| DiLO | 0.96x | 00-10 | - | one setting; 01 needs the degenerate-direction remap (chosen on 01) |
| IMLS-SLAM | 1.05x | 00-10 | - | one setting; coarse first registration fixes 04 |
| Mesh-LOAM | 1.28x | 00-10 | - | one setting; no sequence above 2x |
| ELO | 1.22x | 00-10 | - | gap spread over all sequences (0.84-1.43x) |
| InTEn-LOAM | 2.19x | 00-10 | - | one setting; 01 25.96x, 02 5.97x, 10 2.57x |
| L-LO | 1.57x | 00/02-10 | - | one setting selected on 07; 01 is NA in the paper; above 2x: 09 (2.18x), 10 (2.16x) |

Per-method details and negative results live in each `papers/<method>/README.md`;
the claim wording is in [paper_claim.md](paper_claim.md).

### Second lidar (NCLT, HDL-32E on a Segway)

[nclt_paper_paths.md](nclt_paper_paths.md): with sensor settings only,
KISS-ICP 1.75 %, IMLS-SLAM 2.01 %, InTEn-LOAM 2.03 % (large ATE),
ELO 3.36 % with returns within 5 m dropped (18.1 % without), DiLO 7.4-8.5 %,
Mesh-LOAM 13.2 %, SuMa fails. The NCLT data were deleted from the
development machine; re-create them with
[`SETUP_NCLT_BENCHMARK.md`](../evaluation/scripts/SETUP_NCLT_BENCHMARK.md).

### Recent work (PRs #120-#127)

- Paper pipelines for SuMa, InTEn-LOAM, Mesh-LOAM, DiLO, IMLS-SLAM, ELO
  (`--<method>-paper*` flags in `evaluation/src/pcd_dogfooding.cpp`).
- Runner flags for other sensors: `--input-sensor-pose`, `--input-min-range-m`,
  per-method FOV / row / height keys; `check_scan_frame.py`.
- Opt-in robustness fixes, default off: DiLO `--dilo-paper-degenerate-ratio`,
  IMLS-SLAM `--imls-slam-paper-coarse-frames`, InTEn-LOAM
  `s2m_plane_max_dist`.
- README reduced to a landing page; long sections in
  [implementations.md](implementations.md) and
  [benchmark_overview.md](benchmark_overview.md).

## 2. How work is done here

### Build and checks

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure            # C++ unit tests (CI runs this)
python3 -m unittest discover -s tests                  # Python / docs contract tests
python3 evaluation/scripts/validate_showcase.py        # README + docs/index.html leaderboard
python3 evaluation/scripts/generate_leaderboard.py --check
python3 evaluation/scripts/generate_paper_ready_evidence.py --check
```

Check exit codes directly. Piping a test run into `tail` hides a failure.

### Experiments

- A problem is a manifest `experiments/<name>_matrix.json`: dataset
  (`dogfooding_results/kitti_seq_NN_full`, `experiments/reference_data/kitti_seq_NN_full_gt.csv`)
  plus variants, each a list of `pcd_dogfooding` arguments.
- Run with `python3 evaluation/scripts/run_experiment_matrix.py --manifest <path> --output-dir <dir> --docs-dir <dir>`.
  To run one variant, write a copy of the manifest holding only that
  variant to a scratch directory and run that.
- Results go to `experiments/results/<name>_matrix.json` (aggregate) and
  `experiments/results/runs/<name>_matrix/<variant>/` (summary, host, log).
  To import a scratch run, copy its `runs/<name>_matrix/<variant>/` directory
  there and re-aggregate with
  `run_experiment_matrix.py --manifest experiments/<name>_matrix.json --merge-existing-index --reuse-existing`.
  The runner wants the dataset directory to exist; if the scans were
  deleted, an empty directory at that path is enough for `--reuse-existing`.
  Do not let it re-run a variant that has no local summary.
- After importing, regenerate:
  `refresh_study_docs.py`, `generate_full_variant_table.py`
  (`tests/test_full_variant_table.py` fails otherwise),
  `generate_paper_ratio_table.py`, and after editing
  `evaluation/data/paper_reported_numbers.json`:
  `generate_reproduction_status.py`, `generate_paper_comparison.py`.
- Quick runs without a manifest:
  `./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> 0 --methods <m> --no-gt-seed --summary-json out.json <flags>`.

### Data

- KITTI Odometry scans: `evaluation/scripts/fetch_kitti_odometry_sequences.py --output-root <root> --sequence NN`
  (HTTP range requests, about 2 GB per 1000 scans; 02 is 8.9 GB);
  `dogfooding_results/kitti_seq_NN_full` must point at
  `<root>/dataset/sequences/NN/velodyne`. Ground truth is committed.
  Fetch, run and delete one sequence at a time if disk is tight.
- One paper row end to end: `evaluation/scripts/reproduce_paper_row.py --method <m> --sequence NN --kitti-root <root> --fetch`.
- NCLT: `prepare_nclt_inputs.py`; scans are body frame, so add
  `--input-sensor-pose 0 0 -0.957 180 0 0`.

### Rules that keep the numbers trustworthy

- One setting per method across sequences; record which sequence a value was
  chosen on, treat the rest as held out, and say so when a held-out
  sequence influenced a choice.
- Run every sequence that has a paper value. Both recent failures (DiLO 01,
  IMLS-SLAM 04) appeared only on sequences nobody had run.
- Windows are for direction, full sequences for claims (a Mesh-LOAM setting
  fixed a 550-scan window and failed the full run).
- New behaviour that is not in the paper is opt-in and default off, so
  stored results do not change; the method README says it is not in the
  paper.
- Negative results go in the method README with numbers.
- Before a port "fails on a new sensor", check frame, field of view,
  intensity scale and unknown `--*-paper-set` keys
  ([kitti_pitfalls.md](kitti_pitfalls.md), section 6).

## 3. Plan

Phases are ordered by value per effort; items inside a phase are
independent. Each item says what "done" means.

### Phase 1: coverage (finish what the table promises)

1. **Completed: Mesh-LOAM and InTEn-LOAM on KITTI 00-10.** Used the selected 00/07 variant
   unchanged: Mesh-LOAM `paper_odometry_elevation`
   (`experiments/mesh_loam_kitti_seq_{00,07}_full_paper_comp_matrix.json`),
   InTEn-LOAM `paper_odometry_plane_check`
   (`experiments/inten_loam_kitti_seq_{00,07}_full_paper_comp_matrix.json`).
   The 00 manifests were copied for 01-06 and 08-10 with only that variant.
   Existing committed 00/07 results were reused unchanged; the added full
   sequences used one setting per method, fetching and deleting scans one
   sequence at a time.
   - Results imported; Table 6, the README rows, method README sequence
     tables and `paper_reported_numbers.json` updated. Table 6 pins these
     methods to the fixed variants rather than selecting among ablations.
   - Mesh-LOAM: 1.28x geometric mean, no sequence above 2x. InTEn-LOAM:
     2.19x; 01, 02 and 10 exceed 2x. Their README records drift intervals,
     block errors and counts of steps off by more than 20 %.
   - Commands, run provenance, cleanup and checks:
     [`experiments/results/mesh_inten_kitti_coverage/`](../experiments/results/mesh_inten_kitti_coverage/README.md).
2. **Completed: A-LOAM, LF-GICP, L-LO and Quadric-LO fixed-setting coverage.**
   A-LOAM `kitti_default`, LF-GICP `paper_default`, L-LO `seq07_tuned` and
   Quadric-LO `dense_profile` each use their existing arguments unchanged.
   Matching committed full-sequence results are reused; the missing
   sequences are added with one sequence of scans on disk at a time.
   L-LO excludes 01, which is NA in the paper; the other methods cover 00-10.
   - Table 6 pins these variants; README rows, method sequence tables,
     negative-result diagnostics and paper metadata are updated.
   - Selection provenance includes the earlier 02/08 observations that
     influenced LF-GICP calibration and the earlier Quadric-LO 02 evidence.
   - Commands, raw evidence, cleanup and checks:
     [`experiments/results/remaining_kitti_coverage/`](../experiments/results/remaining_kitti_coverage/README.md).
3. **Keep the leaderboard reproducible.** No README cell is currently marked
   † (not re-run since per-run provenance was added). After changes to
   shared code (`common/`, the runner's loaders), run
   `evaluation/scripts/audit_kitti_drift.py`, which re-runs the README
   leaderboard's KITTI cells
   ([last audit](../experiments/results/kitti_drift_audit.json)).
   - [Sequence-at-a-time audit procedure](kitti_drift_audit.md): inspect cells
     with `--list`, select with `--sequence`, and keep each report with its
     replay logs. Filtered audits require an explicit output path. Drift and
     replay failure return nonzero exit codes; RPE, ATE and frame count are
     checked. This tooling update does not constitute a new full KITTI audit.
   - Done: the audit shows no drift, or the drift is documented.

### Phase 2: usability

4. **Sensor setup assistant.** Extend
   [`check_scan_frame.py`](../evaluation/scripts/check_scan_frame.py) into a
   tool that reads a few scans and prints:
   - the sensor pose in the stored frame;
   - the field of view and row count;
   - the intensity range;
   - the fraction of returns within 5 m;
   - for each range-image or ground-model method, the flags to use
     (`--input-sensor-pose`, `--elo-fov`, `--suma-paper-set fov_up_deg=...`,
     `--dilo-paper-top-elevation/height/max-fill-gap`, InTEn-LOAM
     `theta_*`, ...).
   - Done: unit tests, and on KITTI 00 and NCLT it prints the settings used
     in [nclt_paper_paths.md](nclt_paper_paths.md) (KITTI: identity pose,
     +2 / -24.9 deg). The HDL-64E's two beam blocks defeat the periodicity
     score; handle unevenly spaced sensors (for example from the elevation
     histogram's extent instead).
   - Implemented: intensity-preserving readers, row/coverage evidence,
     explicit frame confidence and per-method sensor flag candidates;
     [usage and limits](sensor_setup_assistant.md).
   - Unit tests and KITTI 00 sample validation pass. **NCLT real-data
     validation is still pending**: the official download host is not yet
     in the environment's enforced allowlist, despite approval to add it.
     Synthetic inverted/translated-beam tests do not close that check.
     [Commands and reports](../experiments/results/sensor_setup_assistant/README.md).
5. **Speed of the paper pipelines.** Mesh-LOAM (0.3-1.2 FPS), IMLS-SLAM
   (0.3-1.1 FPS) and InTEn-LOAM (1.6-2.4 FPS) are too slow for routine
   full-sequence runs.
   - Profile first; likely wins are neighbour search and parallel
     per-sample work.
   - Done: at least 3x faster, with official RTE on 07 unchanged to the
     third decimal (or every change explained).
   <!-- PAPER-PIPELINE-SPEED:START -->
   - Full KITTI 07 experiment: Mesh-LOAM 0.83x, IMLS-SLAM 2.01x, InTEn-LOAM 1.70x.
     Official RTE agrees to the third decimal; saved matrices match at their
     recorded precision. **The 3x speed target is not complete.**
   - Mesh's slower candidate is excluded; its original implementation is
     retained. IMLS-SLAM and InTEn-LOAM acceleration remains opt-in and default
     off. Their selected registration arguments are unchanged; 07 is
     development data. Negative tree, packet-cache and scheduling trials
     remain in the method READMEs.
   - Primary ratios use fresh frozen-source full controls immediately before
     each candidate run, with the profiler disabled and fixed thread counts.
     [Commands, profiles, comparisons and limits](../experiments/results/paper_pipeline_speed/README.md).
   - The subsequent shared-component integration preserves the fixed arguments
     and passes frozen-main/default/flag real-fixture comparisons. These are
     compatibility checks; the timing ratios above remain attached to the
     original measured source and have not been remeasured on the merged source.
     [Integration evidence and limits](../experiments/results/paper_pipeline_speed/README.md#integration-with-merged-shared-components).
   <!-- PAPER-PIPELINE-SPEED:END -->

6. **Shared robustness components.** Move the opt-in fixes into `common/`
   so other methods can use them:
   - degenerate-direction remap (DiLO);
   - coarse first registration (IMLS-SLAM);
   - near-range cut (runner).
   - Then evaluate them on the methods that fail in the same way: SuMa 01,
     ELO and DiLO on NCLT.
   - Done: one implementation each with tests, used by at least two methods,
     defaults unchanged.
   - First component implemented and evaluated in [PR #135](https://github.com/rsasaki0109/localization_zoo/pull/135):
     the DiLO translation remap is an Eigen-only helper used by DiLO and
     opt-in SuMa, with mechanism and integration tests. The fixed value 0.03,
     transferred from the existing DiLO 01 selection without SuMa tuning, was
     run on full KITTI 00-10. SuMa 01 worsens from official RTE 13.792690% to
     95.257580%; this transfer is rejected as a general setting and defaults
     stay off. The method README retains every sequence and its negative
     diagnostics. All 22 affected existing DiLO/SuMa cells match RPE, ATE,
     frame count and official translation/rotation RTE.
     [Protocol, committed results and commands](../experiments/results/shared_degeneracy_remap/README.md).
     Near-range sharing and NCLT progress are tracked below.
   - Second component implemented and evaluated in [PR #136](https://github.com/rsasaki0109/localization_zoo/pull/136):
     IMLS-SLAM and opt-in SuMa share the initial coarse/fine stage policy.
     Per-method correspondences, residuals and numerical stopping remain local;
     this is not a shared numerical aligner. The fixed budget and distance scale
     derive from the existing IMLS 04 choice without a SuMa sweep. Full KITTI
     00-10 paired results and saved-pose diagnostics are committed, including
     every negative pair. SuMa 01 RTE changes from 13.792690%
     to 10.686955%, but ATE remains 150.500523 m
     and later drift remains. No setting is reselected and defaults stay off.
     All 22 affected existing cells match
     RPE, ATE, frame count and official translation/rotation RTE.
     [Protocol, completed results and commands](../experiments/results/shared_coarse_start/README.md).
     Near-range sharing and NCLT progress are tracked below.
   - Third component implemented in [PR #137](https://github.com/rsasaki0109/localization_zoo/pull/137)
     and its frozen KITTI audit is complete: the optional sensor-frame near-range
     cut is shared in `common/range_filter.h`, used by ELO/DiLO and intensity-preserving
     loaders. Float arithmetic, strict boundary, processing order and zero/off
     default remain unchanged; standalone cores do not apply it automatically.
     All 177/177 selected full-sequence cells across KITTI 00-10 finish
     with native exits 0: 142 historical matches and 35 differences.
     All 37 frozen pre-extraction controls have zero recorded metric
     differences and byte-identical saved poses. Historical audits retain exit 1;
     their original causes remain unresolved. The controller and full report
     finish with exit 0; official input hashes were verified before deleting each
     owned sequence and alias. These are historical per-cell settings, not a new
     fixed-setting accuracy study. All improving and worsening pairs are retained
     in method READMEs; published values and defaults are preserved.
     [Full report, every numerical pair, commands and saved-path diagnostics](../experiments/results/shared_near_range/README.md)
     retain the complete evidence, including all error-growth blocks and available
     historical pairs. Missing exact paths and differing runtimes limit causal
     interpretation. The environment interruption, exact binary restoration and
     fresh restart are retained in the linked recovery record.
     Native source remains `0b079dbb`; this is not a whole-zoo measurement of
     newer merged acceleration/tracing. Fresh real-NCLT validation remains open
     because the enforced official-host allowlist is unresolved. Its historical
     DiLO cut/uncut pair also changes range-image geometry and worsens ATE, so it
     cannot isolate the cut. Continuous natural InTEn failure tracing is a separate
     next task, not an unfinished part of this KITTI audit.
7. **Completed: paper-pipeline bindings and ROS 2 nodes.** Python now exposes
   SuMa frame-to-model, IMLS-SLAM raw-scan model, DiLO range-image and LiTAMIN2
   Gaussian-map odometry, alongside the existing registration bindings.
   - [Python examples](../python/README.md#stateful-paper-pipelines) and
     [ROS 2 examples](../ros2/localization_zoo_ros/README.md#paper-odometry-nodes)
     cover each method. Separate node/class names preserve existing interfaces.
   - Native defaults and opt-in correction defaults are preserved; no sensor
     profile or ground-truth seed is silently added. ROS output uses acquisition
     stamps and the stored scan frame; the input QoS is configurable explicitly.
   - Fixture checks compare Python diagnostics/poses and real ROS odometry/TF
     with native C++. The identity DiLO result on the real fixture and the
     incomplete best-effort ROS trial remain documented; a moving synthetic
     fixture also exercises non-identity DiLO output with the same defaults.
   - [Commands, committed parity reports, failures and scope](../experiments/results/paper_pipeline_bindings/README.md).
   - [Integration with the merged acceleration and passive trace stack](../experiments/results/paper_pipeline_bindings/README.md#integration-with-the-merged-acceleration-and-passive-trace-stack)
     retains native defaults and existing benchmark evidence. IMLS-SLAM's
     accelerated execution option remains available through the native API and
     runner; these Python/ROS interfaces keep it disabled.

### Phase 3: beyond KITTI

8. **NCLT leaderboard for the zoo.** Run the main methods (A-LOAM, F-LOAM,
   LeGO-LOAM, KISS-ICP, CT-ICP, MULLS, LF-GICP, the paper pipelines) on NCLT
   2013-01-10 full with the sensor settings.
   - Publish a table next to the KITTI one (and in the explorer if it fits).
   - Done: one committed results set, commands and settings on the page.
9. **Unsolved NCLT failures.**
   - SuMa fails at the first restart after a stop. Prediction is not the
     cause; a 5 m cut fixes a window but not the full run.
   - DiLO drifts in rotation (6.8 deg/100 m).
   - Mesh-LOAM: no setting found.
   - InTEn-LOAM: S2S steps 28 % short on the first 600 scans, ATE 49 m.
   - Done per method: a cause with evidence, or a fix with full-run numbers.
10. **A third dataset** (Boreas windows and MulRan are partly wired:
    `fetch_boreas_window.py`, the MulRan evidence snippets).

### Phase 4: remaining gaps to the papers

11. **ELO 1.22x.** Spread over all sequences, not a failure. The paper
    fusion weight w2 is ambiguous, and the reading implemented here does not
    help ([papers/elo/README.md](../papers/elo/README.md)).
12. **InTEn-LOAM 2.19x on 00-10 with one setting.** Large motion deficits
    appear in 01 frames 217-253 / 420-518 and 02 frames 1878-1943 /
    4460-4524. Sequence 10 reaches 2.57x without any moving step exceeding
    20 % length or vector error; its error grows through smaller increments.
    These saved-pose diagnostics locate the failures; the solver-level cause
    remains open. An [offline log/pose join](../experiments/results/inten_support_diagnostics/README.md)
    finds 14 logged bad-length steps each in 01 and 02; all have S2S/S2M
    counts above the low-count cutoff. That excludes low-count fallback at
    those frames only: 122 / 119 other bad steps are unlogged, and high counts
    do not establish conditioning or correct associations. No setting is changed.
    The plane check and correction were chosen on 07 windows
    with 00 influencing the diagnosis, so 00 is not fully held out.
    An opt-in [passive C++ solver trace](inten_loam_passive_trace.md) now records
    the normal S2S/S2M stages, fallback and acceptance decisions, normal
    equations and residuals without a forced pose. Moving synthetic parity
    and short-fixture compatibility checks preserve the existing behavior;
    continuous failure-sequence tracing is now complete (below), while the
    causal solver explanation remains open.
    [Validation evidence and commands](../experiments/results/inten_passive_trace/README.md).
    An [offline conditioning analyzer](../experiments/results/inten_conditioning/README.md)
    now specifies sensor-centred tangent coordinates and a required rotation
    length, with every stored iteration retained. Short fixtures and synthetic
    coordinate/scale/degeneracy tests validate this interpretation; they do not
    establish the full-sequence solver cause or alter estimator gates.
    The default-off evaluation CLI can also stream the observation records;
    [export protocol and validation status](../experiments/results/inten_trace_export/README.md)
    record short-fixture trace-on/off pose parity and explicit skipped-input
    handling separately from full failure-sequence diagnosis.
    An [offline streaming checker](../experiments/results/inten_trace_validation/README.md)
    validates completion, stage structure and native run receipts before
    analysis; it adds no estimator gate or failure-cause finding.
    A [fixed-equation origin experiment](../experiments/results/inten_origin_invariance/README.md)
    establishes origin dependence in the native finite update and coordinate
    damping, with exact-exponential and transported-penalty controls. The
    method README retains the measured differences. No estimator is changed;
    its causal relevance to KITTI failures is not established by the subsequent
    natural traces.
    **Continuous natural collection and stage diagnosis are complete:** full
    KITTI01/02/10 trace-off/on pairs use the unchanged 07-selected setting
    (00 influenced the earlier diagnosis). All 6 native calls exit 0, with
    byte-identical paired poses and no skipped inputs. Every one of 126966
    systems has numerical rank 6 at the declared sensor-centred convention.
    All 136 bad-length steps in 01 and 132 of 133 in 02 are already bad at
    S2S output; S2M does not recover the sustained collapse intervals. No
    low-count fallback occurs anywhere in these executions. KITTI10 has
    no 20% step event and retains gradual drift. Commands, all frames/iterations,
    negative findings and verified scan cleanup are in the
    [completed study](../experiments/results/inten_natural_traces/README.md).
    The subsequent [same-pair seed interventions](../experiments/results/inten_seed_intervention/README.md)
    reproduce natural S2S outputs exactly on all 360 selected pairs. Reusing
    prior mapped motion leaves 136/134 selected S2S bad-length outputs in 01/02, versus
    139/136 naturally; a current-GT oracle still leaves 1/33. At the 02 collapse
    entries 1876/4460, the GT-seeded solver shortens the step to 0.785598/0.404926
    of GT, with zero initial edge support and a backward ground contribution
    in the initial joint system. Initialization alone is insufficient there.
    These are diagnostic interventions, not new odometry scores or a recursive
    predictor evaluation. All raw outcomes and negative cases are retained.
    The [actual plane-correspondence study](../experiments/results/inten_plane_correspondences/README.md)
    completes that inspection on the same 360 GT-seeded pairs, reproducing
    native outputs and initial systems exactly. Ground-PCA first-system controls
    reduce the initial backward correction at the 02 entries but leave it
    backward; adverse cases occur on every sequence. No PCA fix is adopted.
    Query inspection finds additional in-gate triangles beyond the ten native
    candidates: at 02 / 1876 and 4460, 379/337 extra ground triangles versus
    247/260 accepted natively. Many other row-support losses remain even with
    the full gated target. This identifies actual candidate truncation, not a
    proven improvement. All geometry, negative controls and commands are retained.
    The [uniform opt-in row-support experiment](../experiments/results/inten_row_support/README.md)
    is complete on full KITTI00-10: one fixed inherited 07-selected profile,
    the same acceleration in both arms, and no retuning. The candidate only
    completes missing S2S ground/facade row support under the existing gates;
    it is outside the paper and default off. Candidate/reference official RTE
    has geometric mean 1.010678x: 3 sequences improve and 8 worsen. In particular,
    01 changes 16.613803% to 16.877996% and 02 changes 3.221230% to 3.405644%;
    sustained collapse remains. This candidate is not adopted as a general fix.
    Every adverse metric, full error-growth block and execution command is
    retained. Fresh reference RTE matches every historical primary aggregate;
    the full 07 parent control and all 360 default-off pair controls agree.
    Known 01/02 and the previously inspected 10 are development evidence, not
    an independent held-out evaluation.
    The [recursive mapped-motion seed comparison](../experiments/results/inten_mapped_prior_rigid/README.md)
    now also covers every full KITTI 00-10 sequence with one fixed setting.
    It reuses the previous final mapped motion only as the next S2S seed;
    fallback and deskew still use the original accepted S2S cache. This is
    outside the paper and default off. An initial unprojected version aborted
    on 00 with exit 134 as non-rigid roundoff amplified; that failed execution
    is retained. Restoring the seed rotation through a normalized quaternion
    gives a complete fresh comparison: geometric mean candidate/reference RTE
    0.904792x, with 5 lower and 6 higher sequences. 01 improves 16.613803% to
    5.180428%, but 50 bad-length steps remain; 13 newly bad frames accompany
    99 reference-only bad frames. 02 changes 3.221230% to 3.073932% with 128 bad
    steps remaining, including one newly bad frame. 10 worsens 1.155652% to
    1.183142%; its smaller accumulated errors remain. 01/02/10 still exceed
    twice the cited paper values. This is partial progress, not a general fix;
    no per-sequence winner is adopted. All adverse metrics, new local failures,
    error-growth blocks and commands are retained. The inherited values were
    chosen on 07; 00 informed diagnosis and the numerical correction. Previously
    inspected 01/02/10 are development evidence, with no independent held-out
    claim. All fresh reference RTE values match the historical primary results.
    The remaining association/solver failure needs further actual improvement
    work. Defaults, earlier studies, historical results and Table 6 are preserved.
    The [weak-translation prediction-prior comparison](../experiments/results/inten_translation_prior/README.md)
    also covers every full KITTI00-10 sequence with a fixed new ratio0.03
    versus zero, with the preceding mapped-motion seed and acceleration on
    in both arms. This outside-paper extension remains default off. Its
    quadratic position anchor adds information only in weak sensor-centered
    marginal translation directions; S2M and associations are unchanged.
    Geometric mean candidate/reference official RTE is 0.946172x:
    2 lower, 0 higher and 9 unchanged sequences.
    Failure-sequence RTE: 01 5.180428% to 5.173517%; 02 3.073932% to 1.674736%; 10 1.183142% to 1.183142%.
    Sequences still above twice the cited paper RTE: 01, 02, 10.
    Every adverse metric, newly bad local step and error-growth block is
    retained. This comparison alone does not establish correspondence
    correctness or a general failure remedy; no per-sequence option is
    adopted. The original profile was chosen on07, with00 informing
    diagnosis. The fraction was borrowed from the existing DiLO KITTI01-tuned
    control, fixed before all new InTEn runs and not retuned. Here it is a
    soft marginal-information floor, not DiLO hard remapping. Known failures
    remain development data, with no independent held-out claim.
    Numerical evidence `16577120` precedes this interpretation.

    The [full iterative ground-PCA comparison](../experiments/results/inten_ground_pca/README.md)
    uses one fixed setting on every full KITTI00-10 sequence. Only the
    normals of already accepted S2S ground triangles change, using the
    same gated ten neighbours and original closest-point plane anchor.
    PCA degeneracy retains the triangle. Mapped-motion prediction, soft
    translation prior0.03 and acceleration are on in both arms; row
    completion remains off. This outside-paper extension is default off.
    Geometric mean candidate/reference RTE is 0.924765x:
    7 lower, 4 higher and 0 unchanged sequences.
    Failure-sequence RTE: 01 5.173517% to 5.276027%; 02 1.674736% to 0.761893%; 10 1.183142% to 1.134467%.
    Sequences above twice the cited paper RTE: 01, 10.
    All adverse metrics, new local bad frames and error-growth blocks
    are retained. No per-sequence winner is adopted or default promoted.
    Known-failure first-system controls motivated this intervention;
    all prior outcomes are development evidence, with no independent
    held-out claim. Profile chosen on07,00 informed diagnosis; the
    inherited prior fraction came from DiLO KITTI01 tuning. This tests
    the actual iterative trajectory, not physical-ring correctness.
    Numerical evidence `a9603c00` precedes this interpretation.

    The [full iterative facade-PCA comparison](../experiments/results/inten_facade_pca/README.md)
    uses one fixed setting on every full KITTI00-10 sequence. Only the
    normals of already accepted S2S facade triangles change, using the
    same gated ten neighbours and original closest-point plane anchor.
    PCA degeneracy retains the triangle. Mapped-motion prediction, soft
    translation prior0.03, ground PCA and acceleration are on in both arms; row
    completion remains off. This outside-paper extension is default off.
    Geometric mean candidate/reference RTE is 0.971379x:
    8 lower, 3 higher and 0 unchanged sequences.
    Failure-sequence RTE: 01 5.276027% to 4.481189%; 02 0.761893% to 0.756938%; 10 1.134467% to 1.093310%.
    Sequences above twice the cited paper RTE: 01, 10.
    All adverse metrics, new local bad frames and error-growth blocks
    are retained. No per-sequence winner is adopted or default promoted.
    Committed current01 onset controls motivated this intervention;
    all prior outcomes are development evidence, with no independent
    held-out claim. Profile chosen on07,00 informed diagnosis; the
    inherited prior fraction came from DiLO KITTI01 tuning. This tests
    the actual iterative trajectory, not physical-ring correctness.
    Numerical evidence `f6e023be` precedes this interpretation.

    The [existing S2M plane-spread comparison](../experiments/results/inten_map_plane_spread/README.md)
    runs the same immutable native binary on every full KITTI00-10
    sequence, changing only the existing minimum plane-spread ratio
    from zero to0.2. This outside-paper option remains default off.
    The value comes unchanged from the existing synthetic street test;
    earlier07 exploratory exposure is disclosed. Both arms retain
    mapped prediction, S2S prior0.03, ground/facade PCA and acceleration;
    row completion stays off. No production source is changed.
    Geometric mean candidate/reference RTE is 1.036694x:
    2 lower, 9 higher and 0 equal sequences.
    Failure-sequence RTE: 01 4.481189% to 5.161510%; 02 0.756938% to 0.784905%; 10 1.093310% to 1.187067%.
    Sequences above twice the cited paper RTE: 01, 10.
    All adverse metrics, new local failures and error-growth blocks
    are retained. Every fresh baseline numeric trace and saved pose/GT
    exactly matches the preceding candidate. Current01 normal-prefix
    controls reproduce the actual S2M map history and output, and
    terminal matched spread controls retain positive and negative
    endpoints. Their diagnostic producer never supplies accuracy scores.
    Rejected support is not proof of physical correspondence truth.
    All prior KITTI outcomes are development evidence, without an
    independent held-out claim or per-sequence adoption. Profile07,
    diagnosis00 and inherited DiLO01 prior tuning are disclosed.
    Numerical evidence `da4c325b` precedes this interpretation.

    The [current01 scene-motion diagnosis](../experiments/results/inten_current_correspondences/README.md)
    reproduces all434 normal-prefix poses and solver traces exactly and retains
    all2766 terminal matched-seed cluster controls. At204, removing one inspected
    right-side facade region changes S2S length/GT0.607064→1.004733. Independent
    full-point profiles require an additional-1.60m right-side shift after GT,
    while the opposite side is near zero; this supports local moving-surface
    correspondence contamination. The same region worsens247,
    0.522727→0.039218, and does not undo the already collapsed205 seed.
    All adverse/neutral/oracle cases and empty-region failures are retained.
    This region was selected on committed204 development geometry, is not a
    proposed estimator, and changes no production default or full-sequence score.
    Automatic static-support selection, full01 recovery and10 gradual drift remain
    open. Any new accuracy setting requires all full00–10 fixed-arm evidence.
    Numerical commits96197955/59925767 precede this interpretation.

    The [predicted-DOR facade-gate comparison](../experiments/results/inten_prediction_dor/README.md)
    freezes one new default-off Boolean across every full KITTI 00-10 sequence.
    Existing DOR thresholds remain unchanged; at the initial prediction with
    the prior local map, only marked source facade S2S queries are rejected.
    Extraction, other query classes and S2M/insertion rules keep their original
    definitions; later trajectory/map feedback changes actual histories.
    Geometric mean candidate/reference RTE is 1.142753x:
    3 lower, 8 higher, 0 equal sequences.
    Failure-sequence RTE: 01 4.481189% to 16.682628%; 02 0.756938% to 0.768594%; 10 1.093310% to 1.208837%.
    Above twice paper: 01, 10.
    Every adverse metric, new bad frame and error-growth block is retained.
    Full01 detector masks and terminal controls were committed before selection;
    terminal repairs did not establish recursive recovery. All prior KITTI is
    development, with profile selection on 07 / diagnosis on 00 and inherited DiLO 01 prior tuning
    disclosed. No per-sequence adoption or default promotion.
    Numerical evidence `993b539f` precedes this interpretation.

    The [facade Cauchy influence comparison](../experiments/results/inten_facade_loss/README.md)
    freezes one default-off Boolean on every full KITTI00-10 sequence.
    Only S2S facade IRLS influence changes from Huber to Cauchy at the same
    inherited native scale0.1; association gates and S2M definitions stay fixed.
    Adaptive-prior information and later trajectory/map feedback can change.
    Geometric mean candidate/reference RTE is 1.231530x:
    3 lower, 8 higher, 0 equal sequences.
    Failure-sequence RTE: 01 4.481189% to 38.696732%; 02 0.756938% to 0.776016%; 10 1.093310% to 1.182742%.
    Above twice paper: 01, 10.
    Every adverse metric, new bad frame and error-growth block is retained.
    Full01 matched-seed controls preceded selection:204 improved locally,
    while247 worsened and36 of48 bad baseline pairs worsened in length.
    These terminal results did not establish recursive recovery. Profile07,
    diagnosis00 and inherited DiLO01 prior tuning are disclosed; all prior
    KITTI is development. No scale sweep, per-sequence adoption or promotion.
    Numerical evidence `c2775241` precedes this interpretation.

    The [automatic cluster-geometry feasibility trial](../experiments/results/inten_cluster_motion/README.md)
    completes every inherited current01 endpoint with one frozen full-point
    translation-profile mask, no hand region or GT in its decision. All1359 cluster
    profiles and16 matched-seed native calls are retained with exact baseline
    controls. At204 length/GT improves0.607064→0.971692 but rotation worsens;
    at247 length/GT worsens0.522727→0.046618. The rule is not adopted. Numerical
    snapshot `58240931` precedes interpretation; all prior KITTI is development.
    Next, distinguish shared ego-prediction error from independent moving support.
    Full01 recovery and10 gradual drift remain open; no production/default change
    or full-sequence accuracy claim follows from these terminal controls.

    The [shared-translation consensus factorial](../experiments/results/inten_translation_consensus/README.md)
    separates seed correction from support removal at every inherited01 endpoint,
    with the previous rejected mask as an exact negative control. All40 terminal
    calls and actual first-query equations are retained. At205 seed-only length/GT
    is0.936491, mask-only0.394997 and combined0.955054, from0.372446 original.
    A majority fallback avoids extra247 harm but leaves its original failure.
    Combined translation errors improve at6 endpoints and tie at2;204 rotation
    worsens1.716533→2.958909mrad, failing the predeclared progression condition.
    No gate retuning, production option or full-sequence accuracy claim follows.
    Seed rescue is conditional on fixed original history and also changes the
    prior center. Next address the rotation/support tradeoff while retaining this
    effect. Full01 recovery and10 drift remain open. Numerical `ca5bccf9`
    precedes interpretation; every prior KITTI observation remains development.


    The [fixed estimated-rotation factorial](../experiments/results/inten_fixed_rotation/README.md)
    retains all48 calls, including exact original/preceding joint controls and a
    complete translation-only seed×mask factorial. At204 length/GT improves
    0.607064→1.002040 with original1.716533mrad rotation;205 reaches0.954865.
    The fixed local gate passes:6 translation improvements,2 fallback ties,
    rotation bit-for-bit original. Negatives remain: raw247 worsens0.522727→0.420339,
    and holding original205 rotation forfeits the prior joint1.798922mrad gain.
    No full trajectory, production option or accuracy claim. Next freeze recursive
    estimated-rotation/consensus behavior and run one setting across all full00-10;
    retain unresolved247 and10 drift. Numerical `0e2ab939` precedes prose;
    every prior KITTI observation remains development.

    The [recursive fixed-rotation consensus comparison](../experiments/results/inten_consensus_query_acceleration/README.md)
    evaluates one default-off Boolean on every full KITTI 00-10 sequence.
    The unchanged consensus seed/support rule re-solves translation at the
    original estimated rotation; realized map/prediction feedback can change.
    Geometric mean candidate/reference RTE: 1.106697x;
    4 lower, 7 higher, 0 equal sequences.
    Failure-sequence RTE: 01 4.481189% to 8.021836%; 02 0.756938% to 1.143779%; 10 1.093310% to 1.172497%.
    Above twice paper: 01, 02, 10.
    Every adverse metric, new bad frame and error-growth block is retained.
    Cached local feasibility was conditional; this full experiment includes
    changed history. Local fixedR does not guarantee global mapped rotation.
    Original 07/00/DiLO 01 provenance and all past KITTI development disclosed.
    No GT input, threshold sweep, retuning or per-sequence adoption.
    Numerical `b64bbf82` precedes interpretation.

    The [archived consensus stage diagnosis](../experiments/results/inten_consensus_failure_origin/README.md)
    checks every frame of both fixed full00–10 arms. It separates the actual
    corrected seed from the original joint output and accepted refinement,
    retaining all same-frame damage, rescues and remaining bad cases.
    Some new collapse entries already follow a good joint estimate, so
    changed mapping history alone is insufficient to explain them.
    No safeguard is adopted or new accuracy score claimed; native
    same-history rescue and newly adverse controls must precede another
    recursive trial. GT remains offline diagnostic only.
    Numerical `f391d551` precedes interpretation.


    The [native zero-majority safeguard](../experiments/results/inten_zero_consensus_control/README.md)
    is rejected by its frozen gate. All31 conditional endpoints/248 native calls
    preserve exact original/old-consensus same-history controls and every outcome.
    All7 immediate01/02 length collapses improve to non-bad, but original200/202/203
    worsen, actual02/1891 newly collapses0.980923→0.196111 length/GT, and none of7
    immediate08 entries is fixed. At02/1891 the old seed-only correction rescues
    the already-short joint estimate, despite zero-majority support. At08 zero
    can already be selected while direction/history is poor. Next distinguish
    harmful cancellation from necessary correction using estimated inputs; retain
    these adverse controls before any new recursive trial. No production option,
    full-score claim, threshold retuning or paper-table change. Numerical
    `883aa3d6` precedes interpretation; every prior KITTI observation is development.


    The [native non-facade support veto](../experiments/results/inten_nonfacade_consensus_support/README.md)
    also fails its prospective gate: all31 endpoints/93 terminal calls and62
    passive attribution queries remain. It loses204/205 and02/1891 rescues,
    yet accepts02/1892 length/GT1.015129→0.125855 with complete support because
    reflector-loss improvement outweighs worsening edge/ground losses.
    Passive fresh association exposes original-support bias and changed counts;
    raw loss decrease is not a motion-accuracy witness. Next inspect longitudinal
    information, intensity ambiguity and support changes in accepted-reference
    failures rather than promote another aggregate-loss safeguard. No production
    option, relaxed gate, recursive trial or paper-table change. Numerical
    `82c98752` precedes interpretation; every prior KITTI observation is development.

    The [native intensity continuity audit](../experiments/results/inten_intensity_continuity/README.md)
    identifies a concrete representation defect: query-local mean fill changes
    shared missing cubic controls across valid cell faces. Dense-ramp and
    sparse-constant controls are continuous, while the sparse nonconstant native
    control jumps0.023746193. All31 inherited endpoints/8829 reflector paths and
    208565 passive real native queries are retained;17658 endpoint controls
    reproduce the parent. Of15763 both-valid crossed faces,15428 exceed a1e-8
    intensity jump, maximum0.163630951. Within-cell derivatives pass; missing
    gradient scaling is not the cause. Mixed/invalid and zero-crossing controls
    remain. This does not establish trajectory causation or a better estimator.
    Next freeze a consistent-control construction and coverage policy, keep it
    opt-in/default-off, and check every inherited rescue/adverse case before
    another full fixed-setting run. Numerical `63951376` precedes interpretation;
    all prior KITTI observations are development. Defaults and Table6 are unchanged.

    The [normalized S2S intensity candidate](../experiments/results/inten_intensity_normalized/README.md)
    is implemented opt-in/default-off and rejected by its fixed motion gate.
    Every31 inherited endpoint runs all4 joint/consensus × legacy/normalized
    arms:124 outcomes, all4 replay and5 field-probe subprocesses exit0, and62
    legacy traces match exactly. The rational
    occupied-weight field is continuous at15758 positive-weight both-valid faces,
    but5 zero-weight holes still have different neighboring limits, up to
    0.169819977. No global-continuity claim. Joint bad-length cases increase7→8;
    total conditional length error4.741213404→5.121149401 and vector error
    14.083975645→14.443564156 worsen. Actual08/39 becomes newly bad, and original
    247 worsens length/GT0.522727335→0.105443532. All cases, initial numerical
    failure, stable full rerun and exact commands remain. No threshold retuning,
    full trajectory trial, accuracy claim or paper-table change follows.
    Numerical `3055bb2e` precedes interpretation; all prior KITTI is development.
    Next evaluate a query-independent node completion that explicitly addresses
    the zero-weight holes and retains every rescue/adverse case. Smooth covered
    interpolation alone has not remedied the motion failures.


    The [fixed-node completion trial](../experiments/results/inten_intensity_consistent/README.md)
    now addresses the remaining zero-weight holes, S2S-only/default-off and
    outside the paper. Radius three follows from cubic support; observed means
    are unchanged, missing values use original observations without cascading.
    Every31 endpoint runs all6 arms (186 outcomes); four replay and five field
    processes exit0. All124 inherited legacy/normalized traces match exactly.
    All208565 queries reconstruct; all15763 both-valid faces, including the
    previous five holes, have no intensity/gradient jump above1e-8. Mixed184
    and neither-valid191 count-coverage faces remain. Motion gate still fails:
    joint length error4.741213404→4.804046127 and vector error
    14.083975645→14.133971193 worsen; bad labels stay7/14. Original247
    length/GT worsens0.522727335→0.464794342. Numerical `6eec3472` precedes
    interpretation. All prior KITTI is development. No full run or paper-table
    change. Next isolate the remaining motion/geometry failure with all controls;
    consistent sparse interpolation alone has not resolved it.

    The [fixed S2S budget trial](../experiments/results/inten_iteration_budget/README.md)
    tests10→100 using only the existing iteration cap, chosen on original01
    traces. All31 endpoints/all12 arms produce372 outcomes, four replay exits0.
    All186 inherited traces and all original joint prefixes are exact. Original247
    recovers length/GT0.522727335→1.023840306;202 and204 worsen to0.399223855
    and0.398534369.204 converges after23 iterations to that worse motion;
    convergence alone is not an accuracy witness. Actual08/39 becomes newly bad
    (0.801801894→0.797852053). Totals length4.741213404→4.865650571 and vector
    14.083975645→14.175508759 worsen; fixed gate fails. Numerical `c6832c2a`
    precedes interpretation. Default10/native solver/S2M/paper tables unchanged;
    no budget sweep or full candidate trial. All KITTI is development. Next test
    independent full-cloud evidence before selecting extended motion.

    The [independent dense witness](../experiments/results/inten_dense_continuation/README.md)
    tests one fixed bidirectional full-cloud selector on all31 endpoints/all12
    frozen poses:372 scores in four exit0 processes. Every cost/choice
    reconstructs independently from archived native centroids; cell bounds,
    population totals and coverage are checked. It rejects202/204's worse continuation, retains
    247's0.522727335→1.023840306 recovery and improves conditional totals:
    length4.741213404→4.308546562, vector14.083975645→13.618380519,
    rotation0.059478496→0.053191638rad. The frozen gate still fails because08/39
    newly crosses the length threshold0.801801894→0.797852053; it already had
    bad vector error, but the guard is not waived. All other adverse cases and
    secondary arms remain. Numerical `7b398407` precedes interpretation. No
    production selector/full trial/accuracy/SOTA claim or post-hoc retuning.
    Native outputs stay unchanged through the archived missing-SciPy import
    failure and existing-runtime reconstruction recovery. All KITTI is development.

    The [official GT input audit](../experiments/results/inten_gt_input_audit/README.md)
    rules out the suspected08 camera/LiDAR frame conversion defect: all11
    official pose/calibration regenerations exit0 and every numeric CSV field
    matches exactly;06/09 differ only in LF/CRLF line endings. Four portable
    input-integrity tests exit0. Numerical `4c3d972b`/`6802c1e0` precedes this
    interpretation. The08 vertical motion component is already present in the
    official source. No GT correction or reinterpretation of failed gates follows.

    The [reciprocal S2S support trial](../experiments/results/inten_reciprocal_support/README.md)
    fixes original202/204 conditionally (length/GT0.748597638→0.973614185 and
    0.607064437→0.985996689), but fails its frozen progression gate. Every31
    endpoint/all14 arms remains (434 outcomes); all372 inherited full traces
    are exact. All11,709,443 backward checks and every new update reconstruct.
    Vector total14.083975645→12.126070193 and rotation improve, but length total
    4.741213404→5.053685504 worsens.247 loses its uniform100 recovery;08/39,41–46
    become newly bad in length. Paired consensus retains original rescues.
    Numerical `f22127ab` precedes interpretation. No production integration,
    retuning, full candidate trial, official-GT change or paper-table change.
    Source, all raw measurement hashes and the frozen verifier survive lossless
    XZ transport; original gzip archives and initial build-directory failure remain.

    The [fixed-reference prior control](../experiments/results/inten_prior_control/README.md)
    separates reciprocal associations from first-system prior feedback. All31
    endpoints/all18 arms remain (558 outcomes); every434 inherited terminal trace
    and every old feature/profile/mask record is exact. Unfiltered common-prior
    negative controls are exact. All10,748 prior contributions/centers and every
    new mask/update reconstruct. Numerical `a358a572` and full-trace audit
    `2eca2585` precede interpretation. All23 old/01/08 endpoints have zero own/
    reference information and identical complete own/common-prior traces.
    Therefore202/204 recovery and247/08 worsening are not prior-strength feedback
    in these histories. Only eight02 endpoints change, with both signs retained.
    Primary length/vector totals5.053685504/12.126070193→5.027384128/12.098923936
    slightly improve, but length exceeds original10's4.741213404.247 recovery is
    still lost and08/39,41–46 stay newly bad length; the unchanged gate fails.
    No production integration, reference/fraction/mask/budget/gate retuning,
    full candidate trial, paper-table change or SOTA claim follows. The canonical
    omitted-zero-field failure remains before decoder-only recovery; native
    measurements, frozen mathematics and all original inputs stay unchanged.
    Full local Python410 tests exit0 (five existing environment skips), both CPU
    reconstructions match at original tolerances, and canonical source/results
    and regenerated tables stay byte-preserved (`9d20f6a8`).
    All KITTI is development; do not tune the prior to repair cases where its
    information is already zero.

    The [passive native support study](../experiments/results/inten_support_information/README.md)
    preserves all558 complete outcomes and original stdout bytes exactly while
    partitioning every query of the predeclared passive arms at matched poses.
    All42,992 class observations/27,360,255 repeated row observations reconstruct:
    3,637,657 valid residuals are removed. The frozen invalid/non-kept counter
    8,529,472 includes untested rows; complete raw audit `88738af2` separates
    8,185,839 tested rejections that also fail native validity from343,633
    empty-neighbor rows without backward testing. Numerical `a2fd354d`/all-case `dcfef100` precede prose.
    Initial kept/all longitudinal Schur fractions are0.569157979/0.595870322 at
    rescued202/204 but0.637155545/0.716923510/0.692297857 at adverse247/08-39/08-45.
    A larger retained fraction does not distinguish recovery. Raw facade axis gt
    changes0.888587984→-0.219212193 at202 but-5.95953280→-0.209317804 at08/39;
    the same predicate removes opposing/forward components respectively for an
    isolated displacement along this fixed axis. At247's late filtered pose edge gt reverses-0.745024029→
    0.203276439. These are coupled raw equation terms, not independent motion
    effects or proof of static objects. Every case/class/refinement remains.
    Full input/arithmetic/allocation failures survive verifier-only recovery;
    original strict thresholds/data stay fixed and final complete checks exit0.
    Full Python421 tests exit0 (five existing environment skips); all11 new
    regressions run. Canonical preservation and table/case regeneration pass
    (`8fa02101`/`88738af2`).
    Earlier candidate gates stay failed: no retuning, production/full trial or
    paper-table change. The following local rigidity study localizes those
    neighborhoods and tests a pose-free conditional equation. All KITTI is development.

    The [local rigidity/spatial study](../experiments/results/inten_local_rigidity/README.md)
    retains every inherited case/arm, all540 geometric histories,50,776,466
    eligible directed relations and247,032 first/last valid point rows. Numerical
    `7e8ef2ec` and complete every-case/region tables `d86b9c01` precede prose.
    One prospectively fixed source fanout10 applies everywhere. Primary initial
    facade kept/removed center medians are0.0938321054/0.136805516m at rescued202,
    but0.0778109345/0.177262541m at adverse08/39 and0.0628724483/0.0955789368m
    at08/45. Larger removed discrepancy appears with both signs of motion
    outcome; it does not supply a safe selector. At08/45 the largest-absolute-g
    removed source-facade region has gt-1.65775595 and discrepancy0.0789391984m,
    overlapping rescued202's0.0858749218m region with gt0.284267071. This
    descriptive location ranking retains every other source cluster/point.
    Rigid moving objects can preserve distances; assignments are still native
    pose-dependent and source clusters are not tracked object IDs. Therefore
    this is not independent static truth or proof of solved trajectory causality.
    All raw point contributions, empty classes, unexecuted phases, opposing
    signs and earlier failures remain. Renderer-empty-class and ENOSPC failures
    survive before renderer/complete-lossless-transport recovery; no native
    value or threshold changes. Earlier candidate gates remain failed. No
    rigidity threshold, retuning, production/default change, new full candidate
    trial, trajectory/paper-table improvement or SOTA claim follows. Before
    another selector, test consistency across different source regions under
    common background motion while retaining every rescue/adverse control.
    The next finite diagnostic should use region-crossing relations rather
    than retuning this local discrepancy or promoting another mask. Rigid
    motion within one local region alone cannot identify static support.
    Complete CPU controls retain the initial byte-equality failure and pass
    original strict numerical tolerances (`1d4e2a9e`/`6100e04e`); all11 fresh
    local study/renderer regressions run and pass. The first full432-test
    local run fails in an inherited group verifier amid a recorded memory
    limit kill (`d89405e2`), with exact child status unavailable after legacy
    temporary cleanup. That failure remains before isolated full revalidation.
    Quantitative scientific outputs and all earlier failures are committed;
    final local/CI proofs remain private to keep the tested source head fixed.
    That historical ready gate required isolated local checks and final-head
    Native/ROS2 CI logs. PR #170 was subsequently merged as `6d1023fe` after
    the owner explicitly accepted local verification while remote CI waited.
    The queued remote run was not counted as a pass; the scientific failures
    above remain failures. The current next task is the independent-initializer
    experiment described at the top of this roadmap.

13. **CT-ICP 1.15x, MULLS 1.12x.** Their 00-10 averages equal the official
    code's, so the gap is upstream's. Only revisit if a paper detail is
    found that upstream also lacks.

### Backlog

- Full-coverage failures still need a solver-level explanation: LF-GICP 10
  (6.71x), L-LO 09/10 (2.18x / 2.16x). Saved-pose drift intervals and block
  errors are in their method READMEs; these diagnostics locate failures
  without establishing their cause.
- New paper ports: follow [CONTRIBUTING.md](../CONTRIBUTING.md) (KITTI 00/07
  full, `--no-gt-seed`, at least 3 mechanism tests, README with negative
  results); run all paper sequences before claiming a ratio.
- Small-object removal for IMLS-SLAM (paper Table II, 0.58 -> 0.55 %).
- Loop closure / place recognition: DTD fails on real scans
  ([place_recognition_benchmark.md](place_recognition_benchmark.md)).

## 4. Traps already paid for

| Trap | Where it is written up |
|---|---|
| KITTI vertical-angle correction (0.18-0.22 deg) | [kitti_elevation_correction.md](kitti_elevation_correction.md) |
| Deskew on KITTI odometry makes things worse | [kitti_pitfalls.md](kitti_pitfalls.md) section 1 |
| Association with untransformed points, biased correspondence choice | same, section 2 |
| Map / model biases (closest-point creep, ground normals, blended lookups, vertical fill) | same, section 3 |
| Degenerate highway stretches; starting in motion | same, sections 2 and 4 |
| Body-frame scans, beam spacing, intensity 0-255, silently ignored keys, near objects, short windows | same, section 6 |
| Table 6 selects within each method's allowed pool; multi-variant selection per sequence is optimistic | [paper_claim.md](paper_claim.md) |
| `pkill -f` / `pgrep -f` with a pattern that matches your own shell kills it | match on the binary name instead |

## 5. Open questions for the owner

- Should the opt-in robustness fixes (DiLO remap, IMLS-SLAM coarse start)
  become defaults of the paper pipelines, accepting that the default no
  longer follows the paper exactly?
- Is NCLT the second benchmark to grow (Phase 3), or is another dataset
  preferred?
- Priority between speed (item 5) and bindings (item 7).
