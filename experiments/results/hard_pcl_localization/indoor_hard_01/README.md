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
