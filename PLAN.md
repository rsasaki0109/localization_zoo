# Localization Zoo - Codex / Cursor 引き継ぎ PLAN

> **最終更新: 2026-06-02**
>
> この文書は、次の AI アシスタントが repo の現在地、最近の差分、次にやるべきことを短時間で掴むための handoff。
>
> 最初に本ファイルを読み、その次に:
> 1. [`README.md`](README.md)
> 2. [`docs/index.html`](docs/index.html)
> 3. [`docs/methods.json`](docs/methods.json)
> 4. [`evaluation/scripts/demo_localization_zoo.sh`](evaluation/scripts/demo_localization_zoo.sh)
> 5. [`evaluation/scripts/generate_demo_report.py`](evaluation/scripts/generate_demo_report.py)
> 6. [`evaluation/scripts/validate_demo_artifacts.py`](evaluation/scripts/validate_demo_artifacts.py)
> 7. [`evaluation/scripts/validate_showcase.py`](evaluation/scripts/validate_showcase.py)
> 8. [`experiments/results/index.json`](experiments/results/index.json)
> 9. [`docs/status_taxonomy.md`](docs/status_taxonomy.md)
> 10. [`docs/budget_profiles.md`](docs/budget_profiles.md)
> 11. [`experiments/families.json`](experiments/families.json)
> 12. [`docs/reproduction_status.md`](docs/reproduction_status.md)
> 13. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 14. [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)

---

## 0. Latest Handoff: OSS Showcase / Star Growth Push (2026-06-02)

This section is the authoritative current handoff. Older sections below still
matter for benchmark history, recipe provenance, and paper-grade claims, but
they describe the May 2026 research state. The active June 2026 direction is
different: make the repository easier to understand, easier to try, and harder
to accidentally regress.

The user's current theme has been:

- "omosiroi kihatu wo siyou. star wo huyasitai."
- "motto syuhouwo huyasou! ittai kensyou dekiruyouni siyou!"
- "ittan plan md wo naganga to kousin."

Interpretation: this repo should look and feel like a strong OSS project, not
only a private research scratchpad. First-clone users should immediately see a
usable method explorer, a runnable demo, a generated report, and CI-backed proof
that many methods can be validated together.

### 0.0 Update — Showcase merged + 2 new methods (2026-06-02, later session)

This subsection supersedes 0.1–0.7 where they conflict; those describe the
pre-merge dirty-worktree state.

- **Showcase merged to `main`.** PR #2 (`wip/profile-expansion-refresh` → `main`)
  was merged (merge commit `4ddb7c3`). The merge brought in `main`'s 14 commits
  (HDL-400 B benchmark docs, `run_local_evaluation_suite.sh` / `eval_local_suite`
  CMake target, Ceres/OpenCV build tweaks, KISS-ICP empty-frame + common golden
  tests). Conflicts were resolved keeping the branch's newer research/showcase
  state for data/docs and merging both sides for source (`run_experiment_matrix.py`
  kept the lower-case `is_metric_valid` taxonomy + new `--merge-existing-index`;
  the duplicate `problem_run_from_aggregate` was disambiguated into
  `problem_run_from_aggregate` (manifest form) and `problem_run_from_aggregate_file`
  (aggregate-only form)).
- **Method count 33 → 35.** Two new from-scratch LiDAR ports were added:
  - `papers/genz_icp/` — GenZ-ICP style degeneracy-robust odometry (adaptive
    point-to-plane / point-to-point hybrid, normal+planarity estimation on the
    KISS-style voxel map). Selector `genz_icp`, profiles `--genz-fast-profile` /
    `--genz-dense-profile` / `--genz-planarity-threshold`. `test_genz_icp`.
  - `papers/rko_lio/` — RKO-LIO style scan-to-map odometry with an IMU gyro
    rotation prior (via `imu_preintegration`), constant-velocity fallback when no
    `imu.csv`. Selector `rko_lio`. `test_rko_lio`. Always runs (does not skip);
    note text records whether the IMU prior was used.
  - Both have `docs/methods.json` entries (catalog now 41 entries = 41 `papers/*`).
- **Drift column.** `pcd_dogfooding` results table now prints `Drift[m/100m]`
  (the existing KITTI-style `rpe_trans_pct`, previously only in summary JSON;
  shows `-` for trajectories shorter than the RPE segment).
- **GenZ-ICP KITTI Odom 108-window finding** (memory
  `genz_icp_vs_kiss_kitti_windows.md`): GenZ-ICP beats KISS-ICP on drift in 3/5
  windows (00/02/07) but is not universal (loses on 05), is slower, and does not
  rescue the seq 08 no-seed divergence (~33 m/100m, like KISS). NDT (GT-seeded)
  remains the overall window winner. RKO-LIO full benchmark intentionally **not
  yet run** (user deferred).
- **Commits on branch after the merge:** `dd69400` (GenZ-ICP), `e1ecc6a` (drift
  column), `3645d67` (RKO-LIO). Branch pushed to origin.
- **Verification:** `cmake --build build --target pcd_dogfooding multimodal_dogfooding`
  clean; `ctest -R "genz_icp|rko_lio"` pass; `python3 -m unittest discover -s tests`
  → 49 pass; one-command demo (broad) + `validate_showcase.py --require-demo` valid.
  The demo all-OK profiles were intentionally **not** changed (genz_icp/rko_lio
  are available via `--methods` but not in the default broad/full sets).

### 0.1 Current Git State

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD at update time | `947912d` |
| Worktree | Dirty by design; current showcase/demo changes are not committed yet |
| Local instruction | Prefix shell commands with `rtk` when operating as Codex |
| Main current artifact | One-command demo + GitHub Pages showcase + validators |
| Current generated demo output | Ignored under `experiments/results/runs/demo_localization_zoo/` |

Current dirty/untracked files at the time of this PLAN update:

- Modified:
  - `.github/workflows/ci.yml`
  - `README.md`
  - `docs/assets/site.css`
  - `docs/index.html`
  - `tests/test_experiment_scripts.py`
- Untracked:
  - `docs/assets/explorer_preview.png`
  - `docs/methods.json`
  - `evaluation/scripts/demo_localization_zoo.sh`
  - `evaluation/scripts/generate_demo_report.py`
  - `evaluation/scripts/validate_demo_artifacts.py`
  - `evaluation/scripts/validate_showcase.py`

The generated demo run artifacts are intentionally not tracked. They are
produced by the demo script and uploaded by CI as an artifact:

- `experiments/results/runs/demo_localization_zoo/report.html`
- `experiments/results/runs/demo_localization_zoo/manifest.json`
- `experiments/results/runs/demo_localization_zoo/synthetic_benchmark.log`
- `experiments/results/runs/demo_localization_zoo/lidar_fixture_summary.json`
- `experiments/results/runs/demo_localization_zoo/multimodal_fixture_summary.json`
- trajectory text outputs under `benchmark_results/` and `dogfooding_results/`

