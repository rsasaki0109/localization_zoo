# indoor_hard_01 full-trajectory result

The six default window-odometry configurations completed all 2,379 Azure
Kinect frames. Input and GT stamps match exactly one-to-one; the bag also
contains 44,398 IMU samples.

The canonical easy-vs-hard table and interpretation are in the parent
`experiments/results/hard_pcl_localization/README.md`. Machine-readable traces
for this sequence are in `full/*.json`.

Reproduction:

```bash
/root/lz-venv/bin/python3 evaluation/scripts/run_hard_pcl_odometry_benchmark.py \
  --pcd-dir dogfooding_results/hard_pcl_localization/indoor_hard_01 \
  --gt-csv experiments/reference_data/hard_pcl_indoor_hard_01_gt.csv \
  --output-dir experiments/results/hard_pcl_localization/indoor_hard_01/full \
  --jobs 6
```

## Upstream BIEVR-LIO

The official upstream core was run with the 5 cm bump-image map, map-informed
sampling, IMU backend, and the indoor LiDAR-to-IMU calibration. It associates
2,373 poses but diverges:

| ATE-XY (m) | RPE-XY (m/f) | Estimated path (m) | GT path (m) | Final error (m) |
|---:|---:|---:|---:|---:|
| 8,882.579 | 21.947 | 28,754.33 | 114.71 | 28,405.94 |

Error first exceeds 1 m at associated frame 943, 10 m at frame 1,124, and
100 m at frame 1,228. The excellent easy result (0.422 m ATE) validates the
adapter/calibration; this hard failure is not the simplified local
reimplementation result.
