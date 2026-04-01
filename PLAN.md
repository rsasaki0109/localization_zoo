# Localization Zoo - Codex 引き継ぎ Plan

> このドキュメントはCodexが自律的に作業するための完全な引き継ぎ資料。
> 各タスクは独立して実行可能。優先度順に並んでいる。

## 現在の状態 (2026-04-01)

**18コミット / 91ファイルtracked + 大量のuntracked / 25,314行(papers/) / 38実装**

- GitHub: https://github.com/rsasaki0109/localization_zoo
- 38個のアルゴリズム実装が `papers/` にある（全てsrc/test/CMakeLists.txt完備）
- 全38実装がトップレベル `CMakeLists.txt` に登録済み、ビルド通る
- ROS2ノード: 6個 (litamin2, aloam, ct_icp, ct_lio, relead, xicp)
- 未コミットの変更が15ファイル + 28個の新ディレクトリ (papers/balm2, papers/ct_lio, etc.)

### 全38実装一覧

#### LiDAR Odometry / Registration (17個)

| # | ディレクトリ | 手法 | 会場/年 | カテゴリ |
|---|---|---|---|---|
| 1 | `papers/litamin2/` | LiTAMIN2 (KLダイバージェンスICP) | ICRA 2021 | Distribution-based |
| 2 | `papers/aloam/` | A-LOAM (3段パイプライン) | RSS 2014 | Feature-based |
| 3 | `papers/ct_icp/` | CT-ICP (Continuous-Time 12DoF) | ICRA 2022 | CT |
| 4 | `papers/kiss_icp/` | KISS-ICP (適応的閾値+Welsch) | RA-L 2023 | Point-to-Point |
| 5 | `papers/ndt/` | NDT (正規分布変換) | IROS 2003 | Distribution-based |
| 6 | `papers/gicp/` | GICP (一般化ICP) | RSS 2009 | Plane-based |
| 7 | `papers/voxel_gicp/` | Voxel-GICP | - | Distribution-based |
| 8 | `papers/small_gicp/` | Small-GICP (高速GICP) | RA-L 2024 | Distribution-based |
| 9 | `papers/floam/` | F-LOAM (高速LOAM) | IROS 2021 | Feature-based |
| 10 | `papers/loam_livox/` | LOAM Livox (ソリッドステートLiDAR) | - | Feature-based |
| 11 | `papers/lego_loam/` | LeGO-LOAM (地面最適化LOAM) | IROS 2018 | Feature-based |
| 12 | `papers/isc_loam/` | ISC-LOAM (Intensity Scan Context) | IROS 2020 | Feature-based |
| 13 | `papers/mulls/` | MULLS (Multi-metric Linear LS) | ICRA 2021 | Multi-metric |
| 14 | `papers/suma/` | SuMa (Surfel-based Mapping) | RSS 2018 | Surfel-based |
| 15 | `papers/dlo/` | DLO (Direct LiDAR Odometry) | RA-L 2022 | Direct |
| 16 | `papers/balm2/` | BALM2 (Bundle Adjustment for LiDAR) | T-RO 2023 | BA-based |
| 17 | `papers/scan_context/` | Scan Context (場所認識) | IROS 2018 | Place Recognition |

#### LiDAR-Inertial Odometry (10個)

| # | ディレクトリ | 手法 | 会場/年 | カテゴリ |
|---|---|---|---|---|
| 18 | `papers/ct_lio/` | CT-LIO (CT-ICP + IMU) | RA-L 2024 | CT-LIO |
| 19 | `papers/fast_lio2/` | FAST-LIO2 (iKD-tree + ESIKF) | T-RO 2022 | LIO |
| 20 | `papers/point_lio/` | Point-LIO (点単位のLIO) | Adv.Intel.Sys 2023 | LIO |
| 21 | `papers/dlio/` | DLIO (Direct LiDAR-Inertial) | ICRA 2023 | LIO |
| 22 | `papers/lins/` | LINS (LiDAR-Inertial SLAM) | ICRA 2020 | LIO |
| 23 | `papers/clins/` | CLINS (Continuous-Time LINS) | T-RO 2022 | CT-LIO |
| 24 | `papers/lio_sam/` | LIO-SAM (Tightly-coupled LIO) | IROS 2020 | LIO |
| 25 | `papers/fast_lio_slam/` | FAST-LIO-SLAM (+ループクロージャ) | - | LIO+LC |
| 26 | `papers/relead/` | RELEAD (退化検知+制約ESIKF) | ICRA 2024 | Degeneracy-LIO |
| 27 | `papers/vilens/` | VILENS (Visual-Inertial-LiDAR) | - | VIO-LIO |

