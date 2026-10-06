# IMU attitude estimation (Madgwick, Mahony, VQF, Valenti, Seel) on BROAD

IMU orientation filters and a reproduction of their published errors on the
public BROAD benchmark. This is the repository's first IMU **orientation**
family; the other IMU modules estimate motion or health, not attitude.

## Papers

- **Madgwick**: S. O. H. Madgwick, "An efficient orientation filter for inertial
  and inertial/magnetic sensor arrays", report, Univ. of Bristol, 2010
  (gradient-descent filter).
- **Mahony**: R. Mahony, T. Hamel, J.-M. Pflimlin, "Nonlinear complementary
  filters on the special orthogonal group", IEEE TAC 53(5), 2008 (explicit
  complementary filter with gyro-bias estimation).
- **VQF**: D. Laidig, T. Seel, "VQF: Highly accurate IMU orientation estimation
  with bias estimation and magnetic disturbance rejection", *Information
  Fusion* 91, 2023 (arXiv:2203.17024). The pieces are:
  - BasicVQF (Algorithm 1): strapdown integration, inclination correction from
    the accelerometer low-passed in the almost-inertial frame, and heading
    correction as a scalar offset.
  - Gyro-bias Kalman filter with rest detection and the motion measurement of
    eq. (42) (Algorithm 2, Appendices C-E).
  - Magnetic disturbance rejection (Algorithm 3).

Both have widely used public implementations (x-io Technologies C code). The
code here is written from the papers; the x-io code was only read to match its
conventions (gains `2 Kp`/`2 Ki`, forward-Euler quaternion integration,
normalised acc/mag, the earth-field reference trick), so that BROAD's published
per-trial numbers can be reproduced exactly.

- **Valenti**: R. G. Valenti, I. Dryanovski, J. Xiao, "Keeping a good
  attitude: a quaternion-based orientation filter for IMUs and MARGs",
  *Sensors* 15(8), 2015 (open access). Written from the paper:
  - gyro prediction;
  - accelerometer and magnetometer delta quaternions with LERP/SLERP
    filtering;
  - adaptive gain (optional).

  Two details the paper leaves to the implementation follow the authors' ROS
  package (imu_tools `imu_complementary_filter`, BSD): the steady-state test
  of the bias low-pass, and initialisation from the first sample.
- **Seel**: T. Seel, S. Ruppin, "Eliminating the effect of magnetic
  disturbances on the inclination estimates of inertial sensors", IFAC 2017.
  The paper is not openly available, so this is a **port** of the authors' MIT
  implementation (`CsgOriEstIMU`, shipped in qmt as `oriEstIMU`), not a
  reimplementation from the paper.

## Benchmark

- **BROAD** (D. Laidig, M. Caruso, A. Cereatti, T. Seel, *Data* 6(7):72, 2021;
  CC BY 4.0): 39 trials, 286 Hz IMU with optical motion-capture ground truth,
  grouped into undisturbed / disturbed, slow / fast, rotation / translation and
  more. The files are pinned to commit `7e2f8189` and verified by git blob
  SHA-1 (`evaluation/broad_dataset.json`).
- **Metric**: BROAD's own (`example_code/broad_utils.py`), ported to C++. The
  error quaternion `q_est * q_ref^-1` is taken in the earth frame, and its
  total, heading, and inclination angles are RMS-averaged over the movement
  phases. Samples without motion-capture data are skipped, as in `nanmean`.
- **Protocol**: also BROAD's. The initial state comes from the first acc/mag
  sample (`quatFromAccMag`), and the output is rotated by 90 deg about z into
  ENU. `quatFromAccMag` is ported exactly, including its choice of earth frame.
  Its first estimate is a few degrees off in inclination, and the filters
  correct this within seconds.

## Results (all 39 trials, degrees)

`docs/attitude_benchmark.md` holds the generated table. Summary:

