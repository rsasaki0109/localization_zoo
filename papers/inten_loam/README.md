# InTEn-LOAM: Intensity and Temporal Enhanced LOAM (from-paper reimplementation)

**Paper:** Shuaixin Li, Bin Tian, Zhu Xiaozhou, Gui Jianjian, Yao Wen, Guangyun Li,
*"InTEn-LOAM: Intensity and Temporal Enhanced LiDAR Odometry and Mapping"*，
Remote Sensing / arXiv:2209.05708 (2022/23). No public author code found.
Reconstructed from the paper.

## Core idea

InTEn-LOAM exploits **geometric + intensity + temporal** information in LiDAR sweeps:

1. **Cylindrical-image feature extraction (FEF).** Points are projected to range +
   intensity cylindrical images (paper eq. 2). Ground (column-wise line fit),
   facade, edge (range curvature), and reflector (intensity delta + absolute
   threshold) features are adaptively extracted.
2. **Intensity-based scan registration.** Reflector features are aligned by
   minimizing intensity residuals on a local **B-spline intensity surface**
   (simplified here as trilinear interpolation over 8 voxel control points; paper
   eq. 6–7).
3. **Joint scan-to-scan LO.** Geometric edge (point-to-line) and ground/facade
   (point-to-plane) factors are combined with intensity factors in Ceres.
4. **Temporal voxel filter (TVF) + dynamic object removal (DOR)** (2026-06-09):
   voxel occupancy ages features over time; DOR drops voxels with high motion
   inconsistency before mapping updates.
5. **Scan-to-map mapping** (2026-06-09): local voxel map + Ceres scan-to-map
   refinement after scan-to-scan odometry (paper §3.3 mapping thread, simplified).

## Scope / approximations for KITTI

- KITTI Odometry velodyne scans are **unordered** (not native HDL-64 ring order);
  cylindrical projection uses spherical angles from `(x,y,z)` directly.
- TVF/DOR/mapping are simplified approximations of the paper's full pipeline.

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `cylindrical.width/height` | Cylindrical image size | 1024 × 64 |
| `use_intensity_registration` | B-spline intensity factors | true |
| `enable_tvf` | Temporal voxel filter | true |
| `enable_dor` | Dynamic object removal | true |
| `enable_mapping` | Scan-to-map refinement | true |
| `max_edge/surface/reflector_features` | Feature caps | 200 / 400 / 120 |
| `intensity_voxel_size` | Local intensity model voxel [m] | 0.5 |

Ablation flags: `--inten-loam-no-intensity`, `--inten-loam-no-tvf`,
`--inten-loam-no-dor`, `--inten-loam-no-mapping`.

## Paper pipeline

`--inten-loam-paper-odometry` runs `InTenLoamPaper`
(`include/inten_loam/inten_loam_paper.h`), written against the paper after an
audit of the pipeline below found why it does not track:

- it associated the **untransformed** current points once per frame, so every
  correspondence assumed zero motion (trajectory 0.83x the true length
  scan-to-scan);
- scan-to-map repeated that bias against a denser map and **overwrote** the
  scan-to-scan motion (0.60x);
- DOR ran before feature extraction with the previous pose and a fixed 0.6 m
  range test, removing ~40 % of the valid pixels, mostly ahead of and behind
  the car (0.40x overall);
- `EdgeFactor` used a wrong cross product (nonzero on inclined lines; fixed
  for both pipelines).

The paper pipeline:

- **FEF** (Sec. 3.1): 2048 x 64 range image (smallest range, largest
  intensity); segment image of the lowest z per azimuth segment (512) and
  0.5 m radial bin with per-segment ground lines (Himmelsbach, Th_g 0.2 m);
  angle-based clustering (10 deg), clusters under 15 points masked;
  curvature C = sum(D - D_nb) / (N D) along the row, edges |C| > 0.3, facades
  |C| < 0.1 (Sec. 4.1.1) per row sector with non-maximum suppression, points
  behind an occluder skipped; reflectors from the row intensity difference
  (Th_dI 80 / 255) or the 16 x 4 block median.
- **S2S** (Alg. 1): edge residual to the line through the closest point and
  its neighbour on an adjacent scan line, plane residual from the closest
  point and neighbours on the same and an adjacent line, cubic B-spline
  intensity residual (eqs. 6-7, 0.3 m knots); Gauss-Newton on se(3) with
  association redone at every iteration from the previous relative motion.