### 0.2 What Was Just Built

The repo now has a public-facing showcase layer in addition to the older
research benchmark layer.

#### GitHub Pages Explorer

[`docs/index.html`](docs/index.html) has been turned into a richer interactive
front page:

- Loads latest benchmark data from `docs/benchmarks/latest/results.json`.
- Loads method metadata from the new `docs/methods.json` instead of hardcoded
  JavaScript arrays.
- Renders a method explorer covering every `papers/*` directory.
- Shows starter tracks for different user intents.
- Shows a benchmark scatter/leaderboard for the committed latest snapshot.
- Includes OpenGraph/Twitter metadata for link previews.
- Points users toward the one-command local demo.

[`docs/assets/site.css`](docs/assets/site.css) was redesigned for the explorer:

- More dashboard-like and scan-friendly.
- Responsive layout for desktop/mobile.
- No giant marketing-only hero; the page is meant to be useful immediately.

[`docs/assets/explorer_preview.png`](docs/assets/explorer_preview.png) is a new
preview image referenced from the README and validated as a PNG by the showcase
validator.

#### Method Catalog

[`docs/methods.json`](docs/methods.json) is new:

- `schema_version: 1`
- 39 method entries, one for each `papers/*` directory.
- Required fields per method:
  - `name`
  - `family`
  - `scope`
  - `signals`
  - `href`
  - `summary`
  - `tags`
- 4 starter tracks:
  - quick start / first run
  - accuracy oriented
  - fusion / multimodal
  - degeneracy / robustness

The method catalog is now test-covered. If a new paper directory is added
without a catalog entry, the Python tests fail.

#### One-command Demo

[`evaluation/scripts/demo_localization_zoo.sh`](evaluation/scripts/demo_localization_zoo.sh)
is new and is now the first-run path for users. It:

- Builds the C++ targets unless `--skip-build` is provided.
- Runs `synthetic_benchmark`.
- Runs the committed three-frame MCD fixture through selected LiDAR methods.
- Runs the same fixture through selected multimodal methods.
- Writes logs, summary JSON, trajectories, `report.html`, and `manifest.json`.
- Calls the demo artifact validator at the end, so the command proves its own
  output contract.

The demo now has method profiles:

| Profile | LiDAR methods | Multimodal methods | Intended use |
|---------|---------------|--------------------|--------------|
| `quick` | 4 | 2 | Fast old-style local loop |
| `broad` | 24 | 6 | Default; best first-clone OSS proof |
| `full` | 25 | 6 | Adds LiDAR FAST-LIO2 fixture validation |

Current LiDAR `quick`:

```text
litamin2,gicp,ndt,kiss_icp
```

Current LiDAR `broad`:

```text
litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,xicp,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio
```

Current LiDAR `full`:

```text
litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,xicp,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio,fast_lio2
```

Note: `full` is not literally every LiDAR selector in `pcd_dogfooding`. It is
the largest all-OK set for the committed MCD fixture without requiring IMU-only
methods that skip when `imu.csv` is absent. `ct_lio` and `clins` currently skip
on this fixture because there is no synchronized IMU CSV. Do not include them
in the default all-OK profile unless the fixture gains IMU data or the validator
learns about expected skips.

Current multimodal `quick`:

```text
okvis,fast_livo2
```

Current multimodal `broad` / `full`:

```text
vins_fusion,okvis,orb_slam3,lvi_sam,fast_livo2,r2live
```

Useful invocations:

```bash
bash evaluation/scripts/demo_localization_zoo.sh
bash evaluation/scripts/demo_localization_zoo.sh --skip-build
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile quick
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile broad
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile full
bash evaluation/scripts/demo_localization_zoo.sh --skip-build --methods litamin2,gicp --multimodal-methods okvis
```

As Codex in this workspace, remember to run those through `rtk`, e.g.
`rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build`.

#### Demo Report and Manifest

[`evaluation/scripts/generate_demo_report.py`](evaluation/scripts/generate_demo_report.py)
is new:

- Standard-library-only report generator.
- Parses synthetic benchmark logs.
- Reads LiDAR and multimodal summary JSON.
- Reads trajectory text files.
- Produces self-contained `report.html` with inline CSS/SVG.
- Produces `manifest.json` with:
  - schema version
  - command
  - profile
  - requested LiDAR method list
  - requested multimodal method list
  - actual method counts and statuses
  - artifact paths

The manifest is not just a pretty output. It is now the contract between the
demo script, validator, CI artifact upload, README claims, and showcase tests.

#### Validators

[`evaluation/scripts/validate_demo_artifacts.py`](evaluation/scripts/validate_demo_artifacts.py)
is new:

- Validates `manifest.json`.
- Validates `report.html` required snippets.
- Validates synthetic/LiDAR/multimodal summary artifacts.
- Requires every selected method to have `status == ok`.
- Validates exact method-set match by normalized method name.
- Can read expected method lists from CLI or from the manifest.
- Supports `--skip-multimodal`.

Important behavior: if someone silently shrinks the default profile from 24
LiDAR methods to 4, the validator catches it when expected methods are provided
or when manifest-vs-summary diverges.

[`evaluation/scripts/validate_showcase.py`](evaluation/scripts/validate_showcase.py)
is new:

- Validates README links and required snippets.
- Validates the preview PNG signature.
- Validates `docs/index.html` snippets and repo link.
- Validates `docs/methods.json` coverage against `papers/*`.
- Validates starter-track references.
- Validates the latest benchmark snapshot and trajectory plot.
- Optionally validates generated demo artifacts with `--require-demo`.

#### CI Integration

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) now includes:

- One-command demo report generation after existing smoke checks.
- Showcase validation with `--require-demo`.
- Upload of `experiments/results/runs/demo_localization_zoo/` as the
  `localization-zoo-demo-report` artifact.

The CI path intentionally demonstrates the same thing a new user sees locally:
build, run a real committed fixture, generate a report, validate it, and keep
the report downloadable.

### 0.3 Validation Already Run

The following commands passed during the 2026-06-02 update:

```bash
rtk bash -n evaluation/scripts/demo_localization_zoo.sh
rtk python3 -m py_compile evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py
rtk python3 evaluation/scripts/validate_showcase.py --skip-demo
rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build
rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build --profile full
rtk python3 evaluation/scripts/validate_showcase.py --require-demo --demo-dir experiments/results/runs/demo_localization_zoo
rtk python3 -m unittest discover -s tests -p 'test_*.py' -v
rtk proxy git diff --check -- README.md docs/index.html evaluation/scripts/demo_localization_zoo.sh evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py tests/test_experiment_scripts.py
```

