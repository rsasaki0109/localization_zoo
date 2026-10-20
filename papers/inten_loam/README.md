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

<!-- INTEN_SUPPORT_DIAGNOSTICS -->
#### Pair support at the failure steps

An offline join of the committed native progress logs and saved poses covers
the fixed-setting runs 01-06 and 08-10. In 01, 14
of the 136 bad-length steps have progress records; their
minimum returned S2S / S2M pair counts are
**694 / 1381**.
In 02, 14 of 133 have
records, with minima **927 / 1413**.
None of these sampled bad steps has a count below 30. At those frames the
code keeps the returned S2S transform without low-count replacement and the
S2M pose passes its count-based acceptance test. The low-count branches do
not explain those individual steps; this does not establish solver convergence.

This historical negative check leaves 122 bad steps unlogged in 01 and 119
in 02. Their decisions were unavailable in these sparse logs. The subsequent
complete natural-trace study below fills that coverage and excludes low-count
fallback throughout those executions. High pair counts do not establish
geometric conditioning or correct associations; the causal explanation of the
paper gap remains open.

[Protocol, all sequence counts, CSVs, commands and limits](../../experiments/results/inten_support_diagnostics/README.md)
derive from evidence committed at `283bc86fa939062d80b36057ccca2ce42a891f9c`. No native replay or new
accuracy measurement is performed.
For continuous solver observations, use the opt-in
[passive trace workflow](../../docs/inten_loam_passive_trace.md). Complete
failure-sequence observations are reported below; they do not alter these
historical records or establish a causal fix.
<!-- /INTEN_SUPPORT_DIAGNOSTICS -->

<!-- INTEN_NATURAL_FAILURE_TRACES -->
#### Continuous natural failure diagnosis

Full continuous KITTI01/02/10 trace-off/on pairs complete with all 6 native
exits 0, no skipped inputs, matching accuracy/frame fields and byte-identical
saved poses within each pair. The unchanged `paper_odometry_plane_check`
setting was selected on 07; 00 influenced the earlier diagnosis and is not
held out. Acceleration is off. Raw commits `04417e46` / `e117e876` / `1fe4bc54`
and offline result `2a65d382` precede these numbers.

The inherited bad-length label is a step error above 20% with GT motion at
least 0.1 m. Indices are zero-based destination frames, inclusive. The table
compares the realized S2S output with the final S2M-corrected local motion;
these are not independently integrated trajectories.

| Sequence / bad-length range | Steps | Median S2S length / GT | Median final length / GT |
|---|---:|---:|---:|
| 01: 217–253 | 37 | 0.236199 | 0.258888 |
| 01: 420–518 | 99 | 0.089893 | 0.104688 |
| 02: 1787 | 1 | 1.161876 | 1.216550 |
| 02: 1878–1943 | 66 | 0.047178 | 0.056479 |
| 02: 3308 | 1 | 1.276501 | 1.283396 |
| 02: 4460–4524 | 65 | 0.044211 | 0.037301 |
| 10 | 0 | — | — |

All 136 final bad-length steps in 01 and 132 of 133 in 02 are already bad at
S2S output. S2M crosses the criterion at 02 frame 1787 and brings 3/4 other
S2S-bad steps in 01/02 within it, but does not recover the sustained collapse
runs. Across all 6960 non-initialization steps, S2S fallback occurs 0 times
and S2M acceptance occurs every time. Minimum returned S2S/S2M support at bad
steps is 633/1336 in 01 and 724/1358 in 02, above the existing cutoff of 30.
This excludes an earlier low-count fallback as well as rejection of S2M in
these executions.

All 126966 stored systems have numerical rank 6 and finite applied updates
at the inherited sensor-centred `L=1 m`, `rank_rtol=1e-9` convention. Full rank
does not prove correct associations or sufficient observability. Last-H
condition medians at bad versus other moving steps are 510.42/383.04 (01 S2S),
2602.98/1341.87 (01 S2M), 1017.85/185.38 (02 S2S) and 998.52/420.94 (02 S2M);
the ranges overlap. S2S also marks 50/19 bad steps in 01/02 as converged, while
S2M reaches the iteration limit at 738/2350 other moving steps. Neither rank,
support nor the stop label supplies a demonstrated rejection rule or fix.

KITTI10 has 0 bad-length or bad-vector steps among 1153 moving steps;
47 shorter steps are excluded from relative labels. Its median S2S/final
length ratios are 0.988111/0.995839. Its largest inherited 100-step block error
is 2.027488% over frames 600–700, illustrating accumulated smaller errors;
this diagnostic is not official KITTI distance-segment RTE.

[Protocol, all frames/iterations, commands, timelines and limits](../../experiments/results/inten_natural_traces/README.md)
document the complete observation milestone. Correspondence-level cause,
the relevance of origin dependence and an accuracy-improving change remain
unproven. Estimator defaults, settings and historical paper results are unchanged.
<!-- /INTEN_NATURAL_FAILURE_TRACES -->

<!-- INTEN_SEED_INTERVENTION -->
#### Same-pair initialization interventions

The next [fixed-pair experiment](../../experiments/results/inten_seed_intervention/README.md)
uses the existing forced-pose diagnostic API to change only the S2S initial
transform on the same real scan pair. It replays every S2S/final bad-length
step and its neighbours, plus predetermined block endpoints on 01/02/10.
This is an explicit intervention outside the paper, not a trajectory benchmark
or deployable GT-seeded method. The 07-selected settings are unchanged;
00 influenced the earlier diagnosis. Raw commits `e30917d1` / `35d3f87b` /
`fd085cb5` and report `9b4051fd` precede these numbers.

