# SuMa

Paper-path interfaces: [Python API and examples](../../python/README.md#stateful-paper-pipelines),
[ROS 2 nodes](../../ros2/localization_zoo_ros/README.md#paper-odometry-nodes).
[Native fixture checks](../../experiments/results/paper_pipeline_bindings/README.md).

The initial ROS interface trial observed 0/3 odometry messages before its
per-scan deadline with best-effort input. The recorded reliable-input trial
completed 3/3 and matched the native pipeline; commands and both trials are
linked above. This short interface check retains the benchmark evidence below.

lightweight `SuMa` 再現版です。生点群を range image に射影して surfel を作り、
sliding window surfel map に対する point-to-plane で pose を更新します。

この実装で入れている要素:

- spherical projection による range image 生成
- 近傍画素差分からの法線推定と surfel 化
- surfel map への point-to-plane scan-to-map
- 直近フレームだけを保持する sliding window map
- 前回相対 motion を使う constant-velocity pose prediction

full `loop closure` や GPU 実装は含めず、`SuMa` の surfel odometry / mapping 部分を
この repo の最小構成に寄せています。

## Current dogfooding result

KITTI Odometry seq07 full, `--no-gt-seed`, exact frame-ID association:

| Variant | ATE [m] | RPE [%/100 m] | FPS | Notes |
|---|---:|---:|---:|---|
| Default | 2.738 | 2.352 | 94.6 | Fast reference after motion prediction |
| Dense profile | 3.971 | **0.944** | 44.6 | Best current seq07 RPE |

Before constant-velocity prediction, the same seq07 full run diverged to
52.943 m / 10.023% with the default profile. The dense profile is promoted only
for the seq07 full RPE cell; remaining KITTI full sequences still need reruns
before broader SuMa claims.

## 公式実装との比較 (未完了, 2026-10-11)

公式 SuMa (jbehley/SuMa) は GPU なしのコンテナでもビルドでき
(`-DENABLE_NVIDIA_EXT=Off`。glow と GTSAM 4.2.0 は CMake が取得する)、
genvideo.cpp を元にしたヘッドレスのポーズ出力ドライバも動いた。しかし実行には
OpenGL 4.x が要り、ソフトウェア描画ではどれも動かなかった:

- Mesa llvmpipe (LLVM 20.1.2): frame-to-model の段階でシェーダのコード生成が
  `LLVM ERROR: Cannot emit physreg copy instruction` で落ちる
  (`LP_NATIVE_VECTOR_WIDTH=128`、`GALLIVM_PERF=no_opt` でも同じ)。
- softpipe: GLSL 1.50 / 3.30 までで、SuMa のシェーダ (GLSL 4.00 以上) を読めない。
- zink + lavapipe: Xvfb 上で GL コンテキストを作れない。

公式コードの KITTI 値は GPU のある環境か、この LLVM の不具合がない Mesa で測る
必要がある。それまで Table 6 の 2.21x が実装側の差か公式コードと論文の差かは
切り分けられていない。

## Paper pipeline (`SuMaPaper`, `suma/suma_paper.h`)

`SuMa` above is not the paper's method: it associates with a KD-tree against a
sliding window of voxelised points, and on KITTI its default `n_scans = 16`
(never set by the harness) puts the HDL-64 into a 16-row image and drops every
point below -17 deg (17.6 % of a scan). `SuMaPaper` is a separate, opt-in class
that follows Behley & Stachniss (RSS 2018, Sec. III-A to III-D, Table I) and the
official `config/default.xml` on the CPU:

- 900 x 64 vertex map (nearest point, f_up 3 deg, f_down 25 deg, 2-75 m) and
  forward-difference normal map with horizontal wrap (eqs. 2-3).
- Model vertex / normal maps rendered at the last pose from the active surfel
  map (created or updated in the last 100 scans, stability above the
  threshold): each surfel is rasterised as a disk of its radius with a z-buffer.
- Frame-to-model ICP with projective association, bilinear model lookup as in
  the official shader, 2 m / 30 deg gates, Huber k = 0.5, Gauss-Newton on se(3)
  (<= 10 iterations), warm start with the last increment, and the official
  frame-to-frame fallback after a track loss (> 0.4 m / 0.1 rad change).
- Map update: radius eq. 10 (0.05-1 m), compatibility 0.2 m / 45 deg (official;
  Table I says 30 deg), stability log odds eq. 4 (p_stable 0.6, p_prior 0.5,
  sigma 1), refinement with gamma = 0.9 when the measurement is more precise
  (eqs. 11-13), new surfels for unexplained pixels, removal of unstable surfels
  (l_s < odds(0.4), or l_s < 0 and not updated for 3 scans).

No loop closure (Table II "Frame-to-Model" rows). About 0.2-0.3 s per scan,
single-threaded. Surfels farther than 90 m (x/y) and inactive surfels are
dropped (stand-in for the GPU submaps).

KITTI Odom, official RTE (100-800 m segments, here 100-200 m), scratch driver
over the raw `.bin` scans, no GT seed:

| Window | `SuMa --suma-dense-profile` | `SuMa`, `n_scans = 64` | `SuMaPaper` | paper (full seq.) |
|---|---:|---:|---:|---:|
| 07 frames 0-399 | 0.895 % / 0.80 deg | 0.616 % / 0.69 deg | **0.432 % / 0.26 deg** | 0.4 % / 0.3 deg |
| 07 frames 400-799 | 1.288 % / 1.06 deg | | **0.368 % / 0.28 deg** | |
| 00 frames 0-399 | 1.227 % / 1.12 deg | | **1.104 % / 0.80 deg** | 0.7 % / 0.3 deg |

(deg = deg / 100 m.) The KITTI vertical-angle correction (0.22 deg) makes both
classes worse on every window (`SuMaPaper` 07 f0-399: 1.473 %, f400-799:
1.349 %, 00 f0-399: 1.678 %). It moves almost only the pitch (about -3.5 deg per
250 m); the official GL_LINEAR model lookup, which blends empty texels into up
to 25 % of a sample, carries a pitch bias of the opposite sign, and the
official KITTI reader does not correct the scans. With a clean lookup
(`normalize_bilinear = true`) the correction does help (07 f0-399: 1.051 % ->
0.468 %), but stays behind the official combination. Use `SuMaPaper` without
the correction.

Tests: `test_suma_paper` (synthetic street canyon ray-cast with 64 beams:
vertex / normal maps, rendering, projective Gauss-Newton, map update, known
motion over 15 scans).

### Seq 02: forward drift in corridors and the recommended KITTI setting

With the official defaults on raw scans, full seq 02 reaches 3.51 % (paper
1.1 %): around frames 1850-1960 and 4430-4540 (13 m/s, a road between a wall /
hedge and vegetation) the estimated step grows from 1.3 m to 2-3.3 m. Measured
on windows that start 100 scans earlier (scratch driver, official RTE):

- The forward direction is nearly unconstrained there: the translation block
  of J'WJ has eigenvalues about 40-90 along x against 1500-3700 (y) and
  3600-7400 (z); only about 80 of 12000 associated pixels have |n_x| > 0.5.
- The frame-to-model objective itself is biased, not only the warm start:
  Gauss-Newton started at the ground-truth increment converges to the same
  too-long step (frame 1910: GT 1.38 m, from GT 2.09 m, estimate 2.58 m).
  The frame-to-frame fallback gives the right step (about 1.37 m), but the
  next frame-to-model step runs off again.
- At the ground-truth increment, 73 % of the forward gradient comes from
  ground pixels. Their surfel normals tilt towards the sensor by about
  1.3 deg on average (the KITTI elevation "bowl"), so the many ground pixels
  carry a small, biased forward term that wins when nothing else constrains x.
- The official bilinear lookup (empty texels blended in) adds its own bias.
  Either fix alone is not enough; both together are:

| Variant (seq 02 window) | f1750-1959 | f4330-4559 |
|---|---:|---:|
| official defaults, raw scans | 25.4 % | 14.0 % |
| + 0.22 deg vertical-angle correction | 11.4 % | |
| `normalize_bilinear = true`, raw scans | 7.6 % | 2.63 % |
| `normalize_bilinear = true` + 0.22 deg | **2.00 %** | **1.67 %** |
| `normalize_bilinear = true` + 0.165 deg | 2.20 % | 1.48 % |
| nearest lookup, raw scans | | 19.1 % |
| constant-velocity scan deskew (driver), raw scans | 25.5 % | 21.3 % |
| keep the warm start along weak translation directions (< 2 % of the strongest) | 28.1 % | |
| active window 10 scans instead of 100 | 57.9 % | 19.9 % |

Regression windows with `normalize_bilinear = true` + 0.22 deg: 07 f0-399
0.468 % (official raw 0.432 %), 07 f400-799 0.736 % (0.368 %), 00 f0-399
1.151 % (1.104 %). The defaults stay official; for KITTI runs where seq 02
matters, use `normalize_bilinear = true` with the 0.22 deg correction.

### Full sequences

Official KITTI RTE, full sequences, no GT seed (`--suma-paper`;
`experiments/suma_kitti_seq_*_full_*_matrix.json`):

| seq | dense profile | `paper_odometry` (official defaults, raw) | `paper_odometry_clean_lookup_elevation` | paper, Table II |
|---|---:|---:|---:|---:|
| 00 | 1.602 % | **0.697 %** | 0.884 % | 0.7 % |
| 01 | | 13.793 % | **3.550 %** | 1.7 % |
| 02 | 1.785 % | 3.512 % | **0.784 %** | 1.1 % |
| 03 | | **0.651 %** | 1.186 % | 0.7 % |
| 04 | | **0.417 %** | 0.802 % | 0.4 % |
| 05 | 1.279 % | **0.379 %** | | 0.5 % |
| 06 | | **0.286 %** | 0.530 % | 0.4 % |
| 07 | 1.095 % | **0.331 %** | 0.521 % | 0.4 % |
| 08 | 2.035 % | **1.060 %** | 1.179 % | 1.0 % |
| 09 | | **0.510 %** | 0.962 % | 0.5 % |
| 10 | | **0.698 %** | 0.897 % | 0.7 % |

Table 6 takes the better variant per sequence: geometric mean 0.97x over
00-10, with seq 01 (highway) at 2.09x the only sequence clearly behind. One
setting for every sequence does worse: the official defaults give 1.25x
(01 fails at 8.1x, 02 at 3.2x, every other sequence 0.71-1.06x), the
normalised lookup with the correction 1.41x (05 not run with it). The two
failures are the degenerate stretches described above (corridor, highway),
where the forward direction is weakly constrained. About 4-6 FPS, single
thread. The paper's numbers have one decimal.