Observed validation result:

- `broad` demo: LiDAR 24 / multimodal 6 all OK.
- `full` demo: LiDAR 25 / multimodal 6 all OK.
- Showcase validation: OK.
- Python tests: 49 tests OK.
- Whitespace check: OK.

Because `full` was the last demo run before this PLAN update, the ignored local
`experiments/results/runs/demo_localization_zoo/manifest.json` currently records
`profile: full`, `lidar_fixture.method_count: 25`, and
`multimodal_fixture.method_count: 6`.

### 0.4 Test Coverage Added

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) now
includes three new showcase/demo-focused groups:

- `MethodCatalogTests`
  - `docs/methods.json` schema is version 1.
  - Catalog has unique method names and hrefs.
  - Catalog hrefs exactly cover all `papers/*` directories.
  - Required fields are present.
  - Each referenced method README exists.
  - Starter tracks reference known methods.
- `ShowcaseContractTests`
  - Runs `validate_showcase.py --skip-demo`.
- `DemoReportScriptTests`
  - Builds a minimal fake demo directory.
  - Runs `generate_demo_report.py`.
  - Checks report snippets.
  - Checks manifest profile and requested method lists.
  - Runs `validate_demo_artifacts.py`.
  - Verifies an expected-method mismatch fails.

### 0.5 Current Product Story

The README and Pages story should now be:

1. This repo contains many localization/SLAM method ports.
2. Users can browse the catalog on GitHub Pages.
3. Users can run a one-command demo after clone.
4. The demo validates a committed real-data MCD fixture, not just synthetic data.
5. The default path compares many methods together.
6. CI runs that same path and uploads the HTML report.

This is intentionally a stronger OSS adoption story than "read a long paper
doc and manually pick scripts."

### 0.6 Rules for Future Claims

Keep these distinctions sharp:

- The committed MCD fixture is a smoke/demo fixture. It proves integration and
  output contracts, not paper-grade accuracy.
- The `broad` and `full` profiles are all-OK fixture profiles, not a statement
  that every method is production-ready.
- The demo report is reproducible from the repo because the fixture is
  committed.
- Full dataset benchmark claims still come from `experiments/results/*.json`,
  `docs/reproduction_status.md`, and the older benchmark matrix.
- Do not claim exact original-paper reproduction unless the taxonomy/docs say
  so.
- Do not include methods that skip on the fixture in default all-OK profiles
  unless the validator is changed to support expected skips with explicit
  reasons.

### 0.7 Immediate Next Work

Recommended order:

1. **Review the new untracked files as if preparing a commit.**
   - Confirm no generated demo outputs are accidentally staged.
   - Confirm `docs/assets/explorer_preview.png` is intentionally tracked.
   - Confirm `docs/methods.json` names/summaries read well enough for public
     Pages.

2. **Run one final local verification before commit.**
   - `rtk bash evaluation/scripts/demo_localization_zoo.sh --skip-build`
   - `rtk python3 evaluation/scripts/validate_showcase.py --require-demo --demo-dir experiments/results/runs/demo_localization_zoo`
   - `rtk python3 -m unittest discover -s tests -p 'test_*.py' -v`
   - `rtk proxy git diff --check -- README.md docs/index.html evaluation/scripts/demo_localization_zoo.sh evaluation/scripts/generate_demo_report.py evaluation/scripts/validate_demo_artifacts.py evaluation/scripts/validate_showcase.py tests/test_experiment_scripts.py PLAN.md`

3. **Commit the showcase/demo expansion.**
   - Suggested commit scope: README, docs page/assets, method catalog, demo
     scripts, validators, CI, tests, PLAN.
   - Suggested commit message:
     `Add showcase explorer and broad demo validation`

4. **Then consider one star-growth follow-up, not all at once.**
   - Add README badges for CI / Pages / license.
   - Add a compact "What runs in 60 seconds?" section.
   - Add a GIF or screenshot from `report.html`.
   - Add issue templates for "method request" and "dataset request".
   - Add a `CONTRIBUTING.md` focused on adding a method to `papers/*` and
     `docs/methods.json`.

5. **If expanding validation again, prefer expected-skip semantics before adding
   IMU-only selectors.**
   - Current fixture lacks `imu.csv`.
   - `ct_lio` and `clins` skip correctly on this data.
   - A robust next step would let the manifest encode expected skips with
     reason text, but this is a different contract from the current all-OK demo.

### 0.8 Do Not Do Next

Avoid these unless the user explicitly asks:

- Do not start a new paper-writing pass.
- Do not rerun large KITTI/MulRan full-sequence sweeps just to improve the
  README.
- Do not add more untracked generated artifacts.
- Do not collapse the old benchmark/reproduction docs into the new showcase
  page; the showcase should point into them, not replace them.
- Do not loosen validators to make demos pass. If a method is not OK, either
  remove it from an all-OK profile or add an explicit expected-skip contract.

---

## 1. Executive Summary

### 1.1 Current indexed artifact

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD | `458c81a` |
| Worktree | **clean after NDT + LOAM-family KITTI Odom checkpoint commit** |
| Indexed manifests | **392** |
| Indexed ready | **378** |
| Indexed blocked | **1** |
| Indexed skipped | **13** |
| Pending manifests | **200** |
| LiDAR families | **27** |
| Camera-aware families | **6** |
| Total active selectors | **33** |
| Python tests | **14/14 pass** (last full run; not rerun after NDT/LOAM-family docs/artifact refresh) |

### 1.2 What changed recently (2026-05-19..21 session, GICP family + NDT/A-LOAM KITTI checks)

**NDT T1 transfer confirmation** on KITTI Odom seq 02/05/08 full. `--ndt-resolution 0.5 --ndt-max-iterations 12` stayed sub-10cm on all three held-out long sequences: seq 02 = 0.0585 m ATE, seq 05 = 0.0594 m ATE, seq 08 = 0.0761 m ATE. Together with the existing seq 00 r05+i12 result (0.0707 m) and seq 07 r05+i12 result (0.0763 m), NDT now has a seeded KITTI Odom 5/5 sub-8cm T1-family recipe. The direct confirmation runs were slow on this export path (0.23-0.25 FPS), so the result is an accuracy/universality confirmation rather than a throughput recommendation.

