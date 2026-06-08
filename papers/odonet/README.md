# OdoNet

**OdoNet: Untethered Speed Aiding for Vehicle Navigation Without Hardware
Wheeled Odometer** (Tang et al., IEEE Sensors Journal 2022, arXiv:2109.03091).
From-paper reimplementation; no author code.

## Core idea

1. **1D-CNN pseudo-odometer** — 50×6 IMU window (gyro + accel) regresses forward
   speed (scaled by 30 m/s).
2. **Data cleaning** — subtract gyro/accel biases; optional body→vehicle rotation.
3. **Strapdown INS + NHC/ZUPT** — integrate attitude from gyro; CNN speed sets
   forward body velocity; NHC zeros lateral/vertical; ZUPT when speed < 0.1 m/s.

Full paper GNSS/INS EKF is out of scope; this module implements the **pseudo-
odometer + vehicle constraints** path for IMU-only dead reckoning.

## Training (KITTI Raw OXTS)

```sh
python3 papers/odonet/scripts/build_kitti_odonet_dataset.py
python3 papers/odonet/scripts/train_odonet.py
```

Exports `papers/odonet/weights/odonet_kitti.json` for C++ inference.

## Tests

`test_odonet` — CNN forward, bias cleaning, trajectory sampling (4 cases).

## Reproduce (requires `imu.csv`)

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 \
  experiments/reference_data/hdl_400_public_reference.csv \
  --methods odonet --no-gt-seed
```

## Result (cross-sensor transfer, honest negative)

| Dataset | Drift | ATE | Notes |
|---------|-------|-----|-------|
| HDL-400 120f | **890%** | 75 m | KITTI-OXTS weights on HDL IMU |
| NCLT 600f | **249%** | 300 m | KITTI-OXTS weights on MS25 IMU |

Speed regression on held-out KITTI drive: val MAE **~2.1 m/s**. Full GNSS/INS EKF
from the paper is out of scope; cross-IMU eval shows weights do **not** transfer
off the training sensor family without retraining (expected honest negative).

Ablation: `--odonet-nhc-only` (INS+NHC without CNN speed).

## KITTI / scope notes

- KITTI **Odometry** trees have no IMU → method **skips** (like CT-LIO).
- Train labels from KITTI **Raw OXTS** forward velocity; eval on HDL-400 / NCLT
  IMU sequences. Cross-sensor transfer is approximate (honest scope boundary).