| Variant | Mode | Repo | Published | Source |
|---|---|---:|---:|---|
| Madgwick, x-io code, beta 0.12 | 9D total | 4.959 | 4.959 | BROAD (per trial: max diff 0.0001) |
| Mahony, x-io code, Kp 0.74 Ki 0.0012 | 9D total | 7.489 | 7.489 | BROAD (per trial: max diff 0.001) |
| Madgwick, corrected field scale, beta TAGP | 9D total | 4.69 | 4.69 | BROAD issue #1 |
| Madgwick, corrected, beta 0.29 | 9D total | 7.44 | 7.4 | VQF paper Fig. 9 |
| Mahony, Kp 1.44 Ki 0.0027 | 9D total | 8.92 | 8.9 | VQF paper Fig. 9 |
| Madgwick, beta 0.29 | 6D inclination | 5.04 | 5.0 | VQF paper Fig. 9 |
| Mahony, Kp 1.44 Ki 0.0027 | 6D inclination | 5.16 | 5.2 | VQF paper Fig. 9 |
| VQF, defaults | 9D total | 2.302 | 2.3 | VQF paper Fig. 9; per trial vs VQF code: max diff 0.0002 |
| VQF, defaults | 6D inclination | 0.696 | 0.7 | VQF paper Fig. 9; per trial vs VQF code: max diff 0.0000 |
| BasicVQF | 9D total / 6D incl. | 3.372 / 0.977 | - | per trial vs VQF code: max diff 0.0000 |
| Valenti, VQF gains, old ROS interpolation | 9D total / 6D incl. | 6.07 / 3.21 | 6.1 / 3.2 | VQF paper Fig. 9; per trial vs ROS code (pre-#234): max diff 0.0000 |
| Valenti, VQF gains, paper interpolation | 9D total / 6D incl. | 7.07 / 3.81 | - | per trial vs current ROS code: max diff 0.0000 |
| Seel, VQF parameters | 9D total / 6D incl. | 5.11 / 2.59 | 5.1 / 2.6 | VQF paper Fig. 9; per trial vs qmt code: max diff 0.0000 |

The per-trial differences against BROAD's published results come from the
reference implementation using `float`; this code uses `double`.

### The x-io field-scale error

The x-io C code computes the earth-field reference `b` at half the magnitude
that the paper's objective function implies
([dlaidig/broad#1](https://github.com/dlaidig/broad/issues/1)).
`--legacy-xio-field-scale` reproduces that behaviour, and is needed to match
BROAD's published 4.96 deg. The corrected form, re-tuned on BROAD's own beta
grid, reaches 4.69 deg, the value the dataset author reported after the fix.

The VQF paper's Madgwick value (7.4 deg at beta 0.29) is reproduced only with
the corrected scale (7.44 deg). The x-io scale gives 6.61 deg. The VQF
evaluation therefore evidently used an implementation without the x-io error.
That variant was first run with the x-io scale and switched after comparing
the two.

### Valenti: the interpolation switch

Valenti et al. filter each delta quaternion towards identity. They use LERP
when it is close to identity (q0 > 0.9) and SLERP otherwise (eqs. 50-52).

- **The ROS package's history.** Until 2026-09 the package switched to SLERP
  only for q0 < 0, so it effectively always used LERP. Its fix
  ([imu_tools #234](https://github.com/CCNYRoboticsLab/imu_tools/pull/234))
  restored the paper's 0.9.
- **What reproduces the VQF paper (2022).** Its values (6.1° / 3.2°) are
  matched only with the old behaviour: `--valenti-legacy-ros` gives 6.07° /
  3.21°.
- **The paper's switch** with the same gains gives 7.07° / 3.81°. Those gains
  were tuned (in the VQF paper) with the old switch.
- Both variants are kept, and each equals the matching ROS code on every
  trial.

### VQF: what the paper leaves open

VQF is written from the paper. To check it, the authors' MIT-licensed package
(`pip install vqf`, 2.1.2) was run on the same 39 trials. Its per-trial errors
are stored in `evaluation/broad_reference.json` as "VQF code". BasicVQF and the
bias estimation matched exactly from the paper alone. The magnetic disturbance
rejection did not (2.32 deg vs 2.30 deg). The package source was read only for
the points below, which the paper does not state. They are now matched:

- Defaults the paper does not list: `mag_ref_tau` = 20 s (the `k_ref` time
  constant) and `mag_new_first_time` = 5 s (acceptance time for the very first
  field reference). They come from the package's documented parameters.
- The rejection timer starts at its 60 s limit. Without a reference, heading is
  corrected at half gain rather than not at all.
- New-field acceptance counts motion time on the rest detector's low-passed
  gyro norm. The paper writes `|omega|`.
- The initial heading averaging (gain 1, 1/2, 1/3, ...) overrides the rejection
  gain. It stops once `k * tau_mag < Ts`.

Other choices follow the paper:

- The second-order Butterworth filters come from a bilinear transform at
  `f_c = sqrt(2) / (2 pi tau)` (eq. 8).
- Each low-pass outputs the running mean for its first `tau / Ts` samples
  (Appendix A.2) and then starts from that mean.
- The motion bias measurement is `[-a_y/Ts, a_x/Ts, 0] + diag(1, 1, 0) LPF(R b)`
  with `C = LPF(R)`, as in eqs. (19) and (42).

VQF runs with its own initialisation and outputs ENU directly, so it does not
use BROAD's `quatFromAccMag` / 90 deg step (the VQF paper evaluates it the same
way).

### Caveats

- **TAGP** (BROAD's term) is the parameter set with the lowest error averaged
  over all 39 trials. It is tuned on the evaluated data by definition.
- The VQF-paper values use VQF's TAGPx parameters, which were tuned on six
  datasets rather than on BROAD alone.
- 6D filters cannot observe heading, so 6D variants are compared on
  inclination.

## Magnetometer calibration (hard / soft iron)

A real magnetometer reads `m = A h + b`:

- `b` is the hard-iron offset, for example from a magnetised part or a current
  on the board.
- `A` is the soft-iron distortion, from nearby ferromagnetic material and the
  sensor's own scale and cross-axis errors.

Over a full rotation the readings lie on an ellipsoid instead of a sphere.
`fitMagnetometerCalibration` (CLI: `magnetometer_calibration_cli`) fits that
ellipsoid with the ellipsoid-specific least squares of Li and Griffiths (GMP
2004). It returns the correction `m_cal = W (m - b)`, the fitted field norm,
the RMS norm residual, and the direction coverage (of 26 sphere bins; warns
below 18). `--profile-yaml` writes the `mag_offset_*` / `mag_matrix_*` keys
that `imu_motion_health` applies before its VQF attitude.

[`evaluation/broad_mag_calibration.py`](evaluation/broad_mag_calibration.py)
checks it on BROAD. BROAD's magnetometer is already calibrated, so the script
applies a known distortion to every trial: soft iron with scale 0.9-1.15 and
cross terms up to 0.08, plus a 31 µT offset. It fits the calibration from
trial 01 (a slow rotation) only, and applies it to all 39 trials:

| Magnetometer | VQF 9D total RMSE (mean / max) |
|---|---:|
| original BROAD | 2.30° / 7.7° |
| distorted | **70.5°** / 100.9° |
| distorted, calibrated from trial 01 | **2.26°** / 7.6° |
| same, end to end through `imu_motion_health_cli` + the written profile | 2.27° |

- **Fit accuracy.** The fit recovers the offset within 0.5 µT and the soft
  iron within 0.022, with 24 of 26 directions covered.
- **Why calibrated beats the original.** The calibrated case is slightly
  better than the original because the fit also absorbs BROAD's own small
  residual distortion (its field norm varies by ~3 %).
- **Noise-free check.** A unit test on noise-free synthetic data recovers the
  offset and soft iron to 1e-6.

## Usage

```bash
cmake --build build --target attitude_benchmark_cli
E=papers/attitude_estimation/evaluation
python3 $E/broad_benchmark.py fetch      # ~160 MB into dogfooding_results/broad
python3 $E/broad_benchmark.py convert
python3 $E/broad_benchmark.py run        # all variants, ~10 s on 8 cores
python3 $E/broad_benchmark.py report     # experiments/results/broad_attitude_benchmark.json + docs/attitude_benchmark.md
python3 $E/broad_benchmark.py check      # drift check against evaluation/broad_expected.json
```

`attitude_benchmark_cli <trial.csv> --rate HZ --method madgwick|mahony|vqf
[--mode 9d|6d] [--beta B] [--legacy-xio-field-scale] [--kp KP] [--ki KI]
[--tau-acc S] [--tau-mag S] [--vqf-basic] [--vqf-no-rest-bias]
[--vqf-no-motion-bias] [--vqf-no-mag-rejection] [--output-quat out.csv]`
prints the three RMSE values as JSON.