- **S2M** (Alg. 2): prediction dT_{k-1} T_w'k, local map (100 m) of keyframe
  features in a temporal voxel filter (voxel mean position, intensity
  averaged over the last 50 frames), 5-NN PCA lines and planes.
- **DOR** (Sec. 3.3): after S2M, the local map and the previous scan are
  rendered into a range image; a cluster is dynamic when half of its pixels
  differ by more than 10 % of the range; ground never is. Dynamic points are
  only kept out of the map.
- No motion compensation: KITTI scans are used as given.

Official KITTI RTE, full sequences, no GT seed, 0.2 deg vertical-angle
correction (Sec. 4.2.1), `experiments/inten_loam_kitti_seq_{00,07}_full_paper_comp_matrix.json`:

| variant | seq 00 | seq 07 | FPS |
|---|---:|---:|---:|
| previous pipeline (`--inten-loam-dense-profile`, edge factor fixed) | 47.783 % | 42.158 % | 9 |
| paper pipeline (`paper_odometry`) | 0.769 % | **0.352 %** | 2.4 |
| plane check 0.05 m, 0.17 deg correction (`paper_odometry_plane_check`, below) | **0.697 %** | 0.364 % | 1.6-1.7 |
| without the correction | 1.464 % | 0.905 % | 2.4 |
| without the intensity residual | 0.772 % | 0.351 % | 2.6 |
| S2S only (no S2M, map, DOR) | 3.048 % | 1.895 % | 8 |
| paper, Table 1 | 0.52 % | 0.34 % | |

Two values the paper does not give for KITTI were chosen on seq 07 only, with
seq 00 held out: the plane map voxel (0.8 m: 0.803 %, 0.4 m: 0.444 %, 0.2 m:
0.368 %, 0.1 m: 0.358 %) and the edge threshold (the paper's Th_E = 0.3 is for
its 32-beam data; 0.15 with 4 edges per sector: 0.352 % on 07, and 0.816 ->
0.769 % on 00). That gives 1.04x on 07 and 1.48x on 00 (geometric mean 1.24x).
With the plane check below, 00 goes to 0.697 % (1.34x) and 07 to 0.364 %
(1.07x): 1.20x with one setting for both, 1.18x taking the better variant per
sequence in the historical 00/07 comparison.
Mapping and the correction matter most; the intensity residual is worth
0-1 %.

**Where seq 00 loses accuracy.** Per 100-frame segment the translational
error ranges from 0.2 % to 2.8 %, with no single failure; the worst stretches
are the start and the faster section around frames 4000-4300 (12-13 m/s,
about 90 edge features per scan instead of 150-250). The yaw error has a
consistent sign: summed per frame it reaches -5.95 deg over the sequence
(-0.0013 deg per frame), independent of the turn direction. About -3.3 deg of that also shows
up in KISS-ICP on the same scans (0.544 %), so it comes from the data; the
part specific to InTEn-LOAM is the fast section, where yaw drifts by +0.5 to
+1.0 deg per 100 frames with too few edges (a 400-frame window there: 1.88 %
with Th_E 0.3, 1.38 % with 0.08 and 4 per sector; S2M, DOR and intensity do
not change it). With the 0.15 / 4 edge setting chosen on 07, seq 00 goes
from 0.816 to 0.769 %. Other free parameters move seq 00 within +-5 % (2000
ground points: 0.775 %, facade threshold 0.05: 0.819 %, 0.1 m plane voxels:
0.822 %). The paper's motion compensation (Sec. 3.1.1) is available as
`--inten-loam-paper-set deskew=+-1` (point time from the azimuth, constant
velocity); both spin directions make seq 07 much worse (1.502 / 1.835 %), so
the KITTI odometry scans are treated as already compensated. Free parameters can be changed with `--inten-loam-paper-set key=value`
(`map_plane_voxel`, `max_ground`, `huber`, ...).

### KITTI 00-10 with one configuration

The coverage variant is `paper_odometry_plane_check` on every sequence:
`--no-gt-seed --inten-loam-paper-odometry --input-max-range-m 120
--input-vertical-angle-correction-deg 0.17 --inten-loam-paper-set
s2m_plane_max_dist=0.05`. The plane check and correction values were chosen
on 07 windows. Diagnosing 00 frames 4000-4399 suggested which plane check
to investigate, so 00 is not fully held out. The added sequences 01-06 and
08-10 are evaluation only and do not influence these values.

