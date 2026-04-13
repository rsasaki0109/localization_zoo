# Native Time Provenance

本文書は、HDL-400 ベンチマークにおける「native per-point time」と「synthetic time」の2系統を明文化し、exact reproduction に必要な条件を定義する。

---

## 1. 用語定義

| 用語 | 意味 |
|------|------|
| **native per-point time** | センサーが記録した各点の発射時刻。PointCloud2 の `time` フィールドに格納されている。`hdl_400_ros2` bag から抽出した PCD には本フィールドが全点に存在する。 |
| **synthetic time** | 公開 ROS1 bag に `time` フィールドが存在しないため、`extract_ros1_lidar_imu.py --time-mode index|azimuth` で後付け生成した疑似時刻。点群内のインデックス順や方位角に基づいて割り振られる。 |
| **reference aggregate** | `experiments/results/ct_lio_reference_profile_matrix.json` 等に格納されている、native-time 由来のベンチマーク数値。CT 系手法の正式なベースライン指標として扱う。 |

---

## 2. 現状のデータフロー図

```
hdl_400_ros2 bag (native per-point time 付き)
  |
  +-- extract_ros1_lidar_imu.py --time-mode preserve
  |     |
  |     +-- PCD シーケンス (time フィールド保持: 全点非ゼロ・単調増加)
  |           |
  |           +-- pcd_dogfooding --methods ct_lio
  |                 |
  |                 +-- reference aggregate (native-time truth)
  |                     [experiments/results/ct_lio_reference_profile_matrix.json]
  |
  |                                     <<<< 別系統 >>>>
  |
hdl_400 公開 ROS1 bag (time フィールド無し)
  |
  +-- extract_ros1_lidar_imu.py --time-mode index
        |
        +-- PCD シーケンス (synthetic time 付き: インデックス由来の疑似時刻)
              |
              +-- pcd_dogfooding --methods ct_lio
                    |
                    +-- public synthetic-time aggregate
                        (stored reference aggregate とは一致しない)
```

上記の2系統は**入力の time フィールドの出自が異なる**ため、出力 aggregate は別物として扱う必要がある。特に CT 系（Continuous-Time）手法はスキャン内の各点の時刻情報を用いて補間を行うため、time フィールドの品質が結果に直接影響する。

---

## 3. Exact Native-Time Reproduction に必要な Source Artifact 条件

以下の全てが揃った場合にのみ、stored reference aggregate との exact reproduction が可能となる。

- **必須 artifact**: `hdl_400_ros2` bag、もしくはそこから抽出された各点に native `time` フィールドを持つ PCD シーケンス
- **必須 artifact の所在**: `../lidarloc_ws/data/official/hdl_localization/hdl_400_ros2`（manifest 内 `dataset.source_bag` に記載）
- **GT trajectory**: `experiments/reference_data/hdl_400_public_reference.csv`（リポジトリ内に存在する）
- **抽出スクリプト**: `evaluation/scripts/extract_ros1_lidar_imu.py` の `--time-mode preserve` モード（native time が無い bag では `PointCloud2 is missing fields: time` エラーで失敗する）
- **PACKED_POINT_DTYPE の構造**: 抽出 PCD は `x(f4), y(f4), z(f4), intensity(f4), time(f4)` の 5 フィールド・20 bytes/point で格納される
- **検証方法**: 抽出した PCD の `time` フィールドが全点で非ゼロかつ単調増加であること（ゼロや非単調が混在する場合は native time ではなく synthetic time が混入している）

---

## 4. Synthetic-Time でできること/できないことの表

| 観点 | synthetic-time | native-time |
|------|---------------|-------------|
| non-CT 系 (GICP, KISS-ICP 等) のベンチマーク | OK | OK |
| CT 系 (CT-LIO, CT-ICP) の variant 間比較 | OK（同条件内での相対比較） | OK |
| CT 系の stored reference aggregate との一致 | **NG** | OK |
| "exact reproduction" の主張 | **NG** | OK |
| 公開データだけで実行 | OK | **NG**（非公開 bag が必要） |

**補足**:

- synthetic-time であっても CT 系の variant 間で「どちらが良いか」の相対比較は有効である。全 variant が同一の synthetic time を使っているため、条件は揃っている。
- ただし「リポジトリの stored reference aggregate と同じ数値が出る」ことは保証されない。reference aggregate は native-time で取得されたものであり、time フィールドの値が異なれば CT 補間の結果も異なる。

---

## 5. Blocking 状態の記述

現在 `experiments/ct_lio_public_readiness_matrix.json` は `state: "blocked"` である。

> **blocker**: "A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence."

> **next_step**: "No action planned. CT-LIO GT-backed is excluded from the main study scope. If an independent GT source becomes available for HDL-400, this can be revisited."

> **scope_decision**: "Scoped out of main empirical study. CT-LIO remains reference-based only on HDL-400. KITTI Odometry lacks IMU data for LIO methods, and HDL-400 has no independently curated GT. This problem stays blocked and is documented as appendix/artifact material only."

この blocked 状態は本文書で定義する「native-time provenance の不在」と直接関連する。非公開の `hdl_400_ros2` bag から抽出した native-time PCD がなければ、GT-backed な公開ベンチマークを独立に再現する手段がない。

一方、`experiments/results/ct_lio_reference_profile_matrix.json` に格納されている reference-based 比較（`status: "ready"`）は既に利用可能であり、variant 間の相対評価は完了している。

---

## 6. 解除条件チェックリスト

blocked 状態を解除し、native-time による exact reproduction を確立するには以下の全てを満たす必要がある。

- [ ] `hdl_400_ros2` bag または native-time PCD が入手可能な場所に置かれている
- [ ] `extract_ros1_lidar_imu.py --time-mode preserve` で抽出が成功する
- [ ] 抽出 PCD の `time` フィールドが全点で有効値を持つ（非ゼロ・単調増加）
- [ ] `pcd_dogfooding` で CT-LIO / CT-ICP を実行し、stored reference aggregate と ATE が +/-5% 以内で一致する
- [ ] 上記の再現手順が本文書に記載されている
