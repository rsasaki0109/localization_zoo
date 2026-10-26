# Experimental stereo visual odometry

A small image-driven baseline, independent of the known-landmark prototypes.
It uses ORB stereo/temporal matches, rectified disparity and RANSAC PnP.
No ground-truth pose or prebuilt landmark map is accepted. This is not a
VINS-Fusion, OKVIS or ORB-SLAM3 reproduction and has no real-data accuracy claim.

Install `numpy` and `opencv-python-headless==4.11.0.86` in a virtual environment.
Run `python3 -m unittest discover -s python/test -p test_stereo_vo.py -v`.
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

Next: EuRoC calibration/rectification adapter, timestamp validation and full
sequence evaluation, then IMU integration. Synthetic patches and projected
points establish plumbing and geometry only, not real-world robustness.