**A-LOAM KITTI Odom full transfer check**: added `aloam_kitti_seq_07_full_sweep` plus seq 00/02/05/08 transfer matrices. Seq 07 `fast` wins the combined benchmark by throughput (4.0307 m ATE, 0.691% RPE, 7.26 FPS), while `kitti_default` is the seq 07 accuracy winner (2.5052 m ATE, 0.605% RPE, 3.26 FPS). Transfer is **non-universal**: accuracy winner is `kitti_default` on all held-out seqs, with ATE 9.5206 m (00), 50.7898 m (02), 5.1927 m (05), 18.6614 m (08). `fast` is faster but less accurate on all held-out seqs: 19.3711 m (00), 74.9663 m (02), 7.7893 m (05), 23.3578 m (08). Treat A-LOAM as a drift-level baseline, not a sub-meter KITTI Odom recipe.

**F-LOAM KITTI Odom seq 07 full cluster probe**: added `floam_kitti_seq_07_full_sweep`. `dense` is the accuracy/RPE winner (3.1736 m ATE, 0.590% RPE, 3.74 FPS), `kitti_default` is close in accuracy but much faster (3.2455 m, 0.789% RPE, 13.23 FPS), and `fast` wins the combined benchmark by throughput (5.0260 m, 1.262% RPE, 29.84 FPS). F-LOAM is faster than A-LOAM on this sequence but still drift-level; transfer to seq 00/02/05/08 remains unverified.

**LeGO-LOAM KITTI Odom seq 07 full cluster probe**: added `lego_loam_kitti_seq_07_full_sweep`. `kitti_default` is the accuracy/RPE winner (2.5579 m ATE, 0.527% RPE, 3.89 FPS), `fast` wins the combined benchmark by throughput (4.3850 m, 0.597% RPE, 10.03 FPS), and `dense` underperformed both (3.9454 m, 0.650% RPE, 3.68 FPS). LeGO-LOAM is the strongest LOAM-family accuracy profile on seq 07 so far, slightly worse than A-LOAM `kitti_default` in ATE but better in RPE; held-out transfer remains unverified.

**MULLS KITTI Odom seq 07 full cluster probe**: added `mulls_kitti_seq_07_full_sweep`. `dense` is the accuracy/RPE winner (8.2878 m ATE, 2.640% RPE, 1.27 FPS), `kitti_default` is close but slower/worse (8.4591 m, 2.720% RPE, 1.42 FPS), and `fast` wins the combined benchmark by throughput (10.5014 m, 2.994% RPE, 4.13 FPS). MULLS is the weakest LOAM-family seq 07 candidate so far and very slow on full KITTI Odom; do not spend transfer budget on it unless specifically needed as a negative baseline.

**F-LOAM + LeGO-LOAM held-out transfer**: added seq 00/02/05/08 full transfer matrices for F-LOAM `kitti_default`/`dense` and LeGO-LOAM `kitti_default`. No universal LOAM-family recipe emerged. Per-seq LOAM accuracy winners are mixed: seq 00 = F-LOAM `dense` (8.5561 m ATE, 0.991% RPE, 3.37 FPS), seq 02 = LeGO-LOAM `kitti_default` (42.2088 m, 0.883% RPE, 3.79 FPS), seq 05 = A-LOAM `kitti_default` (5.1927 m, 0.512% RPE, 2.26 FPS; LeGO is nearly tied at 5.2182 m), seq 08 = F-LOAM `kitti_default` (16.6614 m, 1.566% RPE, 12.83 FPS). These improve over A-LOAM on seq 00/02/08 but remain drift-level and far from NDT/LiTAMIN2/GICP seeded winners.

**GICP family recipe discovery + seed-dependence verification** across all KITTI Odom sequences. State delta: 336 → 371 indexed manifests (35 new), HEAD `d22a172` → `458c81a`.

**GICP family findings** (memory entries: `gicp_family_seq_07_recipe_divergence.md`, `small_gicp_fast_kitti_universal.md`, `small_gicp_seed_dependence.md`):

- **small_gicp `--small-gicp-fast-profile`** は KITTI Odom 5/5 + KITTI Raw 4/4 + MCD KTH で ATE winner = **9/10 scenes universal sub-meter** (0.68-0.98 m). LiTAMIN2 T1 と並ぶ唯一の cross-KITTI universal recipe。indoor static (MCD NTU/TUHH) のみ dense_profile に転換。
- **voxel_gicp `--voxel-gicp-dense-profile`** は KITTI Odom 5/5 で both ATE & RPE universal winner (ATE 0.94-1.05 m, RPE 1.5-1.8%). 強い but KITTI Raw / MCD では dataset-dependent (4/9 only).
- **KISS-ICP** は long-trajectory で catastrophic: seq 02 で 39.23 m, seq 00 で 11.98 m, seq 08 で 19.41 m。dense_profile が 4/5 で best ATE recipe も全体的に他 method 比 10-30× worse on long-traj. Local-map-only architecture が drift-bound.
- **LiTAMIN2 T1 (voxel=0.5+iter=12) transfer は GICP family で non-universal**: KISS-ICP seq 07 のみ局所勝利、small_gicp で ATE は fast に負ける、voxel_gicp で全 seq 退行。T1 transfer は LiTAMIN2 専用 recipe.

**Seed-dependence critical finding** (memory entry: `small_gicp_seed_dependence.md`):

KITTI Odom seq 00 full no-seed test で 4 GICP family methods + LiTAMIN2 全てが完全発散:

| method | seeded ATE | no-seed ATE | drift |
|---|---|---|---|
| LiTAMIN2 T1 | 0.731 | ~110 m | +15,000% |
| small_gicp fast | 0.890 | 202.72 | +22,800% |
| voxel_gicp dense | 1.047 | 87.91 | +8,300% |
| **CT-ICP best (cluster A)** | 4.91 (seeded) | **12.69** | **唯一 functional** |

→ **production deployment (no-seed) on KITTI Odom long-trajectory では CT-ICP が唯一の選択肢**。seeded benchmark での compositional ranking (LiTAMIN2 T1 / small_gicp fast / voxel_gicp dense / CT-ICP) が完全 reversed.

**Earlier (2026-05-18..19, LiTAMIN2 saturation + CT-ICP completion):**

LiTAMIN2 cluster T1 (voxel=0.5 + iter=12 + GT seed) は KITTI Odom 5/5 + MulRan 2/2 + MCD 3/3 + KITTI Raw 4/4 = **11/12 universal winner**, 1/12 tied (noise floor 0.5 m). CT-ICP 比 1.8-70× dominance on seeded benchmark.

**Earlier CT-ICP findings** (memory entries: `ct_icp_kitti_full_per_seq_best.md`, `ct_icp_cluster_a_cross_dataset_transfer.md`, `ct_icp_gt_seed_dataset_dependence.md`):