The stricter plane acceptance and the 0.17-degree correction are KITTI
choices, not the paper's stated settings. They are enabled explicitly by
the arguments above; this coverage run does not change the defaults.
The added manifests are
`experiments/inten_loam_kitti_seq_NN_full_paper_comp_matrix.json`, each with
only the fixed variant. The 00/07 comparison manifests also contain
historical ablations. To run just the fixed setting on any sequence:

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full \
  experiments/reference_data/kitti_seq_NN_full_gt.csv 0 \
  --methods inten_loam --no-gt-seed --inten-loam-paper-odometry \
  --input-max-range-m 120 --input-vertical-angle-correction-deg 0.17 \
  --inten-loam-paper-set s2m_plane_max_dist=0.05 --summary-json inten_seq_NN.json
```

See [the roadmap](../../docs/roadmap.md#experiments) for one-sequence-at-a-time
fetching, result import and regeneration of the derived tables.

<!-- FIXED-KITTI-RESULTS:START -->

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
The 00/07 rows retain their existing committed results; the other rows are
the added coverage runs. Every row below uses the same fixed variant.

| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (development) | 0.52 | 0.697 | 1.34x | 1.736 |
| 01 | 0.64 | 16.614 | 25.96x | 1.655 |
| 02 | 0.54 | 3.221 | 5.97x | 1.822 |
| 03 | 0.63 | 0.891 | 1.41x | 1.552 |
| 04 | 0.37 | 0.676 | 1.83x | 1.708 |
| 05 | 0.36 | 0.497 | 1.38x | 1.215 |
| 06 | 0.24 | 0.359 | 1.49x | 1.789 |
| 07 (development) | 0.34 | 0.364 | 1.07x | 1.555 |
| 08 | 0.71 | 0.938 | 1.32x | 1.264 |
| 09 | 0.48 | 0.661 | 1.38x | 2.111 |
| 10 | 0.45 | 1.156 | 2.57x | 1.835 |

Geometric mean over all sequences with paper values: **2.19x**
with the fixed setting. Table 6 is restricted to this same variant by
`repo_variants` in `evaluation/data/paper_reported_numbers.json`.
The historical 00/07 ablations above remain available in their original
result aggregates.

FPS describes the recorded execution: the new runs use two OpenMP
threads per method and may overlap on the same sequence. The 00/07 FPS
comes from the historical runs.

Sources: `experiments/results/inten_loam_kitti_seq_NN_full_paper_comp_matrix.json`.
New per-run summaries, host records, logs, estimated poses, evaluated GT
and step diagnostics are committed under the corresponding
`experiments/results/runs/` directory. Exact execution commands and the
isolated-output launcher used for overlapping the two methods are recorded
in `experiments/results/mesh_inten_kitti_coverage/`.

#### Sequences above twice the paper value

**01: 16.614 % vs 0.64 % (25.96x).**
The estimated path length is 0.872 of GT.
For GT steps of at least 0.1 m,
136 of 1100 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 136 steps.
The longest runs of step length errors have destination-frame indices 217-253, 420-518 (zero-based; inclusive).
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 400-500 | 263.2 | 0.276 | 81.7 / 312.5 | 88.13 |
| 200-300 | 268.1 | 0.705 | 2.3 / 80.8 | 29.77 |
| 500-600 | 256.6 | 0.834 | 312.5 / 360.3 | 18.84 |

Diagnostic source: `experiments/results/runs/inten_loam_kitti_seq_01_full_paper_comp_matrix/paper_odometry_plane_check/step_diagnostics.json`.

**02: 3.221 % vs 0.54 % (5.97x).**
The estimated path length is 0.965 of GT.
For GT steps of at least 0.1 m,
133 of 4660 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 139 steps.
The longest runs of step length errors have destination-frame indices 1787-1787, 1878-1943, 4460-4524 (zero-based; inclusive).
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 1900-2000 | 115.0 | 0.519 | 58.8 / 119.2 | 50.28 |
| 4400-4500 | 137.0 | 0.605 | 112.8 / 173.8 | 40.83 |
| 4500-4600 | 131.9 | 0.765 | 173.8 / 225.4 | 27.20 |

Diagnostic source: `experiments/results/runs/inten_loam_kitti_seq_02_full_paper_comp_matrix/paper_odometry_plane_check/step_diagnostics.json`.

**10: 1.156 % vs 0.45 % (2.57x).**
The estimated path length is 0.996 of GT.
For GT steps of at least 0.1 m,
0 of 1153 steps differ in length by more than 20 %;
the local translation-vector error exceeds 20 % on 0 steps.
No step length error exceeds that threshold.
All intervals are retained in the diagnostic JSON.

The largest relative-translation-error blocks locate where drift grows.
Each block contains up to 100 consecutive steps; these endpoint errors
are diagnostics, not official KITTI RTE.

Blocks spanning at least 1 m of GT travel are ranked below; the JSON
also retains blocks with less travel.

| Pose interval | GT distance [m] | Est / GT path | Position error start / end [m] | Block error [%] |
|---|---:|---:|---:|---:|
| 600-700 | 50.7 | 0.990 | 6.7 / 9.2 | 2.03 |
| 400-500 | 97.1 | 0.990 | 3.3 / 4.6 | 1.42 |
| 1100-1200 | 17.1 | 0.999 | 12.7 / 12.3 | 1.28 |

Diagnostic source: `experiments/results/runs/inten_loam_kitti_seq_10_full_paper_comp_matrix/paper_odometry_plane_check/step_diagnostics.json`.


Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "InTEn-LOAM"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->

The 01/02 failures contain bursts of underestimated motion: for example,
01 frames 400-500 cover only 0.276 of the GT path length while position
error grows from 81.7 to 312.5 m. Sequence 10 has a different pattern:
its overall path ratio is 0.996 and no moving step exceeds either 20 %
threshold, but smaller errors accumulate (6.7 to 9.2 m over frames
600-700). These measurements locate the drift; they do not establish
which association or optimization mechanism causes it. No new setting
was selected to repair these evaluation sequences.

### Remaining KITTI gap (400-frame windows)

This earlier diagnosis concerns the 00/07 development data. A scratch
driver ran the pipeline on 400-frame windows (official KITTI RTE
over the window's 100-200 m segments, so the numbers are higher than full
runs and only show direction). Choices were made on 07 (frames 0-399,
400-799, 700-1099, plus 07 at every second scan, about 1.3 m per scan, as a
stand-in for the fast part of 00); 00 (0-399, 400-799, 1600-1999, 4000-4399)
was initially treated as held out. Its 4000-4399 diagnosis subsequently
influenced which plane check was investigated, so it is not fully held out
for the final choice. The full sequences were run afterwards (last two
lines of the table).

| variant | 07 windows | 07 every 2nd scan | 00 windows (selection note above) | 00 4000-4399 |
|---|---:|---:|---:|---:|
| current (0.2 deg, plane check 0.2 m) | 0.402 | 0.444 | 0.957 | 1.393 |
| plane check 0.05 m | 0.375 | - | 0.841 | 0.923 |
| correction 0.17 deg | 0.364 | 0.346 | 0.951 | 1.413 |
| plane check 0.05 m, correction 0.17 deg | **0.364** | **0.339** | **0.847** | **1.007** |
| KISS-ICP, 0.205 deg (07: official 1.3.0, full 0.375 %; 00: repository run, full 0.544 %) | 0.500 | - | 0.744 | 0.480 |
| IMLS-SLAM, 0.22 deg (repository run, full 00 0.529 %) | - | - | 0.822 | 0.826 |

| full sequences: current | | | 0.769 (00) | |
| full sequences: plane check 0.05 m, correction 0.17 deg | | | 0.697 (00) | |

On full 07 the setting gives 0.364 % against 0.352 % for the current one, so
the gain on the 07 windows (where the values were chosen) does not carry over
to the full sequence; 00 gains 0.769 -> 0.697 %. The setting is the
manifest variant `paper_odometry_plane_check`: `--input-vertical-angle-correction-deg 0.17
--input-max-range-m 120 --inten-loam-paper-set s2m_plane_max_dist=0.05`.

What the windows show, per axis of the frame-to-frame increment (estimate
minus ground truth):

- **Lateral drift is the data.** Every method moves 0.25-0.9 cm per frame to
  the left of the ground truth (5-7.5 mrad of the step): InTEn-LOAM, KISS-ICP,
  IMLS-SLAM and point-to-plane ICP between consecutive scans started at the
  ground truth all show it, on straight road too, so it is an angle between
  the lidar and the ground-truth frame. It dominates the window RTE of all
  methods.
- **Pitch follows the correction angle.** InTEn-LOAM's pitch error on 07 is
  linear in the angle (0.15 / 0.20 / 0.25 deg: 0.000 / -0.032 /
  -0.066 mrad per frame; KISS-ICP at 0.205 deg: -0.001 to -0.015), the
  estimate climbing (100 m segments 15-20 cm too high). Pitch comes from the
  ground (69 % of the ground features lie within 8 m): against a map placed
  with ground-truth poses, the ground 20-80 m behind the car sits 14-38 mm
  above the map at 0.2 deg (3-18 mm at 0.15 deg) and the ground ahead below
  it, a bowl rising with range that the forward motion turns into pitch.
  The near ground (lower laser block) prefers a larger angle (lower block
  at 0.3 deg, upper at 0.2: -0.021). The angle without pitch drift is about 0.16 deg
  on 07 and on three of the four 00 windows (0-399 starts with the ground
  truth artefacts); 0.17 deg was chosen on 07 (0.15: 0.367 / 0.379 with the
  0.05 m check). On those 00 windows the pitch error is -0.035 to
  -0.065 mrad per frame with the current setting and -0.006 to -0.009 with
  the recommended one, but the window RTE does not change; whether the full
  sequence (segments up to 800 m) gains is untested.
- **The fast section's yaw came from loose plane fits.** On 00 4000-4399
  InTEn-LOAM turned left by +0.053 mrad per frame (KISS-ICP -0.031, IMLS-SLAM
  +0.010). Placing every frame at the ground truth (yaw-corrected by 7 mrad)
  and taking one Gauss-Newton step per feature class there, the facades
  pulled yaw by +0.32 mrad per frame. The S2M plane check (all 5 neighbours
  within 0.2 m of the PCA plane) passes curved and mixed neighbourhoods; at
  0.05 m 40 % of the facade pairs drop out, the facade pull falls to
  +0.19 mrad, S2M from the ground truth goes from +0.198 to -0.048 mrad, and
  the running yaw error from +0.053 to -0.004 mrad per frame. 0.05 m was
  chosen on 07 (0.402 -> 0.375 at 0.2 deg; 0.03 m: 0.376, 0.1 m: 0.367 at
  0.17 deg). This diagnosis was made on the held-out 00 window (it decided
  which parameter to look at, not its value).
- **The estimate is 0.2-0.35 % short.** On 07 the scale is 0.996-0.998 of the
  ground truth while KISS-ICP and consecutive-scan ICP are 1.0007-1.0018 (the
  07 ground truth itself is about 0.15 % shorter than the scans). The edges
  cause it: against a ground-truth map, the scan's edge points lie 15 mm
  ahead of the map lines (23 mm on vertical lines such as poles and trunks,
  growing with the residual) and the S2M edge step is -0.4 to -0.8 cm per
  frame. Removing the forward component of the edge residuals brings the
  scale to 0.998-0.9997 but makes 07 worse (0.364 -> 0.375) and the 00 fast window
  worse (1.007 -> 1.235, yaw drift returns), so it is not used. In the fast
  section the shortfall is now the largest InTEn-specific error (100 m
  segments 37-42 cm short, KISS-ICP 13 cm).
- **00 frames 0-399** carry the ground-truth artefacts at the start; no
  variant changed that window by more than 5 %.

Tried on 07 without gain (scratch code, window mean). Against the current
setting (0.402): ground residuals without their forward component 0.402; at
most one facade per 32 x 4 / 16 x 2 pixel cell 0.442 / 0.401; ground
features within 20 / 40 m only 0.524 / 0.430; edge weight 0.5 0.431; edge
map from the last 10 frames only 0.409. Against the recommended setting
(0.364): one edge per 16 x 2 cell 0.366; edge weight 0.7 0.371; edge map
from the last 2 / 5 frames only 0.377 / 0.382; edge map voxel 0.2 m 0.355
(2nd scan 0.335); plane map voxel 0.1 m 0.350 at 3 fps; Huber 0.05 0.365;
S2M gate 0.5 m 0.377; line check 10:1 0.389; 30 S2M iterations 0.352
(2nd scan 0.344).
`s2m_plane_min_spread` (planes need a two-dimensional spread of the five
neighbours, opt-in) removes a backward pull from neighbours along one line
on the unit-test street (3.2 -> 1.2 cm) but is neutral to slightly worse on 07
(0.365, 2nd scan 0.359).

`InTenLoamPaperDiagnostics` (pass to `process`) places a frame at a given
pose and returns the S2S / S2M estimates from it and the normal equations of
each feature class there; the window analysis above used it with the
ground truth.

## Result of the simplified pipeline (KITTI Odometry full, `--no-gt-seed`)

Post-fix ablation artifacts (2026-07-03, quaternion-layout fix applied):
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam_ablation.json`

