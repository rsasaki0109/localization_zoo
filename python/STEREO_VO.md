# Experimental stereo visual odometry

A small image-driven baseline, independent of the known-landmark prototypes.
It uses ORB stereo/temporal matches, rectified disparity and RANSAC PnP.
No ground-truth pose or prebuilt landmark map is accepted. This is not a
VINS-Fusion, OKVIS or ORB-SLAM3 reproduction and has no real-data accuracy claim.

Install `numpy`, `opencv-python-headless==4.11.0.86` and `PyYAML==6.0.3` in a virtual environment.
Run `python3 -m unittest discover -s python/test -p 'test_*.py' -v`.
Tests cover metric depth, pose direction, image-generated motion and explicit loss.

For replay, supply a CSV with `timestamp,left,right` columns (timestamps in
seconds; image paths relative to the CSV) and calibration JSON:

```json
{"K": [[400,0,320],[0,400,240],[0,0,1]], "baseline_m": 0.2}
```

These numbers are illustrative; supply actual calibrated intrinsics/baseline.
Images must be undistorted and rectified grayscale stereo with identical K,
zero skew and a horizontal right-camera baseline along positive left-camera x.
Each pair must be synchronized. The loader cannot verify synchronization or
rectification from the CSV; raw EuRoC images must not be passed directly.

```sh
python3 python/stereo_vo.py pairs.csv calibration.json trajectory.jsonl
```

Output poses are `world <- left camera` in metres, with the first camera as
world origin. Failed tracking yields `lost` and a null pose, never a repeated
pose presented as successful tracking. Reset explicitly starts a new trajectory;
there is no relocalization, loop closure, IMU fusion or bundle adjustment.
Replay refuses to overwrite output. A read/validation error exits nonzero and
leaves the completed prefix for diagnosis; it is not a completed trajectory.

## EuRoC MAV replay

For an extracted EuRoC sequence, pass the directory containing `cam0/` and
`cam1/` (normally `mav0/`). Each camera must contain `sensor.yaml`, `data.csv`
and the `data/` images:

```sh
python3 python/euroc_stereo.py /data/MH_01_easy/mav0 trajectory.jsonl
```

The adapter reads pinhole/radial-tangential calibration, interprets EuRoC
`T_BS` as body-from-camera, computes cam1-from-cam0, and undistorts/rectifies
images with OpenCV (`CALIB_ZERO_DISPARITY`, `alpha=0`). The images remain at
calibrated resolution. It passes rectified intrinsics and metric baseline to VO.
Vertical/reversed stereo, invalid calibration, wrong image dimensions, duplicate
or decreasing timestamps, and unequal left/right timestamp lists are rejected.
No approximate synchronization or silent frame dropping is performed.

Epoch nanoseconds stay integers in `timestamp_ns`; VO uses seconds relative to
`timestamp_origin_ns` to avoid rounding epoch timestamps. Poses map the current
**rectified cam0** to the first rectified cam0, not the EuRoC body or raw camera.
`T_rectified_cam0_raw_cam0` records the rectification rotation for downstream
frame conversion. Ground truth and IMU are never loaded. Output refuses overwrite;
read errors leave a diagnostic prefix, while tracking losses remain explicit rows.
There is no automatic recovery after loss.

Tests use synthetic calibration, projections, images and EuRoC-format fixtures.
They establish plumbing and geometry only; no real EuRoC sequence has yet been
evaluated. Next: full-sequence replay, coverage and failure reporting, body/camera
frame-correct trajectory evaluation, then IMU integration. Keep visual results
separate from the LiDAR leaderboard until a fixed visual evaluation protocol exists.

## EuRoC trajectory evaluation

```sh
python3 python/evaluate_euroc.py /data/MH_01_easy/mav0 trajectory.jsonl metrics.json
```

This reads `state_groundtruth_estimate0/data.csv` only in the evaluator, never
in VO. EuRoC world-from-body poses are converted to world-from-rectified-cam0
using `T_BS` and rectification rotation, including the camera lever arm.
The first camera timestamp anchors the reference frame. There is no global
SE(3)/Sim(3) fitting and no scale correction. Reported translation and rotation
RMSE are first-pose-anchored errors, not a benchmark's globally aligned ATE.
GT translation is interpolated linearly and rotation on SO(3); extrapolation
and gaps over `--max-gt-gap-ns` (default 20000000 ns) are rejected. Missing GT
at the first camera timestamp makes all errors unavailable, rather than
silently changing the anchor. Use the same gap setting across sequences.

The report includes expected/output/tracked/evaluated frame counts, tracking
and evaluation fractions over the **entire camera index**, lost timestamps,
unwritten timestamps, and timestamps without usable GT. `complete_output`
means all camera frames have output rows, not that tracking succeeded.
An incomplete JSONL prefix remains incomplete; skipped or reordered frames,
unknown pose frames, inconsistent calibration and implicit resets are errors.
RMSE is null when nothing is evaluable; a low RMSE over a short tracked prefix
must always be reported with coverage. SHA-256 hashes identify trajectory,
GT, camera indices and calibrations (not every raw image). Existing metric
files are never overwritten.

The evaluator is verified with synthetic rotation, camera lever arms, known
translation errors, missing GT, loss and truncated output. Actual EuRoC
accuracy and full-sequence runtime have not yet been measured.
