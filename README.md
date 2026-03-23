<p align="center">
  <h1 align="center">Localization Zoo</h1>
  <p align="center">
    <b>公式コード未公開のLocalization論文を、ROS2対応のC++で再現実装</b>
  </p>
  <p align="center">
    <img src="https://img.shields.io/badge/C%2B%2B-17-blue" alt="C++17">
    <img src="https://img.shields.io/badge/ROS2-Humble-green" alt="ROS2 Humble">
    <img src="https://img.shields.io/badge/Papers-7-orange" alt="7 Papers">
    <img src="https://img.shields.io/badge/Tests-19%2F19-brightgreen" alt="19/19 Tests">
    <img src="https://img.shields.io/badge/License-MIT-yellow" alt="MIT License">
  </p>
</p>

---

## Why Localization Zoo?

多くのLocalization論文は**公式コードが公開されない**まま埋もれていく。
このリポジトリは、そうした論文を**統一されたC++ API**で再現実装し、**ROS2ノード付き**で即座に使えるようにする。

- **純粋C++**: ROS非依存のコアライブラリ。組み込み・研究・教育に
- **ROS2 Humble**: 5つのノードが`ros2 run`で即実行
- **ベンチマーク内蔵**: ビルド後すぐ`./synthetic_benchmark`で全手法を比較
- **退化対策**: トンネル・地下でもドリフトしないRELEAD/X-ICPを収録

---

## Benchmark

### Real LiDAR Data (108 frames, 915m trajectory)

<p align="center">
  <img src="docs/images/real_data_benchmark.png" alt="Real Data Benchmark" width="100%">
</p>

```
Method         ATE [m]     Distance    Frames
──────────────────────────────────────────────
LiTAMIN2       0.77        915 m       108      Scan-to-Map, Voxel=1.0m
```

> `./pcd_dogfooding <pcd_dir> <gt_csv>` で任意のPCD連番データを評価可能

### Synthetic Urban (30 frames)

```
Method         ATE [m]     FPS
─────────────────────────────────
CT-ICP         0.124       0.1   << 最高精度
A-LOAM         2.059       4.6   << バランス型
X-ICP          16.634      5.9
LiTAMIN2       31.155      2.6
```

> `./synthetic_benchmark` で再現可能。外部データ不要。

---

## Implementations

### Point Cloud Registration

| Paper | Venue | Key Idea | Code |
|-------|-------|----------|------|
| **[LiTAMIN2](papers/litamin2/)** | ICRA 2021 | KLダイバージェンスICP。95%点群削減で高速化 | [arXiv](https://arxiv.org/abs/2103.00784) |
| **[A-LOAM](papers/aloam/)** | RSS 2014 | 曲率ベース特徴抽出 + 3段パイプライン (Odom→Map) | [GitHub (ROS1)](https://github.com/HKUST-Aerial-Robotics/A-LOAM) |
| **[CT-ICP](papers/ct_icp/)** | ICRA 2022 | 1フレーム=2ポーズ(12DoF)。SLERP補間でモーション歪み補償 | [GitHub (ROS1)](https://github.com/jedeschaud/ct_icp) |
| **[X-ICP](papers/xicp/)** | T-RO 2024 | ヘシアンSVDで6方向のローカライズ可能性を分類。制約付きICP | [arXiv](https://arxiv.org/abs/2211.16335) |

### Degeneracy-Aware (退化対策)

| Paper | Venue | Key Idea | Code |
|-------|-------|----------|------|
| **[RELEAD](papers/relead/)** | ICRA 2024 | 制約付きESIKF + 退化方向への更新を射影で除去 | [arXiv](https://arxiv.org/abs/2402.18934) |
| **[CT-ICP + RELEAD](papers/ct_icp_relead/)** | Hybrid | CT-ICPの連続時間補間 + RELEADの退化検知 | - |

### Foundation

| Paper | Venue | Key Idea | Code |
|-------|-------|----------|------|
| **[IMU Preintegration](papers/imu_preintegration/)** | T-RO 2017 | SO(3)多様体上の事前積分。バイアス1次補正。全LIOの基盤 | [Paper](https://arxiv.org/abs/1512.02363) |

---

## Quick Start

```bash
# 依存 (Ubuntu 22.04)
sudo apt install libeigen3-dev libpcl-dev libopencv-dev libceres-dev libgtest-dev

# ビルド & テスト
mkdir build && cd build
cmake .. && make -j$(nproc)
ctest                          # 7/7 PASS

# ベンチマーク (外部データ不要)
./evaluation/synthetic_benchmark
```

### ROS2

```bash
cd ros2 && colcon build
source install/setup.bash

# 任意のアルゴリズムを起動
ros2 run localization_zoo_ros litamin2_node
ros2 run localization_zoo_ros aloam_node
ros2 run localization_zoo_ros ct_icp_node
ros2 run localization_zoo_ros relead_node   # + IMU対応, 退化検知publish
ros2 run localization_zoo_ros xicp_node     # 退化検知publish

# rosbag再生
ros2 launch localization_zoo_ros play_rosbag.launch.py \
  bag_path:=/path/to/bag points_topic:=/velodyne_points
```

### Evaluate

```bash
# 任意のデータセット (KITTI/MulRan/nuScenes/TUM) のポーズを比較
python3 evaluation/scripts/benchmark.py \
  --gt gt_poses.txt \
  --est LiTAMIN2:litamin2_poses.txt A-LOAM:aloam_poses.txt \
  --output_dir results/
```

---

## Architecture

```
localization_zoo/
├── common/                    # Eigen/PCL共通ユーティリティ
├── papers/
│   ├── litamin2/              # KLダイバージェンスICP
│   ├── aloam/                 # LOAM 3段パイプライン
│   ├── ct_icp/                # Continuous-Time ICP
│   ├── relead/                # 退化検知 + 制約付きESIKF
│   ├── xicp/                  # ローカライズ可能性ICP
│   ├── ct_icp_relead/         # ハイブリッド
│   └── imu_preintegration/    # IMU事前積分
├── evaluation/                # ベンチマーク & 評価ツール
├── ros2/                      # ROS2 Humble ラッパー
└── .github/workflows/ci.yml   # CI
```

各 `papers/*/` は **ヘッダ + ソース + テスト** の自己完結構造。
コアライブラリはROS非依存なので、ROS2なしでも単体で使える。

---

## Degeneracy Detection Demo

RELEAD / X-ICP はトンネルなどの退化環境を自動検知:

```
=== Tunnel Environment ===
Has degeneracy: yes
Degenerate translation dirs: 1
  dir: [1, 0, 0]              # x方向 (トンネル軸) が退化

=== Normal Environment ===
Has degeneracy: no             # 壁+地面で全方向拘束
```

ROS2ノードでは `/degeneracy` トピック (`std_msgs/Bool`) でリアルタイム通知。

---

## Adding a New Paper

```bash
mkdir -p papers/your_method/{include/your_method,src,test}
# 1. ヘッダ・ソース・テストを書く
# 2. CMakeLists.txt を追加
# 3. トップレベル CMakeLists.txt に add_subdirectory
# 4. ctest で全テスト通過を確認
```

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Eigen3 | >= 3.3 | Linear algebra |
| PCL | >= 1.10 | Point cloud processing |
| Ceres Solver | >= 2.0 | Nonlinear optimization |
| GTest | >= 1.11 | Unit testing |
| OpenCV | >= 4.0 | I/O utilities |

## License

MIT
