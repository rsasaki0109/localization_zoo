# 2D Laser Scan Odometry Benchmark

Planar `LaserScan` matchers evaluated with [`scan_dogfooding`](../../../evaluation/src/scan_dogfooding.cpp).
Metrics: **ATE [m]** and **Drift [%]** (KITTI-style RPE over a segment scaled to trajectory length).

## 結論

Nine from-paper / extension 2D odometry ports (papers 43–50 + Karto-style map matcher) share one harness.
**No single method wins every fixture** — CSM/Karto lead Intel val (Felzenszwalb EDT), Karto/CSM lead fr079 val, Kinematic-ICP leads the short MIT
window, PL-ICP leads the synthetic corridor, **NDT-2D leads the Intel train window** (16.5%), and **MbICP leads `fr079_train_1200`** (**5.7%**, NDT-2D 7.5%).
**NDT-2D** — 3-level pyramid + outlier trimming + robot-frame local map; **Intel 14.8%**, fr079 val **14.5%**, corridor **0.3%**; long train **`fr079_train_1200` 7.5%**.
**CSM / Karto-Matcher** — Felzenszwalb EDT + tuned Olson BnB; **Intel 14.0%**, fr079 **13.7%**, corridor **30.5%**.

## Leaderboard (drift % — lower is better)

GT-seed on frame 0; `--no-gt-seed` supported for pure odometry runs.

| # | Method | Paper | Intel val | fr079 val | MIT val | Synth corridor |
|---|--------|-------|----------:|----------:|--------:|---------------:|
| | | | _73 fr / 378 m_ | _384 fr / 373 m_ | _33 fr / 267 m_ | _120 fr / 9.5 m_ |
| 43 | **RF2O** | ICRA 2016 | **14.3** | 15.4 | 27.6 | 1.3 |
| — | **Karto-Matcher** | Olson/Karto ext. | **14.0** | **13.7** | 28.1 | 30.5 |
| 48 | **NDT-2D** | IROS 2003 | 14.8 | 14.5 | 27.3 | 0.3 |
| 49 | **IDC** | Lu & Milios 1997 | 15.2 | **14.3** | 27.8 | 3.6 |
| 45 | **CSM** | ICRA 2009 | **14.0** | **13.7** | 28.1 | 30.5 |
| 50 | **MbICP** | ICRA 2005 | 14.5 | 15.4 | 27.5 | **0.05** |
| 44 | **PL-ICP** | IROS 2008 | 15.0 | **14.1** | 27.2 | **0.01** |
| 46 | **Kinematic-ICP** | ICRA 2025 | 18.4 | 18.9 | **23.4** | 83.8 |
| 47 | **PSM** | ICRA 2003 | **15.3** | 14.3 | 28.5 | 4.4 |

