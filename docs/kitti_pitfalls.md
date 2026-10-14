# KITTI pitfalls for LiDAR odometry ports

Checks that closed most of the gaps between this repository's ports and the
papers' KITTI numbers. Each item gives the symptom, a quick check, and the fix,
with the measured effect (official KITTI RTE, full sequences unless a window
is named). Read this before tuning a port on KITTI.

## 1. Input

**Vertical-angle (elevation) correction.** The HDL-64E's points appear about
0.2 deg too low. Upstream KISS-ICP and CT-ICP rotate every point up by
0.205 deg in their loaders, MULLS by 0.195 deg; several papers say so
(InTEn-LOAM Sec. 4.2.1, ELO Sec. IV-A), most do not.
- Symptom: pitch and scale drift; rotation error well above the paper.
- Check: rerun with `--input-vertical-angle-correction-deg 0.205` (0.195-0.22).
- Effect: KISS-ICP 0.910 -> 0.528 % (00), IMLS-SLAM 1.327 / 0.706 -> 0.523 /
  0.339 % (00 / 07), InTEn-LOAM 1.464 / 0.905 -> 0.769 / 0.352 %, Mesh-LOAM
  1.011 -> 0.598 % (00), ELO 0.893 -> 0.706 % (07). Details:
  [kitti_elevation_correction.md](kitti_elevation_correction.md).
- Exception: SuMa's official bilinear model lookup carries a bias of the
  opposite sign; it is best on raw scans unless the lookup is normalised
  (see 4).
- Re-check per method: when pitch comes mostly from the ground, the angle
  without pitch drift can differ. InTEn-LOAM: pitch error linear in the
  angle (0.15 / 0.20 / 0.25 deg: 0.000 / -0.032 / -0.066 mrad per frame on
  07), zero near 0.16 deg; 0.17 deg chosen on 07 (windows 0.402 -> 0.364 %
  with the plane check below).

**Motion compensation.** KITTI odometry scans are already motion-compensated.
- Symptom: a paper describes deskewing (e.g. InTEn-LOAM Sec. 3.1.1), the port
  adds it, and results get worse.
- Effect: azimuth-timed constant-velocity deskew, InTEn-LOAM 07 0.368 ->
  1.502 / 1.835 % (both spin directions); SuMa seq 02 windows 25.5 / 21.3 %.
- Fix: leave it off for KITTI odometry.

**No ring index.** KITTI `.bin` scans are not ordered by laser ring and carry
no ring id; the HDL-64E has two laser blocks with different vertical spacing.
- Symptom: code that assumes 16 or 32 rows, or a ring field, silently drops
  points or merges rings. The old SuMa class used `n_scans = 16`, which put the
  HDL-64 into 10 rows and dropped everything below -17 deg (17.6 % of a scan).
- Fix: derive rows from elevation (64 rows, about -24.9 to +2 deg) and check
  the fill rate of the range image.

**Downsampling and caps.** Heavy voxelisation plus a point cap can leave a
fraction of the scan the method was designed for.
- Example: Mesh-LOAM's dense profile kept about 6.5 % of the raw points
  (0.25 m voxels, 8000-point cap); the paper uses full scans with a 0.1 m map.

## 2. Registration mechanics

**Association with untransformed points.** Correspondences must be searched
with the points moved by the current estimate, and redone every iteration.
- Symptom: the trajectory is too short; scale of the estimated path well below
  1 at about 1 m per scan.
- Effect: the earlier InTEn-LOAM associated once with raw points (scale 0.83
  scan-to-scan, 0.4 with its scan-to-map), 42-48 % RTE; re-association per
  iteration brought it to 0.4-0.8 %.
- Check: path-length ratio estimate / ground truth on a 150-frame window.

**Biased correspondence choice.** Picking, among many candidates, the one with
the smallest residual (instead of the nearest) makes every residual small and
every step too short.
- Effect: Mesh-LOAM's "smallest plane residual among all facets in 27 cells"
  vs the paper's nearest facet: 0.419 -> 0.346 % on 07 (same corrected input).

