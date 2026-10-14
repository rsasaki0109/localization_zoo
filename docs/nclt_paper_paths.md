# Do the paper paths hold up on another lidar? NCLT

The paper pipelines in this repository (`--suma-paper`, `--dilo-paper-odometry`,
`--elo-*`, `--inten-loam-paper-odometry`, `--imls-slam-paper-odometry`,
`--mesh-loam-paper-odometry`) were brought near the papers' KITTI numbers with
values chosen on KITTI. This page runs them on data none of them was tuned
on: NCLT 2013-01-10, a Velodyne HDL-32E on a Segway on the University of
Michigan campus (5105 scans at 5 Hz, 1134 m), with KISS-ICP as the
reference. No paper reports these methods on NCLT, so there is no paper
number to compare with; the question is only whether a port that works on
KITTI still works.

Data preparation: [`SETUP_NCLT_BENCHMARK.md`](../evaluation/scripts/SETUP_NCLT_BENCHMARK.md)
(`prepare_nclt_inputs.py --include-full`). Official KITTI RTE (100-800 m
segments), no GT seed, first pose anchored.

## Sensor settings

Only values that describe the sensor were changed, never values a method
chose on KITTI, except where a value is a size that does not fit 32 beams
(marked "sized"):

- **Frame.** NCLT stores body-frame scans: z points down and the HDL-32E sits
  0.957 m above the body origin. `--input-sensor-pose 0 0 -0.957 180 0 0`
  moves every scan into a gravity-up frame at the sensor and composes the
  ground truth with the same pose. Found with
  [`check_scan_frame.py`](../evaluation/scripts/check_scan_frame.py); see
  [KITTI pitfalls, section 6](kitti_pitfalls.md#6-moving-a-kitti-tuned-port-to-another-lidar).
- **Field of view** seen from that frame: up 31 deg, down 11 deg, 32 rows.
- **Ground** about 0.8 m below the sensor (ELO sensor height 0.8; InTEn-LOAM
  ground lines start below -0.6 m).
- **Intensity** 0-255, scaled to [0, 1] for InTEn-LOAM.
- **Sized:** DiLO bridges 8 image rows between beams (4 for KITTI);
  IMLS-SLAM uses h / r = 0.15 / 0.5 m and 0.3 / 1.0 m (paper 0.06 / 0.2 m).
  These were chosen on the first 600 scans, which are part of the full run.

No vertical-angle correction is applied (it is a KITTI calibration issue).

## Results

| Method | NCLT RTE [%] | rot [deg/100 m] | ATE [m] | FPS | KITTI 00 / 07 RTE [%] | Reading |
|---|---:|---:|---:|---:|---|---|
| KISS-ICP (reference) | **1.75** | 1.22 | 6.2 | 1.7 | 0.535 / - | works |
| IMLS-SLAM (h 0.3, r 1.0) | 2.01 | 1.38 | **5.3** | 0.3 | 0.523 / 0.339 | works once the radii fit the sparser scans; 1.15x KISS-ICP |
| InTEn-LOAM | 2.03 | **1.20** | 49.2 | 1.8 | 0.697 / 0.352 | works, 1.16x KISS-ICP; drifts in position (ATE) |
| IMLS-SLAM (h 0.15, r 0.5) | 3.35 | 2.55 | 16.1 | 0.8 | 0.523 / 0.339 | radii still too small |
| Mesh-LOAM | 13.2 | 9.15 | 52.3 | 0.3 | 0.565 / 0.342 | degrades at the restart after a stop (scans 700-1000) |
| DiLO | 8.53 | 6.78 | 54.3 | 1.9 | 0.549 / 0.372 | degrades: slow drift in rotation, no single failure |
| ELO | 18.1 | 6.61 | 126.6 | 3.3 | 0.586 / 0.391 | degrades: a jump between scans 3000 and 3500 |
| ELO, returns within 5 m dropped | 3.36 | 1.54 | 51.6 | 4.1 | | near objects caused the jumps (below) |
| SuMa | 64.6 | 27.9 | 467.9 | 3.5 | 0.697 / 0.331 | fails when the Segway starts moving after a stop (scan 668) |
| SuMa, normalised lookup | 1624 | 38.3 | 10174 | 4.9 | | fails |
| SuMa, returns within 5 m dropped | 392 | 41.2 | 1258 | 5.0 | | fails |
| Mesh-LOAM, 0.2 m map and input | 42.7 | 27.0 | 228.0 | 0.7 | | worse (2.05 % on scans 550-1100) |
| DiLO, returns within 5 m dropped | 7.37 | 5.55 | 75.8 | 3.0 | | little change |
| KISS-ICP, returns within 5 m dropped | 1.76 | 1.13 | 6.5 | 3.4 | | unchanged |

KITTI columns: the repository's best full-sequence values
([`paper_ratio_table.csv`](assets/paper/paper_ratio_table.csv)); KISS-ICP 07
has no paper value in that table.

Before the frame was fixed the same methods gave SuMa 880 %, DiLO 64 %,
IMLS-SLAM 54 %, InTEn-LOAM 31 % and ELO 9.3 % RTE, while KISS-ICP scored
1.78 %: a port that only fails on a new sensor is first a frame or unit
problem.

## What the failures look like

- **SuMa** tracks the first 600 scans (RPE 4.9 % on that window). Around scan
  668 the Segway starts moving after a stop, rocking by 1-2 deg per scan in
  roll, pitch and yaw; SuMa estimates the step backwards or sideways for
  about 60 scans and never recovers. The normalised lookup, which helped on
  KITTI 01 and 02, makes it worse.
- **Mesh-LOAM** tracks the first 750 scans within 1 m (RPE 1.35 % on the
  first 600), then misses yaw by 2-4 deg per scan at the same restart (scans
  700-735) and reaches 15 m at scan 1000; the paper's 0.1 m input and map
  are kept.
- **ELO** tracks with gradual drift until about scan 3000 (19 m), then jumps
  (123 m at scan 3500).
- **DiLO** drifts without large single steps, mostly in rotation
  (6.8 deg/100 m against 1.2 for KISS-ICP).
- **InTEn-LOAM** is close to KISS-ICP in RTE but its position error grows to
  59 m by scan 2000. On the first 600 scans its steps are 12 % short
  (S2S alone: 28 % short); the scans give few edges (5-30 per scan) and
  ground dominates the features. Removing DOR or the intensity term, a
  coarser plane map, the paper's edge threshold or looser clustering did not
  change this.

## Diagnosis

Done on the full sequence and on scans 550-1100 (the restart after a stop).

- **The platform rocks.** Predicting the next motion with constant velocity
  misses the rotation by 1.96 deg (median, 95 % 4.2 deg) on NCLT against
  0.18 deg on KITTI 00 and 0.12 deg on 07; the Segway pitches and yaws by
  1-3 deg per scan with alternating sign. This is not what breaks SuMa: with
  the identity as initial guess SuMa still fails on scans 550-1100 (82.7 %
  against 87.5 %), and its frame-to-frame fallback threshold changes nothing
  (87.3 %; without the fallback 1280 %). KISS-ICP and IMLS-SLAM, which
  associate by 3D nearest neighbours, follow the rocking.
- **Near objects break the range-image methods.** A person walks about
  2.4 m behind the sensor in about a quarter of the scans, and buildings pass
  within 3-5 m (scans 696-744: 9000 points of a wall on the left; scan 3248:
  10 000 of 38 000 points within 5 m, where ELO estimates a 3.47 m step for a
  0.27 m one). `--input-min-range-m 5` (a runner flag, applied in the sensor
  frame) takes ELO from 18.1 to 3.4 % and its rotation error from 6.6 to
  1.5 deg/100 m, leaves KISS-ICP unchanged (1.76 %) and DiLO almost so
  (7.4 %). SuMa improves on scans 550-1100 (87.5 to 17.9 %, a 3 m cut does
  not: 97 %) but fails on the full run (392 %).
- **Windows can mislead.** Mesh-LOAM with a 0.2 m map scores 2.05 % on scans
  550-1100 (0.1 m: 15.4 %) and 42.7 % on the full run, where it misses steps
  from scan 80 on; the window starts from an empty map. The 5 m cut makes
  Mesh-LOAM worse on the window (25.3 %).

No setting found here makes SuMa, DiLO or Mesh-LOAM track NCLT; ELO does
with the 5 m cut. These are diagnoses on the evaluated data, not tuned
results.

## Commands

```bash
T="--input-sensor-pose 0 0 -0.957 180 0 0"
D=dogfooding_results/nclt_2013_01_10_full
GT=experiments/reference_data/nclt_2013_01_10_full_gt.csv
run() { ./build/evaluation/pcd_dogfooding $D $GT 0 --no-gt-seed --summary-json "$1.json" --methods "${@:2}" $T; }

run kiss  kiss_icp --kiss-upstream-profile --kiss-model-deviation-threshold
run inten inten_loam --inten-loam-paper-odometry --input-max-range-m 120 \
  --inten-loam-paper-set theta_up_deg=31 --inten-loam-paper-set theta_down_deg=11 \
  --inten-loam-paper-set height=32 --inten-loam-paper-set ground_start_below=-0.6
run imls_r1 imls_slam --imls-slam-paper-odometry --imls-slam-paper-r 1.0 --imls-slam-paper-h 0.3
run imls  imls_slam --imls-slam-paper-odometry --imls-slam-paper-r 0.5 --imls-slam-paper-h 0.15
run mesh  mesh_loam --mesh-loam-paper-odometry
run dilo  dilo --dilo-paper-odometry --dilo-paper-min-planarity 0.98 --dilo-paper-keypoint-cell 8 \
  --dilo-paper-fill-slope 0.1 --dilo-paper-top-elevation 31 --dilo-paper-height 240 --dilo-paper-max-fill-gap 8
run elo   elo --elo-dense-profile --elo-normal-full-patch --elo-sri-fuse-same-surface --elo-ground-no-forward \
  --elo-curvature-threshold 0.03 --elo-fov 31 -11 --elo-sri-height 32 --elo-sensor-height 0.8
run suma  suma --suma-paper --suma-paper-set fov_up_deg=31 --suma-paper-set fov_down_deg=11 \
  --suma-paper-set height=32
# near-range cut: append --input-min-range-m 5 to any run
```

Run summaries: [`benchmarks/nclt_paper_paths/`](benchmarks/nclt_paper_paths/).