Canonical default-pipeline artifacts (TVF+DOR+mapping, dense profile):
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam.json`

The 2026-06-19 evaluator build fixes the Ceres/Eigen quaternion layout used by
the geometric and intensity residuals. Earlier artifacts interpreted Eigen
`{x,y,z,w}` parameter blocks as Ceres `{w,x,y,z}` inside
`QuaternionRotatePoint`, so the identity initial guess was not actually
identity inside the residuals.

### Post-fix ablation (7 variants, `--inten-loam-dense-profile`)

| Sequence | Variant | Drift | ATE [m] | FPS | Notes |
|----------|---------|------:|--------:|----:|-------|
| 07 (1101 fr) | **no_mapping** | **29.597%** | 129 | 39.7 | best seq07 RPE |
| 07 | mapping_only | 50.131% | 143 | 14.1 | TVF/DOR off |
| 07 | no_dor | 52.318% | 109 | 13.3 | |
| 07 | no_intensity | 63.511% | 124 | 10.4 | intensity near-redundant |
| 07 | **full** (default) | 64.599% | 126 | 10.3 | TVF+DOR+mapping |
| 07 | no_tvf | 69.874% | 96 | 10.9 | |
| 00 (4541 fr) | **full** (default) | **63.008%** | 455 | 10.6 | best stable seq00 row |
| 00 | no_intensity | 68.624% | 423 | 11.1 | |
| 00 | no_tvf | 71.336% | 349 | 11.0 | |
| 00 | no_mapping | 218.170% | 13265 | 47.5 | severe divergence |
| 00 | no_dor | 299.979% | 11569 | 15.5 | severe divergence |
| 00 | mapping_only | 4362.094% | 223579 | 21.3 | severe divergence |
| 00 | no_tvf_no_dor | 4362.094% | 223579 | 21.6 | severe divergence |

Per-variant raw JSON:
`docs/benchmarks/kitti_full_new_methods/seq{00,07}_inten_loam_{variant}.json`.

**Honest negative (T3).** Post-fix, the simplified pipeline remains far from the paper's
reported ~0.54% drift on KITTI. seq07 scan-to-scan without mapping is the
least-bad local row (~30% RPE), but seq00 scan-to-scan diverges catastrophically
while the default full pipeline stays merely high-drift (~63% RPE). Mapping +
TVF/DOR without each other is unstable on seq00. Intensity on/off is
near-redundant on both sequences. **Not promoted to the README from-paper
leaderboard** (high-drift policy); raw artifacts and ablation bundles are kept
for reproducibility.

## Cross-dataset (`--no-gt-seed`)

| Dataset | Drift | ATE | Notes |
|---------|------:|----:|---|
| MulRan 120 | **185.4%** | 32 m | TVF/DOR/mapping ON |
| NCLT 600 | **46.5%** | 40 m | ICPSC/MCC ~0.5% drift on same data |

See `docs/benchmarks/cross_dataset/`.

## Reproduce

Full ablation (7 variants × seq00 + seq07):

```sh
python3 evaluation/scripts/run_inten_loam_ablation.py --sequence seq07 --sequence seq00
```

Single run (default = TVF+DOR+mapping ON, dense profile):

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile
```