**Starting in motion.** Most KITTI sequences start with the car already
moving (first step 0.6-1.5 m; 07, 09 and 10 start at 0.1-0.3 m) while a
constant-velocity guess starts at zero. Methods that only accept
correspondences within a small radius (IMLS-SLAM: r = 0.2 m) can skip the
very samples that would correct the guess and stay short for tens of scans:
on 04 (straight road, little across the direction of travel, 1.3 m per scan)
IMLS-SLAM gives 11.9 % against the paper's 0.33 %. One coarse stage on the
first registration (r = 1.0 m, h = 0.3 m) gives 0.43 % (06: 0.404 -> 0.360 %;
00 and 07 unchanged).
- Check: run every sequence, not only 00 and 07; both of these failures only
  showed up on sequences the values were not chosen on.

**Filters that remove registration input.** Dynamic-object removal or temporal
filters belong to map insertion, not to the scan being registered.
- Effect: the earlier InTEn-LOAM removed about 40 % of valid pixels before
  registration, mostly ahead of and behind the car.

## 3. Map and model biases

**Closest-point updates creep towards the sensor.** Keeping the minimum range
per pixel every frame on noisy ranges moves model surfaces toward the sensor.
- Effect: ELO's SRI model surfaces sat 12-13 mm in front of the scan; a static
  sensor drifted 0.12 m in 12 scans in a unit test. Same-surface fusion fixed
  it (07 window 0.622 -> 0.451 %).

**Ground-normal bias.** On KITTI, ground normals from range images tilt toward
the sensor (the elevation "bowl"); with many ground pixels and little else to
constrain the forward direction, they push it.
- Effect: SuMa seq 02 corridors (forward step 1.3 -> 3.3 m, 3.5 % RTE); ELO's
  BEV ground term added about +1.3 mm forward per scan.
- Check: one Gauss-Newton step from the ground-truth increment; split the
  forward gradient by class (ground / non-ground).

**Lookups that blend empty pixels.** A bilinear lookup over a rendered model
that mixes in empty texels pulls samples toward the sensor.
- Effect: SuMa with the lookup normalised over valid texels plus the
  correction: seq 02 3.51 -> 0.78 %, but 07 0.33 -> 0.52 %.

**Vertical fill of a range image.** Filling vertical gaps with the far range
along pixel rays puts the far ground under the road (about 10 cm at 20 m)
once the ground range changes between beams faster than the fill allows.
- Effect: DiLO's ground behind the car pulled pitch; with a slope-aware fill
  (and key points spread over the image, below) seq 00 / 07 went 0.797 /
  0.579 -> 0.549 / 0.372 %, and the best correction angle moved from 0.22 to
  0.18 deg (0.22 had been cancelling the fill bias).

**Key points that cluster.** Lists ranked by a score pick neighbouring pixels
(the ground strip just ahead of the car, vegetation for the sideways lists),
leaving roll weakly constrained; a per-pixel planarity score near 1 everywhere
filters nothing.
- Fix: a stricter planarity threshold plus at most one key point per image
  cell per list (DiLO: roll error per pair 7.2 -> 2.3 mrad, ICP 2.1).

**Loose plane checks.** A plane fit that only asks all neighbours to lie
within some distance of the PCA plane passes curved and mixed neighbourhoods
when the distance is generous; their normals pull yaw.
- Effect: InTEn-LOAM's S2M check 0.2 -> 0.05 m: 00 frames 4000-4399 window
  1.39 -> 0.92 %, yaw error +0.053 -> -0.004 mrad per frame (07 windows
  0.402 -> 0.375 %). Five neighbours along one line pass any distance check
  with a free normal (a spread check removes that on a synthetic street,
  neutral on KITTI).
- Check: one Gauss-Newton step per class from ground-truth poses
  (`InTenLoamPaperDiagnostics`); the facade class pulled yaw +0.32 mrad.

**Edge silhouettes shorten the path.** Edge points on poles and trunks sit
ahead of map lines built from earlier viewpoints (InTEn-LOAM: 15 mm on
average, 23 mm on vertical lines), so scan-to-map pulls back: the estimate
is 0.2-0.35 % shorter than KISS-ICP. Dropping the forward component of edge
residuals fixes the scale but costs yaw; open.

**Map resolution.** Paper values are often for other sensors or not given.
- Effect: InTEn-LOAM plane map voxel 0.8 -> 0.2 m: 07 0.803 -> 0.368 %.

## 4. Degenerate stretches

