# Public IMU dataset benchmark

This benchmark is IMU-only. Raw public datasets and normalized recordings are
downloaded under `build/` and are never committed. The registry pins source,
DOI, expected license, and checksum in `public_datasets.json`.

## Sources and data contract

| Dataset | Role | Rights | Conversion |
| --- | --- | --- | --- |
| CGU-BES | primary fall/ADL evaluation | CC BY 4.0, DOI 10.6084/m9.figshare.7016306.v1 | 200 Hz; acceleration g to m/s²; published angular velocity used as rad/s |
| UCI HAR | ADL generalization | CC BY 4.0, DOI 10.24432/C54S4K | each official 128-sample inertial window is a separate 50 Hz replay; total acceleration g to m/s² |
| Parkinson | gait generalization | CC BY 4.0, DOI 10.6084/m9.figshare.21800738.v1 | timestamps normalized; metadata-specified acceleration m/s² and angular velocity deg/s converted to rad/s |

The Parkinson column names resemble Apple API names whose customary units
differ from the Figshare description. This benchmark follows the publisher's
explicit metadata rather than guessing from magnitude. CGU-BES supplies only
activity-level fall labels. Its latency therefore uses a documented kinematic
proxy: the first acceleration/gravity deviation over 3 m/s² or angular rate
over 0.5 rad/s within two seconds before peak acceleration. It is not a
video-derived fall-onset label.

## Reproduce

```sh
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py fetch \
  --dataset cgu_bes --dataset uci_har --dataset parkinson \
  --output build/imu_public_data
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py convert \
  --cgu build/imu_public_data/cgu_bes \
  --uci build/imu_public_data/uci_har \
  --parkinson build/imu_public_data/parkinson \
  --output build/imu_public_data/normalized
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py tune \
  --manifest build/imu_public_data/normalized/manifest.json \
  --output build/imu_public_data/results
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py benchmark \
  --manifest build/imu_public_data/normalized/manifest.json \
  --root build/imu_public_data/normalized \
  --cli build/imu_motion_health/imu_motion_health_cli \
  --profile papers/imu_motion_health/config/wearable.yaml \
  --output build/imu_public_data/baseline --allow-fail
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py benchmark \
  --manifest build/imu_public_data/normalized/manifest.json \
  --root build/imu_public_data/normalized \
  --cli build/imu_motion_health/imu_motion_health_cli \
  --profile build/imu_public_data/results/wearable-public-v1.yaml \
  --policy build/imu_public_data/results/wearable-public-v1-policy.json \
  --output build/imu_public_data/results
python papers/imu_motion_health/evaluation/public_dataset_benchmark.py report \
  --input build/imu_public_data/results/benchmark.json \
  --baseline build/imu_public_data/baseline/benchmark.json \
  --output build/imu_public_data/results/report.html
```

CGU-BES subjects 01-09 are tuning, 10-12 validation, and 13-15 untouched test.
UCI's official train/test subject partition is retained. No test subject is
used to choose thresholds. The checked-in aggregate fixture contains no raw
participant signal and locks attribution, split isolation, and the decision
boundary in CI.

## Reproduced result (2026-08-24)

The full run used all 195 CGU-BES recordings, 300 deterministic UCI windows,
and 100 Parkinson ankle recordings. `wearable-public-v1` produced 96.7% CGU
fall sensitivity, 0% CGU ADL false positives, 0% UCI/Parkinson false
positives, and no CLI failures. Median impact latency relative to the
kinematic proxy was 1.78 s. Thus the post-impact safety detector passes the
95% sensitivity and 2% false-positive gates, but it does not meet the earlier
500 ms pre-impact ambition; that requires a separately labeled predictive
model such as KFall and must not be claimed from CGU-BES.

The unchanged built-in `wearable` baseline reached 100% CGU sensitivity but
also triggered on 45.9% of CGU ADLs and 14.3% of the balanced UCI HAR windows
(0% on Parkinson). The tuned profile therefore trades 3.3 percentage points
of sensitivity for a measured elimination of false alarms in these evaluated
sets. Both raw result objects are embedded in the comparison HTML.

The 30° posture confirmation is a downstream fall-candidate policy applied
after an impact lifecycle event. It is intentionally stored beside, not
silently embedded in, the streaming profile because it needs post-event data.