#### Visual / Multi-sensor (5個)

| # | ディレクトリ | 手法 | 会場/年 | カテゴリ |
|---|---|---|---|---|
| 28 | `papers/vins_fusion/` | VINS-Fusion (VIO) | T-RO 2019 | VIO |
| 29 | `papers/okvis/` | OKVIS (Keyframe VIO) | IJRR 2015 | VIO |
| 30 | `papers/orb_slam3/` | ORB-SLAM3 (Visual SLAM) | T-RO 2021 | Visual SLAM |
| 31 | `papers/lvi_sam/` | LVI-SAM (LiDAR-Visual-Inertial) | ICRA 2021 | Multi-sensor |
| 32 | `papers/r2live/` | R2LIVE (LiDAR-Visual-Inertial) | IROS 2021 | Multi-sensor |

#### その他 (6個)

| # | ディレクトリ | 手法 | 会場/年 | カテゴリ |
|---|---|---|---|---|
| 33 | `papers/xicp/` | X-ICP (ローカライズ可能性ICP) | T-RO 2024 | Degeneracy |
| 34 | `papers/ct_icp_relead/` | CT-ICP+RELEAD (ハイブリッド) | - | Hybrid |
| 35 | `papers/imu_preintegration/` | IMU Preintegration (事前積分) | T-RO 2017 | Foundation |
| 36 | `papers/hdl_graph_slam/` | HDL Graph SLAM | IROS 2018 | Graph SLAM |
| 37 | `papers/vgicp_slam/` | VGICP SLAM | RA-L 2021 | SLAM |
| 38 | `papers/fast_livo2/` | FAST-LIVO2 (LiDAR-Visual-Inertial) | T-RO 2024 | Multi-sensor |

---

## 未コミットの変更 (要コミット)

以下が `git status` で未追跡 or 変更済み。**最初にこれを全部コミットすること。**

### 変更済みファイル (M)

```
M CMakeLists.txt                           # 全38実装をadd_subdirectory
M README.md                                # 更新が必要
M evaluation/CMakeLists.txt
M evaluation/src/pcd_dogfooding.cpp        # scan-to-map方式に修正済み
M papers/ct_icp/include/ct_icp/ct_icp_registration.h  # スライディングウィンドウ追加
M papers/ct_icp/src/ct_icp_registration.cpp            # 同上
M papers/ct_icp/test/test_ct_icp.cpp                   # SlidingWindowMapテスト追加
M papers/kiss_icp/include/kiss_icp/kiss_icp.h          # 速度改善
M papers/kiss_icp/src/kiss_icp.cpp                     # 同上
M ros2/localization_zoo_ros/CMakeLists.txt              # ct_lio_node追加
M ros2/localization_zoo_ros/launch/all_nodes.launch.py  # 全アルゴリズム対応に改善
M ros2/localization_zoo_ros/launch/benchmark.launch.py  # 同上
M ros2/localization_zoo_ros/launch/play_rosbag.launch.py # 同上
M ros2/localization_zoo_ros/package.xml                 # std_msgs追加
M ros2/localization_zoo_ros/src/ct_icp_node.cpp         # max_frames_in_mapパラメータ追加
```

### 新規ディレクトリ (??)

