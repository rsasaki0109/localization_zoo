# AD-VLO: Accurate Direct Visual-Laser Odometry

**Paper:** Kaihong Huang, Junhao Xiao, and Cyrill Stachniss, *Accurate Direct
Visual-Laser Odometry with Explicit Occlusion Handling and Plane Detection*,
ICRA 2019, https://www.ipb.uni-bonn.de/wp-content/papercite-data/pdf/huang2019icra.pdf.
No author implementation was used for this port.

## Core Idea

The paper aligns camera and laser data directly while handling occlusions and
detecting planar structure. The main idea is to avoid throwing away geometric
laser information when photometric alignment becomes ambiguous.

This port keeps the parts that fit the KITTI PCD harness:

1. **Occlusion-aware visual proxy.** Range-image discontinuities gate visual
   point selection so unstable depth edges are not over-trusted.
2. **Planar emphasis.** The LiDAR point-to-plane term is weighted slightly above
   the default to reflect the paper's explicit plane handling.
3. **Direct visual-laser coupling.** Sparse visual-depth proxy points add direct
   metric residuals beside the scan-to-map plane solve.
4. **LiDAR-first fallback.** Line residuals are disabled in the dense profile;
   the direct point/plane path is kept conservative.

## Scope / Deviations

- The original method uses RGB direct image alignment. The repo's KITTI Odometry
  PCD path has no RGB frames, so the visual side is represented by spherical
  range-image structure.
- Occlusion handling is approximated by range-jump gating, and plane detection
  by an elevated point-to-plane residual weight in the shared backend.
- The dense profile uses `plane_weight=1.15`,
  `visual_point_curvature_threshold=0.16`, `visual_range_jump=0.55`, and
  disables line/direction residuals.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical range/visual proxy image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `plane_weight` | point-to-plane residual weight | 1.15 |
| `visual_point_weight` | visual point residual weight | 0.025 |
| `visual_point_curvature_threshold` | visual proxy edge threshold | 0.16 |
| `visual_range_jump` | occlusion/range-jump gate | 0.55 |

## Tests

`test_ad_vlo` covers profile selection, occlusion-aware visual point extraction,
and short translation tracking with the plane-emphasis profile.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods ad_vlo --no-gt-seed \
  --ad-vlo-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_ad_vlo.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual points/frame | fallback |
|---|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.052%** | 12.95 m | 8.2 | 83.4 | 1 |
| 07 _(1101 fr)_ | **0.939%** | 3.08 m | 10.0 | 87.8 | 0 |

AD-VLO is the best seq00 RPE among the four new adapters and has the strongest
seq07 ATE in the group. The gain is still modest and mostly comes from the
conservative plane/visual-point balance, not from a full direct RGB alignment.