- **5-cluster structure for CT-ICP recipes**: cluster A (`map=50 + c2f σ×2`) wins KITTI seq 00 (12.69 m) / 05 (7.76 m) / 08 (27.85 m). cluster B = A + corr=4 for seq 05. cluster C = `bare + corr=8` for seq 02 (50.63 m). cluster D = `ms_chol + map=20` for seq 07 (1.61 m) and KITTI Raw 0061 full (4.50 m). KITTI Raw 0009 is its own balanced-only family.
- **Cross-dataset transfer**: cluster A + GT seed wins on MulRan parkinglot full (9.19 m, -36% vs prior best) and parkinglot 120 (2.55 m, -84%). cluster A + seed is the universal seeded winner on MCD KTH/TUHH/NTU/MulRan parkinglot — **but not on KITTI Odom seq 07** where cluster D dominates seed-independently.
- **GT seed dataset-dependence**: seed helps drift-limited scenes (-44 to -88% on MulRan parkinglot, MCD KTH/TUHH) but hurts self-anchoring scenes (MCD NTU +39%, KITTI seq 07 neutral). On KITTI seq 00 full it produces the sharpest ATE/RPE flip yet observed: ATE -61% (12.69 → 4.91 m) but RPE +174% (2.10 → 5.76%).
- **Knob axes exhaustively mapped** on seq 00 cluster A: cauchy_coarse_mult (2.0 winner), cauchy_fine_sigma (default 0.5 winner), coarse_search_radius (2 winner, 4 ties), coarse_iter (2 winner), map_size (50 winner), corr_dist (default 100 winner). 2-D cauchy plane has its true minimum at (fine=0.5, coarse_mult=2.0).

Earlier (2026-05-17) foundational taxonomy layer (still in effect):

- **Claim-level taxonomy**: `reproduced > approximately_reproduced > indicative > smoke > ported`. Bumped into [`evaluation/data/paper_reported_numbers.json`](evaluation/data/paper_reported_numbers.json) (schema v3) and rendered into [`docs/reproduction_status.md`](docs/reproduction_status.md) as a legend column.
- **Budget profile contract**: [`docs/budget_profiles.md`](docs/budget_profiles.md) defines `smoke_200f_120s`, `practical_full_300s`, `extended_full_1800s`, `reference_full_unbounded`. Manifests reference them via `problem.budget_profile`.
- **Family registry**: [`experiments/families.json`](experiments/families.json) classifies the 33 method families into `core` / `extended` tiers and `maintained` / `timeout_prone` / `input_constrained` / `legacy` / `experimental` status.
- **Status taxonomy migration**: [`docs/status_taxonomy.md`](docs/status_taxonomy.md) defines the target per-variant enum. C++ binaries (`pcd_dogfooding`, `multimodal_dogfooding`) and the runner now emit the lower-case enum (`ok`, `skipped`, `timeout_budget`). Reserved values (`tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`) have field space in `MethodResult::status` but no detection logic yet. Legacy uppercase (`OK`/`SKIPPED`/`TIMED_OUT`) is normalized at ingest.

Earlier session work (already in `main` history):

- MulRan / Newer College benchmark scaffolding (commit `aee7611`).
- Short-trajectory RPE fix and `paper_comparison.md` refresh.
- `pcd_dogfooding --summary-json` exports optional `rpe_trans_pct` / `rpe_rot_deg_per_m`.
- KITTI Odometry preparation script `evaluation/scripts/prepare_kitti_odometry_inputs.py` generalized; `setup_kitti_benchmark.sh` is a wrapper.

### 1.3 Current direction

**Saturated:**
- CT-ICP: 5-cluster recipe structure mapped across 13 dataset/window combinations. Knob axes + seed-dependence saturated.
- LiTAMIN2: cluster T1 universal across 12 locally-available CT-ICP-comparable datasets (11/12 wins, 1/12 tied at noise floor).
- KISS-ICP / small_gicp / voxel_gicp: recipe pattern on 5 KITTI Odom seqs + 7 cross-dataset scenes. Seed-dependence verified (all 3 + LiTAMIN2 require GT seed for long-traj).
- NDT: T1 r05+i12 (`--ndt-resolution 0.5 --ndt-max-iterations 12`) confirmed on KITTI Odom 5/5 full with sub-8cm seeded ATE. Remaining gap is throughput/implementation efficiency, not recipe universality on this benchmark.
- A-LOAM: KITTI Odom 5/5 full transfer checked. `kitti_default` is stable but drift-level; no universal sub-meter recipe.
- F-LOAM: KITTI Odom 5/5 checked via seq 07 cluster + seq 00/02/05/08 transfer. Useful on seq 00/08, especially `dense` on seq 00 and `kitti_default` on seq 08, but non-universal and drift-level.
- LeGO-LOAM: KITTI Odom 5/5 checked via seq 07 cluster + seq 00/02/05/08 transfer. Strongest LOAM-family result on seq 02 and near-tie on seq 05, but non-universal and drift-level.
- MULLS: KITTI Odom seq 07 full cluster probe checked. `dense` is the seq 07 accuracy/RPE winner, but all profiles are slow and drift-heavy; deprioritize held-out transfer.

**Method-level production deployment recommendation:**
- **Seeded benchmark winners (KITTI Odom 5/5)**: NDT T1 r05+i12 (0.058-0.076 m), LiTAMIN2 T1 (0.65-0.75 m), small_gicp fast (0.68-0.98 m), voxel_gicp dense (0.94-1.05 m).
- **No-seed deployment (production realism)**: **CT-ICP cluster A only** (12.69 m on seq 00; all others ≥87 m).

Priority order for next assistant:

- **A (highest leverage)**: checkpoint the NDT + LOAM-family artifact/docs refresh before more sweeps. The current dirty set is large and internally consistent.
- **B**: resume cross-method universal recipe survey: apply LiTAMIN2 T1 / small_gicp fast / voxel_gicp dense to remaining local datasets (autoware_istanbul, hdl_400) to fill the cross-dataset matrix.
- **C (broader, blocked on external data)**: `MulRan dcc01` and Newer College ingestion. 2 pending dcc01 manifests already exist; data download required.

Do **not** spend the next turn on paper drafting, PR polishing, or speculative refactoring. The user has been explicit that this is OSS infrastructure work, not paper writing.

---

## 2. Worktree State

Working tree should be clean after the NDT seq 02/05/08 confirmation artifact refresh and LOAM-family KITTI Odom full transfer/cluster sweep checkpoint commit. Previous handoffs warned about a dirty worktree with mass untracked multimodal work; that state has since been committed. Treat `git status` as authoritative.

The branch is currently ahead of `origin/wip/profile-expansion-refresh` by some commits; verify with `git status` before pushing.

---

## 3. Stable Core vs Experiment Layer

### 3.1 Stable binaries