```
papers/balm2/         papers/clins/         papers/ct_lio/
papers/dlio/          papers/dlo/           papers/fast_lio2/
papers/fast_lio_slam/  papers/fast_livo2/    papers/floam/
papers/gicp/          papers/hdl_graph_slam/ papers/isc_loam/
papers/lego_loam/     papers/lins/          papers/lio_sam/
papers/loam_livox/    papers/lvi_sam/       papers/mulls/
papers/okvis/         papers/orb_slam3/     papers/point_lio/
papers/r2live/        papers/scan_context/  papers/small_gicp/
papers/suma/          papers/vgicp_slam/    papers/vilens/
papers/vins_fusion/   papers/voxel_gicp/

ros2/localization_zoo_ros/src/ct_lio_node.cpp
evaluation/scripts/extract_ros1_lidar_imu.py
dogfooding_results/    # .gitignoreに追加すべき
```

### コミット手順

```bash
# 1. dogfooding_resultsを.gitignoreに追加
echo "dogfooding_results/" >> .gitignore

# 2. 全部コミット
git add -A
git commit -m "38実装に拡張: 29個の新アルゴリズム追加 + CT-ICPスライディングウィンドウ + CT-LIOノード"
git push
```

---

## Codexタスク一覧 (優先度順)

### P0: 即座にやるべき (ブロッカー)

#### TASK-01: 未コミット変更をコミット&プッシュ
- 上記の手順でgit add -A && commit && push
- dogfooding_results/ は .gitignore に追加

#### TASK-02: CTestを全PASSにする
- `cd build && cmake .. && make -j$(nproc) && ctest --timeout 60`
- 失敗するテストがあれば閾値を緩めるか、テストデータを調整
- 全テストPASSが必須条件

### P1: README & ドキュメント更新

#### TASK-03: READMEを38実装に更新
- `README.md` の実装済み論文テーブルを38個に更新
- カテゴリ分け: LiDAR Odom / LIO / Visual / Multi-sensor / Foundation
- バッジの `Papers-7` を `Papers-38` に更新
- ベンチマーク結果セクションの更新

#### TASK-04: 各実装のREADME.md作成
- 38個の `papers/*/README.md` を全部作成
- テンプレート:
  ```markdown
  # 手法名
  ## 論文情報
  - 著者, 会場, 年, arXiv/GitHub URL
  ## 手法概要
  - 2-3行で核心を説明
  ## 核心数式
  - コスト関数
  ## パラメータ
  | パラメータ | デフォルト値 | 説明 |
  ```
- 各 `papers/*/include/` のヘッダからパラメータ構造体を読んで埋める

### P2: ROS2ノード拡充

#### TASK-05: 残り32個のROS2ノード追加
- 現状6ノード: litamin2, aloam, ct_icp, ct_lio, relead, xicp
- 残り: kiss_icp, ndt, gicp, floam, lego_loam, dlo, dlio, fast_lio2, lio_sam, etc.
- 各ノードの構造は既存ノードをコピーして修正:
  - `ros2/localization_zoo_ros/src/<method>_node.cpp`
  - subscribe: `points_raw` (PointCloud2), `imu_raw` (Imu) ※LIO系のみ
  - publish: `odom` (Odometry), TF
  - LIO系は `imu_raw` もsubscribe
- `ros2/localization_zoo_ros/CMakeLists.txt` に追加
- `launch/all_nodes.launch.py` のALGORITHMS辞書に追加
- **テンプレートパターン**: `ros2/localization_zoo_ros/src/litamin2_node.cpp` を参照

#### TASK-06: ROS2パッケージのcolcon buildをCIに追加
- `.github/workflows/ci.yml` にROS2ビルドジョブを追加
- `ros2:humble` Dockerイメージを使用
- `colcon build --packages-select localization_zoo_ros`

### P3: 品質改善

#### TASK-07: NDT精度改善
- ファイル: `papers/ndt/src/ndt_registration.cpp`
- 問題: `computeDerivatives()` のガウスニュートン近似が粗い
- 対策: More-Thuente line searchを実装
- 目標: `test/test_ndt.cpp` の `EXPECT_LT(err, 1.0)` でPASS
- 参考: PCLの `pcl::NormalDistributionsTransform` 実装