Across 360 selected pairs, the natural-seed control reproduces the original
continuous S2S output exactly, with matching initial class counts and only
floating-point reduction differences in summed H/g. The alternate seeds are
the previous accepted mapped-motion increment from the same baseline and the
current saved-GT increment. All modes use the same unchanged S2S solver.

Counts below are **bad length / bad vector** outputs among selected moving
pairs, using the inherited 20% criterion and GT motion of at least 0.1 m.

| Sequence (selected moving pairs) | Natural seed | Previous mapped seed | Current GT oracle |
|---|---:|---:|---:|
| 01 (155) | 139 / 139 | 136 / 136 | 1 / 1 |
| 02 (192) | 136 / 137 | 134 / 135 | 33 / 34 |
| 10 (12) | 0 / 0 | 0 / 0 | 0 / 0 |

One additional nonmoving 10 pair is retained without relative-error labels.
Most already-bad baseline pairs do not recover with a one-shot mapped seed;
the prior mapped increment is often already short. This does not reject a
recursive predictor that might prevent entry earlier, and no such candidate
is selected. At 02 frame 1876, the GT-seeded output length ratio is 0.785598;
at frame 4460 it is 0.404926. At 01 frame 420 it is 0.712260. Wrong initial
motion alone therefore cannot explain these fixed-pair failures.

At the GT seed in 02 frames 1876/4460, initial edge support is 0 and the
ground gradient contributes a backward first-order sensor-origin correction
of 3.724821%/4.308664% of GT step length. Contributions use the same total
damped H and sum to the joint step; they are not class-removal ablations or
the final reassociated trajectory. In 01 frame 420, edge/facade contributions
are -1.536647%/-2.484957%, showing a different pattern. This motivates actual
association/normal inspection; a general ground-only cause or fix is unproven.

The linked study retains all oracle failures, every boundary, full initial
class systems, commands/exits and verified scan deletion. Its offline angular
metric handles rounded GT rotations without altering solver seeds. No default,
historical accuracy cell, paper ratio or full-trajectory benefit changes.
<!-- /INTEN_SEED_INTERVENTION -->

<!-- INTEN_PLANE_CORRESPONDENCES -->
#### Actual plane geometry and query support

The [actual-correspondence study](../../experiments/results/inten_plane_correspondences/README.md)
replays all 360 inherited pairs at the current-GT diagnostic seed. Native
outputs and every initial class system/count exactly match the parent study;
all plane equations are reconstructed from retained feature coordinates and
ordered queries. Raw commits `2a583b85` / `ac28d11f` / `d6d157c6`, report
`5373bf0d` and scope result `6218139a` precede these numbers. The original
07-selected setting is unchanged; 00 influenced earlier diagnosis, and known
01/02 failures are development data.

Replacing only ground-plane representation in the **fixed first system** gives
the following joint forward corrections, in percent of GT step. These are
outside-paper offline diagnostics, not iterative solves or trajectory scores.

| Oracle case | Edge pairs | Native equations [%] | Ground PCA through nearest [%] | Ground PCA through centroid [%] |
|---|---:|---:|---:|---:|
| 01 / 420 | 79 | -3.947608 | -3.919565 | -4.022629 |
| 02 / 1876 | 0 | -5.426342 | -3.815493 | -4.002539 |
| 02 / 4460 | 0 | -4.777499 | -3.630112 | -3.904649 |

Both 02 entries retain a backward correction after replacement. Across selected
moving pairs, smaller / larger absolute first-forward corrections are:

| Sequence | Moving pairs | PCA through nearest: smaller / larger | PCA through centroid: smaller / larger |
|---|---:|---:|---:|
| 01 | 155 | 74 / 81 | 69 / 86 |
| 02 | 192 | 121 / 71 | 117 / 75 |
| 10 | 12 | 10 / 2 | 10 / 2 |

The mixed result does not justify adopting ground PCA; 01 retains its distinct
edge/facade pattern. All adverse cases and vector errors remain in the report.
Per-class/per-residual attribution shares the total damped H and is not a
class-removal experiment.

Actual query inspection also identifies a support loss: the ten-neighbour search
rejects same/adjacent-row triangles even when the same target has suitable rows
inside the existing distance gate. At 02 / 1876 and 4460, native ground support
is 247/260, and 379/337 extra triangles are constructible beyond those ten
candidates. Across all selected pairs, extra ground/facade triangles total
15230/6510 (01), 64767/69593 (02), 3045/4570 (10). Many same-row rejections still
have no support anywhere in the gated target, so truncation is only part of the
support loss. Constructibility does not establish correspondence correctness
or motion recovery; no broader-search solver is evaluated here.

