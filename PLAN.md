# Localization Zoo - Codex / Cursor 引き継ぎ PLAN

> **最終更新: 2026-04-15**
>
> この文書は、次の AI アシスタントが repo の現在地、dirty worktree、直近の実装差分、次にやるべきことを短時間で掴むための handoff。
>
> 最初に本ファイルを読み、その次に:
> 1. [`experiments/results/index.json`](experiments/results/index.json)
> 2. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
> 3. [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
> 4. [`docs/reproduction_status.md`](docs/reproduction_status.md)
> 5. [`docs/variant_analysis.md`](docs/variant_analysis.md)

---

## 1. Executive Summary

### 1.1 Current indexed artifact

| Item | Value |
|------|-------|
| Branch | `wip/profile-expansion-refresh` |
| HEAD | `9d1b2ab` |
| Indexed manifests | **274** |
| Indexed ready | **260** |
| Indexed blocked | **1** |
| Indexed skipped | **13** |
| LiDAR families | **27** |
| Camera-aware families | **6** |
| Total active selectors | **33** |
| Canonical multimodal manifests | **24** |
| Pending manifests | **94** |

### 1.2 What changed recently

- `pcd_dogfooding --summary-json` now exports optional `rpe_trans_pct` and `rpe_rot_deg_per_m`.
- `run_experiment_matrix.py` now preserves those RPE fields through aggregate JSON.
- `generate_paper_comparison.py` now renders repo-side RPE when available.
- canonical KITTI Raw `0009` aggregates for `LiTAMIN2` and `CT-ICP` were rerun, so those two families now have real RPE in repo-managed aggregate JSON.
- KITTI Odometry public velodyne+poses preparation was generalized:
  - new [`evaluation/scripts/prepare_kitti_odometry_inputs.py`](evaluation/scripts/prepare_kitti_odometry_inputs.py)
  - old [`evaluation/scripts/setup_kitti_benchmark.sh`](evaluation/scripts/setup_kitti_benchmark.sh) is now a thin wrapper
  - new pending full-sequence manifests for `LiTAMIN2` and `CT-ICP` were added under `experiments/pending/`
- multimodal integration, timeout handling, smoke tests, and wrapper regressions already exist and are working.

### 1.3 Current user preference

- User explicitly said: **paper related work is not the priority right now**.
- Prioritize:
  - benchmark infra
  - reproducibility
  - public-data ingestion
  - making the OSS actually useful
- Do **not** spend the next turn writing prose for a paper unless the user asks again.

---

## 2. Critical Worktree Warning

This worktree is intentionally dirty. There are many tracked deletions and many untracked files.

### 2.1 Do not revert these automatically

- many tracked deletions under `experiments/*_matrix.json`
- many untracked multimodal manifests under `experiments/`
- many untracked multimodal aggregate JSON files under `experiments/results/`
- untracked docs/scripts/tests such as:
  - [`docs/reproduction_status.md`](docs/reproduction_status.md)
  - [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)
  - [`evaluation/scripts/run_multimodal_study.py`](evaluation/scripts/run_multimodal_study.py)
  - [`evaluation/scripts/prepare_kitti_multimodal_inputs.py`](evaluation/scripts/prepare_kitti_multimodal_inputs.py)
  - [`evaluation/scripts/prepare_kitti_odometry_inputs.py`](evaluation/scripts/prepare_kitti_odometry_inputs.py)
  - [`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py)

Treat these as active local work, not accidental clutter.

### 2.2 Important implication

- `experiments/results/index.json` is currently canonical for this dirty worktree.
- `git status` does **not** represent a simple "few local edits" state.
- Any cleanup or promotion work must be deliberate.

---

## 3. Stable Core vs Experiment Layer

### 3.1 Stable binaries

- [`build/evaluation/pcd_dogfooding`](build/evaluation/pcd_dogfooding)
  - LiDAR-only stable benchmark binary
  - 27 method families
  - shared `--summary-json` contract
- [`build/evaluation/multimodal_dogfooding`](build/evaluation/multimodal_dogfooding)
  - camera-aware sibling binary
  - 6 method families
  - same aggregate style and same runner contract

### 3.2 Stable result contract

The shared per-method summary now includes:

- `status`
- `ate_m`
- `rpe_trans_pct` (optional)
- `rpe_rot_deg_per_m` (optional)
- `frames`
- `time_ms`
- `fps`
- `note`

The RPE fields are now part of the stable contract for `pcd_dogfooding` outputs, but older aggregate JSON files may still contain `null` or omit them.

### 3.3 Experiment layer

- manifests: `experiments/*_matrix.json`
- pending manifests: `experiments/pending/*_matrix.json`
- aggregates: `experiments/results/*.json`
- docs generated from aggregates:
  - `docs/experiments.md`
  - `docs/decisions.md`
  - `docs/interfaces.md`
  - `docs/paper_comparison.md`
  - `docs/variant_analysis.md`

---

## 4. Local Data Actually Available Right Now

This is important because the next assistant should not assume that every public dataset mentioned in docs is locally present.

### 4.1 Present in `dogfooding_results/`

| Dataset | Path | Frames | Extra files |
|--------|------|--------|-------------|
| HDL-400 native/reference-like | `dogfooding_results/hdl_400_open_ct_lio_120` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 synthetic time (azimuth) | `dogfooding_results/hdl_400_open_ct_lio_120_time_azimuth` | 120 | `imu.csv`, `frame_timestamps.csv` |
| HDL-400 public ROS1 synthetic time | `dogfooding_results/hdl_400_ros1_open_ct_lio_120_time_index` | 120 | `imu.csv`, `frame_timestamps.csv` |
| KITTI Raw 0009 short | `dogfooding_results/kitti_raw_0009_200` | 200 | `frame_timestamps.csv`, `landmarks.csv`, `visual_observations.csv`, `camera_args.txt` |
| KITTI Raw 0009 full | `dogfooding_results/kitti_raw_0009_full` | 443 | same multimodal extras |
| KITTI Raw 0061 short | `dogfooding_results/kitti_raw_0061_200` | 200 | same multimodal extras |
| KITTI Raw 0061 full | `dogfooding_results/kitti_raw_0061_full` | 703 | same multimodal extras |
| MCD KTH day-06 | `dogfooding_results/mcd_kth_day_06_108` | 108 | `frame_timestamps.csv` |
| MCD NTU day-02 | `dogfooding_results/mcd_ntu_day_02_108` | 108 | `frame_timestamps.csv` |
| MCD TUHH night-09 | `dogfooding_results/mcd_tuhh_night_09_108` | 108 | `frame_timestamps.csv` |

### 4.2 Present in `experiments/reference_data/`

- `hdl_400_public_reference.csv`
- `hdl_400_public_reference_b.csv`
- `kitti_raw_0009_200_gt.csv`
- `kitti_raw_0009_full_gt.csv`
- `kitti_raw_0061_200_gt.csv`
- `kitti_raw_0061_full_gt.csv`
- `mcd_kth_day_06_108_gt.csv`
- `mcd_ntu_day_02_108_gt.csv`
- `mcd_tuhh_night_09_108_gt.csv`

### 4.3 Not present locally right now

- KITTI Odometry public `sequences/00`, `sequences/07`, `poses/00.txt`, `poses/07.txt`
- Istanbul local windows
- any newly downloaded external public dataset such as MulRan / Newer College / Oxford Spires

This matters because:

- full KITTI Odometry reruns are **prepared in code but not yet runnable here**
- next assistant should not claim those runs were executed

---

## 5. Indexed Method Surface

### 5.1 LiDAR-only indexed families (27)

`aloam`, `balm2`, `clins`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `fast_lio2`, `fast_lio_slam`, `floam`, `gicp`, `hdl_graph_slam`, `isc_loam`, `kiss_icp`, `lego_loam`, `lins`, `lio_sam`, `litamin2`, `loam_livox`, `mulls`, `ndt`, `point_lio`, `small_gicp`, `suma`, `vgicp_slam`, `voxel_gicp`, `xicp`

### 5.2 Camera-aware indexed families (6)

`vins_fusion`, `okvis`, `orb_slam3`, `lvi_sam`, `fast_livo2`, `r2live`

### 5.3 Non-indexed or out-of-surface papers

Still outside the current benchmark surface:

- `vilens`
- `relead`
- `ct_icp_relead`
- `scan_context`
- `imu_preintegration`
- `cube_lio_repro`

LiDAR-only benchmark coverage is already broad. The main missing frontier is not "more paper folders", but "more public data / stronger reproduction protocol".

---

## 6. Multimodal State

### 6.1 Canonical scope

- binary: [`evaluation/src/multimodal_dogfooding.cpp`](evaluation/src/multimodal_dogfooding.cpp)
- manifests: 24 canonical KITTI Raw multimodal manifests
- windows:
  - `kitti_raw_0009_200`
  - `kitti_raw_0009_full`
  - `kitti_raw_0061_200`
  - `kitti_raw_0061_full`

### 6.2 Input contract

Each multimodal run expects:

- `sequence_dir`
- `gt_csv`
- `landmarks.csv`
- `visual_observations.csv`
- camera intrinsics via CLI / `camera_args.txt`

### 6.3 Current results

Fully practical under current study budget:

- `okvis`
- `vins_fusion`
- `fast_livo2`

Practical-budget timeouts:

- `orb_slam3`
- `lvi_sam`
- `r2live`

Current canonical timeout budget:

- `200-frame windows`: `120s`
- `full windows`: `300s`

This is already encoded in multimodal manifests via `problem.variant_timeout_seconds`.

### 6.4 Key evidence already known

- `okvis`, `vins_fusion`, `fast_livo2` each finish all four KITTI Raw windows
- all three currently adopt `fast`
- `lvi_sam` and `r2live` can finish the easiest `0061_200 fast` probe, but only around `0.5-0.6 FPS`
- `orb_slam3` remained too heavy for the practical budget

### 6.5 Infra around multimodal

Already exists and works:

- [`evaluation/scripts/prepare_kitti_multimodal_inputs.py`](evaluation/scripts/prepare_kitti_multimodal_inputs.py)
- [`evaluation/scripts/generate_kitti_visual_observations.py`](evaluation/scripts/generate_kitti_visual_observations.py)
- [`evaluation/scripts/run_multimodal_study.py`](evaluation/scripts/run_multimodal_study.py)
- [`evaluation/scripts/smoke_multimodal_fixture.sh`](evaluation/scripts/smoke_multimodal_fixture.sh)

---

## 7. Reproduction / Paper-Result Status

### 7.1 What was fixed recently

The repo now states reproduction status more honestly and more concretely:

- [`docs/reproduction_status.md`](docs/reproduction_status.md)
- [`evaluation/data/paper_reported_numbers.json`](evaluation/data/paper_reported_numbers.json)
- [`evaluation/scripts/generate_reproduction_status.py`](evaluation/scripts/generate_reproduction_status.py)

This is the current truth boundary:

- the repo is useful as a benchmark OSS
- it is **not** yet generally entitled to say "paper results reproduced"

### 7.2 What became better

`LiTAMIN2` and `CT-ICP` are now closer to reproducible paper-style evaluation because:

- `pcd_dogfooding` exports RPE
- aggregates preserve RPE
- `paper_comparison.md` now shows repo-side RPE where available

### 7.3 Canonical KITTI Raw 0009 reruns with RPE

These two aggregate files were actually rerun locally:

- [`experiments/results/litamin2_kitti_raw_0009_matrix.json`](experiments/results/litamin2_kitti_raw_0009_matrix.json)
- [`experiments/results/ct_icp_kitti_raw_0009_matrix.json`](experiments/results/ct_icp_kitti_raw_0009_matrix.json)

Current adopted defaults from those reruns:

- `LiTAMIN2`
  - adopted: `fast_cov_half_threads`
  - `ATE 1.053 m`
  - `RPE trans 0.742 %`
  - `RPE rot 0.004224 deg/m`
  - `34.16 FPS`
- `CT-ICP`
  - adopted: `fast_window`
  - `ATE 2.728 m`
  - `RPE trans 2.198 %`
  - `RPE rot 0.023386 deg/m`
  - `49.27 FPS`

### 7.4 What is still missing for actual reproduction

The real missing piece is **full KITTI Odometry data**, not code.

The code path now exists for:

- `kitti_seq_00_108`
- `kitti_seq_07_108`
- `kitti_seq_00_full`
- `kitti_seq_07_full`

But those inputs are not present locally, so no full-sequence public KITTI Odometry reruns have been performed in this worktree.

---

## 8. KITTI Odometry Preparation State

### 8.1 New preparation script

Added:

- [`evaluation/scripts/prepare_kitti_odometry_inputs.py`](evaluation/scripts/prepare_kitti_odometry_inputs.py)

It converts:

- KITTI Odometry `sequences/<seq>/velodyne/*.bin`
- KITTI Odometry `poses/<seq>.txt`

into:

- `dogfooding_results/kitti_seq_<seq>_<window>`
- `dogfooding_results/kitti_seq_<seq>_full`
- `experiments/reference_data/kitti_seq_<seq>_<window>_gt.csv`
- `experiments/reference_data/kitti_seq_<seq>_full_gt.csv`

### 8.2 Compatibility wrapper

[`evaluation/scripts/setup_kitti_benchmark.sh`](evaluation/scripts/setup_kitti_benchmark.sh) now delegates to the new Python script.

### 8.3 New pending full-sequence manifests

Added:

- [`experiments/pending/litamin2_kitti_seq_00_full_matrix.json`](experiments/pending/litamin2_kitti_seq_00_full_matrix.json)
- [`experiments/pending/litamin2_kitti_seq_07_full_matrix.json`](experiments/pending/litamin2_kitti_seq_07_full_matrix.json)
- [`experiments/pending/ct_icp_kitti_seq_00_full_matrix.json`](experiments/pending/ct_icp_kitti_seq_00_full_matrix.json)
- [`experiments/pending/ct_icp_kitti_seq_07_full_matrix.json`](experiments/pending/ct_icp_kitti_seq_07_full_matrix.json)

### 8.4 Regression coverage

[`tests/test_experiment_scripts.py`](tests/test_experiment_scripts.py) now includes a fake KITTI Odometry root test that verifies:

- window export works
- full export works
- GT CSV files are generated
- the new preparation script stays contract-compatible

---

## 9. Available Public-Data Directions Beyond KITTI

User asked about good public datasets beyond KITTI. The current recommendation order is:

### 9.1 Best next LiDAR-only target: MulRan

Why:

- official LiDAR + trajectory
- multiple sequences
- format is relatively friendly to this repo
- strong practical value for the OSS

Good fit for next engineering task:

- add import script
- prepare manifests
- benchmark existing LiDAR families on a new public family

### 9.2 Best next camera-aware target: Newer College

Why:

- LiDAR + IMU + camera
- strong multimodal public benchmark
- useful if the repo wants to become more than a KITTI-only multimodal pilot

### 9.3 Strong but heavier future option: Oxford Spires

Why:

- newer
- larger
- visually attractive as a research OSS target

But ingestion cost is higher than MulRan.

### 9.4 Practical recommendation

If the user says "do something useful without waiting for KITTI Odometry data", the best next move is:

- **MulRan ingestion first**

If the user says "push multimodal harder", then:

- **Newer College**

---

## 10. Tests and Verification

### 10.1 Recent local verification that did run

- `python3 -m unittest discover -s tests -p 'test_*.py' -v`
  - **12/12 passed**
- `python3 -m py_compile ...`
  - passed
- `python3 evaluation/scripts/verify_environment.py`
  - passed
- `cmake --build build --target pcd_dogfooding`
  - passed

### 10.2 What those tests cover now

- `run_experiment_matrix.py`
  - reuse-aggregate
  - timeout
  - RPE parsing
- `refresh_study_docs.py`
- `generate_reproduction_status.py`
- `generate_paper_comparison.py` data plumbing
- `run_multimodal_study.py`
- synthetic multimodal fixture generator
- KITTI Odometry preparation script

### 10.3 What did not run recently

- full `ctest`
- full Docker rebuild
- any real KITTI Odometry full-sequence benchmark run
- any new external dataset import beyond what is already on disk

So be precise when describing verification scope.

---

## 11. Current Generated Docs Situation

### 11.1 Generated docs that already reflect current canonical aggregate state

- [`docs/interfaces.md`](docs/interfaces.md)
- [`docs/experiments.md`](docs/experiments.md)
- [`docs/decisions.md`](docs/decisions.md)
- [`docs/paper_comparison.md`](docs/paper_comparison.md)

### 11.2 Generated docs caveat

Because the worktree contains large intentional deletions and many untracked files:

- do not casually run broad cleanup
- do not assume generated docs can be safely recommitted without understanding which manifests are intentionally in or out of the current local state

---

## 12. What Cursor Should Do Next

This section is the actual operational handoff.

### Option A: stay fully local, no new external data

Best next task:

- extend the new RPE-aware comparison surface to existing local public datasets:
  - MCD
  - HDL-400

Concretely:

1. rerun a small curated subset of `LiTAMIN2` and `CT-ICP` on MCD / HDL-400 if needed
2. inspect whether current aggregates already contain enough information for updated reproduction-facing comparisons
3. avoid touching unrelated manifest deletions

### Option B: continue the reproduction path

Blocked only by data:

1. obtain KITTI Odometry root with:
   - `sequences/00/velodyne`
   - `sequences/07/velodyne`
   - `poses/00.txt`
   - `poses/07.txt`
2. run:
   - `python3 evaluation/scripts/prepare_kitti_odometry_inputs.py --kitti-root <path> --sequence 00 --sequence 07 --window-size 108 --include-full`
3. execute pending manifests:
   - `litamin2_kitti_seq_00_matrix.json`
   - `litamin2_kitti_seq_07_matrix.json`
   - `ct_icp_kitti_seq_00_matrix.json`
   - `ct_icp_kitti_seq_07_matrix.json`
   - then full variants
4. only after successful runs, consider promotion from `experiments/pending/` to active manifests

### Option C: best new public dataset for the OSS

If the user wants progress without KITTI:

- build `MulRan` support next

Suggested scope:

1. write import/prep script for MulRan LiDAR + GT
2. create a small canonical window set
3. start with `LiTAMIN2`, `CT-ICP`, `GICP`, `KISS-ICP`
4. keep the same stable contract and manifest shape

### Option D: do not do this next unless explicitly asked

- paper drafting
- PR polishing
- broad git cleanup
- removing large untracked multimodal work
- reverting `experiments/*` deletions

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
  --sequence 00 \
  --sequence 07 \
  --window-size 108 \
  --include-full
```

Compatibility wrapper:

```bash
bash evaluation/scripts/setup_kitti_benchmark.sh /path/to/data_odometry --include-full
```

### 13.8 Targeted pending KITTI Odometry runs

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/pending/litamin2_kitti_seq_00_matrix.json \
  --manifest experiments/pending/litamin2_kitti_seq_07_matrix.json \
  --manifest experiments/pending/ct_icp_kitti_seq_00_matrix.json \
  --manifest experiments/pending/ct_icp_kitti_seq_07_matrix.json
```

Then full-sequence:

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/pending/litamin2_kitti_seq_00_full_matrix.json \
  --manifest experiments/pending/litamin2_kitti_seq_07_full_matrix.json \
  --manifest experiments/pending/ct_icp_kitti_seq_00_full_matrix.json \
  --manifest experiments/pending/ct_icp_kitti_seq_07_full_matrix.json
```

### 13.9 Safe exploratory runs

Use `/tmp` outputs when you do not want to disturb canonical repo docs/index:

```bash
python3 evaluation/scripts/run_experiment_matrix.py \
  --manifest experiments/<something>.json \
  --output-dir /tmp/localization_zoo_results \
  --docs-dir /tmp/localization_zoo_docs
```

---

## 14. Final Notes For Cursor

- The repo is already valuable as a benchmark OSS.
- The next leverage point is **data**, not another large refactor.
- The most honest short-term improvement is either:
  - get KITTI Odometry full data and run it, or
  - add MulRan as the next public family.
- Be careful with the dirty worktree.
- Do not oversell reproduction status.
- Keep the stable contract small and the experiment layer explicit.