Scan-to-scan ablation (seq07 least-bad RPE; seq00 diverges on long runs):

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods inten_loam --no-gt-seed --inten-loam-dense-profile \
  --inten-loam-no-mapping
```

## Tests

`test_inten_loam`: cylindrical projection validity; reflector labeling pipeline;
intensity residual scaling; Eigen/Ceres quaternion layout in residual factors;
edge factor on an inclined line; short-sequence pipeline stability; `clear()`
reset.

`test_inten_loam_paper`: ground segmentation and feature classes on a
ray-cast street; the B-spline intensity field on a linear ramp; 1 m per scan
tracked at full scale (also with the 5 cm plane check); ground-truth-placed
frames with their per-class normal equations; the plane spread check
against neighbours along one line.

## Optional execution acceleration

`--paper-pipeline-accelerated` enables repository implementation optimizations
for `--inten-loam-paper-odometry`. It is not a processing step from the paper and
is **off by default**. Independent tree builds and residual queries use OpenMP;
residuals are reduced in the original feature order, including the Huber weights
and diagnostic class systems. KD-trees used only to hold points for rendering
or spline construction are omitted; their point data are retained. Feature
counts, gates, S2S/S2M iterations, TVF and DOR settings are unchanged.

[Execution protocol and CPU profiles](../../experiments/results/paper_pipeline_speed/README.md)
record the development on KITTI 07. Paired full-sequence timing and accuracy
verification are in progress; this option has no published speed claim yet.