#### TASK-08: KISS-ICP速度改善
- ファイル: `papers/kiss_icp/src/kiss_icp.cpp`
- 問題: `VoxelHashMap::getCorrespondences()` が遅い (27ボクセルx全点のブルートフォース)
- 対策: ボクセル内の点数制限が既にあるが、クエリ側のサブサンプリングが不足
- `registerFrame()` で `registration_points` (ICP用) と `downsampled` (マップ追加用) を分離する改善は既に入っている
- 目標: テスト実行時間を10秒以下に

#### TASK-09: CT-ICPの実データ対応
- ファイル: `papers/ct_icp/src/ct_icp_registration.cpp`, `papers/ct_icp/include/ct_icp/ct_icp_registration.h`
- 改善済み: スライディングウィンドウ (`max_frames_in_map`) が追加されている (未コミット)
- テスト: `test/test_ct_icp.cpp` に `SlidingWindowMap` テストが追加済み
- 残タスク: `pcd_dogfooding.cpp` でCT-ICPを有効化して実データ評価

#### TASK-10: RELEADノードのマップマッチング実装
- ファイル: `ros2/localization_zoo_ros/src/relead_node.cpp`
- 問題: `cloudCallback()` の対応関係生成がダミー (法線を全部(0,0,1)に設定)
- 対策: ボクセルマップに蓄積し、PCAで法線を計算する実装に変更
- 参考: `papers/ct_icp/src/ct_icp_registration.cpp` の `findCorrespondences()`

### P4: 評価・ベンチマーク

#### TASK-11: synthetic_benchmarkに全手法追加
- ファイル: `evaluation/src/synthetic_benchmark.cpp`
- 現状: LiTAMIN2, A-LOAM, CT-ICP, X-ICPの4手法のみ
- 追加すべき: KISS-ICP, NDT, GICP, FLOAM, DLO
- 各手法の `registerFrame()` or `align()` APIを呼ぶ形で追加

#### TASK-12: pcd_dogfoodingに全手法追加
- ファイル: `evaluation/src/pcd_dogfooding.cpp`
- 現状: LiTAMIN2のみ (CT-ICPはコメントアウト)
- 追加: NDT, KISS-ICP, GICP (scan-to-map方式)
- 実データパス: `/media/autoware/aa/2024-06-14-18-05-02/20250415-154129/graph/`
- GT CSV: `traj_corrected.csv` (カラム: `lidar_pose.x/y/z/roll/pitch/yaw`)

#### TASK-13: ベンチマーク結果画像の更新
- `evaluation/scripts/plot_trajectory.py` を拡張して複数手法の軌跡を重ねてプロット
- `docs/images/real_data_benchmark.png` を更新
- READMEに最新の結果を反映

### P5: 新機能

#### TASK-14: GICPのROS2ノード追加
- `papers/gicp/` は既に実装済み
- `ros2/.../src/gicp_node.cpp` を作成
- litamin2_nodeをテンプレートに

#### TASK-15: FAST-LIO2のROS2ノード追加
- `papers/fast_lio2/` は既に実装済み
- IMU subscriptionが必要 (relead_nodeをテンプレートに)

#### TASK-16: Doxygenドキュメント自動生成
- `Doxyfile` は設定済み
- GitHub Actions で `doxygen` を実行し、GitHub Pagesにデプロイ
- `.github/workflows/docs.yml` を新規作成

---

## アーキテクチャ

### ディレクトリ構成ルール

```
papers/<method_name>/
├── README.md                    # 論文情報、手法概要、パラメータ
├── CMakeLists.txt               # add_library + add_executable(test) + add_test
├── include/<method_name>/
│   └── *.h                      # 公開ヘッダ (ROS非依存)
├── src/
│   └── *.cpp                    # 実装 (ROS非依存)
└── test/
    └── test_*.cpp               # GTestテスト
```

### ROS2ノードの構造

