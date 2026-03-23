# Localization Zoo - 引き継ぎ Plan

## 現在の状態 (2026-03-24)

**17コミット / 90ファイル / 9,382行 / CTest 9/9 全PASS**

GitHub: https://github.com/rsasaki0109/localization_zoo

### 実装済みアルゴリズム (9個)

| # | ディレクトリ | 論文/手法 | 会場 | 核心 | テスト状態 |
|---|---|---|---|---|---|
| 1 | `papers/litamin2/` | LiTAMIN2 | ICRA 2021 | 対称KLダイバージェンスICP、ボクセル正規分布間マッチング | 3テスト PASS |
| 2 | `papers/aloam/` | A-LOAM | RSS 2014 | 曲率ベース特徴抽出→scan-to-scan→scan-to-map 3段パイプライン | 3テスト PASS |
| 3 | `papers/ct_icp/` | CT-ICP | ICRA 2022 | 1フレーム=2ポーズ(12DoF)、SLERP補間で連続時間モーション補償 | 2テスト PASS |
| 4 | `papers/relead/` | RELEAD | ICRA 2024 | ヘシアンSVDで退化検知 + 制約付きESIKF (退化方向の更新を射影で除去) | 5テスト PASS |
| 5 | `papers/xicp/` | X-ICP | T-RO 2024 | ローカライズ可能性スコア(full/partial/none) + Lagrange乗数法による制約付きICP | 4テスト PASS |
| 6 | `papers/ct_icp_relead/` | CT-ICP+RELEAD | ハイブリッド | CT-ICPの連続時間レジストレーション + RELEADの退化検知を組み合わせ | 2テスト PASS |
| 7 | `papers/imu_preintegration/` | IMU Preintegration | T-RO 2017 | SO(3)多様体上の事前積分、バイアス1次補正ヤコビアン、共分散伝播 | 6テスト PASS |
| 8 | `papers/kiss_icp/` | KISS-ICP | RA-L 2023 | ボクセルハッシュマップ + 適応的閾値 + Welschカーネル | 2テスト PASS |
| 9 | `papers/ndt/` | NDT | IROS 2003 | 正規分布変換、ボクセル内の平均+共分散でガウシアンモデル化、Newton法 | 3テスト PASS |

### インフラ

| コンポーネント | 場所 | 状態 |
|---|---|---|
| 共通ライブラリ | `common/` | Pose, PointCloud, ImuData, math_utils, io |
| ROS2ノード (5個) | `ros2/localization_zoo_ros/` | litamin2/aloam/ct_icp/relead/xicp_node。Humble対応。colcon build済み |
| GitHub Actions CI | `.github/workflows/ci.yml` | Ubuntu 22.04でビルド+CTest |
| GTestテスト | 各`papers/*/test/` | CTest 9/9 PASS (54秒) |
| 合成ベンチマーク | `evaluation/src/synthetic_benchmark.cpp` | 4手法を合成都市データで比較。外部データ不要 |
| 実データ評価 | `evaluation/src/pcd_dogfooding.cpp` | PCD連番+GT CSVで評価。LiTAMIN2 ATE=0.77m/915m |
| マルチデータセットローダー | `evaluation/src/dataset_loader.h` | KITTI/MulRan/nuScenes形式対応 |
| 評価スクリプト | `evaluation/scripts/benchmark.py` | ATE/RPE計算、比較プロット、CSV出力 |
| 軌跡プロット | `evaluation/scripts/plot_trajectory.py` | KITTI形式ポーズファイルの軌跡比較 |
| rosbag再生launch | `ros2/.../launch/play_rosbag.launch.py` | bag再生+ノード同時起動 |
| Doxygen | `Doxyfile` | 設定済み、doxygenインストールで生成可能 |
| 軌跡画像 | `docs/images/real_data_benchmark.png` | READMEに埋め込み済み |

### 依存ライブラリ

```bash
sudo apt install libeigen3-dev libpcl-dev libopencv-dev libceres-dev libgtest-dev
```

### ビルドコマンド

```bash
# コアライブラリ + テスト
mkdir build && cd build && cmake .. && make -j$(nproc)
ctest  # 9/9 PASS

# ROS2
cd ros2 && source /opt/ros/humble/setup.bash && colcon build

# ベンチマーク
./evaluation/synthetic_benchmark
./evaluation/pcd_dogfooding <pcd_dir> <gt_csv> [max_frames]
```

---

## 既知の課題・改善ポイント

### 高優先度

1. **NDTの精度改善**
   - 現状: 合成データでATE 2-3m (テスト閾値を緩めて通してる)
   - 原因: `computeDerivatives()` のヘシアン近似が粗い。ガウスニュートン近似のみで二次項が不足
   - 対策: More-Thuente line searchの実装、PCLのNDT実装 (`pcl::NormalDistributionsTransform`) と比較検証
   - ファイル: `papers/ndt/src/ndt_registration.cpp:computeDerivatives()`

