# CT-ICP: 現状実装と論文との gap analysis (2026-05-18)

> 目的: ms_chol (multi-scale + DENSE_NORMAL_CHOLESKY) で頭打ちになった paper RPE gap (geom-mean 4.45x) を更に縮めるための、次の架構改善候補を整理する。
>
> 重要訂正: **pyramid voxel resolution は CT-ICP 論文 (Dellenbach 2022) の default config ではない**。これ以前の memo で言及していたが、jedeschaud/ct_icp の公式 YAML を確認したところ単一 resolution。よって pyramid 追求は中止。

## 1. 現状コードのサマリ

`papers/ct_icp/src/ct_icp_registration.cpp`, `cost_functions.h`, `ct_icp_registration.h` を読んだ結果:

| 項目 | 現状値 | Paper 値 |
|---|---|---|
| Outer ICP iterations | 30 (default) / 8 (KITTI eval) | 5 |
| Ceres inner iterations | 5 / 6 (KITTI) | 5 |
| Linear solver | DENSE_NORMAL_CHOLESKY (ms_chol 適用後) | DENSE_NORMAL_CHOLESKY |
| Cauchy σ | 0.5 | **0.1** |
| Planarity threshold | hard cutoff a2D < 0.3 で reject | **soft weight a2D^2、低 floor のみ** |
| Correspondence weight | raw a2D | **α·a2D^p + β·exp(-d/dmax/kmin)** (composite) |
| Reference point | **mean of knn=5** | **closest neighbor** (single) |
| Plane fit | PCA over knn=5 | PCA over 20 points |
| Regularizer weight | `√(N_corr · β)` (22-30x amplified) | flat `β=0.001` |
| Map kMaxPoints/voxel | 20 | 30 |
| `min_distance_between_points` on insert | **なし** | 0.1 m |
| Keypoint subsample | 0.5 m | 1.5 m |
| Sliding window | 30 frames | 30 frames |

## 2. 上位 6 ギャップ (RPE 影響順)

| # | Gap | 仮説 RPE 改善 | 実装規模 |
|---|---|---|---|
| **A** | knn=5 mean reference → closest-neighbor reference (+ PCA over 20) | -15〜-25% | S (~15 LOC) |
| **B** | weight = α·a2D^p + β·exp(-d/dmax/kmin) (paper composite, p=2) | -15〜-20% | S (~10-15 LOC) |
| **C** | Voxel insertion に min_distance_between_points=0.1 を入れる | -10〜-20% | M |
| **D** | Cauchy σ 0.5 → 0.1 | -10〜-15% | S (parameter) |
| **E** | Planarity hard cutoff a2D<0.3 → soft weight (low floor 0.01) | -5〜-10% | S |
| **F** | Regularizer weight から √N_corr 倍率を除去 (flat β=0.001) | -5〜-15%, **velocity gate dead-end の真因の可能性** | S |

### Gap F の意味

`cost_functions.h` 内で LocationConsistency / OrientationConsistency / ConstantVelocity の3つの regularizer は `sqrt(corrs.size() * β)` の重み付けをしている。N_corr ≈ 500-1000 では √N ≈ 22-30、つまり regularizer が **22-30x amplified**。これにより velocity prior が geometry residual を overrule し、velocity-model bootstrap の seed がそのまま最終解になる現象が起きる。

これは前回の「gate + CT-ICP は incompatible」結論を部分修正する: Gap F を直せば velocity gate が機能する可能性がある。今回は実装に含めるが、gate 再検証は別 PR で。

### 既に実装済み (除外項目)
- DENSE_NORMAL_CHOLESKY linear solver (ms_chol)
- Multi-scale findCorrespondences (3x3x3 → 5x5x5) (ms_chol)

## 3. 推奨実装プラン

### Pick 1: Paper weight scheme bundle (B+D+E+F)

**スコープ**: ~30-40 LOC、低リスク、複合効果見込み。`papers/ct_icp/` のみで完結 (driver 側変更最小)。