```cpp
// テンプレート (litamin2_node.cpp を参照)
class XxxNode : public rclcpp::Node {
  // subscribe: "points_raw" (PointCloud2)
  // subscribe: "imu_raw" (Imu)  ※LIO系のみ
  // publish: "odom" (Odometry)
  // publish: "degeneracy" (Bool)  ※退化検知系のみ
  // broadcast: TF (map -> base_link)
};
```

### launchファイルの構造

`launch/all_nodes.launch.py` のALGORITHMS辞書に追加するだけで全launchから使える:

```python
ALGORITHMS = {
    'method_name': {
        'executable': 'method_node',
        'params': {'param1': value1, ...},
        'needs_imu': True/False,
    },
}
```

### 新しい手法を追加する完全手順

1. `papers/<name>/` にディレクトリ構造を作成
2. `include/<name>/*.h` にROS非依存のAPI定義
3. `src/*.cpp` に実装
4. `test/test_<name>.cpp` にGTestテスト (最低2テスト: 基本動作 + エッジケース)
5. `CMakeLists.txt` を作成: `add_library`, `add_test`, `GTest::gtest_main`
6. トップレベル `CMakeLists.txt` に `add_subdirectory(papers/<name>)` 追加
7. `ctest` で全テスト通過を確認
8. `ros2/.../src/<name>_node.cpp` にROS2ノード追加
9. `ros2/.../CMakeLists.txt` にビルドターゲット追加
10. `launch/all_nodes.launch.py` のALGORITHMS辞書に追加
11. `README.md` のテーブルに追加

### コスト関数パターン

| パターン | 手法 | 数式 |
|---|---|---|
| Point-to-Point | KISS-ICP | `‖Tp - q‖²` |
| Point-to-Plane | CT-ICP, X-ICP, RELEAD, FLOAM | `(n·(Tp - q))²` |
| Distribution-to-Distribution | LiTAMIN2, BALM2 | `KL(N(μp,Σp) ‖ N(μq,Σq))` |
| Generalized ICP | GICP, VGICP, Small-GICP | `(Tp-q)ᵀ(Σp+Σq)⁻¹(Tp-q)` |
| NDT | NDT | `-exp(-0.5 (p-μ)ᵀΣ⁻¹(p-μ))` |
| Feature-based | A-LOAM, LeGO-LOAM, LOAM Livox | エッジ距離 + 平面距離 (Ceres) |
| Surfel-based | SuMa | Surfelへの射影距離 |

### 退化対策パターン

| 手法 | 検知方法 | 対策 |
|---|---|---|
| RELEAD | ヘシアンSVD + 固有値比 | 制約付きESIKF |
| X-ICP | 寄与スコア (κ1/κ2/κ3分類) | Lagrange乗数法 |
| CT-ICP+RELEAD | CT-ICPの対応点→退化検知 | 等速仮定で補間 |

### 最適化手法パターン

| 手法 | 最適化器 | ライブラリ |
|---|---|---|
| LiTAMIN2 | Gauss-Newton (手動) | Eigen LDLT |
| A-LOAM, CT-ICP, FLOAM | Ceres AutoDiff | Ceres Solver |
| X-ICP | GN + Lagrange (手動) | Eigen SVD |
| RELEAD, FAST-LIO2 | ESIKF (Kalman) | Eigen |
| KISS-ICP, NDT | ICP/Newton (手動) | Eigen LDLT |
| LIO-SAM | GTSAM | GTSAM (ファクターグラフ) |

---

## 実データ評価の知見

### PCD連番データ (108フレーム, 915m)

- **LiTAMIN2**: ATE=0.77m (voxel=1.0m, scan-to-map, GT初期推定)
- フレーム間距離が平均8.6mあるため、初期推定なしのscan-to-scanは使えない
- scan-to-map方式が必須
- マップは蓄積し、50万点超えたら間引き
- PCDファイルパス: `/media/autoware/aa/2024-06-14-18-05-02/20250415-154129/graph/`
- GT CSV: `/media/autoware/aa/2024-06-14-18-05-02/20250415-154129/traj_corrected.csv`
  - カラム: `lidar_pose.x/y/z/roll/pitch/yaw` (ヘッダ付きCSV)
  - 3607行 (高頻度) → PCDフレーム数に等間隔で間引いて使用

