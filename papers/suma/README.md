# SuMa

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