2. **KISS-ICPの速度改善**
   - 現状: テストで14秒かかる (合成データ3フレーム)
   - 原因: `getCorrespondences()` が全ボクセルを27近傍ブルートフォース探索
   - 対策: ボクセル内の点数を制限する、KD-treeとのハイブリッド近傍探索
   - ファイル: `papers/kiss_icp/src/kiss_icp.cpp:VoxelHashMap::getCorrespondences()`

3. **CT-ICPの実データ対応**
   - 現状: 合成データでは最高精度(ATE=0.12m)だが、実データだとボクセルマップが肥大化して実行不能
   - 原因: `addPointsToMap()` がマップサイズを制限していない
   - 対策: スライディングウィンドウでマップを最新N個に制限、古いボクセルを削除
   - ファイル: `papers/ct_icp/src/ct_icp_registration.cpp`

### 中優先度

4. **実データdogfooding結果の改善**
   - 現状: LiTAMIN2のみ評価済み (ATE=0.77m)。他の手法は未評価
   - 対策: pcd_dogfooding.cppにNDT/KISS-ICP/A-LOAMを追加
   - 注意: GT初期推定を使うscan-to-map方式。初期推定なしだとフレーム間距離(平均8.6m)が大きすぎてICPが収束しない

5. **ROS2ノードの拡充**
   - 現状: 5ノード (litamin2/aloam/ct_icp/relead/xicp)。KISS-ICP/NDTノードが未作成
   - 対策: `ros2/localization_zoo_ros/src/` に `kiss_icp_node.cpp`, `ndt_node.cpp` を追加
   - CMake: `ros2/localization_zoo_ros/CMakeLists.txt` にソース追加

6. **RELEADノードのマップマッチング**
   - 現状: `relead_node.cpp` のcorrespondence生成がダミー (全点の法線を(0,0,1)に設定)
   - 対策: ikd-treeまたはボクセルマップでの実際のpoint-to-plane対応関係を実装
   - ファイル: `ros2/localization_zoo_ros/src/relead_node.cpp:cloudCallback()`

### 低優先度

7. **Doxygenドキュメント生成**
   - `Doxyfile` は設定済み。`sudo apt install doxygen && doxygen` で生成可能
   - GitHub Pagesへのデプロイは未設定

8. **ベンチマークスクリプトの拡張**
   - `benchmark.py` はポーズファイル比較のみ。処理時間の自動計測は未対応
   - RPE計算の`scipy`依存 (TUM形式ロード時) を除去するか、requirements.txtを追加

9. **CI改善**
   - 現状のCIはビルド+CTestのみ
   - ROS2パッケージのcolcon buildもCIに追加すべき (ただしROS2 dockerが必要)

---

## 追加実装候補

### すぐ実装可能 (コードベースの延長)

| 手法 | 難易度 | 説明 |
|---|---|---|
| **GICP** (Generalized ICP) | 低 | Point-to-Plane + コバリアンス。LiTAMIN2と構造が似ている |
| **Voxel-GICP** (fast_gicp) | 低 | GICPのボクセル版。koide3/fast_gicpから移植 |
| **ScanContext** | 中 | ループクロージャ用の場所認識。リングキー+セクターキーの記述子 |
| **LOAM Livox** | 中 | ソリッドステートLiDAR用LOAM。A-LOAMベースで変更は少ない |

### 要調査 (論文読みから)

| 手法 | 難易度 | 説明 |
|---|---|---|
| **FAST-LIO2** | 高 | iKD-tree + ESIKF。iKD-treeの実装が核心 |
| **Point-LIO** | 高 | 点単位のLiDAR-IMU融合。各点にタイムスタンプベースの状態推定 |
| **LODESTAR** | 中 | 適応Schmidt-KFで退化対策LIO (RA-L 2025) |
| **Equivariant IMU Preintegration** | 高 | ガリレイ群上の事前積分。リー群の数学が重い |

---

## アーキテクチャメモ

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

### 新しい手法を追加する手順

1. `papers/<name>/` にディレクトリ構造を作成
2. ヘッダ+ソース+テストを実装 (ROS非依存)
3. `papers/<name>/CMakeLists.txt` を作成 (`add_library`, `add_test`)
4. トップレベル `CMakeLists.txt` に `add_subdirectory(papers/<name>)` 追加
5. `ctest` で全テスト通過を確認
6. (任意) `ros2/localization_zoo_ros/` にROS2ノードを追加
7. (任意) `evaluation/src/synthetic_benchmark.cpp` にベンチマーク追加

### コスト関数のパターン

各手法のコスト関数は以下のパターンに分類:

| パターン | 手法 | 数式 |
|---|---|---|
| Point-to-Point | KISS-ICP | `‖Tp - q‖²` |
| Point-to-Plane | CT-ICP, X-ICP, RELEAD | `(n·(Tp - q))²` |
| Distribution-to-Distribution | LiTAMIN2 | `KL(N(μp,Σp) ‖ N(μq,Σq))` |
| NDT | NDT | `-exp(-0.5 (p-μ)ᵀΣ⁻¹(p-μ))` |
| Feature-based | A-LOAM | エッジ距離 + 平面距離 (Ceres AutoDiff) |

