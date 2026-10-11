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