Public logs: [Bonn 2D-SLAM JSON](https://www.ipb.uni-bonn.de/html/projects/kuang2023ral/2dslam.zip)
(Radish CARMEN conversions). GT is dataset odometry (scan-matched proxy, not centimeter truth).

### Long train windows (P4 / P8)

| Fixture | Frames | Traj [m] | Best | RF2O | PL-ICP | CSM | Karto | MbICP | NDT-2D |
|---------|--------|----------|------|-----:|-------:|----:|------:|------:|-------:|
| `mit_train_120` | 120 | 150 | RF2O | **29.5%** | 30.6% | 30.2% | 30.2% | 30.5% | 31.6% |
| `intel_train_150` | 150 | 154 | **NDT-2D** | 17.5% | 19.6% | 21.5% | 21.5% | 20.7% | **16.5%** |
| `fr079_train_1200` | 1200 | 150 | **MbICP** | 10.6% | 9.2% | 17.6% | 17.6% | **5.7%** | 7.5% |
| `fr079_train_200` | 200 | 27 | **NDT-2D*** | 30.3% | 29.4% | 40.3% | 40.3% | 7.1% | **0.9%** |

Note: `fr079_train_200` covers only ~27 m at the start of Bonn fr079 **train** (slow initial motion) — drift is **not comparable** to val or `fr079_train_1200`. Prefer **`fr079_train_1200`** (~150 m, aligned with MIT/Intel train windows).

Refresh: `evaluation/scripts/run_scan2d_long_benchmark.sh` (after `prepare_bonn_long_fixtures.sh`).

![Public fixture GT trajectories](../../assets/scan2d_public_gt.png)

## 確認済み事実

| Item | Detail |
|------|--------|
| Harness | `scan_dogfooding` — `scan_meta.json`, `NNNNNNNN/scan.csv`, `gt.csv` |
| Methods | `rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp,karto_matcher` |
| CI smoke | `evaluation/scripts/smoke_scan2d_fixture.sh` (Intel 20 frames, all 9 methods) |
| CI long smoke | `evaluation/scripts/smoke_scan2d_long_fixture.sh` (MIT/Intel/fr079 train 20f × 9 methods) |
| Batch refresh | `evaluation/scripts/run_scan2d_benchmark.sh` |
| Long train refresh (P4) | `evaluation/scripts/run_scan2d_long_benchmark.sh` |
| Long fixture prep | `evaluation/scripts/prepare_bonn_long_fixtures.sh` |
| Prep (Bonn JSON) | `evaluation/scripts/prepare_bonn_2dslam_inputs.py` |
| Prep (ROS1 bag) | `evaluation/scripts/prepare_2d_scan_inputs.py` |
| Setup guide | [`evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md`](../../../evaluation/scripts/SETUP_2D_SCAN_BENCHMARK.md) |

### Committed fixtures

| Fixture | Source | Frames | Beams | Traj [m] |
|---------|--------|--------|-------|----------|
| `evaluation/fixtures/intel_val_73` | Bonn `intel/val.json` | 73 | 180 | ~378 |
| `evaluation/fixtures/fr079_val_384` | Bonn `fr079/val.json` | 384 | 360 | ~373 |
| `evaluation/fixtures/mit_val_33` | Bonn `mit/val.json` | 33 | 360 | ~267 |
| `evaluation/fixtures/mit_train_120` | Bonn `mit/train.json` (first 120) | 120 | 360 | ~900 |
| `evaluation/fixtures/intel_train_150` | Bonn `intel/train.json` (first 150) | 150 | 180 | ~780 |
| `evaluation/fixtures/fr079_train_200` | Bonn `fr079/train.json` (first 200) | 200 | 360 | ~27* |
| `evaluation/fixtures/fr079_train_1200` | Bonn `fr079/train.json` (first 1200) | 1200 | 360 | ~150 |
| `evaluation/fixtures/rf2o_corridor` | synthetic raycast | 120 | 360 | ~9.5 |
| `evaluation/fixtures/rf2o_smoke` | synthetic raycast | 60 | 360 | ~18 |

### Per-method notes (honest)

- **RF2O** — strong Intel baseline; range-flow dense constraint. Robot-frame local map is implemented but **opt-in (default off)**: the min-range polar reference helps val windows (fr079 13.9%, corridor 0.6%) but degrades every long train window under a full param sweep (`fr079_train_1200` 10.6%→20%+) — see [`papers/rf2o/README.md`](../../../papers/rf2o/README.md).
- **Karto-Matcher** — robot-frame rolling map + Felzenszwalb EDT + tuned Olson BnB; fr079 **13.7%**, Intel **14.0%**; corridor **30.5%**.
- **NDT-2D** — 3-level pyramid + outlier trimming (match-only range jump + finest-level Mahalanobis trim) + robot-frame local map; **`fr079_train_1200` 7.5%** (was 8.8%); Intel **14.8%**; corridor **0.3%**.
- **IDC** — robot-frame rolling local map; fr079 val **14.3%** (was 27.7% scan-to-scan); corridor **3.6%** (was 42.6%); Intel **15.2%**; `fr079_train_1200` **35.0%** (was 46.4%).
- **CSM** — Felzenszwalb EDT + tuned Olson BnB; matches Karto on public fixtures; corridor **30.5%** (was 41% with chamfer).
- **PL-ICP** — robot-frame rolling local map in harness; Intel **15.0%**, fr079 **14.1%**, corridor **0.01%**; fr079 ~26 s (stamp-indexed map cache).
- **MbICP** — config-space metric ICP with **robot-frame rolling local map** in harness; Intel **14.5%**, fr079 **15.4%**, corridor **0.05%**; fr079 full refresh ~2.3 min.
- **Kinematic-ICP** — needs `--wheel-odom-from-gt`; best on short MIT window only. Point-style local map implemented but **opt-in (default off)**: it rescues `mit_train_120` (46.4%→12.8%) and `fr079_train_200` (11.0%→5.9%) yet breaks MIT val (23.4%→30.5%) and `fr079_train_1200` (10.7%→18.7%) with no safe shared config — see [`papers/kinematic_icp/README.md`](../../../papers/kinematic_icp/README.md).
- **PSM** — robot-frame rolling local map (polar profile rebuilt from point cache); Intel **15.3%** (was 21.8%); fr079 **14.3%**; corridor **4.4%** (was 11.6%); long train `fr079_train_1200` **46.7%** (was 72.2%).

## Artifact index

Canonical multi-method JSON (refresh with `run_scan2d_benchmark.sh`):

| Fixture | Artifact |
|---------|----------|
| Intel val | [`intel_val_73.json`](intel_val_73.json) |
| fr079 val | [`fr079_val_384.json`](fr079_val_384.json) |
| MIT val | [`mit_val_33.json`](mit_val_33.json) |
| Corridor | [`rf2o_corridor.json`](rf2o_corridor.json) |
| fr079 train (1200f) | [`fr079_train_1200.json`](fr079_train_1200.json) |
| Public summary | [`public_bundle.json`](public_bundle.json) |

Method-specific reruns (partial method sets): `*_idc.json`, `*_ndt2d.json`, `*_csm_dt.json`.

## Reproduce

```bash
cmake --build build --target scan_dogfooding
bash evaluation/scripts/run_scan2d_benchmark.sh
python3 evaluation/scripts/plot_scan2d_gt_overview.py
```

Single fixture, all methods:

```bash
./build/evaluation/scan_dogfooding \
  evaluation/fixtures/intel_val_73 evaluation/fixtures/intel_val_73/gt.csv \
  --methods rf2o,pl_icp,csm,kinematic_icp,psm,ndt_2d,idc,mb_icp,karto_matcher \
  --wheel-odom-from-gt \
  --summary-json docs/benchmarks/scan2d/intel_val_73.json
```

## 未確認 / 要確認項目

- **fr079_train_200** — short ~27 m prefix only; CSM/Karto drift misleading (~40%). Use **`fr079_train_1200`** (~150 m) for train-window comparison.
- **Karto-Matcher** — point-cache local map (not log-odds grid); tuned Olson BnB (64-node, finest-only refine, score lookup).

## 次アクション

1. `docs/methods.json` 2D tag 強化 / `validate_showcase.py` 2D セクション (optional).