- [`build/evaluation/pcd_dogfooding`](build/evaluation/pcd_dogfooding) — LiDAR-only stable benchmark, 27 method families, shared `--summary-json` contract.
- [`build/evaluation/multimodal_dogfooding`](build/evaluation/multimodal_dogfooding) — camera-aware sibling, 6 method families, same aggregate style and runner contract.

### 3.2 Stable per-method summary contract

Each method emits:

- `status` — lower-case enum from [`docs/status_taxonomy.md`](docs/status_taxonomy.md). Current emitting set: `ok`, `skipped`, `timeout_budget`. Reserved (not yet emitted): `tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`.
- `ate_m`
- `rpe_trans_pct` (optional; null when not computable)
- `rpe_rot_deg_per_m` (optional)
- `frames`
- `time_ms`
- `fps`
- `note`

`MethodResult::status` is a `std::string` in C++ side; future detection logic should set it directly (e.g. `result.status = "tracking_lost";`) rather than introducing new boolean flags.

### 3.3 Experiment layer

- Manifests: `experiments/*_matrix.json` (336 active)
- Pending manifests: `experiments/pending/*_matrix.json` (200)
- Aggregates: `experiments/results/*.json` (336)
- Family registry: [`experiments/families.json`](experiments/families.json) — used by docs, not by the runner.
- Generated docs:
  - `docs/experiments.md`
  - `docs/decisions.md`
  - `docs/interfaces.md`
  - `docs/paper_comparison.md`
  - `docs/variant_analysis.md`
  - `docs/reproduction_status.md`

---

## 4. Local Data Actually Available

Important: do not assume datasets mentioned in docs are locally present.

### 4.1 Present in `dogfooding_results/`

| Dataset | Path | Frames | Multimodal extras |
|--------|------|--------|-------------------|
| HDL-400 native/reference-like | `hdl_400_open_ct_lio_120` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 synthetic time (azimuth) | `hdl_400_open_ct_lio_120_time_azimuth` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 public ROS1 synthetic time | `hdl_400_ros1_open_ct_lio_120_time_index` | 120 | `imu.csv`, `frame_timestamps.csv` |
| KITTI Odometry seq 00 short | `kitti_seq_00_108` | 108 | (LiDAR-only) |
| KITTI Odometry seq 00 full | `kitti_seq_00_full` | 4542 | (LiDAR-only) |
| KITTI Odometry seq 02 full | `kitti_seq_02_full` | 4661 | (LiDAR-only) |
| KITTI Odometry seq 05 full | `kitti_seq_05_full` | 2761 | (LiDAR-only) |
| KITTI Odometry seq 07 short | `kitti_seq_07_108` | 108 | (LiDAR-only) |
| KITTI Odometry seq 07 full | `kitti_seq_07_full` | 1102 | (LiDAR-only) |
| KITTI Odometry seq 08 full | `kitti_seq_08_full` | 4071 | (LiDAR-only) |
| KITTI Raw 0009 short | `kitti_raw_0009_200` | 200 | full multimodal extras |
| KITTI Raw 0009 full | `kitti_raw_0009_full` | 443 | full multimodal extras |
| KITTI Raw 0061 short | `kitti_raw_0061_200` | 200 | full multimodal extras |
| KITTI Raw 0061 full | `kitti_raw_0061_full` | 703 | full multimodal extras |
| MCD KTH day-06 | `mcd_kth_day_06_108` | 108 | `frame_timestamps.csv` |
| MCD NTU day-02 | `mcd_ntu_day_02_108` | 108 | `frame_timestamps.csv` |
| MCD TUHH night-09 | `mcd_tuhh_night_09_108` | 108 | `frame_timestamps.csv` |
| MulRan parkinglot 120 | `mulran_parkinglot_120` | 120 | (LiDAR-only) |
| MulRan parkinglot full | `mulran_parkinglot_full` | 1177 | (LiDAR-only) |

### 4.2 Present in `experiments/reference_data/`

GT CSVs for every dataset listed in 4.1. Verify by listing the directory before quoting paths.

### 4.3 Not present locally right now

- KITTI Odometry seq 01, 03, 04, 06, 09, 10 (only 00/02/05/07/08 are dogfooded)
- Istanbul local windows
- MulRan dcc01 / kaist / riverside (parkinglot only is dogfooded; dcc01 pending manifests exist)
- Newer College, Oxford Spires

This matters because:

- The next assistant must not claim runs on KITTI Odom seqs outside 00/02/05/07/08 without first ingesting those seqs.
- The 5 ingested KITTI Odom seqs have been **exhaustively probed by CT-ICP**; LiTAMIN2 and other LiDAR families have NOT yet been similarly probed.

---

## 5. Indexed Method Surface

