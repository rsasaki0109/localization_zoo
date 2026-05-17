# Localization Zoo - Codex / Cursor 引き継ぎ PLAN

> **最終更新: 2026-05-17**
>
> この文書は、次の AI アシスタントが repo の現在地、最近の差分、次にやるべきことを短時間で掴むための handoff。
>
> 最初に本ファイルを読み、その次に:
> 1. [`experiments/results/index.json`](experiments/results/index.json)
> 2. [`docs/status_taxonomy.md`](docs/status_taxonomy.md)
> 3. [`docs/budget_profiles.md`](docs/budget_profiles.md)
> 4. [`experiments/families.json`](experiments/families.json)
> 5. [`docs/reproduction_status.md`](docs/reproduction_status.md)
> 6. [`evaluation/data/paper_reported_numbers.json`](evaluation/data/paper_reported_numbers.json)
> 7. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 8. [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)

---

## 1. Executive Summary

### 1.1 Current indexed artifact

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD | `5a96dec` |
| Worktree | **clean** |
| Indexed manifests | **282** |
| Indexed ready | **268** |
| Indexed blocked | **1** |
| Indexed skipped | **13** |
| Pending manifests | **100** |
| LiDAR families | **27** |
| Camera-aware families | **6** |
| Total active selectors | **33** |
| Python tests | **14/14 pass** |

### 1.2 What changed recently (2026-05-17 session)

Foundational taxonomy layer was added without changing runtime numerics:

- **Claim-level taxonomy**: `reproduced > approximately_reproduced > indicative > smoke > ported`. Bumped into [`evaluation/data/paper_reported_numbers.json`](evaluation/data/paper_reported_numbers.json) (schema v3) and rendered into [`docs/reproduction_status.md`](docs/reproduction_status.md) as a legend column.
- **Budget profile contract**: [`docs/budget_profiles.md`](docs/budget_profiles.md) defines `smoke_200f_120s`, `practical_full_300s`, `extended_full_1800s`, `reference_full_unbounded`. Manifests reference them via `problem.budget_profile`.
- **Family registry**: [`experiments/families.json`](experiments/families.json) classifies the 33 method families into `core` / `extended` tiers and `maintained` / `timeout_prone` / `input_constrained` / `legacy` / `experimental` status.
- **Status taxonomy migration**: [`docs/status_taxonomy.md`](docs/status_taxonomy.md) defines the target per-variant enum. C++ binaries (`pcd_dogfooding`, `multimodal_dogfooding`) and the runner now emit the lower-case enum (`ok`, `skipped`, `timeout_budget`). Reserved values (`tracking_lost`, `init_failed`, `input_unsupported`, `metric_invalid`, `no_gt`) have field space in `MethodResult::status` but no detection logic yet. Legacy uppercase (`OK`/`SKIPPED`/`TIMED_OUT`) is normalized at ingest.

Earlier session work (already in `main` history):

- MulRan / Newer College benchmark scaffolding (commit `aee7611`).
- Short-trajectory RPE fix and `paper_comparison.md` refresh.
- `pcd_dogfooding --summary-json` exports optional `rpe_trans_pct` / `rpe_rot_deg_per_m`.
- KITTI Odometry preparation script `evaluation/scripts/prepare_kitti_odometry_inputs.py` generalized; `setup_kitti_benchmark.sh` is a wrapper.

### 1.3 Current direction (post-GPT-pro consultation)

The repo is useful as a benchmark OSS. The next leverage point is **publication-grade reproduction evidence**, not more methods or more refactoring. Priority order:

- **B (main)**: KITTI Odometry full-sequence reruns for `LiTAMIN2` and `CT-ICP` with `RPE translation [%]`. Target: graduate those two families from `indicative` to `approximately_reproduced` (or `reproduced` if metrics agree). Pending manifests already exist in `experiments/pending/`; only data + run time is missing.
- **A (parallel)**: Stay-local reruns of `LiTAMIN2` / `CT-ICP` on `MCD` and `HDL-400` with RPE exported. Lower-cost evidence that complements B and produces immediate `approximately_reproduced`-grade aggregates against local public data.
- **C (next)**: Broaden ingestion. `MulRan parkinglot` is already partially ingested (8 indexed manifests, local data present); next steps are `MulRan dcc01` (2 pending manifests already exist) and Newer College.

Do **not** spend the next turn on paper drafting, PR polishing, or speculative refactoring. The user has been explicit that this is OSS infrastructure work, not paper writing.

---

## 2. Worktree State

