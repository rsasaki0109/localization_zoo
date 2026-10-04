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

## Regression check

`.github/workflows/imu-public-benchmark.yml` re-runs this full pipeline weekly,
on manual dispatch, and on pull requests that touch `papers/imu_motion_health/`.
After `benchmark`, run:

```sh
python papers/imu_motion_health/evaluation/check_public_benchmark.py \
  --results build/imu_public_data/results
```

It fails if `tune` no longer selects the committed `wearable-public-v1`
thresholds and policy, or if any group metric differs from
[`public_benchmark_expected.json`](public_benchmark_expected.json). Update that
file only together with a documented reason in this section.

## Reproduced result (corrected 2026-10-04)

The full run used all 195 CGU-BES recordings, 300 deterministic UCI windows,
and 100 Parkinson ankle recordings. `wearable-public-v1` with **causal**
posture confirmation produced 96.7% CGU fall sensitivity, 0.7% CGU ADL false
positives (1 of 135), 0% UCI/Parkinson false positives, and no CLI failures.
Median latency to the confirmation time, relative to the kinematic proxy, is
2.53 s. On the untouched CGU test subjects (13-15) sensitivity is 100% but
the ADL false-positive rate is 3.7% (1 of 27: Subject13 UpwardJump), so the
**2% false-positive acceptance gate fails** and `passed` is now false. The
500 ms pre-impact ambition remains out of reach; that requires a separately
labeled predictive model such as KFall and must not be claimed from CGU-BES.

**Correction.** The 2026-08-24 result (0% ADL false positives, 1.78 s, gates
passed) gated impacts with `posture_change_deg` computed from the first and
last second of each whole recording, which a device cannot know at detection
time, and measured latency from the impact instead of from the moment the
posture check could complete. The policy now carries `pre_window_s`
[1.5, 0.5] and `post_window_s` [0.25, 0.75]: an impact at t is confirmed when
the mean gravity direction over [t-1.5, t-0.5] s and [t+0.25, t+0.75] s
differs by at least 30°, and detection time is t+0.75 s. The windows were
chosen on CGU train subjects only; train and validation subjects could not
separate the candidate windows, and the test result was not used to choose
them. The jump false positive comes from the landing impact briefly changing
the body's gravity direction inside the short post-impact window.

The unchanged built-in `wearable` baseline reached 100% CGU sensitivity but
also triggered on 45.9% of CGU ADLs and 14.3% of the balanced UCI HAR windows
(0% on Parkinson). The tuned profile therefore trades 3.3 percentage points
of sensitivity for a measured elimination of false alarms in these evaluated
sets. Both raw result objects are embedded in the comparison HTML.

The 30° posture confirmation is a downstream fall-candidate policy applied
after an impact lifecycle event. It is intentionally stored beside, not
silently embedded in, the streaming profile because it needs post-event data
(0.75 s after the impact).
