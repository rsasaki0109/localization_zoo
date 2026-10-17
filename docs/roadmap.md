# Roadmap and handoff

Long-term plan and development handoff for Localization Zoo.
It is written for the next agent or contributor taking over: where things
stand, how work is done here, what to do next and how to tell it is done.

The goal is a usable implementation zoo with numbers people can trust: every
method runs from one harness, every reported number can be re-run, and every
gap to a paper is either closed or explained. Writing a paper is not a goal.

## 1. Where things stand

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
   - Third component implemented: the runner's opt-in sensor-frame near-range
     cut now lives in `common/range_filter.h`, used by ELO and DiLO through
     their shared loader and by intensity-preserving loaders. Float arithmetic,
     strict boundary, processing order and zero/off default remain unchanged;
     standalone method libraries do not apply it automatically.
     Whole-zoo KITTI replay is in progress: full 04 covers all 14 selected cells,
     with 12 matching the stored metrics. CT-ICP and Mesh-LOAM differ from the
     historical values, but the frozen pre-extraction executable reproduces
     both differences with identical saved poses; their causes remain open
     and their method READMEs retain the values. The historical audit exits 1,
     not a clean pass. Full 00's 20 candidate runs also finish: 15 match;
     A-LOAM, F-LOAM, LeGO-LOAM, Mesh-LOAM and VLOM differ, with RPE worsening
     on all five. Their numeric pairs are retained in the method READMEs;
     frozen pre-extraction comparisons and cleanup are still in progress.
     00 is not yet a completed sequence checkpoint. The historical NCLT cutoff is retained
     without reselection. DiLO's historical cut/uncut profiles also differ in
     range-image geometry, so their RTE difference cannot isolate the cut's
     effect; its ATE worsens from 54.344593 to 75.832748 m. Real NCLT validation
     remains blocked by the enforced official-host allowlist.
     [Protocol, scope and execution records](../experiments/results/shared_near_range/README.md).
7. **Bindings.** The Python bindings cover KISS-ICP, NDT, GICP and LiTAMIN2
   (`python/`). Add the paper pipelines that are near paper (SuMa,
   IMLS-SLAM, DiLO, LiTAMIN2 paper path) and ROS 2 nodes for them.
   - Done: an example per method and a test on a short fixture.

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
    remains open. The plane check and correction were chosen on 07 windows
    with 00 influencing the diagnosis, so 00 is not fully held out.
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
