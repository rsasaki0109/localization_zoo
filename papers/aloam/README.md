# A-LOAM: Advanced implementation of LOAM

## 論文情報

- **原著**: Ji Zhang, Sanjiv Singh, "LOAM: Lidar Odometry and Mapping in Real-time", RSS 2014
- **A-LOAM実装者**: Tong Qin, Shaozu Cao (HKUST)
- **元リポジトリ**: https://github.com/HKUST-Aerial-Robotics/A-LOAM
- **ライセンス**: BSD-3-Clause

## 手法概要

LiDAR点群から幾何特徴（エッジ・平面）を抽出し、2段階のマッチングで高精度オドメトリを実現。

### 3ステージパイプライン

1. **ScanRegistration**: 曲率ベースでエッジ点・平面点を抽出
2. **LaserOdometry**: scan-to-scan マッチング（高頻度・低精度）
3. **LaserMapping**: scan-to-map マッチング（低頻度・高精度）

### コスト関数

**エッジ残差** (点と直線の距離):
```
r_edge = ((Rp+t - a) × (Rp+t - b)) / ||a - b||
```

**平面残差** (点と平面の距離):
```
r_plane = n · (Rp+t) + d
```

## 実装ノート

- ROS非依存の純粋C++実装
- 元のA-LOAMからアルゴリズム部分のみを抽出・再構成
- Ceres Solver による AutoDiff 最適化

<!-- FIXED-KITTI-RESULTS:START -->

## Full KITTI comparison with one configuration

Fixed variant: `kitti_default`.

kitti_default was the accuracy candidate in the existing sequence 07 profile comparison (lowest stored 100 m RPE and official RTE there). Its arguments are frozen; the added sequences do not select a profile.

This pipeline never uses per-frame GT seeding; GT is used only for the initial rigid evaluation anchor. Keep the original variant arguments verbatim, including an explicit --no-gt-seed where present.

Reference values are LOAM values cited by later papers, not a per-sequence A-LOAM author table.

No new estimator behavior or default change is introduced by this coverage run.

Exact fixed-setting CLI (substitute the sequence for NN):

```bash
./build/evaluation/pcd_dogfooding dogfooding_results/kitti_seq_NN_full experiments/reference_data/kitti_seq_NN_full_gt.csv 0 --methods aloam --aloam-kitti-profile --summary-json aloam_seq_NN.json
```

Full sequences, official KITTI translational RTE (100-800 m), no GT seed.
Existing committed results are reused on 00, 02, 05, 07, 08.
The other rows are added full-sequence runs. Every row uses the same variant.


| Seq | Paper RTE [%] | Fixed RTE [%] | Repo / paper | FPS |
|---|---:|---:|---:|---:|
| 00 (reused) | 0.780 | 0.855 | 1.10x | 1.265 |
| 01 | 1.430 | 2.189 | 1.53x | 1.100 |
| 02 (reused) | 0.920 | 1.029 | 1.12x | 1.269 |
| 03 | 0.860 | 0.964 | 1.12x | 1.145 |
| 04 | 0.710 | 0.880 | 1.24x | 0.814 |
| 05 (reused) | 0.570 | 0.535 | 0.94x | 1.182 |
| 06 | 0.650 | 0.602 | 0.93x | 0.954 |
| 07 (reused) | 0.630 | 0.681 | 1.08x | 0.639 |
| 08 (reused) | 1.120 | 0.975 | 0.87x | 1.180 |
| 09 | 0.770 | 0.673 | 0.87x | 0.765 |
| 10 | 0.790 | 0.873 | 1.11x | 0.831 |

Geometric mean over all sequences with paper values: **1.07x**
with the fixed setting. Table 6 is restricted to this same variant by
`repo_variants` in `evaluation/data/paper_reported_numbers.json`.
Historical ablations remain available in their original
result aggregates.

FPS describes the recorded execution: the new runs use one OpenMP
thread per method and may overlap on the same sequence. Reused FPS
comes from historical runs on their recorded hosts.

Source aggregates and all unrounded rows are recorded in
`experiments/results/remaining_kitti_coverage/summary.json`.
New per-run summaries, host records, logs, estimated poses, evaluated GT
and step diagnostics are committed under the corresponding
`experiments/results/runs/` directory. Exact execution commands and the
isolated-output launcher used for overlapping methods are recorded
in `experiments/results/remaining_kitti_coverage/`.

### Sequences above twice the paper value

No sequence exceeds twice its paper value with this fixed setting.

Saved-pose diagnostics are generated with
`evaluation/scripts/diagnose_kitti_steps.py --estimate <trajectory_kitti.txt>
--gt <evaluated_gt_kitti.txt> --summary <summary.json> --method "A-LOAM"
--output <step_diagnostics.json>`; each JSON records the exact command.
The script checks its full-sequence RTE against the original run summary.

<!-- FIXED-KITTI-RESULTS:END -->

<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 comparison in progress

The full 00 replay of the stored `kitti_default` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.891327%** to
**0.914174%**; ATE changes from **12.043760 m**
to **9.520570 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction comparison is still running. The cause of this
historical difference and whether extraction affects this cell remain
**unresolved** until that comparison completes. No setting is reselected;
the stored aggregate and published paper comparison are preserved.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `485fa00601644731827dae601eb3b43c7543efeb`.
[Protocol and current scope](../../experiments/results/shared_near_range/README.md)
distinguish these completed runs from a cleaned sequence checkpoint: 00 input
cleanup and the whole-zoo audit are incomplete. Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->
