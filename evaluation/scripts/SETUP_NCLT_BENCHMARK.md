# NCLT benchmark prep (velodyne_sync + GT + IMU)

The [NCLT dataset](http://robots.engin.umich.edu/nclt/) (UMich North Campus
Long-Term) is a fully open, **no-form** download hosted on S3. It provides a
Velodyne HDL-32E, a Microstrain MS25 IMU, and 6-DoF ground truth, which makes it
a good LiDAR(-inertial) odometry benchmark for this repo.

## Download (direct, no form)

Pick a session date (smallest velodyne is `2013-01-10`, ~2.9 GB). URL pattern:

```bash
DATE=2013-01-10
BASE=https://s3.us-east-2.amazonaws.com/nclt.perl.engin.umich.edu
wget "$BASE/velodyne_data/${DATE}_vel.tar.gz"          # velodyne_sync/<utime>.bin
wget "$BASE/ground_truth/groundtruth_${DATE}.csv"      # utime,x,y,z,roll,pitch,yaw
wget "$BASE/sensor_data/${DATE}_sen.tar.gz"            # <date>/ms25.csv (IMU)
tar xzf ${DATE}_vel.tar.gz                              # -> velodyne_sync/
tar xzf ${DATE}_sen.tar.gz                              # -> <date>/ms25.csv
```

Other sessions range ~12-16 GB (largest `2012-03-31` is 95 GB).

## Format notes

- `velodyne_sync/<utime>.bin`: motion-corrected, **body-frame** scan. Each point
  is 8 bytes (`uint16 x, y, z` + `uint8 intensity` + `uint8 layer`); metres are
  `value * 0.005 - 100.0`.
- ground truth is at ~150 Hz with NaN gaps where the offline SLAM lost track; the
  prep script drops NaN rows and associates each scan with the nearest valid pose.
- `ms25.csv` columns: `utime, mag x/y/z, accel x/y/z, gyro x/y/z` (gyro rad/s,
  accel m/s², z-down so accel_z ≈ -9.8 at rest).

## Export for `pcd_dogfooding`

```bash
python3 evaluation/scripts/prepare_nclt_inputs.py \
  --velodyne-dir /path/to/velodyne_sync \
  --ground-truth /path/to/groundtruth_2013-01-10.csv \
  --ms25 /path/to/2013-01-10/ms25.csv \
  --date 2013-01-10 \
  --max-frames 120 --include-full
```

This writes:

- `dogfooding_results/nclt_2013_01_10_120/00000000/cloud.pcd`, … (+ `frame_timestamps.csv`, `imu.csv`)
- `experiments/reference_data/nclt_2013_01_10_120_gt.csv`

## Smoke benchmark

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/nclt_2013_01_10_120 \
  experiments/reference_data/nclt_2013_01_10_120_gt.csv \
  --methods litamin2,kiss_icp,genz_icp,ndt,rko_lio \
  --summary-json /tmp/nclt_smoke.json
```

The committed repo does **not** track the raw NCLT download or the generated
`dogfooding_results/nclt_*` trees; only the prep script + this doc are tracked.
