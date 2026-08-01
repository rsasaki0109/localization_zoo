# LiDAR Odometry SOTA Development Protocol

This is the frozen development contract for the SOTA-oriented LiDAR odometry
track. The machine-readable suite lives in
`evaluation/data/lidar_odometry_sota_suite.json`.

## Data placement

Large public datasets, converted PCDs, and temporary outputs stay outside the
repository on an external SSD. Set one machine-local environment variable:

```powershell
$env:LOCALIZATION_ZOO_DATA_ROOT = 'E:\datasets\loc_zoo'
python evaluation/scripts/verify_lidar_sota_data.py
```

Linux uses the same variable, for example
`LOCALIZATION_ZOO_DATA_ROOT=/media/.../loc_zoo`. Manifests may reference
`${LOCALIZATION_ZOO_DATA_ROOT}`; `run_experiment_matrix.py` expands the variable
at runtime and reports an unresolved variable instead of silently treating it
as a repository-relative path.

Only acquisition/conversion scripts, manifests, compact aggregate results,
tests, and reproduction instructions belong in Git.

KITTI Odometry is kept in its official four-float Velodyne `.bin` form. The
dogfooding runner reads these scans directly, so a second PCD copy is not
needed. Fetch only the sequences assigned by this protocol:

```powershell
wsl -d Ubuntu-22.04 -- /path/to/python `
  evaluation/scripts/fetch_kitti_odometry_sequences.py `
  --output-root /media/external/loc_zoo/raw/kitti_odometry `
  --sequence 00 --sequence 05 --sequence 07
```

The selective fetcher reads the official ZIP directory, transfers only the
contiguous byte ranges containing requested sequences, checks every newly
written frame against its ZIP CRC, and can resume by reusing completed frames.
It requires the small `remotezip` Python package for remote central-directory
inspection.

## Leakage boundary

- Ground truth is used for metrics only.
- Initialization is a first-pose anchor, not a per-frame ground-truth seed.
- Development sequences may be used for implementation and parameter search.
- Validation sequences select a candidate configuration.
- Held-out sequences are run only after the candidate configuration is frozen.
- NTNU tunnel/fog sequences have no ranking-quality GT and are robustness
  probes, not accuracy leaderboard rows.
- A configuration promoted from held-out results must be rerun unchanged on
  every held-out sequence.
- External odometry subprocesses must not receive a reference path and must run
  against a dataset mirror that contains no sequence GT. The parent evaluator
  may open GT only after that subprocess exits.
- Multi-odometry fusion must be causal: output frame i may depend only on input
  frames 0 through i. Any offline smoothing with future poses is excluded.

## Frozen split

| Role | Datasets |
|---|---|
| Development | KITTI Odometry 00, 05 |
| Validation | KITTI Odometry 07, MulRan ParkingLot |
| Held out | KITTI Odometry 02, 08, NCLT 2013-01-10 |
| Robustness only | NTNU tunnel full, NTNU fog full |
| Diagnostic | corrected KITTI Raw 0009 |

Candidate-specific fresh sets are additive to this original split. Their
manifests must name and freeze a new sequence before its scans or GT are
downloaded. KITTI 04, 06, 09, and 10 have already been consumed by earlier
generations; the dual-KISS rotation-consensus v3 generation predeclares KITTI
03 and may not substitute an already observed sequence after seeing its result.
Dual-KISS failover v5 subsequently consumed KITTI Raw
2011_09_26_drive_0001_sync as its predeclared fresh sequence.

The initial frozen baselines are KISS-ICP, A-LOAM, LeGO-LOAM, Small-GICP, and
CT-ICP. Additional current public baselines may be added, but existing baseline
rows must not be removed after seeing candidate results.

## Promotion gate

The primary score is geometric-mean translational RPE across the GT-backed
held-out set. A candidate is promoted only if it:

1. beats the strongest frozen baseline on that primary score;
2. does not catastrophically fail any held-out or robustness sequence;
3. reports rotational RPE, ATE, FPS, and tracking success alongside the primary
   metric;
4. sustains at least 10 FPS on the declared benchmark machine; and
5. has one frozen configuration, with dataset-specific tuning reported only as
   an ablation.

The runtime gate uses per-frame odometry algorithm time and excludes dataset
file I/O, matching the timing boundary used by the official external baseline.
Every result must also report end-to-end FPS including scan loading. This
boundary is fixed before any held-out candidate run; end-to-end timing is not
discarded or substituted for the gate metric.

For a dual-front-end candidate, the reported algorithm FPS must cover both
front ends and fusion. Summing isolated component times is a conservative
sequential measurement; a parallel-runtime claim requires one concurrent run
on the declared benchmark machine and reports its wall time and CPU allocation.

Passing this repository gate means "SOTA candidate under the Localization Zoo
protocol." A broader SOTA claim additionally requires comparison with current
official implementations under their published protocols.