**変更箇所**:
- `papers/ct_icp/src/ct_icp_registration.cpp:147-186` (findCorrespondences の weight 算出)
- `papers/ct_icp/include/ct_icp/cost_functions.h:69-144` (regularizer weight の `sqrt(corrs.size() * β)` → `sqrt(β)`)
- `papers/ct_icp/include/ct_icp/ct_icp_registration.h` (新パラメータ追加: `weight_alpha=0.9`, `weight_neighborhood=0.1`, `power_planarity=2`, `min_planarity=0.01`, `regularizer_flat=true`, `cauchy_loss_param=0.1`)
- `evaluation/src/pcd_dogfooding.cpp` (CLI フラグ `--ct-icp-paper-weights` バンドル + 個別フラグ)

**検証**:
- KITTI 07 (1101 frames) で ablation (B alone, D alone, E alone, F alone, bundle): ~7 variants × 1 分 = ~10 分
- bundle が成功なら cross-seq (00, 02, 05, 08) で確認: ~30 分

**Risk と early-fail**:
- Cauchy σ=0.1 + cold start → 残差大で weight ほぼ 0 で degenerate。Mitigation: iter 0-1 は σ=0.5、iter 2+ で σ=0.1
- Planarity threshold 撤廃で Ceres に低 weight residual が大量流入 → 遅くなる可能性。Mitigation: min_planarity=0.01 で floor
- Early-fail: KITTI 07 bundle で ATE > 2.5m (現状 ms_chol 1.61m から 50% 以上悪化) なら abort

### Pick 2 (後続候補): Closest-neighbor reference + 20-neighbor PCA (Gap A)

Pick 1 後、まだ gap が残れば挑む。~15 LOC + knn=5→20 パラメータ変更。

**Risk**: knn=20 で PCA cost が 16x。Mitigation: 早期 stop で 30 collected で打ち切り。closest-point reference は map sparsity に sensitive なので、Gap C (min_distance_between_points insertion filter) を併用しないと distant region で noisy になる可能性。

## 4. 推奨ワークフロー

1. **今日**: Pick 1 (B+D+E+F bundle) を実装、KITTI 07 で ablation、効果確認
2. **bundle が ms_chol を超えたら**: cross-seq で確認、commit、production default 更新検討
3. **更に gap が残れば**: Pick 2 (closest-neighbor + 20-PCA) + Gap C (min-dist insertion) を統合 PR
4. **pyramid voxel は追求しない** (paper の default ではない)

## 5. Out of scope

- Loop closure module (paper の Part II) - 本リポジトリの benchmark scope は odometry only
- IMU integration / CT-LIO 系の拡張
- Map のスパースリプレゼンテーション (octree 等)

## 6. Follow-up: motion-gated regularizer cap (2026-06-12)

The fixed `--ct-icp-regularizer-n-cap` sweep showed the trade-off clearly:
seq07 RPE improves strongly with caps around 30, while seq02 regresses. To avoid
turning that into a sequence-specific oracle, `pcd_dogfooding` now has an
opt-in `--ct-icp-auto-regularizer-cap` mode that uses only previous estimated
poses. It lowers the regularizer cap on low-step, high-rotation-per-meter
segments after a warmup period.

Smoke evidence:

| Run | ATE [m] | Drift [%/100m] | Capped frames |
|---|---:|---:|---:|
| KITTI seq07 first 200, default | 0.610 | 0.525 | 0 |
| KITTI seq07 first 200, `--ct-icp-auto-regularizer-cap` | 0.599 | 0.431 | 51 |

Cross-sequence 200-frame smoke:

| Seq | Default ATE | Auto-cap ATE | Default drift | Auto-cap drift | Capped frames | Verdict |
|---|---:|---:|---:|---:|---:|---|
| 02 | 1.570 | 1.688 | 1.198 | 1.157 | 1 | RPE-only win; ATE regresses |
| 05 | 0.839 | 0.798 | 1.627 | 1.559 | 33 | ATE/RPE win |
| 07 | 0.610 | 0.599 | 0.525 | 0.431 | 51 | ATE/RPE win |
| 08 | 14.037 | 14.031 | 7.305 | 7.199 | 39 | small ATE/RPE win |

The full-sequence cap sweep remains the stronger evidence. The 200-frame checks
show the non-oracle gate consistently improves drift on these four sequences,
but seq02's ATE regression means this should remain opt-in/RPE-prioritized
rather than replacing the default profile.
