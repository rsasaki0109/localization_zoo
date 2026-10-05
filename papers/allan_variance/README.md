# Allan variance (IMU noise parameters)

Measures an IMU's noise from a static recording: the white noise density and
the bias random walk that LIO/VIO configurations ask for, plus the bias
instability. Checked against the published values of the TUM VI benchmark.

## References

- N. El-Sheimy, H. Hou, X. Niu, "Analysis and modeling of inertial sensors
  using Allan variance", *IEEE Trans. Instrum. Meas.* 57(1), 2008; IEEE Std
  952-1997. These cover the overlapping Allan variance and how noise terms map
  to slopes.
- D. Schubert et al., "The TUM VI benchmark for evaluating visual-inertial
  odometry", IROS 2018 (arXiv:1804.06120). Sec. III-C and Appendix A give the
  estimator (eq. 11), the fit protocol, and the reference values.

## What it computes

- The **overlapping Allan deviation** `sigma^2(m tau0) = sum_i (g_{i+m} - g_i)^2 / (2 (M - 2m + 1))`
  (TUM VI eq. 11). It runs in O(M) per cluster size from a cumulative sum, with
  the mean removed first to keep precision. Cluster sizes are log-spaced, 20
  per decade by default.
- **White noise density** `sigma_w`: a line of slope -1/2 read at tau = 1 s.
  Units are signal/sqrt(Hz), e.g. rad/s/sqrt(Hz), the Kalibr
  `*_noise_density`.
- **Bias random walk** `sigma_b`: a line of slope +1/2 read at tau = 3 s. Units
  are signal/s/sqrt(Hz), the Kalibr `*_random_walk`.
- **Bias instability**: the minimum of the Allan deviation divided by
  `sqrt(2 ln 2 / pi)` = 0.664.

The lines can be fitted in two ways. `fixedSlopeFit` averages the log offset
over a given tau range (the TUM VI protocol). `tangentFit` uses the single
point whose local slope is closest to the target (the usual automatic choice).

## Result: TUM VI `dataset-calib-imu-static2`

The recording is 111 h of a resting BMI160 (79,964,381 samples at 199.37 Hz;
CC BY 4.0, 4.8 GB, md5-verified). The paper's Fig. 5 caption sets the protocol:

- Slope -1/2 is fitted over 0.02-1 s on the Allan deviation averaged over all
  three axes.
- Slope +1/2 is fitted over 1000-6000 s, averaged over all accelerometer axes
  but only gyro y and z.

| Parameter | Repo | TUM VI paper | Repo / paper |
|---|---:|---:|---:|
| gyro white noise density [rad/s/√Hz] | 8.05e-5 | 8.0e-5 | 1.01x |
| gyro bias random walk [rad/s²/√Hz] | 2.17e-6 | 2.2e-6 | 0.98x |
| accel white noise density [m/s²/√Hz] | 1.34e-3 | 1.4e-3 | 0.96x |
| accel bias random walk [m/s³/√Hz] | 8.50e-5 | 8.6e-5 | 0.99x |

The paper gives two significant digits. Its text does not say how the three
axes are combined. Here the Allan deviation curves are averaged before the fit;
that choice was made before running and was not tuned. The full table, per-axis
values, and plot are in [`docs/allan_variance.md`](../../docs/allan_variance.md).
`evaluation/tumvi_bmi160_imu.yaml` holds the result as a Kalibr-style IMU noise
file.

### Caveats

- The automatic tangent fit for the random walk is fragile on real data. On
  this recording the slow temperature cycle (the bump near tau = 10^4 s) and
  gyro x's flat curve make the per-axis automatic values spread by a factor of
  6, which is why TUM VI fits fixed ranges and excludes gyro x. Prefer
  `fixedSlopeFit` with ranges chosen from the plot.
- Values are as measured. Estimators usually need them inflated; TUM VI's own
  evaluation inflates them.

## Usage

```bash
cmake --build build --target allan_variance_cli
# any static log: CSV with a header line, or a 2-D float64 .npy
build/papers/allan_variance/allan_variance_cli imu.csv --columns 1,2,3,4,5,6 \
    --names gx,gy,gz,ax,ay,az --time-column 0      # or --rate HZ
# TUM VI reproduction (downloads 4.8 GB into dogfooding_results/)
E=papers/allan_variance/evaluation
python3 $E/tumvi_allan.py fetch && python3 $E/tumvi_allan.py run
python3 $E/tumvi_allan.py report && python3 $E/tumvi_allan.py check
```

The CLI prints JSON: per column, the curve (`tau`, `adev`) and the automatic
parameters. Channels are processed in parallel; the 111 h recording takes about
30 s on 8 cores.