### 退化対策のパターン

| 手法 | 検知方法 | 対策 |
|---|---|---|
| RELEAD | ヘシアンSVD + 固有値比 | 制約付きESIKF (退化方向のΔxを射影) |
| X-ICP | 寄与スコア (κ1/κ2/κ3分類) | Lagrange乗数法 |
| CT-ICP+RELEAD | CT-ICP対応点→RELEADの退化検知 | 等速仮定で退化方向を補間 |

### 最適化手法のパターン

| 手法 | 最適化器 | ライブラリ |
|---|---|---|
| LiTAMIN2 | Gauss-Newton (手動) | Eigen LDLT |
| A-LOAM | Ceres AutoDiff + Huber Loss | Ceres Solver |
| CT-ICP | Ceres AutoDiff + Cauchy Loss | Ceres Solver |
| X-ICP | Gauss-Newton + Lagrange乗数 (手動) | Eigen SVD |
| RELEAD | ESIKF (Kalman) | Eigen |
| KISS-ICP | Point-to-Point ICP + Welsch kernel (手動) | Eigen LDLT |
| NDT | Newton法 (手動) | Eigen LDLT |

---

## 実データ評価の知見

### PCD連番データ (108フレーム, 915m)

- LiTAMIN2: ATE=0.77m (voxel=1.0m, scan-to-map, GT初期推定)
- フレーム間距離が平均8.6mあるため、初期推定なしのscan-to-scanは使えない
- scan-to-map方式が必須。マップは蓄積し、50万点超えたら間引き
- PCDファイルパス: `/media/autoware/aa/2024-06-14-18-05-02/20250415-154129/graph/`
- GT CSV: `/media/autoware/aa/2024-06-14-18-05-02/20250415-154129/traj_corrected.csv`
  - カラム: `lidar_pose.x/y/z/roll/pitch/yaw` (ヘッダ付きCSV)
  - 3607行 (高頻度) → PCDフレーム数に等間隔で間引いて使用

### 合成ベンチマーク (30フレーム, 29m)

```
CT-ICP     ATE=0.12m  (精度最高、ただし実データでは遅すぎ)
A-LOAM     ATE=2.1m   (バランス型)
X-ICP      ATE=16.6m  (簡易対応関係のため精度低)
LiTAMIN2   ATE=31.2m  (voxel=3.0mが大きすぎ、1.0mにすると改善)
```

---

## ファイル一覧 (主要なもの)

```
localization_zoo/
├── CMakeLists.txt                              # トップレベル (C++17, Eigen3, PCL, Ceres, GTest)
├── README.md                                   # GitHub表示用 (バッジ、ベンチマーク画像)
├── PLAN.md                                     # このファイル
├── LICENSE                                     # MIT
├── Doxyfile
├── .github/workflows/ci.yml
├── .gitignore
│
├── common/
│   ├── include/common/{types.h, math_utils.h, io.h}
│   └── src/{math_utils.cpp, io.cpp}
│
├── papers/
│   ├── litamin2/    (include/litamin2/{gaussian_voxel_map.h, litamin2_registration.h}, src/, test/)
│   ├── aloam/       (include/aloam/{types.h, scan_registration.h, laser_odometry.h, laser_mapping.h, lidar_factor.h}, src/, test/)
│   ├── ct_icp/      (include/ct_icp/{types.h, cost_functions.h, ct_icp_registration.h}, src/, test/)
│   ├── relead/      (include/relead/{types.h, degeneracy_detection.h, cesikf.h}, src/, test/)
│   ├── xicp/        (include/xicp/{types.h, xicp_registration.h}, src/, test/)
│   ├── ct_icp_relead/ (include/ct_icp_relead/degeneracy_aware_ct_icp.h, src/, test/)
│   ├── imu_preintegration/ (include/imu_preintegration/imu_preintegration.h, src/, test/)
│   ├── kiss_icp/    (include/kiss_icp/kiss_icp.h, src/, test/)
│   └── ndt/         (include/ndt/ndt_registration.h, src/, test/)
│
├── evaluation/
│   ├── src/{kitti_eval.cpp, synthetic_benchmark.cpp, pcd_dogfooding.cpp, dataset_loader.h}
│   └── scripts/{benchmark.py, plot_trajectory.py}
│
├── ros2/localization_zoo_ros/
│   ├── package.xml, CMakeLists.txt
│   ├── src/{ros_utils.h, litamin2_node.cpp, aloam_node.cpp, ct_icp_node.cpp, relead_node.cpp, xicp_node.cpp}
│   └── launch/{play_rosbag.launch.py, benchmark.launch.py, all_nodes.launch.py}
│
└── docs/images/real_data_benchmark.png
```