The study retains actual commands/exits, full geometric evidence, exact offline
reproduction, all failures and verified official scan deletion. It changes no
estimator/default, prior score or paper ratio. A uniformly configured opt-in
row-aware matcher was subsequently evaluated on every full sequence; see
[the optional missing-row completion results](#optional-s2s-missing-row-completion).
<!-- /INTEN_PLANE_CORRESPONDENCES -->


<!-- INTEN_ROW_SUPPORT -->
## Optional S2S missing-row completion

`--inten-loam-paper-set s2s_row_support=1` enables a repository extension
**outside the paper, off by default**. For ground and facade planes, the
original ten-neighbour query still chooses the closest point and any
available same-row / adjacent-row support. Only a missing support point is
completed from the same retained feature cloud, using nearest eligible
points under the original distance gate and row gaps. Existing choices,
triangle checks, residuals, solver budgets and S2M remain unchanged.
Rows are elevation bins inferred by the extractor, not measured laser rings.
No new points, PCA model or ground-truth feedback are introduced.

The [full fixed-setting experiment](../../experiments/results/inten_row_support/README.md)
uses the inherited 07-selected profile on every KITTI 00–10 sequence.
00 influenced earlier diagnosis, and known 01/02 failures motivated this
boolean option; these are development data. The preceding diagnosis also
inspected 10; this study makes no independent held-out claim. Full 07 ran first, followed
by every remaining sequence without retuning. No numerical threshold was
selected anew. Existing acceleration is enabled identically in both arms;
both use passive traces, continuous natural prediction and `--no-gt-seed`.
Committed numerical evidence: `7a000590`, [all metrics and steps](../../experiments/results/inten_row_support/report/summary.json).

| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |
|---|---:|---:|---:|---:|---:|
| 00 | 0.696720 | 0.691005 | 17.644136 | 17.589408 | 13 / 14 |
| 01 | 16.613803 | 16.877996 | 275.593270 | 280.153484 | 136 / 139 |
| 02 | 3.221230 | 3.405644 | 122.844508 | 129.507562 | 133 / 146 |
| 03 | 0.890601 | 0.891361 | 6.262153 | 6.255589 | 2 / 1 |
| 04 | 0.676097 | 0.685563 | 1.539561 | 1.557560 | 0 / 0 |
| 05 | 0.497289 | 0.515348 | 9.976094 | 9.878900 | 0 / 0 |
| 06 | 0.358642 | 0.355531 | 1.917879 | 1.928044 | 0 / 0 |
| 07 | 0.364131 | 0.355810 | 1.461444 | 1.428673 | 0 / 0 |
| 08 | 0.937938 | 0.939354 | 27.673206 | 28.737019 | 20 / 19 |
| 09 | 0.661434 | 0.667787 | 11.000039 | 11.454099 | 0 / 0 |
| 10 | 1.155652 | 1.183674 | 9.669713 | 9.507559 | 0 / 0 |

The geometric mean of option / reference official RTE is **1.010678x** across all sequences.
This is an experimental paired comparison, not a replacement paper ratio.
Bad-length labels mean a local step-length error above 20% with GT motion
at least 0.1 m; they are distinct from official KITTI 100–800 m RTE.

Every adverse comparison is retained (reference → option):

- 00: rotation [deg/100 m] 0.315468 → 0.316669; bad-length steps 13 → 14.
- 01: official RTE [%] 16.613803 → 16.877996; rotation [deg/100 m] 0.379025 → 0.380166; ATE [m] 275.593270 → 280.153484; bad-length steps 136 → 139; bad-vector steps 136 → 139.
- 02: official RTE [%] 3.221230 → 3.405644; rotation [deg/100 m] 1.003073 → 1.092187; ATE [m] 122.844508 → 129.507562; bad-length steps 133 → 146; bad-vector steps 139 → 151.
- 03: official RTE [%] 0.890601 → 0.891361; bad-vector steps 2 → 3.
- 04: official RTE [%] 0.676097 → 0.685563; ATE [m] 1.539561 → 1.557560.
- 05: official RTE [%] 0.497289 → 0.515348.
- 06: ATE [m] 1.917879 → 1.928044.
- 08: official RTE [%] 0.937938 → 0.939354; rotation [deg/100 m] 0.324577 → 0.326680; ATE [m] 27.673206 → 28.737019; bad-vector steps 99 → 100.
- 09: official RTE [%] 0.661434 → 0.667787; rotation [deg/100 m] 0.232926 → 0.255413; ATE [m] 11.000039 → 11.454099.
- 10: official RTE [%] 1.155652 → 1.183674; rotation [deg/100 m] 0.386538 → 0.388731.

Full failure-sequence observations:

- 01: final bad-length frames 217–253, 420–518 → 209, 216–253, 420–519; S2S bad-length outputs 139 → 140. These are each run’s realized trajectory, not matched-seed interventions.
- 02: final bad-length frames 1787, 1878–1943, 3308, 4460–4524 → 1787, 1789, 1879–1948, 3308, 4461–4533; S2S bad-length outputs 136 → 152. These are each run’s realized trajectory, not matched-seed interventions.
- 10: final bad-length frames none → none; S2S bad-length outputs 0 → 0. These are each run’s realized trajectory, not matched-seed interventions.

More support does not establish correct surfaces or resolve the highway
failure. The option remains experimental and is not promoted per sequence.
All earlier aggregates, published paper values and defaults are preserved.
Raw timings are retained, but concurrent runners are not a standalone FPS
benchmark. Full 07 legacy-off compatibility and all inherited default-off
pair controls, native tests and command/execution records are linked in
the study. Sensor-transfer behavior and the remaining solver failure are open.
<!-- /INTEN_ROW_SUPPORT -->

<!-- INTEN_MAPPED_PRIOR -->
## Optional recursive mapped-motion S2S seed

`--inten-loam-paper-set s2s_mapped_prior=1` is a repository extension
**outside the paper, off by default**. It replaces only the normal S2S
initial value with the previous final mapped relative pose, updated recursively.
Its rotation uses a normalized quaternion; translation is preserved.
The initial unprojected trial completed 07 worse and aborted 00; see the
[preserved failure](../../experiments/results/inten_mapped_prior/README.md).
00 informed this numerical correction; this fixed revision restarts every sequence.
The original accepted S2S motion still controls low-count fallback and deskew.
Startup is identity; mapping-off and forced-pose diagnostics are unchanged.
Skipped/rejected S2M uses the actual final relative pose for the next seed.
No association, gate, residual, damping or budget changes; row completion stays off.

The inherited profile was chosen on 07 with 00 influencing diagnosis.
Known 01/02/10 failures motivated this parameter-free boolean, so no independent
held-out claim is made. Every KITTI 00–10 sequence uses the same settings,
acceleration and passive trace, with continuous prediction and no GT seed.
The earlier one-shot mapped-seed control did not test recursive prevention.

[Full study and commands](../../experiments/results/inten_mapped_prior_rigid/README.md);
[complete numerical report](../../experiments/results/inten_mapped_prior_rigid/report/summary.json).

The initial unprojected trial is a retained negative result: full 07 official
RTE 0.364131 → 0.379537%,
ATE 1.461444 → 1.668590 m;00 aborted with
exit 134 after 2355 frame records
(no full 00 RTE). Its non-rigid rotation feedback informed the fixed rotation
restoration used here. All 11 sequences restart with the revised setting.
[Original source, commands and diagnosis](../../experiments/results/inten_mapped_prior/README.md).

Geometric mean of option / reference official RTE: **0.904792x** across all sequences.
Lower RTE: 00, 01, 02, 05, 06. Higher RTE: 03, 04, 07, 08, 09, 10.
This is a fixed experimental comparison; no per-sequence default is adopted.
Bad-length labels use the inherited local 20% threshold for GT motion at least 0.1 m,
separate from official KITTI 100–800 m RTE. Raw paired timing is not an FPS benchmark.

Numerical sources committed before prose: `d62570b78d2707cc9c7d0629592c193986a57a29`.

| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |
|---|---:|---:|---:|---:|---:|
| 00 | 0.696720 | 0.691786 | 17.644136 | 17.064446 | 13 / 12 |
| 01 | 16.613803 | 5.180428 | 275.593270 | 100.875751 | 136 / 50 |
| 02 | 3.221230 | 3.073932 | 122.844508 | 119.531515 | 133 / 128 |
| 03 | 0.890601 | 0.917374 | 6.262153 | 6.449763 | 2 / 2 |
| 04 | 0.676097 | 0.678960 | 1.539561 | 1.543638 | 0 / 0 |
| 05 | 0.497289 | 0.491912 | 9.976094 | 9.886842 | 0 / 0 |
| 06 | 0.358642 | 0.354019 | 1.917879 | 1.924468 | 0 / 0 |
| 07 | 0.364131 | 0.378295 | 1.461444 | 1.660935 | 0 / 0 |
| 08 | 0.937938 | 0.973072 | 27.673206 | 28.286838 | 20 / 19 |
| 09 | 0.661434 | 0.668254 | 11.000039 | 11.133601 | 0 / 0 |
| 10 | 1.155652 | 1.183142 | 9.669713 | 9.901906 | 0 / 0 |

Every adverse comparison (reference → option):

- 03: official RTE [%] 0.890601 → 0.917374; rotation [deg/100 m] 0.627926 → 0.660957; ATE [m] 6.262153 → 6.449763.
- 04: official RTE [%] 0.676097 → 0.678960; ATE [m] 1.539561 → 1.543638.
- 06: ATE [m] 1.917879 → 1.924468.
- 07: official RTE [%] 0.364131 → 0.378295; rotation [deg/100 m] 0.274943 → 0.298015; ATE [m] 1.461444 → 1.660935.
- 08: official RTE [%] 0.937938 → 0.973072; rotation [deg/100 m] 0.324577 → 0.339552; ATE [m] 27.673206 → 28.286838.
- 09: official RTE [%] 0.661434 → 0.668254; rotation [deg/100 m] 0.232926 → 0.249751; ATE [m] 11.000039 → 11.133601.
- 10: official RTE [%] 1.155652 → 1.183142; rotation [deg/100 m] 0.386538 → 0.393581; ATE [m] 9.669713 → 9.901906.

New local bad labels in the candidate relative to the reference (all frames):

| Sequence | Newly bad length steps | Reference-only bad length steps | Newly bad vector steps | Reference-only bad vector steps |
|---|---:|---:|---:|---:|
| 00 | 0 | 1 | 0 | 1 |
| 01 | 13 | 99 | 13 | 99 |
| 02 | 1 | 6 | 1 | 7 |
| 03 | 0 | 0 | 0 | 0 |
| 04 | 0 | 0 | 0 | 0 |
| 05 | 0 | 0 | 0 | 0 |
| 06 | 0 | 0 | 0 | 0 |
| 07 | 0 | 0 | 0 | 0 |
| 08 | 0 | 1 | 2 | 4 |
| 09 | 0 | 0 | 0 | 0 |
| 10 | 0 | 0 | 0 | 0 |

Each pair uses the same input frame and GT labels but different realized
trajectories. These are outcome differences, not matched-seed interventions.
Every affected frame index is retained in the numerical report.

Failure-sequence observations along each realized trajectory:

- 01: final bad-length frames 217–253, 420–518 → 204–253; S2S bad-length outputs 139 → 51; final rejected S2M 0 → 0.
- 02: final bad-length frames 1787, 1878–1943, 3308, 4460–4524 → 1787, 1883–1942, 3308, 4460–4525; S2S bad-length outputs 136 → 131; final rejected S2M 0 → 0.
- 10: final bad-length frames none → none; S2S bad-length outputs 0 → 0; final rejected S2M 0 → 0.

Candidate sequences exceeding twice the cited paper RTE:

| Sequence | Option RTE [%] | Paper RTE [%] | Option / paper | Largest position-error growth block | Position error start → end [m] | Local block error [%] |
|---|---:|---:|---:|---|---:|---:|
| 01 | 5.180428 | 0.640000 | 8.094419 | 200–300 | 2.277764 → 100.942226 | 37.317870 |
| 01 | 5.180428 | 0.640000 | 8.094419 | 600–700 | 111.147016 → 116.310783 | 0.813662 |
| 01 | 5.180428 | 0.640000 | 8.094419 | 700–800 | 116.310783 → 120.549185 | 0.371765 |
| 02 | 3.073932 | 0.540000 | 5.692467 | 1900–2000 | 57.507917 → 117.070449 | 49.417329 |
| 02 | 3.073932 | 0.540000 | 5.692467 | 4400–4500 | 105.465085 → 164.579579 | 38.262206 |
| 02 | 3.073932 | 0.540000 | 5.692467 | 2300–2400 | 55.084337 → 113.862431 | 0.460583 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 700–800 | 9.445875 → 14.984990 | 0.947132 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 600–700 | 6.908792 → 9.445875 | 1.995964 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 800–900 | 14.984990 → 17.466513 | 0.485612 |

These fixed-size motion blocks locate error growth; their relative errors are
not official distance-segment RTE and do not establish the solver-level cause.
Paper values are cited context; this extension is not the paper algorithm.

These are natural full-run observations, not matched-seed causal interventions.
The complete report retains every step and all motion-error blocks. The option
remains outside the paper and default off; earlier results are preserved.
<!-- /INTEN_MAPPED_PRIOR -->

<!-- INTEN_TRANSLATION_PRIOR -->
## Optional weak-translation prediction prior

`--inten-loam-paper-set s2s_translation_prior_ratio=0.03` is a repository
extension **outside the paper, off by default** (default ratio zero).
At the first S2S iteration, the data Hessian is expressed at the sensor
origin and rotation is marginalized. The prior adds only the information
needed to raise weak translation eigenvalues to the declared fraction
of the strongest. Its quadratic position residual is anchored to the
initial predicted translation; its information stays fixed during the solve.
It may also resist real acceleration or preserve an incorrect prediction.
Data associations, feature weights, iteration budgets, S2M and the original
damped solve remain unchanged. Trace systems/cost include the added prior;
feature counts and forced diagnostic class equations remain data-only.

Both arms enable the preceding rigid mapped-motion seed and acceleration;
row completion stays off. The new ratio alone differs: zero versus 0.03.
The range/elevation/plane/edge profile was selected on 07, with 00 informing
earlier diagnosis. The fraction was borrowed from the existing
[DiLO KITTI01-tuned control](../dilo/README.md) and fixed before any new
InTEn accuracy run. Here it is a soft information floor after rotation
marginalization, whereas DiLO hard-remaps the raw translation block.
It was not retuned on a new InTEn sequence score. Known 01/02/10 failures
and prior whole-sequence outcomes are
development evidence, so there is no independent held-out claim.
Every sequence uses the same settings, continuous prediction and no GT seed.

[Full study and commands](../../experiments/results/inten_translation_prior/README.md);
[complete numerical report](../../experiments/results/inten_translation_prior/report/summary.json).

Geometric mean of option / reference official RTE: **0.946172x** across all sequences.
Lower RTE: 01, 02. Higher RTE: none. Unchanged RTE: 00, 03, 04, 05, 06, 07, 08, 09, 10.
This is a fixed experimental comparison; no per-sequence default is adopted.
Bad-length labels use the inherited local 20% threshold for GT motion at least 0.1 m,
separate from official KITTI 100–800 m RTE. Raw paired timing is not an FPS benchmark.

Numerical sources committed before prose: `16577120f8e0b525446692d51ac26ea3367fb501`.

| Sequence | Reference RTE [%] | Option RTE [%] | Reference ATE [m] | Option ATE [m] | Bad-length steps, reference / option |
|---|---:|---:|---:|---:|---:|
| 00 | 0.691786 | 0.691786 | 17.064446 | 17.064446 | 12 / 12 |
| 01 | 5.180428 | 5.173517 | 100.875751 | 100.493982 | 50 / 50 |
| 02 | 3.073932 | 1.674736 | 119.531515 | 48.906265 | 128 / 66 |
| 03 | 0.917374 | 0.917374 | 6.449763 | 6.449763 | 2 / 2 |
| 04 | 0.678960 | 0.678960 | 1.543638 | 1.543638 | 0 / 0 |
| 05 | 0.491912 | 0.491912 | 9.886842 | 9.886842 | 0 / 0 |
| 06 | 0.354019 | 0.354019 | 1.924468 | 1.924468 | 0 / 0 |
| 07 | 0.378295 | 0.378295 | 1.660935 | 1.660935 | 0 / 0 |
| 08 | 0.973072 | 0.973072 | 28.286838 | 28.286838 | 19 / 19 |
| 09 | 0.668254 | 0.668254 | 11.133601 | 11.133601 | 0 / 0 |
| 10 | 1.183142 | 1.183142 | 9.901906 | 9.901906 | 0 / 0 |

Every adverse comparison (reference → option):

- 01: rotation [deg/100 m] 0.281769 → 0.288675.

New local bad labels in the candidate relative to the reference (all frames):

| Sequence | Newly bad length steps | Reference-only bad length steps | Newly bad vector steps | Reference-only bad vector steps |
|---|---:|---:|---:|---:|
| 00 | 0 | 0 | 0 | 0 |
| 01 | 0 | 0 | 0 | 0 |
| 02 | 1 | 63 | 2 | 63 |
| 03 | 0 | 0 | 0 | 0 |
| 04 | 0 | 0 | 0 | 0 |
| 05 | 0 | 0 | 0 | 0 |
| 06 | 0 | 0 | 0 | 0 |
| 07 | 0 | 0 | 0 | 0 |
| 08 | 0 | 0 | 0 | 0 |
| 09 | 0 | 0 | 0 | 0 |
| 10 | 0 | 0 | 0 | 0 |

Each pair uses the same input frame and GT labels but different realized
trajectories. These are outcome differences, not matched-seed interventions.
Every affected frame index is retained in the numerical report.

Failure-sequence observations along each realized trajectory:

- 01: final bad-length frames 204–253 → 204–253; S2S bad-length outputs 51 → 51; final rejected S2M 0 → 0.
- 02: final bad-length frames 1787, 1883–1942, 3308, 4460–4525 → 1787, 3308, 4463–4526; S2S bad-length outputs 131 → 68; final rejected S2M 0 → 0.
- 10: final bad-length frames none → none; S2S bad-length outputs 0 → 0; final rejected S2M 0 → 0.

Candidate sequences exceeding twice the cited paper RTE:

| Sequence | Option RTE [%] | Paper RTE [%] | Option / paper | Largest position-error growth block | Position error start → end [m] | Local block error [%] |
|---|---:|---:|---:|---|---:|---:|
| 01 | 5.173517 | 0.640000 | 8.083620 | 200–300 | 2.277764 → 100.942226 | 37.317870 |
| 01 | 5.173517 | 0.640000 | 8.083620 | 600–700 | 111.197578 → 116.048415 | 0.682695 |
| 01 | 5.173517 | 0.640000 | 8.083620 | 500–600 | 106.987368 → 111.197578 | 1.194564 |
| 02 | 1.674736 | 0.540000 | 3.101363 | 4500–4600 | 97.187460 → 151.135803 | 29.264134 |
| 02 | 1.674736 | 0.540000 | 3.101363 | 4400–4500 | 44.569258 → 97.187460 | 35.612773 |
| 02 | 1.674736 | 0.540000 | 3.101363 | 4600–4660 | 151.135803 → 183.965499 | 0.984333 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 700–800 | 9.445875 → 14.984990 | 0.947132 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 600–700 | 6.908792 → 9.445875 | 1.995964 |
| 10 | 1.183142 | 0.450000 | 2.629204 | 800–900 | 14.984990 → 17.466513 | 0.485612 |

These fixed-size motion blocks locate error growth; their relative errors are
not official distance-segment RTE and do not establish the solver-level cause.
Paper values are cited context; this extension is not the paper algorithm.

These are natural full-run observations, not matched-seed causal interventions.
The complete report retains every step and all motion-error blocks. The option
remains outside the paper and default off; earlier results are preserved.
<!-- /INTEN_TRANSLATION_PRIOR -->

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

<!-- INTEN_PASSIVE_TRACE -->
## Optional passive solver trace

The C++ `InTenLoamPaper::process` API accepts an optional fourth argument,
`InTenLoamPaperTrace*`. It observes the normal S2S/S2M path, accepted poses,
fallback decisions, iteration normal equations, weighted residual sums and
stop reasons. This repository analysis tool is outside the paper and off by
default. Combining it with `forced_pose` throws before estimator state changes;
the older forced-pose diagnostic remains an intervention.

All 125 CTest entries pass, including moving trace-on/off
pose and retained-map parity. The default and unchanged 07-selected plane-check
settings also match the pinned old executable on the committed short MCD
fixture: all 4 native calls exit 0, available accuracy/frame
differences are zero and saved poses are byte-identical. Short-fixture KITTI
segment RTE remains unavailable. Evidence was committed at `ad6010cf1c1cecb5c7012f37d92558126132a83e` before
this prose. These checks preserve behavior; the full KITTI solver-level cause
remains open.

The initial whole-suite run exits 8 because an
unrelated existing test tries to overwrite a root-owned shared temporary file.
Its failure is retained; the same tests and binaries pass after selecting a
fresh user-owned `TEST_TMPDIR`. The study documents both attempts.

[API semantics](../../docs/inten_loam_passive_trace.md) and
[protocol, complete evidence, commands and limits](../../experiments/results/inten_passive_trace/README.md)
describe the scope. Published results and estimator defaults are unchanged.
<!-- /INTEN_PASSIVE_TRACE -->

<!-- INTEN_TRACE_EXPORT -->
## Optional runner trace export

The paper runner accepts `--inten-loam-paper-trace-jsonl NEW.jsonl` with
`--methods inten_loam --inten-loam-paper-odometry`. It streams the passive
observations and explicit skipped-input records, with a final traversal
record. This repository diagnostic is outside the paper and default off.
It changes no estimator setting, decision or map input.

[Usage, coordinates and incomplete/nonfinite record handling](../../docs/inten_loam_passive_trace.md#streaming-export-from-the-evaluation-runner)
and the [separate validation record](../../experiments/results/inten_trace_export/README.md)
describe its scope. On the committed short MCD fixture, all 6 native comparisons exit 0:
reference, new trace-off and trace-on have zero available metric differences
and byte-identical poses with both the default and unchanged 07-selected
plane-check setting. Segment RTE is unavailable. A separate empty-middle-input
lifecycle check also has identical poses across all 3 calls; its GT timing
makes accuracy fields unsuitable for interpretation. The initial lifecycle
attempt exits 1 because its builder omitted timestamps; the corrected builder
copies the committed timestamps without relaxing association. Evidence was
committed at `4f235364` / `086d70d0` before these results.

The later [natural-trace study](../../experiments/results/inten_natural_traces/README.md)
completes failure-sequence tracing; the causal solver explanation remains open.
This export validation is not an accuracy result.
The separate fresh Release full build and all 126 CTest entries also
exit 0. [Native evidence](../../experiments/results/inten_trace_export/evidence/full_native/summary.json)
was committed at `c679dde4837dc097b697f18654c6cc51da0ffa0f` before this statement. The postprocessor's
initial exit 120, closed-stdout reproduction and regular-file recovery exit 0
are retained separately; the successful native tests were not rerun.
<!-- /INTEN_TRACE_EXPORT -->

<!-- INTEN_TRACE_VALIDATION -->
## Offline trace validation

The manually invoked `evaluation/scripts/check_inten_trace.py` streams a
saved passive trace with its native summary and command/exit receipt.
It checks completion, input/call indices, stage/iteration structure and run
consistency before publishing counters. This analysis tool is outside the
paper and leaves native code/settings unchanged.

All 9 unit/CLI tests pass with exit 0. All 3 committed normal/lifecycle traces
validate with exit 0; all 7 explicit synthetic corruptions return exit 1
and create no report. Evidence was committed at `e27ff091` before these
numbers. [Results, negative cases and exact commands](../../experiments/results/inten_trace_validation/README.md)
retain the scope: accuracy, GT indexing, conditioning and solver cause
require separate evidence. The later natural-trace study localizes failures by
stage; the causal explanation remains open.
<!-- /INTEN_TRACE_VALIDATION -->
## Optional execution acceleration

`--paper-pipeline-accelerated` enables repository implementation optimizations
for `--inten-loam-paper-odometry`. It is not a processing step from the paper and
is **off by default**. Independent ground fits, image-row stages, tree builds
and residual queries use OpenMP; constant beam-angle trigonometry is reused.
Residuals are reduced in the original feature order, including the Huber weights
and diagnostic class systems. Small-k queries use the same float FLANN single-
tree index and exact search/result-set parameters as PCL, bypassing its per-query
allocation and OpenMP wrapper. Nonfinite index data use the PCL path. KD-trees
used only to hold points for rendering
or spline construction are omitted; their point data are retained. Feature
counts, gates, S2S/S2M iterations, TVF and DOR settings are unchanged.

The initial candidate (`fe607199`) took 18.661 s against 45.685 s for the
frozen baseline on the first 120 KITTI 07 frames (**2.45x**). The changed
binary's default path took 34.105 s. Both saved trajectories match the control
exactly. This development-window attempt falls short of the speed target; it
is not a full-sequence claim. Raw evidence and exact commands were committed in
`8fe732a7` under
[`candidates/fe607199/`](../../experiments/results/paper_pipeline_speed/candidates/fe607199/).

A later soft-pause scheduling candidate (`fe03ed88`) took 26.305 s against
35.078 s for its frozen control (**1.33x**); the changed default took 32.961 s.
Saved poses still matched. The repeat at `67867d0b` used the same InTEn implementation and produced a
larger ratio with a much slower control, as the table below shows. Scheduling
benefit remains uncertain; the final candidate excludes soft-pause calls. These single-window timings vary with the
host; final speed ratios use a fresh full control immediately before each
accelerated run. The rejected candidate's raw screening was committed in
`21935329` under
[`candidates/fe03ed88/`](../../experiments/results/paper_pipeline_speed/candidates/fe03ed88/).

Repeated development windows (first 120 KITTI 07 frames, four workers,
profiler disabled; every source and exact command is in the linked evidence):

| Source | Frozen control [s] | Changed default [s] | Opt-in [s] | Control / opt-in |
|---|---:|---:|---:|---:|
| `fe607199` | 45.685 | 34.105 | 18.661 | 2.45x |
| `fe03ed88` | 35.078 | 32.961 | 26.305 | 1.33x |
| `67867d0b` | 74.308 | 32.637 | 15.464 | 4.81x |

All saved poses agree with the frozen control at the saved precision (nine
significant digits). Execution time varies substantially, including between
unchanged method implementations. These windows do not establish the full
speed target. The primary full comparison uses fresh immediately preceding
controls. Source `67867d0b` raw screening is committed in `f3c6b9f3`.

[Execution protocol and CPU profiles](../../experiments/results/paper_pipeline_speed/README.md)
and [full comparison](../../experiments/results/paper_pipeline_speed/comparisons/f4755573.json).

Full KITTI 07 at measured candidate `f4755573` and control `134ead2f`,
identical selected registration arguments, four OpenMP workers, one method at a
time on the same recorded host; the profiler is disabled. These times belong
to those revisions and are not remeasured by the later shared-component integration:

| Execution | Time [s] | FPS | Official RTE [%] | ATE [m] |
|---|---:|---:|---:|---:|
| Paired frozen control | 636.65 | 1.729 | 0.364131 | 1.461444 |
| Opt-in acceleration | 373.48 | 2.948 | 0.364131 | 1.461444 |

Measured speedup: **1.70x**. Maximum translation difference from the
paired frozen control: 0.000000000 m.
Pose differences use the saved KITTI matrices (nine significant digits).
Official RTE agrees to the third decimal.

**Negative result:** this attempt does not meet the roadmap speed target.
The option remains off by default.
The roadmap item is still open.

KITTI 07 guided implementation and is not held out. This comparison uses a
frozen-source full control run immediately before acceleration on the same host
and thread count. Historical
Table 6 results retain their original evidence and are not replaced.

## Acceleration integration with shared components

The current-main compatibility evidence, committed in `7619f198`, compares
frozen main, merged default and merged global-flag modes on the same 3 real
MCD scans with the historical fixed arguments. Both merged modes have zero
recorded accuracy/frame differences from main and byte-identical saved poses.
The acceleration kernels are unchanged; the option stays off by default.
These short executions check compatibility; they do not establish long-segment
KITTI RTE or remeasure full-sequence throughput. The speed figures above remain
tied to their recorded sources. [Commands, binary/input hashes, comparison and
limits](../../experiments/results/paper_pipeline_speed/README.md#integration-with-merged-shared-components).

<!-- INTEN_TRACE_EXPORT_ACCELERATION -->
## Passive export with optional acceleration

The integrated runner records `params.accelerated` in its trace header. All 12
real-fixture calls exit 0, with zero available metric differences and identical
saved poses within each default/accelerated registration profile (`2e6d7689`).
All 7 expected negative CLI exits are retained. The full incremental Release
build, all 126 CTest entries and four-worker InTEn/serializer checks also exit 0
(`79cd80a3`). Settings are reused unchanged from KITTI07, with KITTI00 influencing
the earlier diagnosis. These short checks provide no new speed or long-segment
accuracy result; the full failure cause remains open.
[Exact commands, hashes, traces and limits](../../experiments/results/inten_trace_export/integration/README.md).
<!-- /INTEN_TRACE_EXPORT_ACCELERATION -->

<!-- INTEN_PASSIVE_TRACE_ACCELERATION -->
## Passive trace and acceleration integration

The source conflict with optional acceleration is resolved while retaining the
original residual-addition order. All 125 CTest entries pass, and the InTEn entry
also passes with four workers; every passive lifecycle case runs in both
execution modes (`7aa83cff`). All 8 short real-fixture native calls exit 0,
with zero available metric differences and byte-identical saved poses against
each mode's prior executable (`fb9318f3`). These are compatibility checks;
long-segment RTE is unavailable, no speed is remeasured, and the full failure
cause remains open. Defaults and historical numbers are unchanged.
[Commands, settings provenance, evidence and limits](../../experiments/results/inten_passive_trace/integration/README.md).
<!-- /INTEN_PASSIVE_TRACE_ACCELERATION -->

<!-- INTEN_TRACE_VALIDATION_ACCELERATION -->
## Offline validation of accelerated exports

The unchanged reader accepts all 4 committed default/accelerated export tuples
with exit 0, preserving their input bytes (`b1ef79df`). The original legacy
corpus and synthetic corruptions remain separate. No estimator or raw scan is
opened by these checks. [Commands, hashes and limits](../../experiments/results/inten_trace_validation/integration/README.md)
cover consistency only; full-sequence accuracy and solver cause remain open.
<!-- /INTEN_TRACE_VALIDATION_ACCELERATION -->

<!-- INTEN-CONDITIONING:START -->
## Passive normal-equation conditioning

An [offline analyzer](../../experiments/results/inten_conditioning/README.md)
now expresses recorded H/g about the current sensor centre and its axes, with
a required explicit rotation length. This analysis tooling is outside the paper
and adds no estimator gate or default behaviour.

Raw evidence `6f96bf4702f2cb6a923a99ac85031bf559ebedce` precedes this statement.
All **7** existing positive trace tuples analyze successfully: every one of
**231** systems has numerical rank **6** at the declared **1 m** rotation length
and **1e-9** relative cutoff. The short fixtures show no rank deficit under that
convention. Full rank/high support cannot establish correct associations, motion
or the cause of the saved KITTI failures.

Singular, zero, nonfinite, asymmetric and indefinite systems retain explicit
results or unavailable markers; failed deltas are not zero-filled.
[Coordinates, spectra, commands and limits](../../docs/inten_loam_passive_trace.md#analyze-normal-equation-conditioning)
describe the first-order convention. The later natural-trace study completes
continuous failure-sequence collection. In this short-fixture analysis, no scans
were read, estimator rerun or registration setting
selected; stored accuracy and opt-in defaults are preserved.
<!-- INTEN-CONDITIONING:END -->

<!-- INTEN-ORIGIN-INVARIANCE:START -->
## Origin dependence of a fixed update

An [equation-level experiment](../../experiments/results/inten_origin_invariance/README.md)
replays every stored short-fixture normal equation with fixed target-origin
translations. The native `expSe3` uses rotation plus direct translation rather
than the exact SE(3) exponential; transporting the same step therefore changes
the finite update. With a **1,000 m** shift, the maximum updated-position
difference is **0.0696058 m** in S2M and **0.966418 m** in S2S. The latter shifts
the local target scan for a mathematical control; it is not an actual S2S world
origin. These are single-iteration differences, not trajectory errors.

The separate native damping comparison changes sensor-centred infinitesimal
translation by at most **0.000526911 m** and rotation by **0.000376920 rad**
over the stored fixtures. The transported-damping numerical control stays
within **7.56956e-13 m / 4.33051e-13 rad**. The independent exact-exponential
same-step control stays within **2.15111e-13 m**. Raw evidence `80cfaf23` and
aggregation `c316b4df` precede these statements; all commands and actual exits
are retained in the linked experiment.

This negative result establishes an origin-dependent implementation property.
It does not explain KITTI 01/02/10 or demonstrate that an alternative update
improves accuracy. The later natural-trace study completes observation but does
not establish origin dependence as the KITTI cause or select an opt-in candidate.
In this fixed-equation experiment no scans or estimator are rerun, no setting is
selected, and existing defaults and published results are preserved.
<!-- INTEN-ORIGIN-INVARIANCE:END -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored InTEn-LOAM full-sequence cells complete with native exits 0. 11 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
