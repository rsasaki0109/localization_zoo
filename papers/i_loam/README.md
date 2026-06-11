# I-LOAM: Intensity Enhanced LiDAR Odometry and Mapping (from-paper reimplementation)

**Paper:** Yeong-Sang Park, Hyesu Jang, Ayoung Kim, *"I-LOAM: Intensity
Enhanced LiDAR Odometry and Mapping"*, 17th International Conference on
Ubiquitous Robots (UR), 2020. No author code released (verified: not on the
authors' RPM Lab repositories; the GitHub "I-LOAM" results are unrelated LOAM
forks). Reconstructed from the paper.

## Core idea

I-LOAM keeps LOAM's geometric edge/plane feature pipeline but injects the LiDAR
**reflectance (intensity)** channel into the scan-to-scan association in two
places:

1. **Intensity-enhanced correspondence search.** When searching the previous
   frame for the second/third point that defines the line (edge) or plane, the
   candidate cost is augmented with a reflectance term

   ```
   cost(j) = ‖p_sel − q_j‖²  +  λ · (I_curr − I_j)²
   ```

   so that among geometrically comparable candidates the one with matching
   reflectance is preferred — this disambiguates repeated geometry (corridors,
   parallel façades).

2. **Intensity-similarity residual weighting.** Each accepted correspondence's
   residual is scaled by

   ```
   w = exp(−(I_curr − I_closest)² / 2σ_I²)        w ∈ (0, 1]
   ```

   down-weighting matches whose reflectance disagrees (likely wrong
   correspondences) in the Ceres least-squares problem.

The geometric feature extraction is reused from the shared `aloam`
`ScanRegistration`. Because that pipeline repurposes the point `intensity` field
to encode the scan-line id, the **true reflectance is recovered per feature
point by a nearest-neighbour lookup into the raw input cloud** (which still
carries `PointXYZI` reflectance).

**Mapping:** after intensity-enhanced scan-to-scan odometry, `aloam::LaserMapping`
refines the pose scan-to-map (same three-stage LOAM pipeline as A-LOAM/F-LOAM).
Disable with `--i-loam-no-mapping` for scan-to-scan-only ablation.

## Scope / approximations for KITTI

- KITTI's intensity is **uncalibrated and coarse** (8-bit, range/incidence
  dependent). I-LOAM uses intensity only as a *correspondence cost term and a
  residual weight* (never as the primary signal), so it degrades gracefully
  rather than catastrophically when reflectance is noisy.
- The reflectance lookup uses the nearest raw point (no incidence/range
  normalisation), documented as a simplification.
- Mapping uses standard geometric LOAM scan-to-map factors (no intensity in the
  map stage in this port).

## Parameters

| Param | Meaning | Default |
|---|---|---|
| `use_intensity_correspondence` | intensity term in association search | true |
| `use_intensity_weight` | intensity residual weighting | true |
| `intensity_sigma` | σ_I in the residual weight | 0.15 |
| `intensity_corr_weight` | λ for the correspondence cost [m²] | 1.0 |
| `enable_mapping` | scan-to-map refinement | true |
| `mapping_update_interval` | mapping every N frames | 1 |
| `distance_sq_threshold` | max correspondence distance² | 25 m² |
| `num_optimization_iters` / `ceres_max_iterations` | outer / inner iters | 2 / 4 |

Ablation flags:
- `--i-loam-no-intensity` — geometric LOAM baseline (intensity paths off)
- `--i-loam-no-mapping` — scan-to-scan only (no map refinement)
- `--i-loam-dense-profile` — tighter feature/map resolution for KITTI full eval

## Result (KITTI Odometry, `--no-gt-seed`, `--i-loam-dense-profile`)

### Odometry + mapping (default)

| Sequence | RPE drift | ATE |
|---|---:|---:|
| Seq 00 _(4541 fr)_ | **0.899%** | 12.7 m |
| Seq 07 _(1101 fr)_ | **0.575%** | 1.7 m |

Competitive with A-LOAM (~0.61% seq07) while retaining the paper's intensity
enhancement in the odometry stage. ~3 FPS on seq07 (mapping-heavy).

### Intensity ablation (scan-to-scan only, `--i-loam-no-mapping`)

| Sequence | Geometric baseline (intensity off) | I-LOAM (intensity on) | Δ drift |
|---|---:|---:|---:|
| Seq 00 | 3.186% <sub>(76.0 m)</sub> | **2.606%** <sub>(49.4 m)</sub> | **−18.2%** |
| Seq 07 | 3.806% <sub>(18.5 m)</sub> | **3.055%** <sub>(15.1 m)</sub> | **−19.7%** |

Reflectance cuts drift ~18–20% vs the identical geometric pipeline even without
mapping — the paper's central claim **reproduces on KITTI**.

Committed raw artifacts:

- [`seq00_i_loam_no_mapping.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json)
  and [`seq00_i_loam_no_intensity.json`](../../docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json)
- [`seq07_i_loam_no_mapping.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json)
  and [`seq07_i_loam_no_intensity.json`](../../docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json)
- Paired summary:
  [`i_loam_intensity_ablation.json`](../../docs/benchmarks/kitti_full_new_methods/i_loam_intensity_ablation.json)

## Reproduce

```sh
# default: intensity + mapping, seq07
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods i_loam --no-gt-seed --i-loam-dense-profile

# scan-to-scan intensity ablation, seq07
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods i_loam --no-gt-seed --i-loam-no-mapping \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_mapping.json
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods i_loam --no-gt-seed --i-loam-no-mapping --i-loam-no-intensity \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq07_i_loam_no_intensity.json

# scan-to-scan intensity ablation, seq00
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_00_full \
  experiments/reference_data/kitti_seq_00_full_gt.csv \
  --methods i_loam --no-gt-seed --i-loam-no-mapping \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_mapping.json
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_00_full \
  experiments/reference_data/kitti_seq_00_full_gt.csv \
  --methods i_loam --no-gt-seed --i-loam-no-mapping --i-loam-no-intensity \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_i_loam_no_intensity.json
```

## Tests

`test_i_loam`: intensity weight / augmented distance helpers; short-sequence
tracking; intensity on/off variants; mapping mode with map updates; `clear()`
reset.
