# M-GCLO

From-paper reimplementation of **M-GCLO: Multiple Ground Constrained LiDAR
Odometry** (ISPRS Annals of Photogrammetry, Remote Sensing and Spatial
Information Sciences, X-1-2024, pp. 283-290, Yang et al. 2024).
著者による公開実装は無く、本実装は論文記述からの独自再構成。

## 動機

マルチビーム回転 LiDAR は鉛直分解能が低く、鉛直方向 (z, roll, pitch) の推定が
劣化しやすい。単一の平坦地面を仮定する手法は傾斜・段差のある現実の地面で破綻
する。M-GCLO は地面を **複数の平面パッチ** として扱い、それらを姿勢拘束に使う
ことで鉛直精度を底上げする。

## アプローチ

3 要素で構成:

1. **地面 / 非地面分類** — 対応する局所平面の法線鉛直性 `|n_z| ≥ ground_normal_threshold`
   と高さ (対応点平均 z がセンサ高 + offset より下) で各点を分類。

2. **複数地面平面 point-to-plane 拘束** — 地面点はボクセル化され、各ボクセルで
   局所平面 (法線 ≈ 鉛直) が抽出される。残差 `n·(p_w − μ)` を `ground_weight`
   (鉛直精度重視のため大きめ) で重み付けして最適化に加える。

3. **非地面 point-to-distribution (NDT)** — 非地面点は NDT ボクセルマップへの
   Mahalanobis 残差 `(p_w − μ)ᵀ Σ⁻¹ (p_w − μ)` でマッチング。`Σ` は最小固有値を
   λmax の比で下限化して正則化。

さらに **各点の距離依存不確かさ重み** `w_unc = ref²/(ref² + range²)` を全残差に
掛け、遠方点 (低信頼) の影響を抑える。地面・非地面を 1 つの Gauss-Newton 系に
統合する。IMU 事前積分は KITTI には無いため等速予測 prior で代替。

## パラメータ

- `ground_normal_threshold` (既定 0.85): 地面とみなす法線鉛直成分 |n_z| 下限。
- `ground_height_offset` (既定 -0.5): 地面候補の高さ条件 (センサ高基準)。
- `ground_weight` (既定 2.0) / `nonground_weight` (既定 1.0): 地面/非地面の相対重み。
- `--m-gclo-no-ground` (dogfooding CLI): 地面候補を統計だけ数え、最適化から外す
  ground-factor off ablation。
- `ground_planarity` (既定 0.3): 地面分類に要求する平面性下限。
- `distribution_regularization` (既定 0.01): NDT 共分散の最小固有値下限比。
- `uncertainty_range_ref` (既定 40 m): 距離依存不確かさ重みの基準距離。
- `robust_scale` (既定 1.0): Cauchy 核スケール。
- `prior_precision` (既定 0.0): 等速予測への prior。

## テスト

`test_m_gclo` の 3 ケース:

1. `RecoversKnownTranslation` — 良条件シーンで既知並進を回復。
2. `UsesGroundAndNongroundConstraints` — 地面平面拘束と非地面 NDT が両方使われる。
3. `GroundConstraintImprovesVertical` — 地面拘束で鉛直 (z) 並進を改善。

## KITTI full での所見

KITTI full (no GT seed, dense profile) では、地面/非地面分類は毎フレーム動作し、
地面 point-to-plane 拘束と非地面 NDT 拘束が両方使われる (`num_ground` /
`num_nonground` で確認可能)。地面拘束は鉛直精度を狙うが、KITTI の都市部は壁・
建物など豊富な鉛直構造があり水平拘束が既に十分なため、地面拘束の寄与は z-drift
抑制に限定される。リーダーボードへは honest に odometry RPE を反映する。

Ground-factor off ablation (`--m-gclo-no-ground`) を KITTI seq00/07 full で追加した。
translational RPE は ground off が seq00 で 0.833% (ground on 0.835%)、seq07 で
0.600% (ground on 0.671%) と同等または低い。一方で ATE は seq00 19.1m→46.9m、
seq07 2.0m→5.3m に悪化し、rotational RPE も両 sequence で悪化する。したがって
この KITTI protocol では、複数地面拘束は translational drift reducer というより
global/attitude stability の anchoring term として働く。raw paired artifact:
[`m_gclo_ground_factor_ablation.json`](../../docs/benchmarks/kitti_full_new_methods/m_gclo_ground_factor_ablation.json).

## Synthetic Non-Flat Ground Stress

Dataset-free mechanism stress:

```bash
python3 evaluation/scripts/run_m_gclo_nonflat_stress.py
```

The runner generates a rolling-ground fixture under
`evaluation/fixtures/nonflat_ground_stress` and writes
`docs/benchmarks/nonflat_ground_stress/m_gclo_nonflat_ground_stress_summary.json`.
On the 60-frame stress, ground constraints track at **0.116 m** ATE and
**0.500%** drift with about **1278 ground correspondences/frame**. Disabling the
ground factor worsens to **0.150 m** ATE and **0.675%** drift, with rotational
RPE rising from **0.0028** to **0.0206 deg/m**. This confirms the multiple-ground
path is active and useful on non-flat synthetic terrain.

## Public KITTI seq08 validation

Hilly public KITTI Odometry seq08 full (4071 frames, no GT seed):

```bash
python3 evaluation/scripts/run_m_gclo_kitti_seq08_validation.py
```

| Variant | RPE | ATE | Artifact |
|---|---:|---:|---|
| ground on | 1.354% | 21.1 m | [`m_gclo_ground_on.json`](../../docs/benchmarks/kitti_seq08_public/m_gclo_ground_on.json) |
| ground off | 1.353% | 52.6 m | [`m_gclo_ground_off.json`](../../docs/benchmarks/kitti_seq08_public/m_gclo_ground_off.json) |

Paired summary: [`m_gclo_kitti_seq08_validation_summary.json`](../../docs/benchmarks/kitti_seq08_public/m_gclo_kitti_seq08_validation_summary.json).
Disabling the ground factor leaves translational RPE unchanged but worsens
whole-run ATE by about 149% on this hilly public sequence — consistent with the
seq00/07 ground-factor ablation. Dedicated off-road / multi-beam benchmarks such
as MulRan remain open before T0 promotion.