### 合成ベンチマーク (30フレーム, 29m)

```
CT-ICP     ATE=0.12m  (精度最高、ただし実データでは遅い)
A-LOAM     ATE=2.1m   (バランス型)
X-ICP      ATE=16.6m  (簡易対応関係のため)
LiTAMIN2   ATE=31.2m  (voxel=3.0mが大きすぎ、1.0mにすると改善)
```

---

## ビルドコマンド

```bash
# 依存
sudo apt install libeigen3-dev libpcl-dev libopencv-dev libceres-dev libgtest-dev

# コアライブラリ + テスト (38実装)
mkdir build && cd build && cmake .. && make -j$(nproc)
ctest --timeout 60  # テストごとに60秒でタイムアウト

# ROS2
cd ros2 && source /opt/ros/humble/setup.bash && colcon build

# ベンチマーク
./evaluation/synthetic_benchmark
./evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames]
```

---

## やってはいけないこと

- コミットメッセージに `Co-Authored-By: Claude` を入れない
- PRの説明に「Generated with Claude Code」を書かない
- `ros2/log/`, `ros2/build/`, `ros2/install/` をコミットしない (APIトークンがログに含まれてpush拒否される)
- `dogfooding_results/` をコミットしない
- テストの閾値を無限に緩めて通すのは禁止。改善不能なら `GTEST_SKIP()` でスキップ

---

## ファイル一覧 (主要)

```
localization_zoo/
├── CMakeLists.txt                              # 全38実装のadd_subdirectory
├── README.md
├── PLAN.md                                     # このファイル
├── LICENSE (MIT)
├── Doxyfile
├── .github/workflows/ci.yml
├── .gitignore
│
├── common/                                     # 共通ライブラリ
│   ├── include/common/{types.h, math_utils.h, io.h}
│   └── src/{math_utils.cpp, io.cpp}
│
├── papers/                                     # 38個のアルゴリズム実装
│   ├── litamin2/   ├── aloam/      ├── ct_icp/
│   ├── kiss_icp/   ├── ndt/        ├── gicp/
│   ├── voxel_gicp/ ├── small_gicp/ ├── floam/
│   ├── loam_livox/ ├── lego_loam/  ├── isc_loam/
│   ├── mulls/      ├── suma/       ├── dlo/
│   ├── balm2/      ├── scan_context/
│   ├── ct_lio/     ├── fast_lio2/  ├── point_lio/
│   ├── dlio/       ├── lins/       ├── clins/
│   ├── lio_sam/    ├── fast_lio_slam/
│   ├── relead/     ├── vilens/
│   ├── vins_fusion/ ├── okvis/     ├── orb_slam3/
│   ├── lvi_sam/    ├── r2live/     ├── fast_livo2/
│   ├── xicp/       ├── ct_icp_relead/
│   ├── imu_preintegration/
│   ├── hdl_graph_slam/ ├── vgicp_slam/
│   └── (各ディレクトリ: include/ src/ test/ CMakeLists.txt)
│
├── evaluation/
│   ├── CMakeLists.txt
│   ├── src/{kitti_eval.cpp, synthetic_benchmark.cpp, pcd_dogfooding.cpp, dataset_loader.h}
│   └── scripts/{benchmark.py, plot_trajectory.py, extract_ros1_lidar_imu.py}
│
├── ros2/localization_zoo_ros/
│   ├── package.xml, CMakeLists.txt
│   ├── src/{ros_utils.h, litamin2_node.cpp, aloam_node.cpp, ct_icp_node.cpp,
│   │        ct_lio_node.cpp, relead_node.cpp, xicp_node.cpp}
│   ├── launch/{all_nodes.launch.py, play_rosbag.launch.py, benchmark.launch.py}
│   └── config/
│
└── docs/images/real_data_benchmark.png
```
