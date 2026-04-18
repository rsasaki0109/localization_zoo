# Paper Roadmap

_Generated at 2026-04-18T23:04:27+00:00 by `evaluation/scripts/generate_publication_docs.py`._

## Chosen Direction

- Primary path: `Track A: Empirical Study` (Medium).
- Parallel path: `Track B: Artifact / Reproducibility` (High).
- Deferred path: `Track C: Focused Method Paper` (Low).

## Phase 1: Lock the Study Contract

- Keep the current `260` ready problems reproducible through `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing`.
- Freeze the meaning of `reference-based` versus `GT-backed` results and keep them in separate tables.
- Do not merge ad-hoc benchmark scripts outside the stable `pcd_dogfooding --summary-json` contract.

## Phase 2: Expand Public Evidence

- Add another public dataset family, or broaden the current 4-family evidence with longer windows.
- Keep the scoped-out GT-backed CT-LIO item in appendix/artifact docs; revisit only if independent GT appears.
- Re-run every ready problem on the same hardware profile and archive the outputs under `experiments/results/`.

## Phase 3: Paper Assets

- Export a paper-ready table: default variant, active challengers, ATE, FPS, and contract type.
- Export Pareto plots from aggregate JSON instead of manually screenshotting benchmark runs.
- Add an original-paper comparison appendix with one row per implemented method family.

## Phase 4: Writing Package

- Draft the empirical paper around one sentence: variant-first benchmarking exposes useful Pareto fronts that single-canonical repos hide.
- Submit the artifact package in parallel using the same generated docs and experiment manifests.
- Keep the focused-method idea as a follow-on only if one method starts winning on multiple open sequences.

## Immediate Next Tasks

| Priority | Task | Why it matters |
|----------|------|----------------|
| P0 | Add another public dataset family or broader windows from the current 4 families. | The study spans multiple families, but external validity improves with another independent source or deeper windows. |
| P1 | Keep GT-backed CT-LIO scoped out of main evidence tables. | The status is decided; future refreshes should not mix blocked evidence with accepted results. |
| P1 | Curate manuscript-facing subsets and captions from the generated paper assets. | The exports exist now; the remaining work is turning them into final paper figures and tables. |
| P1 | Add dataset bootstrap helpers and a pinned environment. | The refresh entrypoint exists, but a clean-machine replay path is still missing. |
| P2 | Add original-paper comparison sheets for LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP, and CT-LIO. | Needed for a stronger empirical framing. |

## Current Non-Ready Problems

- `CT-LIO GT-backed public benchmark readiness on HDL-400 ROS2 data` (`blocked`): A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence. Next step: No action planned. CT-LIO GT-backed is excluded from the main study scope. If an independent GT source becomes available for HDL-400, this can be revisited.
- `HDL Graph SLAM on KITTI Raw drive 0061 full sequence (703 frames, residential)` (`skipped`): Skipped: computation exceeds 1 hour on KITTI Raw 0061 full (703 frames) Next step: Run a lighter slice/profile, or keep this problem out of the ready set until a real result is available.
