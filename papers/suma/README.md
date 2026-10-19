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

The shared initial coarse/fine stage policy is separately available through
`--suma-paper-set coarse_frames=1 --suma-paper-set coarse_distance_scale=5
--suma-paper-set coarse_iterations=10`. It is **not in the SuMa paper** and is
disabled by default (`coarse_frames=0`). On the requested initial registrations,
SuMa runs projective ICP with the distance gate multiplied by the scale, then
starts ordinary ICP from that pose. Its residuals, normal gate and stopping
rules remain SuMa's; the official fallback still starts from the original guess.
Only stage ordering and registration counting are shared with IMLS-SLAM.

The first-registration budget and iteration count come from the existing
IMLS-SLAM 04 choice; the scale comes from its coarse/fine radius ratio.
This is a transfer of the startup pattern, not of the IMLS residual/kernel.
[Fixed protocol and execution](../../experiments/results/shared_coarse_start/README.md)
record full KITTI 00–10 results and compatibility checks. No SuMa sweep
or new held-out claim is made, since historical full-sequence results are known.
The following remap experiment is separate; remapping is off in the coarse study.

<!-- SHARED_COARSE_RESULTS:START -->
**Completed fixed-setting experiment, full KITTI 00–10.** All candidates use
`--no-gt-seed --suma-paper --suma-paper-set coarse_frames=1
--suma-paper-set coarse_distance_scale=5 --suma-paper-set coarse_iterations=10`
unchanged, with translation remapping off. All native notes report one coarse
registration; the iteration budget is a cap and numerical stopping can occur earlier.

| Seq | Raw RTE [%] | Coarse RTE [%] | Raw ATE [m] | Coarse ATE [m] |
|---|---:|---:|---:|---:|
| 00 | 0.697236 | 0.676793 | 8.196443 | 7.918658 |
| 01 | 13.792690 | 10.686955 | 175.826760 | 150.500523 |
| 02 | 3.511588 | 2.562817 | 70.307811 | 57.616595 |
| 03 | 0.651090 | 0.646312 | 1.448501 | 1.476722 |
| 04 | 0.416845 | 0.389970 | 0.974258 | 0.929463 |
| 05 | 0.379344 | 0.380265 | 3.420344 | 3.242128 |
| 06 | 0.285691 | 0.280482 | 2.961054 | 2.970025 |
| 07 | 0.331226 | 0.330252 | 1.059569 | 1.098068 |
| 08 | 1.060143 | 1.043560 | 24.049840 | 23.007073 |
| 09 | 0.510286 | 0.521616 | 6.469266 | 6.326409 |
| 10 | 0.697880 | 0.714919 | 5.252594 | 5.982472 |

[Full committed report and input snapshot](../../experiments/results/shared_coarse_start/report/summary.json),
[exact report command and checked exit](../../experiments/results/shared_coarse_start/report/command.json),
[raw commands, trajectories and cleanup](../../experiments/results/shared_coarse_start/sequences/).
The control is freshly measured raw SuMa with one fixed setting across the cohort.
RTE is higher on 05, 09, 10; ATE is higher on 03, 06, 07, 10.
The full table retains these tradeoffs without reselection.
No setting is reselected, no paper ratio or speed claim is made, and defaults stay off.

| Seq | Coarse path / GT | Raw bad length steps | Coarse bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |
|---|---:|---:|---:|---|---:|---:|
| 00 | 0.996986 | 13/4514 | 13/4514 | 4200-4300 | 131.234 | 7.492 |
| 01 | 1.102829 | 134/1100 | 113/1100 | 800-900 | 193.123 | 160.248 |
| 02 | 1.024914 | 168/4660 | 144/4660 | 2300-2400 | 127.089 | 22.928 |
| 03 | 1.002720 | 1/785 | 2/785 | 600-700 | 83.563 | 2.070 |
| 04 | 1.001324 | 0/270 | 0/270 | 200-270 | 112.094 | 1.479 |
| 05 | 0.998717 | 0/2683 | 0/2683 | 2600-2700 | 102.422 | 3.497 |
| 06 | 1.001468 | 0/1100 | 0/1100 | 900-1000 | 127.875 | 3.467 |
| 07 | 0.997835 | 0/1009 | 0/1009 | 500-600 | 93.673 | 0.507 |
| 08 | 0.997403 | 23/4040 | 18/4040 | 0-100 | 50.648 | 17.318 |
| 09 | 0.999478 | 0/1590 | 0/1590 | 1300-1400 | 144.465 | 4.944 |
| 10 | 0.997566 | 0/1153 | 1/1153 | 700-800 | 127.161 | 2.691 |

