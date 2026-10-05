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

## Reproduced result (2026-10-05): VQF posture confirmation in the SDK

`wearable-public-v1` now turns on the SDK's opt-in `posture_confirmation`.

- **What changed.** The fall decision is a `fall_confirmed` event emitted by
  the C++ SDK itself. It no longer comes from the accelerometer windows in
  this script.
- **The rule.** A 6D VQF attitude (Laidig and Seel 2023,
  [`papers/attitude_estimation`](../../attitude_estimation/)) gives the body's
  up direction at every sample. An impact at t is confirmed when that direction
  has turned by at least 50° from its mean over [t-2.0, t-1.0] s and stayed
  there for 0.1 s, within [t, t+1.5] s.
- **The fallback.** Otherwise the earlier settled-window rule decides at
  t+1.5 s.
- **Why it is faster.** The gyro-driven attitude sees the posture change while
  the body is still moving, so there is no need to wait for it to settle.

| CGU-BES | Accelerometer windows (2026-10-04) | VQF in the SDK |
|---|---:|---:|
| Fall sensitivity (all / test subjects 13-15) | 96.7% / 100% | 96.7% / 100% |
| ADL false positives (all / test) | 0% / 0% | 0% / 0% |
| Median latency (all / test) | 2.95 s / 3.16 s | **2.13 s / 2.30 s** |

UCI HAR and Parkinson stay at 0% false positives, and no CLI replay fails.

**How it was chosen.** `analyze_vqf_confirmation.py` scored 80 candidates on
CGU train and validation subjects (01-12) only. The grid was threshold
40-60°, dwell 0.1-0.5 s, start 0 or 0.25 s, and VQF `tau_acc` 1 or 3 s.

With the committed impact thresholds, no train or validation ADL produces an
impact event at all. The jump margin was therefore measured as if every jump
had triggered one, at its peak acceleration.

The rule was fixed before the test subjects were opened: 0 false positives,
train sensitivity of at least 95%, and the largest dwell-held jump angle at
least 5° below the threshold; then the lowest median latency. The choice
(50°, 0.1 s, start 0, `tau_acc` 3 s) holds jumps to 40.4° and reaches 2.09 s
on train and validation, against 2.91 s for the accelerometer rule. It was
evaluated once on the test subjects. The record is in
[`vqf_confirmation_selection.json`](vqf_confirmation_selection.json).

**Check of the C++ port.** The SDK's `fall_confirmed` times equal the Python
analysis on all 595 recordings (difference 0). The impact thresholds are
unchanged (45 m/s² and 1 rad/s), and `tune` still selects them.

The `causal_confirmation` code path (accelerometer windows) remains in this
script for policies without `"confirmation_source": "sdk"`.

## Reproduced result (2026-10-04)

The full run used all 195 CGU-BES recordings, 300 deterministic UCI windows,
and 100 Parkinson ankle recordings. `wearable-public-v1` with **causal**
posture confirmation produced 96.7% CGU fall sensitivity, 0% CGU ADL false
positives, 0% UCI/Parkinson false positives, and no CLI failures; the
untouched CGU test subjects (13-15) reach 100% sensitivity and 0% false
positives, so the acceptance gates pass. Median latency to the confirmation
time, relative to the kinematic proxy, is 2.95 s (3.28 s without early
confirmation). The 500 ms pre-impact
ambition remains out of reach; that requires a separately labeled predictive
model such as KFall and must not be claimed from CGU-BES.

**How the posture policy was chosen.** An impact at t is confirmed when the
mean gravity direction over [t-2.0, t-1.0] s and [t+0.5, t+1.5] s differs by
at least 50°, and detection time is t+1.5 s. Jumps are the known confuser:
the landing impact briefly tilts the gravity direction. Using CGU train and
validation subjects only (01-12), these windows maximise the margin between
the 5th-percentile fall angle (78.9°) and the largest jump angle (38.7°); the
threshold is the largest value that keeps train sensitivity at or above 95%.
The test subjects were evaluated once, after this choice.

**Early confirmation.** Instead of always waiting until t+1.5 s, a 0.25 s
window slides from t+0.25 s in 0.05 s steps; the impact is confirmed as soon as
that window shows the 50° change and every sample's gravity direction stays
within 10° of the window mean (the body has settled). Otherwise the t+1.5 s
rule applies. On train+validation every candidate (start 0.25/0.5 s, length
0.25/0.5 s, spread 5-20°) kept jumps unconfirmed, so a margin rule was fixed
before looking at test subjects: the largest stable jump angle must stay at
least 5° below the threshold. Spread 15° and 20° reach 48.7° and 49.9°, so 10°
(42.6°) was chosen with the lowest train latency among the remaining
candidates. Evaluated once on the test subjects: 100% sensitivity, 0% false
positives.

The accel/gyro
impact thresholds are still selected by `tune`, which scores train recordings
with a whole-recording posture proxy; that proxy is a training-time choice and
is never used at detection time.

**Correction history.** The 2026-08-24 result (0% false positives, 1.78 s)
gated impacts with a posture change computed from the first and last second of
each whole recording, which a device cannot know, and measured latency from the
impact. A first causal version (windows [1.5, 0.5] / [0.25, 0.75] s, 30°,
chosen on train only) reached 2.53 s but produced one test false positive
(Subject13 UpwardJump, 3.7%) and failed the 2% gate. The current policy fixes
that jump at the cost of 0.75 s more latency; early confirmation recovers
0.33 s of it.

The unchanged built-in `wearable` baseline reached 100% CGU sensitivity but
also triggered on 45.9% of CGU ADLs and 14.3% of the balanced UCI HAR windows
(0% on Parkinson). The tuned profile therefore trades 3.3 percentage points
of sensitivity for a measured elimination of false alarms in these evaluated
sets. Both raw result objects are embedded in the comparison HTML.

The 30° posture confirmation is a downstream fall-candidate policy applied
after an impact lifecycle event. It is intentionally stored beside, not
silently embedded in, the streaming profile because it needs post-event data
(0.5-1.5 s after the impact).