Working tree is clean as of HEAD `5a96dec`. Previous handoffs warned about a dirty worktree with mass untracked multimodal work — that state has since been committed. Treat git status as authoritative.

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

- Manifests: `experiments/*_matrix.json` (282 active)
- Pending manifests: `experiments/pending/*_matrix.json` (100)
- Aggregates: `experiments/results/*.json` (282)
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
| KITTI Raw 0009 short | `kitti_raw_0009_200` | 200 | full multimodal extras |
| KITTI Raw 0009 full | `kitti_raw_0009_full` | 443 | full multimodal extras |
| KITTI Raw 0061 short | `kitti_raw_0061_200` | 200 | full multimodal extras |
| KITTI Raw 0061 full | `kitti_raw_0061_full` | 703 | full multimodal extras |
| MCD KTH day-06 | `mcd_kth_day_06_108` | 108 | `frame_timestamps.csv` |
| MCD NTU day-02 | `mcd_ntu_day_02_108` | 108 | `frame_timestamps.csv` |
| MCD TUHH night-09 | `mcd_tuhh_night_09_108` | 108 | `frame_timestamps.csv` |
| MulRan parkinglot 120 | `mulran_parkinglot_120` | 120 | (LiDAR-only) |
| MulRan parkinglot full | `mulran_parkinglot_full` | full | (LiDAR-only) |

### 4.2 Present in `experiments/reference_data/`

GT CSVs for every dataset listed in 4.1. Verify by listing the directory before quoting paths.

### 4.3 Not present locally right now

- KITTI Odometry public `sequences/00`, `sequences/07`, `poses/00.txt`, `poses/07.txt`
- Istanbul local windows
- MulRan dcc01 (pending manifests exist; data not yet ingested)
- Newer College, Oxford Spires

This matters because:

- Full KITTI Odometry reruns are **prepared in code but not yet runnable here**.
- The next assistant must not claim those runs were executed.

---

## 5. Indexed Method Surface

### 5.1 LiDAR-only families (27)

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

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

- **Priority B** (data-bound): full KITTI Odometry `sequences/00` + `sequences/07` with paper-reported RPE `[%]`.
- **Priority A** (no new data): rerun MCD and HDL-400 with RPE exported, publish side-by-side against paper-style numbers where appropriate.

Both A and B can move the needle on the same claim level; B is closer to a direct paper comparison but blocked on data.

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

### 8.3 Pending full-sequence manifests

- [`experiments/pending/litamin2_kitti_seq_00_full_matrix.json`](experiments/pending/litamin2_kitti_seq_00_full_matrix.json)
- [`experiments/pending/litamin2_kitti_seq_07_full_matrix.json`](experiments/pending/litamin2_kitti_seq_07_full_matrix.json)
- [`experiments/pending/ct_icp_kitti_seq_00_full_matrix.json`](experiments/pending/ct_icp_kitti_seq_00_full_matrix.json)
- [`experiments/pending/ct_icp_kitti_seq_07_full_matrix.json`](experiments/pending/ct_icp_kitti_seq_07_full_matrix.json)

Plus 108-frame variants for the same selectors. Promotion from `experiments/pending/` to active happens after successful runs.

### 8.4 Regression coverage

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) contains a fake KITTI Odometry root test that verifies window export, full export, GT CSV generation, and contract compatibility of the preparation script.

---

## 9. Public-Data Direction Beyond KITTI

### 9.1 Already partially in (do not redo from scratch)

- **MulRan parkinglot**: ingested. 8 indexed manifests covering `LiTAMIN2`, `CT-ICP`, `KISS-ICP`, `GICP` × {120-frame, full}.
- **MulRan dcc01**: 2 pending manifests in `experiments/pending/` (`*_mulran_dcc01_120_matrix.json`). Data not yet ingested.

### 9.2 Best next LiDAR target

**MulRan dcc01 ingestion** to complete the MulRan coverage. Reuse the existing parkinglot ingestion approach. Friendly format, broadens public-data evidence on the LiDAR-only surface.

### 9.3 Best next camera-aware target

**Newer College**. LiDAR + IMU + camera; lets the repo claim multimodal coverage beyond KITTI Raw.

### 9.4 Future heavier option

**Oxford Spires**. Newer, larger, more visually attractive, but higher ingestion cost than MulRan.

### 9.5 Practical recommendation

If the user says "do something useful without waiting for KITTI Odometry data":

- **Phase A reruns first** (LiTAMIN2/CT-ICP on local MCD + HDL-400 with RPE) — cheapest claim-level improvement.
- **Then MulRan dcc01** — natural extension of partially-done work.

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