Bad length steps use GT motion at least 0.1 m and length error above 20%.
Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.
Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.
These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.

| Seq with bad length steps | Longest coarse bad-length intervals (destination frames) |
|---|---|
| 00 | 1967–1970, 1927–1928, 2966–2967 |
| 01 | 823–896, 490–517, 480–488 |
| 02 | 4483–4518, 1920–1953, 1869–1888 |
| 03 | 756–757 |
| 08 | 30–44, 27–28, 46 |
| 10 | 65 |

Sequence 01 has ATE 150.500523 m and 113/1100 bad length steps. Its largest
position-error growth block is 800–900: GT travel 193.123 m, position-error
increase 160.248 m.

[01 trace](../../experiments/results/shared_coarse_start/plots/01/trace.png), [numeric trace](../../experiments/results/shared_coarse_start/plots/01/trace.csv)
and [provenance](../../experiments/results/shared_coarse_start/plots/01/provenance.json) show successive displacement
and pointwise position error from committed poses without fitting or rescaling.
These curves are distinct from the official segment-based RTE.

Intervals above select up to three longest contiguous groups for every sequence
with bad length steps, including sequences whose RTE improves.
Complete ranges and the largest positive pointwise error-growth blocks
remain in the report. RTE can worsen without a moving step crossing this local
threshold. Saved-pose diagnostics locate where errors grow; they do not establish
a solver-level cause. Historical SuMa results were already known, so this is not
a new held-out evaluation.

All 22 existing selected IMLS-SLAM/SuMa full-sequence audit cells match RPE, ATE,
frame count and official translation/rotation RTE, with zero maximum deltas.
The frozen pre-change full 04 comparison also has identical saved-pose bytes
for both methods.
The audit covers the startup helper's two native consumers, not the whole zoo.
The raw controls also match the prior committed raw SuMa controls on every sequence,
and official scan bytes/GT identity match that study. The separate translation-remap
experiment and published results remain unchanged.

<!-- SHARED_COARSE_RESULTS:END -->

The shared local translation remap is separately available through
`--suma-paper-set degenerate_ratio=0.03` (C++ `SuMaPaperParams::degenerate_ratio`).
It is **not in the SuMa paper**, is disabled by default, and projects only the
local solver's translation update. Rotation and SE(3) composition can still
change the accumulated translation. SuMa's translation-first vector ordering is
handled explicitly; DiLO uses rotation first. The existing official defaults and
published results retain their original interpretation.

The experimental ratio is transferred from the existing DiLO choice on KITTI
01 without a SuMa sweep. Sequence 01 motivates the experiment and is not held
out. [Full fixed-setting execution and audit protocol](../../experiments/results/shared_degeneracy_remap/README.md)
are recorded separately; a new paper reproduction claim is not made here.
Historical SuMa results, including the 02 diagnostics, were already known;
no new held-out claim is made.

<!-- SHARED_REMAP_RESULTS:START -->
**Completed fixed-setting experiment, full KITTI 00–10.** All sequences use
`--no-gt-seed --suma-paper --suma-paper-set degenerate_ratio=0.03` unchanged.
The paired results reject this transfer as a general SuMa KITTI setting;
no parameter is reselected.

| Seq | Raw RTE [%] | Remap RTE [%] | Raw ATE [m] | Remap ATE [m] |
|---|---:|---:|---:|---:|
| 00 | 0.697236 | 0.686458 | 8.196443 | 7.945686 |
| 01 | 13.792690 | 95.257580 | 175.826760 | 1398.495188 |
| 02 | 3.511588 | 21.423956 | 70.307811 | 565.520007 |
| 03 | 0.651090 | 7.124333 | 1.448501 | 11.877929 |
| 04 | 0.416845 | 99.229120 | 0.974258 | 218.135538 |
| 05 | 0.379344 | 3.671756 | 3.420344 | 52.905898 |
| 06 | 0.285691 | 16.302826 | 2.961054 | 263.406501 |
| 07 | 0.331226 | 0.331961 | 1.059569 | 1.196831 |
| 08 | 1.060143 | 16.334855 | 24.049840 | 289.339278 |
| 09 | 0.510286 | 20.387333 | 6.469266 | 390.892138 |
| 10 | 0.697880 | 12.100277 | 5.252594 | 219.326465 |