Highways and corridors leave the forward direction weakly constrained
(translation Hessian eigenvalue along x 40-90 against thousands along y, z on
SuMa's seq 02 failure).
- Symptom: a sudden, sustained over- or under-estimate of the step at
  12-14 m/s; KITTI 01 and parts of 02 and of 00 (frames 4000-4300).
- Fallbacks keyed to sudden changes do not catch a gradual drift.
- Keeping weak directions at the prediction helps: DiLO's plane key points on
  01 collapse towards zero motion after frame 473 (8.70 %); leaving
  translation directions whose eigenvalue is below 0.03 of the largest at the
  prediction (solution remapping) gives 1.52 % and changes nothing on 00/07.
- Effect: SuMa 01 13.8 % (official lookup) / 3.6 % (normalised), paper
  1.7 %; InTEn-LOAM's fast section needed more edge features (the paper's edge
  threshold is for its 32-beam data).

## 5. Evaluation

- **Some error is the data, not the port.** About -3.3 deg of summed yaw error
  on seq 00 appears in KISS-ICP as well as in InTEn-LOAM; ELO and MULLS share
  the lateral error on 07. Every method checked (InTEn-LOAM, KISS-ICP,
  IMLS-SLAM, consecutive-scan ICP from the ground truth) moves 5-7.5 mrad of
  each step to the left of the ground truth on 00 and 07, which dominates
  400-frame window RTE. Compare per-axis increments with a reference
  method on the same scans before chasing an axis.
- **Official RTE needs long segments.** 100-800 m segments; a 400-frame window
  (about 260 m) gives only 100-200 m segments, and 150 frames give none. Use
  windows for direction, full sequences for claims.
- **Choose on one sequence, hold out another.** Parameters the paper gives for
  a different sensor (e.g. InTEn-LOAM's Th_E = 0.3 for 32 beams) or does not
  give at all must be chosen somewhere; the repository chooses on 07 and
  reports 00 as held out. Table 6 takes the best variant per sequence, which is
  an optimistic bound (stated in [paper_claim.md](paper_claim.md)).
- **Paper numbers have limited precision.** SuMa's table has one decimal.

## 6. Moving a KITTI-tuned port to another lidar

Measured on NCLT 2013-01-10 (HDL-32E on a Segway, 5105 scans, about
1.1 km); results in [nclt_paper_paths.md](nclt_paper_paths.md).

**Scan frame.** NCLT stores body-frame scans: z points down (the MS25 reads
accel_z = -9.8 at rest) and the HDL-32E sits 0.957 m above the body origin.
- Symptom: range-image methods and ground models fail outright (SuMa
  880 %, DiLO 64 % RTE) while KISS-ICP, which needs no frame, is fine.
- Check: `evaluation/scripts/check_scan_frame.py --beam-spacing-deg 1.333`
  finds the height and up axis at which the beam pattern appears; on NCLT the
  elevations are then -10.7 to +30.7 deg in a gravity-up frame.
- Fix: `--input-sensor-pose 0 0 -0.957 180 0 0` (scans and ground truth are
  moved into that frame) and the field of view seen from there (up 31,
  down 11).

**Beam spacing.** Gaps, fills and radii sized for 64 beams do not bridge 32.
- DiLO's vertical fill bridged at most 4 image rows; HDL-32E beams are about
  8 rows apart (600-scan window 72.7 -> 2.8 %).
- IMLS-SLAM's paper radii (h = 0.06 m, r = 0.20 m) find no model points
  (47.5 -> 2.7 % with 0.15 / 0.5 m, 1.4 % with 0.3 / 1.0 m).

**Intensity scale.** KITTI stores reflectance in [0, 1], NCLT 0-255;
thresholds and residual weights written for one dominate with the other
(InTEn-LOAM 600-scan window 42 -> 15 %). The runner now scales 0-255 input
for the InTEn-LOAM paper path.

**Near objects and a rocking platform.** On NCLT a person walks about 2.4 m
behind the sensor and buildings pass within 3-5 m; range-image methods
(SuMa, ELO) lose track there while 3D nearest-neighbour methods (KISS-ICP,
IMLS-SLAM) do not. `--input-min-range-m 5` takes ELO from 18.1 to 3.4 %
RTE. Constant-velocity prediction misses the rotation by about 2 deg per
scan on the rocking Segway (0.1-0.2 deg on KITTI), but that is not what
breaks SuMa.

**Short windows.** A setting that fixes a 550-scan window can fail on the
full run (Mesh-LOAM 0.2 m map: 2.05 % on the window, 42.7 % in full); the
window starts from an empty map.

**Unknown keys.** A `--*-paper-set` key the runner does not know only warns;
check the log before reading a sweep as "no effect".

