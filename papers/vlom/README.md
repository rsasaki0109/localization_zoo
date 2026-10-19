# Scale-correction VLOM

**Visual-LiDAR Odometry and Mapping with Monocular Scale Correction and Visual
Bootstrapping** (arXiv:2304.08978, 2023). From-paper reimplementation; no author
code.

## Core idea

Loosely coupled fusion of monocular visual odometry (ORB-SLAM2-style, simplified)
and A-LOAM LiDAR odometry + mapping:

1. **Monocular scale correction** — triangulated keypoint depth vs LiDAR depth
   ratio with MAD outlier rejection; periodically rescales visual translation.
2. **Visual bootstrapping** — visual frame-to-frame motion initializes A-LOAM
   scan-to-scan optimization (`aloam::LaserOdometry::setMotionPrior`).

## KITTI Odometry approximations

Same as PL-LOAM on KITTI Odometry: no RGB camera stream is present, so the
default evaluator renders LiDAR intensity into a pseudo-image for point/line
feature detection and keeps the range image as the LiDAR depth prior. Visual
bootstrap is disabled by default on LiDAR-only pseudo-images because it injects
a bad motion prior into A-LOAM; use `--vlom-enable-bootstrap` to reproduce the
old paper-style bootstrap path. The old depth-gradient pseudo-image path is
still available with `--vlom-depth-pseudo-image`.

**KITTI Raw RGB** (2026-06-09): pass `--vlom-rgb-root --vlom-enable-bootstrap`
to use real `image_02` PNG features with visual bootstrapping (see PL-LOAM
README reproduce block; swap `--methods vlom`).

## Tests

`test_vlom` — scale corrector outlier rejection, triangulation, cam/velo transform,
state reset.

## Reproduce

```sh
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_07_full \
  experiments/reference_data/kitti_seq_07_full_gt.csv \
  --methods vlom
```

## Result (KITTI Odometry, default profile, intensity pseudo-image)

| Seq | Drift | ATE | FPS | mean scale | bootstrap frames |
|-----|-------|-----|-----|------------|------------------|
| seq00 | **0.91%** | 9.52 m | 2.8 | 1.000 | 0/4541 |
| seq07 | **0.61%** | 2.51 m | 3.2 | 1.000 | 0/1101 |

**Caveat**: this KITTI Odometry row is not using visual bootstrap, because the
available "visual" signal is only a LiDAR-intensity pseudo-image. The previous
bootstrap-on run diverged badly (seq00 **89.2%**, seq07 **81.6%** drift). With
bootstrap off, the A-LOAM mapping core and scale correction remain stable, but
the monocular front-end is still not a substitute for the paper's full
ORB-SLAM2-style VO stack.

## Result (KITTI Raw RGB, `--vlom-dense-profile`, 200-frame windows)

| Drive | Drift | ATE | mean scale | bootstrap |
|-------|------:|----:|-----------:|----------:|
| raw_0009 | **99.7%** | 122 m | 1.000 | 199/200 |
| raw_0061 | **99.3%** | 81 m | 1.000 | 199/200 |

Real RGB does not rescue drift (~99%, same as pseudo-image path). Scale correction
remains stable; visual bootstrap fires on almost every frame but the simplified
monocular front-end still diverges. Paper seq00 1.18% assumes full ORB-SLAM2-style
VO on KITTI Raw.


<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->
## KITTI 00 current-replay error growth

The committed current replay has 13/4514 moving steps with length error above 20%, and 42/4514 with local translation-vector error above 20%. The existing diagnostic uses the same 100-frame blocks and GT step threshold of 0.1 m for every method.

The largest increase in pointwise position error among those blocks is poses 600–700: GT travels 98.739614 m, while error rises from 7.208486 m to 11.810234 m (increase 4.601748 m). These frame blocks diagnose the current trajectory; they are **not official KITTI RTE**. The tool uses first-pose normalization without fitting or rescaling.

The referenced historical per-run summary is absent at the inspected revision and was not found in this checkout's reachable history. Without its historical saved path, this does not locate the cause of the historical metric difference. [Plot, full trace, commands and limits](../../experiments/results/shared_near_range/diagnostics/00/README.md) use the committed current poses, with diagnostic evidence committed at `61c1f26e1019aa3f0b05b100404c8bb117448b41`. Native source remains `0b079dbbcb7c6f58c1d8367ade881c15f003e525`.
<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_07_DIAGNOSTICS -->
## KITTI07 historical replay and saved-path diagnosis

The full historical `default` cell completes **1,101**
frames with native exit **0**. Stored/replayed 100 m RPE changes from
**0.607458%** to **0.605313%**;
ATE changes from **2.544355 m** to **2.505243 m**.
RPE improves; ATE improves. Both metrics are retained. The historical audit
still exits **1**. Frozen pre-extraction controls reproduce the metrics with
zero recorded differences and byte-identical saved poses; comparison exits **0**.
The difference predates extraction; its original cause remains unresolved.

With inherited **100**-step blocks, the largest current position-error increase
is poses **800-900**: **1.728689 to 4.569446 m**.
This uses first-pose normalization, zero-based indices, no fitted alignment or
rescaling. Every block/frame is retained. The exact referenced historical path
is unavailable in the inspected tree/history, so this diagnoses current error
growth without locating a historical difference or establishing its cause.

[All numerical pairs, commands, complete traces and limits](../../experiments/results/shared_near_range/diagnostics/07/README.md)
use original native evidence `ed37e2fd` (lossless import `9c9976c6`), analysis
source/protocol `59e69241` and raw results `6db8522a`, committed before this prose.
Native source remains `0b079dbb`. Historical per-cell arguments are unchanged;
this is not a new fixed-setting accuracy study. Existing results, paper tables
and defaults are preserved.
<!-- /SHARED_NEAR_RANGE_07_DIAGNOSTICS -->

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 2 selected stored VLOM full-sequence cells complete with native exits 0. 0 match historical 100 m RPE, ATE and frame count; 2 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

Every historical difference is retained below, including improvements. The corresponding historical audits exit **1**.

| Seq | Historical variant | Stored 100 m RPE [%] | Replayed RPE [%] | Stored ATE [m] | Replayed ATE [m] |
|---|---|---:|---:|---:|---:|
| 00 | `default` | 0.875872 | 0.914174 | 9.391448 | 9.520570 |
| 07 | `default` | 0.607458 | 0.605313 | 2.544355 | 2.505243 |

For every differing cell, the frozen pre-extraction executable reproduces the replayed metrics and byte-identical saved poses. Thus these differences predate this extraction; their historical causes remain unresolved. Official translation/rotation RTE also matches for those frozen controls. This is not credited as a filter improvement or used to replace stored results.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