### 5.1 LiDAR-only families (29)

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `genz_icp`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `rko_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

(`genz_icp`, `rko_lio` added 2026-06-02 later session — see §0.0.)

### 5.2 Camera-aware families (6)

`vins_fusion`, `okvis`, `orb_slam3`, `lvi_sam`, `fast_livo2`, `r2live`

### 5.3 Tier classification

See [`experiments/families.json`](experiments/families.json). Roughly:

- **core (15)**: `kiss_icp`, `ct_icp`, `litamin2`, `small_gicp`, `voxel_gicp`, `ndt`, `gicp`, `fast_lio2`, `lio_sam`, `dlio`, `dlo`, `hdl_graph_slam`, `okvis`, `vins_fusion`, `fast_livo2`
- **extended (18)**: everything else, plus `orb_slam3`, `lvi_sam`, `r2live` marked `timeout_prone`.

### 5.4 Non-indexed papers

Outside the current benchmark surface: `vilens`, `relead`, `ct_icp_relead`, `scan_context`, `imu_preintegration`, `cube_lio_repro`.

LiDAR-only coverage is already broad. The missing frontier is publication-grade reproduction evidence, not more method folders.

---

## 6. Multimodal State

### 6.1 Canonical scope

- Binary: [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)
- Windows: `kitti_raw_0009_200`, `kitti_raw_0009_full`, `kitti_raw_0061_200`, `kitti_raw_0061_full`
- Each run expects `sequence_dir`, `gt_csv`, `landmarks.csv`, `visual_observations.csv`, camera intrinsics via CLI / `camera_args.txt`.

### 6.2 Current results

Fully practical under current study budget: `okvis`, `vins_fusion`, `fast_livo2`.
Practical-budget timeouts: `orb_slam3`, `lvi_sam`, `r2live`.

Canonical timeout budget (encoded in manifests via `problem.variant_timeout_seconds`):

- 200-frame windows: 120s
- Full windows: 300s

These align with `smoke_200f_120s` and `practical_full_300s` profiles in [`docs/budget_profiles.md`](docs/budget_profiles.md).

### 6.3 Multimodal infra

- [`evaluation/scripts/prepare_kitti_multimodal_inputs.py`](evaluation/scripts/prepare_kitti_multimodal_inputs.py)
- [`evaluation/scripts/generate_kitti_visual_observations.py`](evaluation/scripts/generate_kitti_visual_observations.py)
- [`evaluation/scripts/run_multimodal_study.py`](evaluation/scripts/run_multimodal_study.py)
- [`evaluation/scripts/smoke_multimodal_fixture.sh`](evaluation/scripts/smoke_multimodal_fixture.sh)

---

## 7. Reproduction / Paper-Result Status

### 7.1 The honest claim today

Per [`docs/reproduction_status.md`](docs/reproduction_status.md):

| Method | Claim level | Why |
|--------|------------|-----|
| `litamin2` | `indicative` | Short-window ATE, not full-sequence KITTI RPE study. |
| `ct_icp` | `indicative` | Core close to paper formulation, but evaluation indirect. |
| `kiss_icp` | `indicative` | Compact local-map pipeline, windowed reruns only. |
| `gicp` | `ported` | Pre-KITTI paper; no standardized numeric target. |
| `ndt` | `ported` | Pre-KITTI; modern NDT codebases differ materially. |
| `ct_lio` | `ported` | Intentionally custom integration; no single paper-faithful target. |

The repo is **not** yet entitled to say "paper results reproduced" generally. The claim-level scheme makes that boundary explicit.

### 7.2 What is already better

`LiTAMIN2` and `CT-ICP` are closer to reproducible paper-style evaluation because:

- `pcd_dogfooding` exports RPE.
- Aggregates preserve RPE.
- `paper_comparison.md` shows repo-side RPE where available.

Canonical KITTI Raw 0009 reruns with RPE exist:

- [`experiments/results/litamin2_kitti_raw_0009_matrix.json`](experiments/results/litamin2_kitti_raw_0009_matrix.json) — adopted `fast_cov_half_threads` (ATE 1.053 m, RPE trans 0.742 %, 34.16 FPS)
- [`experiments/results/ct_icp_kitti_raw_0009_matrix.json`](experiments/results/ct_icp_kitti_raw_0009_matrix.json) — adopted `fast_window` (ATE 2.728 m, RPE trans 2.198 %, 49.27 FPS)

### 7.3 What is still missing for graduation

To move `litamin2` and `ct_icp` from `indicative` toward `approximately_reproduced`:

- **CT-ICP**: full KITTI Odom 00/02/05/07/08 are now ingested and exhaustively probed. Per-seq best numbers (12.69 / 50.63 / 7.76 / 1.61 / 27.85 m ATE) are recipe-tuned, not paper-tuned. Direct paper-comparison requires either (a) committing to a single "paper-style" recipe and reporting per-seq deltas, or (b) reporting per-seq best with explicit recipe attribution.
- **LiTAMIN2**: same 5-cluster recipe-discovery approach has NOT yet been applied. This is the next high-leverage probe — analog of CT-ICP's cluster A/B/C/D/balanced structure may exist for LiTAMIN2.
- **A (parallel)**: extend RPE-aware reruns to MCD/HDL-400 for both methods.

---

## 8. KITTI Odometry Preparation State

### 8.1 Preparation script

- [`evaluation/scripts/prepare_kitti_odometry_inputs.py`](evaluation/scripts/prepare_kitti_odometry_inputs.py)
- Converts `sequences/<seq>/velodyne/*.bin` + `poses/<seq>.txt` into:
  - `dogfooding_results/kitti_seq_<seq>_<window>`
  - `dogfooding_results/kitti_seq_<seq>_full`
  - `experiments/reference_data/kitti_seq_<seq>_<window>_gt.csv`
  - `experiments/reference_data/kitti_seq_<seq>_full_gt.csv`

### 8.2 Compatibility wrapper

[`evaluation/scripts/setup_kitti_benchmark.sh`](evaluation/scripts/setup_kitti_benchmark.sh) delegates to the Python script.

### 8.3 Status of full-sequence manifests

CT-ICP full KITTI Odom 00/02/05/07/08 manifests have been **promoted to active** (this session). Per-seq best results recorded; 5-cluster recipe structure documented in `memory/ct_icp_kitti_full_per_seq_best.md`. Cross-dataset cluster A transfer documented in `memory/ct_icp_cluster_a_cross_dataset_transfer.md`.

LiTAMIN2 full-sequence manifests remain in `experiments/pending/` — runnable now that KITTI Odom data is ingested. This is the **highest-leverage next probe**.

### 8.4 Regression coverage

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) contains a fake KITTI Odometry root test that verifies window export, full export, GT CSV generation, and contract compatibility of the preparation script.

---

## 9. Public-Data Direction Beyond KITTI

### 9.1 Already partially in (do not redo from scratch)

- **MulRan parkinglot**: ingested. 8 indexed manifests covering `LiTAMIN2`, `CT-ICP`, `KISS-ICP`, `GICP` × {120-frame, full}.
- **MulRan dcc01**: 4 pending manifests in `experiments/pending/` (`*_mulran_dcc01_120_matrix.json` for ct_icp/gicp/kiss_icp/litamin2). **Blocked**: raw data not local. Requires MulRan official download form (https://forms.gle/EmUybUiGc8pR3r7Q6) per `evaluation/scripts/SETUP_MULRAN_BENCHMARK.md`.
- **Newer College**: 2 pending manifests in `experiments/pending/` (`ct_icp_newer_college_01_short_120_matrix.json`, `litamin2_newer_college_01_short_120_matrix.json`). **Blocked on two fronts**: (a) the manifests target sequence `01_short_experiment`, which is not in `/media/sasaki/aiueo/ai_coding_ws/ros2/demo_data/newer_college/` — only `math_hard` is local; (b) the local `math_hard` data is a ROS2 bag (`math_hard.bag` + `_rosbag2.db3`) with `/os_node/lidar_packets` (raw Ouster packets), not the flat PCD layout expected by `prepare_newer_college_inputs.py`. Extracting requires either downloading the flat-file release from https://ori-drs.github.io/newer-college-dataset/, or setting up ROS2 + Ouster driver to replay the bag and save PCDs.

### 9.2 Best next LiDAR target

**MulRan dcc01 ingestion** to complete the MulRan coverage. Reuse the existing parkinglot ingestion approach. Friendly format, broadens public-data evidence on the LiDAR-only surface. Pending the download form.

### 9.3 Best next camera-aware target

**Newer College**. LiDAR + IMU + camera; lets the repo claim multimodal coverage beyond KITTI Raw. Pending the flat-file download or a bag-extraction setup.

### 9.4 Future heavier option

**Oxford Spires**. Newer, larger, more visually attractive, but higher ingestion cost than MulRan.

### 9.5 Practical recommendation

If the user says "do something useful without waiting for KITTI Odometry / MulRan dcc01 / Newer College data":

- **Phase A reruns first** (LiTAMIN2/CT-ICP on local MCD + HDL-400 with RPE) — cheapest claim-level improvement.
- **Cross-method --no-gt-seed sweep** on local KITTI Raw 0009 full — already done 2026-05-18, see `docs/dogfooding_methodology.md`. Extending to other local datasets (MulRan parkinglot full, MCD KTH/NTU/TUHH) would cost almost nothing.
- **Then MulRan dcc01** when data lands — natural extension of partially-done work.

---

## 10. Tests and Verification

### 10.1 Last known-green verification (2026-05-17)

- `python3 -m unittest discover -s tests -p 'test_*.py' -v` → **14/14 passed**
- `cmake --build build --target pcd_dogfooding multimodal_dogfooding` → built clean
- End-to-end runner smoke against `experiments/kiss_icp_kitti_raw_0009_matrix.json` → 3 variants completed, aggregate JSON emits new `"status": "ok"` taxonomy.

### 10.2 What the tests cover

- `run_experiment_matrix.py` — reuse-aggregate, timeout (now expects `"timeout_budget"`), RPE parsing.
- `refresh_study_docs.py`
- `generate_reproduction_status.py`
- `generate_paper_comparison.py` data plumbing
- `run_multimodal_study.py`
- Synthetic multimodal fixture generator
- KITTI Odometry preparation script

### 10.3 What did not run recently

- Full `ctest`
- Full Docker rebuild
- Any real KITTI Odometry full-sequence benchmark run
- Any new external dataset import beyond what is already on disk

Be precise about verification scope when reporting.

---

## 11. Generated Docs

Generated docs already reflect canonical aggregate state at HEAD `5a96dec`:

- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/paper_comparison.md`](docs/paper_comparison.md)
- [`docs/variant_analysis.md`](docs/variant_analysis.md)
- [`docs/reproduction_status.md`](docs/reproduction_status.md) — now includes claim-level legend.

To refresh all of them: `python3 evaluation/scripts/refresh_study_docs.py`.

---

## 12. What Cursor / Codex Should Do Next

This is the operational handoff. Pick a single path and finish it before switching.

### Priority B (main path): KITTI Odometry full reruns

Blocked only by data.

1. Obtain KITTI Odometry root containing at minimum:
   - `sequences/00/velodyne` + `poses/00.txt`
   - `sequences/07/velodyne` + `poses/07.txt`
2. Prep:
   ```bash
   python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
     --kitti-root <path> --sequence 00 --sequence 07 \
     --window-size 108 --include-full
   ```
3. Run 108-frame manifests first:
   ```bash
   python3 evaluation/scripts/run_experiment_matrix.py \
     --manifest experiments/pending/litamin2_kitti_seq_00_matrix.json \
     --manifest experiments/pending/litamin2_kitti_seq_07_matrix.json \
     --manifest experiments/pending/ct_icp_kitti_seq_00_matrix.json \
     --manifest experiments/pending/ct_icp_kitti_seq_07_matrix.json
   ```
4. Then full-sequence manifests (longer; use `extended_full_1800s` budget profile if needed).
5. After both succeed and RPE matches paper-reported values within reasonable tolerance, update `paper_reported_numbers.json` `claim_level` for `litamin2` / `ct_icp` from `indicative` → `approximately_reproduced` (or `reproduced` if metrics agree directly).
6. Only after successful runs, promote pending manifests to active.

### Priority A (parallel, no new data needed)

1. Rerun `LiTAMIN2` and `CT-ICP` on `mcd_kth_day_06_108`, `mcd_ntu_day_02_108`, `mcd_tuhh_night_09_108`, `hdl_400_open_ct_lio_120` with RPE exported.
2. Confirm aggregates contain `rpe_trans_pct` (rerun if not).
3. Add a section to `paper_comparison.md` highlighting paper-style comparison on local public data.
4. Avoid touching unrelated manifest changes.

### Priority C (broaden public data)

1. Ingest **MulRan dcc01** — reuse parkinglot ingestion approach.
2. Promote `experiments/pending/*_mulran_dcc01_120_matrix.json` after a successful smoke.
3. Next iteration: **Newer College** scaffolding.

### Do NOT do next (unless explicitly asked)

- Paper drafting / prose generation
- PR / branch cleanup unrelated to current task
- Broad refactor that touches the stable contract
- Reverting any pending manifest
- Adding new method families beyond the 33 already indexed

---

## 13. Commands Reference

### 13.1 Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)" --target pcd_dogfooding multimodal_dogfooding
```

### 13.2 Python regression

```bash
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 evaluation/scripts/verify_environment.py
```

### 13.3 Full docs refresh

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

### 13.4 LiDAR smoke

```bash
./build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> \
  --methods litamin2,gicp,ndt \
  --summary-json /tmp/smoke.json
```

### 13.5 Multimodal smoke

```bash
./build/evaluation/multimodal_dogfooding <sequence_dir> <gt_csv> \
  --methods okvis,vins_fusion,fast_livo2 \
  --landmarks-csv <sequence_dir>/landmarks.csv \
  --visual-observations-csv <sequence_dir>/visual_observations.csv \
  --summary-json /tmp/multimodal_smoke.json
```

### 13.6 KITTI Raw multimodal prep

```bash
python3 evaluation/scripts/prepare_kitti_multimodal_inputs.py --include-full
python3 evaluation/scripts/run_multimodal_study.py --include-full --method okvis
```

### 13.7 KITTI Odometry prep

```bash
python3 evaluation/scripts/prepare_kitti_odometry_inputs.py \
  --kitti-root /path/to/data_odometry \
  --sequence 00 --sequence 07 \
  --window-size 108 --include-full
```

### 13.8 Safe exploratory runs

Use `/tmp` outputs when you do not want to disturb canonical repo docs / index:

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/<something>.json \
  --output-dir /tmp/localization_zoo_results \
  --docs-dir /tmp/localization_zoo_docs
```

---

## 14. Final Notes

- The repo is already valuable as a benchmark OSS. The bottleneck is **publication-grade reproduction evidence**, not more methods.
- The cheapest claim-level upgrade is **Priority A** (local MCD/HDL-400 reruns with RPE). The strongest is **Priority B** (full KITTI Odometry).
- The status / claim-level / budget / families taxonomies are now in place; new evidence layers can plug into them without further scaffolding work.
- Keep the stable summary contract small; new failure modes flow through `MethodResult::status` strings, not new boolean flags.
- Do not oversell reproduction status. The taxonomy exists precisely so the repo can be honest about what each family demonstrates.
