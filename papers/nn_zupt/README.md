# NN-ZUPT

**A zero-velocity update method based on neural network and Kalman filter for
vehicle-mounted inertial navigation system** (Li et al., Measurement Science
and Technology 2023, doi:10.1088/1361-6501/acabde). From-paper reimplementation;
no author code.

## Core idea

1. **1D-CNN zero-velocity detector** — 50×6 IMU window (gyro + accel) outputs
   stop probability (sigmoid).
2. **ZUPT** — when stop is detected, body velocity is zeroed (pseudo-measurement).
3. **NHC** — when moving, lateral/vertical body velocity are damped toward zero.

Full paper Kalman-filter error-state compensation is **out of scope**; this module
implements **NN stop detection + ZUPT/NHC** on gyro-integrated attitude without
forward speed aiding (compare OdoNet pseudo-odometer path).

## Training (KITTI Raw OXTS)

```sh
python3 papers/nn_zupt/scripts/build_kitti_nn_zupt_dataset.py
python3 papers/nn_zupt/scripts/train_nn_zupt.py
```

Labels: `|vf| < 0.5 m/s` → stop. KITTI drives are mostly moving; stop windows
concentrate on `drive_0011` (~36% of that sequence).

Exports `papers/nn_zupt/weights/nn_zupt_kitti.json` for C++ inference.

## Tests

`test_nn_zupt` — CNN forward, threshold ZUPT, trajectory sampling (3 cases).

## Reproduce (requires `imu.csv`)

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 \
  experiments/reference_data/hdl_400_public_reference.csv \
  --methods nn_zupt --no-gt-seed
```

## Result (cross-sensor transfer, honest negative)

| Dataset | Drift | ATE | ZUPT frames | Notes |
|---------|-------|-----|-------------|-------|
| HDL-400 120f | **92%** | 7.0 m | 0 | KITTI-trained NN never fires on HDL IMU |
| NCLT 600f | **27%** | 28 m | 0 | sparse KITTI stops + cross-IMU transfer |

Held-out KITTI drive (stop-free val): val acc **~100%** (predicts moving).
Cross-IMU eval: **zero ZUPT activations** — expected honest negative when stop
labels are scarce and sensor statistics differ from OXTS.

Ablation: `--nn-zupt-threshold-only` (gyro/accel norm SHOE-like detector).

## KITTI / scope notes

- KITTI **Odometry** trees have no IMU → method **skips** (like OdoNet / NHC-Net).
- Train labels from KITTI **Raw OXTS** forward velocity threshold.
- Paper EKF compensation after ZUPT is out of scope; direct velocity reset only.