[Committed full report and input snapshot](../../experiments/results/shared_degeneracy_remap/report/summary.json),
[report command and checked exit](../../experiments/results/shared_degeneracy_remap/report/command.json),
[per-sequence raw runs and cleanup](../../experiments/results/shared_degeneracy_remap/sequences/).
This is a non-paper transfer experiment; no cohort paper ratio is claimed.
The matrix runner assigns `Adopt as current default` within each single-variant
manifest. That label ranks the sole experimental variant; the paired raw-control
results determine the transfer recommendation. Native defaults stay off.

| Seq | Remap path / GT | Raw bad length steps | Remap bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |
|---|---:|---:|---:|---|---:|---:|
| 00 | 0.996997 | 13/4514 | 13/4514 | 4200-4300 | 131.234 | 7.225 |
| 01 | 0.031934 | 134/1100 | 1100/1100 | 600-700 | 269.598 | 268.474 |
| 02 | 0.741858 | 168/4660 | 2040/4660 | 700-800 | 127.784 | 108.485 |
| 03 | 1.017108 | 1/785 | 218/785 | 500-600 | 76.797 | 14.231 |
| 04 | 0.025261 | 0/270 | 270/270 | 100-200 | 144.369 | 142.520 |
| 05 | 0.991562 | 0/2683 | 201/2683 | 100-200 | 60.598 | 41.442 |
| 06 | 0.756785 | 0/1100 | 391/1100 | 0-100 | 119.098 | 118.865 |
| 07 | 0.997956 | 0/1009 | 1/1009 | 500-600 | 93.673 | 0.592 |
| 08 | 1.046515 | 23/4040 | 832/4040 | 3900-4000 | 80.379 | 96.026 |
| 09 | 0.911843 | 0/1590 | 678/1590 | 800-900 | 118.555 | 96.520 |
| 10 | 0.881457 | 0/1153 | 147/1153 | 700-800 | 127.161 | 73.188 |

Bad length steps use GT motion at least 0.1 m and length error above 20%.
Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.
Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.
These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.

The transfer fails from startup in 01, 02, 04 and 06. In 01 all 1100 moving
steps are affected (destination frames 1–1100). In 02 the first continuous
interval is 1–912, with 2040 of 4660 moving steps affected overall. In 04 all
270 moving steps are affected (1–270); path length is 0.025261 of GT. In 06
the first interval is 1–290, followed by 297–323 and 346–419.

Sequence 03 fails later, including 355–385, 556–624 and 718–784. In 05,
length errors affect 101–160 and 2136–2268; over frames 100–200 the saved-frame
position error grows from 0.768797 m to 42.210786 m. In 08 the intervals include
864–1034, 1916–1983 and 3365–3475. Full 07 has only a small RTE change, with
one affected moving step (destination frame 320).

Sequence 09 has 678 of 1590 moving steps above the length-error threshold;
destination-frame intervals: 52–190, 207–637, 640, 1484–1590. Sequence 10
has 147 of 1153 moving steps above the threshold (1–147), a startup failure.
Complete intervals and the largest positive pointwise error-growth blocks are
retained in the report. These saved poses locate failures without establishing
a solver-level cause. Existing settings and published results stay unchanged.

[01 trace](../../experiments/results/shared_degeneracy_remap/plots/01/trace.png) and [02 trace](../../experiments/results/shared_degeneracy_remap/plots/02/trace.png)
show successive displacement and pointwise position error in the saved frame,
without fitting or rescaling either path.

The affected-method audit covers full 00–10 for DiLO and SuMa: all 22 existing
selected cells have zero changes in RPE, ATE, frame count and official
translation/rotation RTE. The frozen pre-change full 01 comparison additionally
has identical saved-pose bytes for both methods. This audits the two native
consumers of the new helper; it does not claim a new whole-zoo audit.

<!-- SHARED_REMAP_RESULTS:END -->

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

<!-- SHARED_NEAR_RANGE_REPLAY -->
## Shared-loader full KITTI replay

All 11 selected stored SuMa full-sequence cells complete with native exits 0. 11 match historical 100 m RPE, ATE and frame count; 0 differ. Each cell uses its original recorded arguments, which may differ by sequence. This audit does not select a new fixed setting or paper value. The optional near-range cut still defaults to zero/off.

[Complete report and commands](../../experiments/results/shared_near_range/completion/03/report/summary.json) retain all original audit exits, summaries, logs, saved poses, frozen comparisons and verified input cleanup. KITTI replay does not establish fresh NCLT accuracy.
<!-- /SHARED_NEAR_RANGE_REPLAY -->
