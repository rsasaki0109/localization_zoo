# Cross-dataset benchmarks (MulRan / NCLT)

LiDAR-only from-paper methods re-evaluated on non-KITTI dogfooding trees with
`--no-gt-seed` (velocity-model prior, no GT initialization).

## Datasets

| Key | Tree | Frames | Trajectory |
|-----|------|--------|------------|
| `mulran_120` | `dogfooding_results/mulran_parkinglot_120` | 120 | ~18 m |
| `mulran_full` | `dogfooding_results/mulran_parkinglot_full` | 1177 | ~432 m |
| `nclt_600` | `dogfooding_results/nclt_2013_01_10_600` | 600 | ~101 m |

## Reproduce

```bash
python3 evaluation/scripts/run_cross_dataset_benchmark.py \
  --dataset mulran_120 --dataset mulran_full --dataset nclt_600
```

Default methods: `kiss_icp`, `i_loam`, `inten_loam`, `mcgicp`, `icpsc`, `mcc_lo`.

## Results (2026-06-09)

### MulRan parkinglot 120

| Method | Drift [m/100m] | ATE [m] |
|--------|---------------:|--------:|
| KISS-ICP | 225.1 | 15.7 |
| MCC-LO | 225.7 | 15.7 |
| ICPSC-LO | 228.6 | 26.4 |
| InTEn-LOAM (TVF/DOR/mapping) | 185.4 | 32.0 |
| I-LOAM | 5535.5 | 379.6 |
| MCGICP-LO | 5723.8 | 722.3 |

Best LiDAR-only: KISS-ICP / MCC-LO (~225% drift). InTEn-LOAM with mapping is
modestly better on drift but worse ATE. Feature-based intensity methods (I-LOAM,
MCGICP) diverge on MulRan HDL-32E without KITTI-style tuning.

Artifact: `mulran_120_bundle.json`

### MulRan parkinglot full (1177 frames)

| Method | Drift [m/100m] | ATE [m] |
|--------|---------------:|--------:|
| KISS-ICP | 103.3 | 74.3 |
| MCC-LO | 103.4 | 74.4 |
| ICPSC-LO | 84.6 | 69.9 |
| InTEn-LOAM (TVF/DOR/mapping) | **59.1** | 111.4 |
| MCGICP-LO | 2945.5 | 7065.3 |
| I-LOAM | 5820.6 | 5442.6 |

Compared to the 120-frame window, drift drops for all non-catastrophic methods
(KISS ~225%→103%, InTEn ~185%→**59%**). **InTEn-LOAM is best on drift** for
MulRan full. I-LOAM / MCGICP-LO still diverge (LOAM/GICP feature pipelines
not tuned for MulRan HDL-32E).

Artifact: `mulran_full_bundle.json`

### NCLT 2013-01-10 600

| Method | Drift [m/100m] | ATE [m] |
|--------|---------------:|--------:|
| ICPSC-LO | 0.49 | 0.38 |
| MCC-LO | 0.47 | 0.39 |
| MCGICP-LO | 0.59 | 0.61 |
| I-LOAM | 1.03 | 1.24 |
| KISS-ICP | 2.76 | 7.25 |
| InTEn-LOAM (TVF/DOR/mapping) | 46.5 | 40.3 |

On NCLT, intensity scan-to-map methods (ICPSC/MCC/MCGICP) transfer well;
InTEn-LOAM diverges (~46% drift). Contrast with MulRan where I-LOAM/MCGICP
fail catastrophically — dataset-dependent transfer.

Artifact: `nclt_600_bundle.json`

### NCLT 2013-01-10 600 — LIO (FR-LIO vs RKO-LIO, 2026-06-09)

`--no-gt-seed`, `--fr-lio-fast-profile`:

| Method | Drift [m/100m] | ATE [m] |
|--------|---------------:|--------:|
| **FR-LIO** | **0.63** | **0.58** |
| RKO-LIO | 1.42 | 1.13 |
| KISS-ICP | 2.76 | 7.25 |

FR-LIO (simplified RC-Vox + sub-frame + ESKS port) beats RKO-LIO and KISS-ICP on
this NCLT window. Sub-frames stayed at 1 for most NCLT frames (low IMU variance).

Artifact: `nclt_600_fr_lio_vs_rko.json`

### NCLT 2013-01-10 600 — PG-LIO vs FR-LIO (2026-06-09)

`--no-gt-seed`, fast profiles:

| Method | Drift [m/100m] | ATE [m] |
|--------|---------------:|--------:|
| FR-LIO | 0.63 | 0.58 |
| PG-LIO | 32.99 | 23.67 |

Simplified PG-LIO NCC photometric fusion **diverges** on NCLT Velodyne (unorganized
points, intensity noise). Geometric-only path tracks in unit tests; photometric
terms need organized scans / better projection (paper uses Ouster OS0).

Artifact: `nclt_600_pg_lio_vs_fr_lio.json`

## Notes

- InTEn-LOAM runs with TVF + DOR + scan-to-map mapping enabled by default.
- I-LOAM uses mapping ON + dense profile (same as KITTI full eval).
- Cross-dataset numbers are **honest negatives** for methods tuned on KITTI;
  they measure transfer, not paper claims.
