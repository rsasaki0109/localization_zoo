# NHC-Net / VMSC

**NHC-Net: A Novel Vehicle Dead Reckoning Method With Adaptive Non-Holonomic
Constraints** (Li et al., GPS Solutions 2023). From-paper reimplementation; no
author code.

## Core idea

1. **VMSC CNN** — 50×6 IMU window (gyro + accel) classifies motion state (stop /
   straight / turn / slip) and regresses lateral + vertical body velocities.
2. **Adaptive NHC** — soft pull toward CNN-predicted lateral/vertical velocity;
   gain scaled by class confidence.
3. **ZUPT** — zero velocity when stop class is detected.

Full paper GNSS/INS EKF and forward speed aiding (wheel odometry / pseudo-
odometer) are **out of scope**. This module implements the **VMSC + adaptive NHC**
path on gyro-integrated attitude without forward speed regression (compare
OdoNet for pseudo-odometer speed).

## Training (KITTI Raw OXTS)

```sh
python3 papers/nhc_net/scripts/build_kitti_nhc_net_dataset.py
python3 papers/nhc_net/scripts/train_nhc_net.py
```

Exports `papers/nhc_net/weights/nhc_net_kitti.json` for C++ inference.

## Tests

`test_nhc_net` — VMSC forward, adaptive NHC step, trajectory sampling (3 cases).

## Reproduce (requires `imu.csv`)

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/hdl_400_open_ct_lio_120 \
  experiments/reference_data/hdl_400_public_reference.csv \
  --methods nhc_net --no-gt-seed
```

## Result (cross-sensor transfer, honest negative)

| Dataset | Drift | ATE | Notes |
|---------|-------|-----|-------|
| HDL-400 120f | **89%** | 6.7 m | KITTI-OXTS weights; no forward speed → under-travels |
| NCLT 600f | **32%** | 29 m | adaptive NHC only; ZUPT never fires off-domain |

Held-out KITTI drive: val class acc **~100%**, lateral/vertical NHC MAE **~0.04 m/s**.
Without pseudo-odometer / GNSS forward speed the vehicle barely advances along the
GT path — expected scope boundary vs. the full paper EKF.

Ablation: `--nhc-net-{no-zupt,fixed-gain}` / `--nhc-net-gain`.

## KITTI / scope notes

- KITTI **Odometry** trees have no IMU → method **skips** (like CT-LIO / OdoNet).
- Train labels from KITTI **Raw OXTS** motion classes + lateral/vertical velocity.
- Cross-sensor eval on HDL-400 / NCLT IMU is approximate (honest scope boundary).
