# V-LOAM2015: Visual-LiDAR Odometry and Mapping

**Paper:** Ji Zhang and Sanjiv Singh, *Visual-Lidar Odometry and Mapping:
Low-drift, Robust, and Fast*, ICRA 2015,
https://frc.ri.cmu.edu/~zhangji/publications/ICRA_2015.pdf. This port targets
the visual-LiDAR fusion behavior; no reusable author implementation for that
path was used.

## Core Idea

The paper combines a low-rate visual odometry estimate with high-rate LiDAR
scan matching. Visual odometry supplies the initial motion and LiDAR odometry
refines the registered point cloud, improving robustness when either modality is
temporarily weak.

This port keeps the parts that can be exercised in the existing KITTI PCD
harness:

1. **Range-image visual bootstrap.** High-curvature range-image cells act as a
   sparse visual-feature proxy when RGB frames are unavailable.
2. **LiDAR depth consistency.** Every visual proxy carries metric LiDAR depth,
   so the visual channel is scale-stable instead of monocular-only.
3. **LiDAR scan-to-map refinement.** The final pose still comes from the shared
   point-to-plane scan-to-map backend.
4. **Conservative visual weighting.** Visual point residuals and a small motion
   prior initialize/refine the LiDAR solve; line residuals are disabled for this
   older loosely coupled profile.

## Scope / Deviations

- The original system uses camera visual odometry and online mapping. KITTI
  Odometry PCD exports in this repo contain Velodyne scans, so visual features
  are range-image proxies rather than RGB features.
- Global mapping, loop closure, and the original camera tracker are out of
  scope. This is a pure odometry adapter over the shared `OPL-LVIO` backend.
- The dense profile keeps `visual_point_weight=0.035`, disables line/direction
  residuals, and uses `motion_prior_weight=3e-4`.

## Parameters

| Param | Meaning | Dense profile |
|---|---|---:|
| `image_width` / `image_height` | spherical range/visual proxy image | 1024 / 80 |
| `voxel_size` | map voxel size | 0.8 m |
| `registration_voxel_size` | source point thinning | 0.7 m |
| `max_visual_points` | visual bootstrap point cap | 420 |
| `visual_point_weight` | visual point residual weight | 0.035 |
| `line_weight` / `direction_weight` | line residual weights | 0.0 / 0.0 |
| `motion_prior_weight` | visual bootstrap prior weight | 3e-4 |

## Tests

`test_v_loam15` covers profile selection, range-image visual feature extraction,
short translation tracking, and map growth.

## Reproduce

```bash
./build/evaluation/pcd_dogfooding data/kitti_pcd/seq00_full \
  data/kitti_pcd/seq00_gt.csv --methods v_loam15 --no-gt-seed \
  --v-loam15-dense-profile \
  --summary-json docs/benchmarks/kitti_full_new_methods/seq00_v_loam15.json
```

## Result (KITTI Odometry, dense profile)

| Seq | RPE drift | ATE | FPS | visual points/frame | fallback |
|---|---:|---:|---:|---:|---:|
| 00 _(4541 fr)_ | **1.066%** | 14.37 m | 7.8 | 100.8 | 0 |
| 07 _(1101 fr)_ | **0.910%** | 3.63 m | 10.0 | 104.8 | 0 |

This is a stable visual-bootstrap adapter: it is far better than the older
pseudo-image PL-LOAM/VLOM paths, but still trails the strongest LiDAR-only
KISS-like front ends on KITTI.
