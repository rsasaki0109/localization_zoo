# Minimal Interfaces

_Generated at 2026-04-07T13:50:03+00:00 by `evaluation/scripts/run_experiment_matrix.py`. Source index: `experiments/results/index.json`._

## Stable Core

### CLI

`build/evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] --methods <selector> --summary-json <path> [variant flags...]`

Current active binaries: `build/evaluation/pcd_dogfooding`

### Summary JSON Contract

| Field | Type | Meaning |
|-------|------|---------|
| `pcd_dir` | string | Input PCD sequence directory. |
| `gt_csv` | string | Reference trajectory CSV matched to the sequence. |
| `num_frames` | integer | Number of frames evaluated. |
| `trajectory_length_m` | number | Total GT path length in meters. |
| `timestamp_source` | string | Timestamp source used by the evaluator. |
| `methods[]` | array | Per-method results emitted by the benchmark. |
| `methods[].name` | string | Human-readable method name. |
| `methods[].status` | string | `OK` or `SKIPPED`. |
| `methods[].ate_m` | number or null | Absolute trajectory error in meters. |
| `methods[].frames` | integer | Number of poses evaluated for the method. |
| `methods[].time_ms` | number or null | End-to-end runtime in milliseconds. |
| `methods[].fps` | number or null | Effective frames per second. |
| `methods[].note` | string | Free-form method note or skip reason. |

## Experimental Layer

### Manifest Contract

Every active search problem lives in `experiments/*.json` and must define:

| Field | Type | Meaning |
|-------|------|---------|
| `problem.id` | string | Stable identifier for the search problem. |
| `problem.state` | string | `ready` or `blocked`. Missing means `ready`. |
| `problem.blocker` | string | Why the problem cannot be benchmarked yet. Optional for blocked problems. |
| `problem.next_step` | string | The next concrete step to unblock the problem. Optional for blocked problems. |
| `problem.dataset` | object | Shared dataset paths for every variant. |
| `stable_interface.binary` | string | Stable benchmark entrypoint. |
| `stable_interface.methods` | string | Shared method selector for comparability. |
| `stable_interface.primary_method` | string | Result key to extract from summary JSON. |
| `variants[]` | array | Concrete variants to compare, keep, or discard. |
| `variants[].args` | array | Extra CLI flags layered on the stable core. |

Current active selectors: `aloam`, `ct_icp`, `ct_lio`, `dlio`, `dlo`, `floam`, `gicp`, `kiss_icp`, `lego_loam`, `litamin2`, `mulls`, `ndt`, `small_gicp`, `voxel_gicp`

### Runner Contract

`python3 evaluation/scripts/run_experiment_matrix.py [--manifest <path>]... [--reuse-existing]`

If no manifest is specified, the runner executes every `experiments/*_matrix.json` file.

The runner is responsible for:

- running every variant against the same dataset and method selector
- saving per-run summary JSON and logs under `experiments/results/runs/`
- regenerating `docs/experiments.md`, `docs/decisions.md`, and `docs/interfaces.md`
- writing per-problem aggregate JSON files plus `experiments/results/index.json`
- optionally reusing existing per-variant summaries to avoid rerunning expensive variants

## Stability Boundary

- Stable core: `build/evaluation/pcd_dogfooding` plus the `--summary-json` result contract.
- Experimental surface: manifests, run logs, aggregate results, and generated decision docs.
- Promotion rule: a new default must emerge from shared data and shared metrics, not from a separate code path.

## Active Problems

