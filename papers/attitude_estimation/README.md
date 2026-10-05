# IMU attitude estimation (Madgwick, Mahony) on BROAD

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

Both have widely used public implementations (x-io Technologies C code). The
code here is written from the papers; the x-io code was only read to match its
conventions (gains `2 Kp`/`2 Ki`, forward-Euler quaternion integration,
normalised acc/mag, the earth-field reference trick), so that BROAD's published
per-trial numbers can be reproduced exactly.

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

### Caveats

- **TAGP** (BROAD's term) is the parameter set with the lowest error averaged
  over all 39 trials. It is tuned on the evaluated data by definition.
- The VQF-paper values use VQF's TAGPx parameters, which were tuned on six
  datasets rather than on BROAD alone.
- 6D filters cannot observe heading, so 6D variants are compared on
  inclination.

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

`attitude_benchmark_cli <trial.csv> --rate HZ --method madgwick|mahony
[--mode 9d|6d] [--beta B] [--legacy-xio-field-scale] [--kp KP] [--ki KI]
[--output-quat out.csv]` prints the three RMSE values as JSON.