| Problem | Status | Manifest | Selector | Current Default | Aggregate |
|---------|--------|----------|----------|-----------------|-----------|
| A-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/aloam_kitti_raw_0009_full_matrix.json` | `aloam` | `fast` | `experiments/results/aloam_kitti_raw_0009_full_matrix.json` |
| CT-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `experiments/ct_icp_hdl_400_reference_b_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_hdl_400_reference_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `experiments/ct_icp_hdl_400_reference_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_hdl_400_reference_matrix.json` |
| CT-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `experiments/ct_icp_istanbul_window_b_matrix.json` | `ct_icp` | `balanced_window` | `experiments/results/ct_icp_istanbul_window_b_matrix.json` |
| CT-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `experiments/ct_icp_istanbul_window_c_matrix.json` | `ct_icp` | `balanced_window` | `experiments/results/ct_icp_istanbul_window_c_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/ct_icp_kitti_raw_0009_full_matrix.json` | `ct_icp` | `balanced_window` | `experiments/results/ct_icp_kitti_raw_0009_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `experiments/ct_icp_kitti_raw_0009_matrix.json` | `ct_icp` | `balanced_window` | `experiments/results/ct_icp_kitti_raw_0009_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `experiments/ct_icp_kitti_raw_0009_nogt_matrix.json` | `ct_icp` | `balanced_window` | `experiments/results/ct_icp_kitti_raw_0009_nogt_matrix.json` |
| CT-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `experiments/ct_icp_kitti_raw_0061_full_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_kitti_raw_0061_full_matrix.json` |
| CT-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `experiments/ct_icp_kitti_raw_0061_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_kitti_raw_0061_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/ct_icp_mcd_kth_day_06_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_mcd_kth_day_06_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/ct_icp_mcd_ntu_day_02_matrix.json` | `ct_icp` | `dense_window` | `experiments/results/ct_icp_mcd_ntu_day_02_matrix.json` |
| CT-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/ct_icp_mcd_tuhh_night_09_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_mcd_tuhh_night_09_matrix.json` |
| CT-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `experiments/ct_icp_profile_matrix.json` | `ct_icp` | `fast_window` | `experiments/results/ct_icp_profile_matrix.json` |
| CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data | `blocked` | `experiments/ct_lio_public_readiness_matrix.json` | `ct_lio` | `-` | `experiments/results/ct_lio_public_readiness_matrix.json` |
| CT-LIO reference-trajectory trade-off on the public HDL-400 120-frame window | `ready` | `experiments/ct_lio_reference_profile_matrix.json` | `ct_lio` | `seed_only_fast` | `experiments/results/ct_lio_reference_profile_matrix.json` |
| DLIO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/dlio_kitti_raw_0009_full_matrix.json` | `dlio` | `fast` | `experiments/results/dlio_kitti_raw_0009_full_matrix.json` |
| DLO trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/dlo_kitti_raw_0009_full_matrix.json` | `dlo` | `fast` | `experiments/results/dlo_kitti_raw_0009_full_matrix.json` |
| F-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/floam_kitti_raw_0009_full_matrix.json` | `floam` | `fast` | `experiments/results/floam_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `experiments/gicp_hdl_400_reference_b_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_hdl_400_reference_b_matrix.json` |
| GICP throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `experiments/gicp_hdl_400_reference_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_hdl_400_reference_matrix.json` |
| GICP throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `experiments/gicp_istanbul_window_b_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_istanbul_window_b_matrix.json` |
| GICP throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `experiments/gicp_istanbul_window_c_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_istanbul_window_c_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/gicp_kitti_raw_0009_full_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_kitti_raw_0009_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `experiments/gicp_kitti_raw_0009_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_kitti_raw_0009_matrix.json` |
| GICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `experiments/gicp_kitti_raw_0009_nogt_matrix.json` | `gicp` | `dense_recent_map` | `experiments/results/gicp_kitti_raw_0009_nogt_matrix.json` |
| GICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `experiments/gicp_kitti_raw_0061_full_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_kitti_raw_0061_full_matrix.json` |
| GICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `experiments/gicp_kitti_raw_0061_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_kitti_raw_0061_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/gicp_mcd_kth_day_06_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_mcd_kth_day_06_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/gicp_mcd_ntu_day_02_matrix.json` | `gicp` | `dense_recent_map` | `experiments/results/gicp_mcd_ntu_day_02_matrix.json` |
| GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/gicp_mcd_tuhh_night_09_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_mcd_tuhh_night_09_matrix.json` |
| GICP throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `experiments/gicp_profile_matrix.json` | `gicp` | `fast_recent_map` | `experiments/results/gicp_profile_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second public HDL-400 reference window | `ready` | `experiments/kiss_icp_hdl_400_reference_b_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_hdl_400_reference_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the public HDL-400 reference window | `ready` | `experiments/kiss_icp_hdl_400_reference_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_hdl_400_reference_matrix.json` |
| KISS-ICP throughput and drift trade-off on the second repository-stored Istanbul sequence | `ready` | `experiments/kiss_icp_istanbul_window_b_matrix.json` | `kiss_icp` | `dense_local_map` | `experiments/results/kiss_icp_istanbul_window_b_matrix.json` |
| KISS-ICP throughput and drift trade-off on the third repository-stored Istanbul sequence | `ready` | `experiments/kiss_icp_istanbul_window_c_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_istanbul_window_c_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/kiss_icp_kitti_raw_0009_full_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_kitti_raw_0009_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `experiments/kiss_icp_kitti_raw_0009_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_kitti_raw_0009_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `experiments/kiss_icp_kitti_raw_0009_nogt_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_kitti_raw_0009_nogt_matrix.json` |
| KISS-ICP trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `experiments/kiss_icp_kitti_raw_0061_full_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_kitti_raw_0061_full_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `experiments/kiss_icp_kitti_raw_0061_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_kitti_raw_0061_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/kiss_icp_mcd_kth_day_06_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_mcd_kth_day_06_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/kiss_icp_mcd_ntu_day_02_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_mcd_ntu_day_02_matrix.json` |
| KISS-ICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/kiss_icp_mcd_tuhh_night_09_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_mcd_tuhh_night_09_matrix.json` |
| KISS-ICP throughput and drift trade-off on the repository-stored Istanbul sequence | `ready` | `experiments/kiss_icp_profile_matrix.json` | `kiss_icp` | `fast_recent_map` | `experiments/results/kiss_icp_profile_matrix.json` |
| LeGO-LOAM trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/lego_loam_kitti_raw_0009_full_matrix.json` | `lego_loam` | `fast` | `experiments/results/lego_loam_kitti_raw_0009_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `experiments/litamin2_hdl_400_reference_b_matrix.json` | `litamin2` | `fast_icp_only_half_threads` | `experiments/results/litamin2_hdl_400_reference_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `experiments/litamin2_hdl_400_reference_matrix.json` | `litamin2` | `paper_icp_only_half_threads` | `experiments/results/litamin2_hdl_400_reference_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `experiments/litamin2_istanbul_window_b_matrix.json` | `litamin2` | `fast_icp_only_half_threads` | `experiments/results/litamin2_istanbul_window_b_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `experiments/litamin2_istanbul_window_c_matrix.json` | `litamin2` | `paper_icp_only_half_threads` | `experiments/results/litamin2_istanbul_window_c_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/litamin2_kitti_raw_0009_full_matrix.json` | `litamin2` | `fast_icp_only_half_threads` | `experiments/results/litamin2_kitti_raw_0009_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `experiments/litamin2_kitti_raw_0009_matrix.json` | `litamin2` | `paper_icp_only_half_threads` | `experiments/results/litamin2_kitti_raw_0009_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `experiments/litamin2_kitti_raw_0009_nogt_matrix.json` | `litamin2` | `paper_cov_half_threads` | `experiments/results/litamin2_kitti_raw_0009_nogt_matrix.json` |
| LiTAMIN2 trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `experiments/litamin2_kitti_raw_0061_full_matrix.json` | `litamin2` | `fast_icp_only_half_threads` | `experiments/results/litamin2_kitti_raw_0061_full_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `experiments/litamin2_kitti_raw_0061_matrix.json` | `litamin2` | `fast_cov_half_threads` | `experiments/results/litamin2_kitti_raw_0061_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/litamin2_mcd_kth_day_06_matrix.json` | `litamin2` | `fast_cov_half_threads` | `experiments/results/litamin2_mcd_kth_day_06_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/litamin2_mcd_ntu_day_02_matrix.json` | `litamin2` | `paper_icp_only_half_threads` | `experiments/results/litamin2_mcd_ntu_day_02_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/litamin2_mcd_tuhh_night_09_matrix.json` | `litamin2` | `fast_cov_half_threads` | `experiments/results/litamin2_mcd_tuhh_night_09_matrix.json` |
| LiTAMIN2 throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `experiments/litamin2_profile_matrix.json` | `litamin2` | `fast_icp_only_half_threads` | `experiments/results/litamin2_profile_matrix.json` |
| MULLS trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/mulls_kitti_raw_0009_full_matrix.json` | `mulls` | `fast` | `experiments/results/mulls_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on the second public HDL-400 reference window | `ready` | `experiments/ndt_hdl_400_reference_b_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_hdl_400_reference_b_matrix.json` |
| NDT throughput and accuracy trade-off on the public HDL-400 reference window | `ready` | `experiments/ndt_hdl_400_reference_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_hdl_400_reference_matrix.json` |
| NDT throughput and accuracy trade-off on the second repository-stored Istanbul sequence | `ready` | `experiments/ndt_istanbul_window_b_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_istanbul_window_b_matrix.json` |
| NDT throughput and accuracy trade-off on the third repository-stored Istanbul sequence | `ready` | `experiments/ndt_istanbul_window_c_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_istanbul_window_c_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/ndt_kitti_raw_0009_full_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_kitti_raw_0009_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0009 (200 frames, urban) | `ready` | `experiments/ndt_kitti_raw_0009_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_kitti_raw_0009_matrix.json` |
| NDT trade-off on KITTI Raw drive 0009 (200 frames, no GT seed) | `ready` | `experiments/ndt_kitti_raw_0009_nogt_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_kitti_raw_0009_nogt_matrix.json` |
| NDT trade-off on KITTI Raw drive 0061 full sequence (703 frames, residential) | `ready` | `experiments/ndt_kitti_raw_0061_full_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_kitti_raw_0061_full_matrix.json` |
| NDT throughput and accuracy trade-off on KITTI Raw drive 0061 (200 frames, residential) | `ready` | `experiments/ndt_kitti_raw_0061_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_kitti_raw_0061_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/ndt_mcd_kth_day_06_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_mcd_kth_day_06_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/ndt_mcd_ntu_day_02_matrix.json` | `ndt` | `balanced_local_map` | `experiments/results/ndt_mcd_ntu_day_02_matrix.json` |
| NDT throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/ndt_mcd_tuhh_night_09_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_mcd_tuhh_night_09_matrix.json` |
| NDT throughput and accuracy trade-off on the repository-stored Istanbul sequence | `ready` | `experiments/ndt_profile_matrix.json` | `ndt` | `fast_coarse_map` | `experiments/results/ndt_profile_matrix.json` |
| Small-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/small_gicp_kitti_raw_0009_full_matrix.json` | `small_gicp` | `fast_recent_map` | `experiments/results/small_gicp_kitti_raw_0009_full_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/small_gicp_mcd_kth_day_06_matrix.json` | `small_gicp` | `fast_recent_map` | `experiments/results/small_gicp_mcd_kth_day_06_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/small_gicp_mcd_ntu_day_02_matrix.json` | `small_gicp` | `dense_recent_map` | `experiments/results/small_gicp_mcd_ntu_day_02_matrix.json` |
| Small-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/small_gicp_mcd_tuhh_night_09_matrix.json` | `small_gicp` | `fast_recent_map` | `experiments/results/small_gicp_mcd_tuhh_night_09_matrix.json` |
| Voxel-GICP trade-off on KITTI Raw drive 0009 full sequence (443 frames, urban) | `ready` | `experiments/voxel_gicp_kitti_raw_0009_full_matrix.json` | `voxel_gicp` | `dense_recent_map` | `experiments/results/voxel_gicp_kitti_raw_0009_full_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD KTH day-06 sequence | `ready` | `experiments/voxel_gicp_mcd_kth_day_06_matrix.json` | `voxel_gicp` | `dense_recent_map` | `experiments/results/voxel_gicp_mcd_kth_day_06_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD NTU day-02 sequence | `ready` | `experiments/voxel_gicp_mcd_ntu_day_02_matrix.json` | `voxel_gicp` | `dense_recent_map` | `experiments/results/voxel_gicp_mcd_ntu_day_02_matrix.json` |
| Voxel-GICP throughput and accuracy trade-off on the MCD TUHH night-09 sequence | `ready` | `experiments/voxel_gicp_mcd_tuhh_night_09_matrix.json` | `voxel_gicp` | `dense_recent_map` | `experiments/results/voxel_gicp_mcd_tuhh_night_09_matrix.json` |
