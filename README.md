# Localization Zoo

公式OSS実装が存在しないLocalization（自己位置推定）系論文の非公式C++実装集。
全実装はROS非依存の純粋C++で、将来的なROS2移植を容易にする設計。

## 実装済み論文

| 論文 | 会場 | 手法 | テスト |
|------|------|------|--------|
| [LiTAMIN2](papers/litamin2/) | ICRA 2021 | 対称KLダイバージェンスICP、ボクセル正規分布マッチング | 4/4 |
| [A-LOAM](papers/aloam/) | RSS 2014 | 曲率ベース特徴抽出 + Ceres最適化 (3ステージ) | 3/3 |
| [CT-ICP](papers/ct_icp/) | ICRA 2022 | Continuous-Time ICP、12DoF SLERP補間 | 3/3 |
| [RELEAD](papers/relead/) | ICRA 2024 | 退化検知 + 制約付きESIKF | 5/5 |

## ビルド

```bash
# 依存ライブラリ (Ubuntu 22.04)
sudo apt install libeigen3-dev libpcl-dev libopencv-dev libceres-dev

# ビルド
mkdir build && cd build
cmake ..
make -j$(nproc)

# テスト実行
./papers/litamin2/test_litamin2
./papers/aloam/test_aloam
./papers/ct_icp/test_ct_icp
./papers/relead/test_relead
```

## 依存ライブラリ

| ライブラリ | バージョン | 用途 |
|-----------|-----------|------|
| C++ | 17 | 言語標準 |
| CMake | >= 3.16 | ビルドシステム |
| Eigen3 | >= 3.3 | 線形代数 |
| PCL | >= 1.10 | 点群処理、KD-tree |
| Ceres Solver | >= 2.0 | 非線形最小二乗最適化 |
| OpenCV | >= 4.0 | (common I/O用) |

## ディレクトリ構成

```
localization_zoo/
├── common/                     # 共通ライブラリ (型定義、数学、I/O)
├── papers/
│   ├── litamin2/               # KLダイバージェンスICP
│   │   ├── include/litamin2/
│   │   │   ├── gaussian_voxel_map.h
│   │   │   └── litamin2_registration.h
│   │   ├── src/
│   │   └── test/
│   ├── aloam/                  # Advanced LOAM
│   │   ├── include/aloam/
│   │   │   ├── scan_registration.h
│   │   │   ├── laser_odometry.h
│   │   │   ├── laser_mapping.h
│   │   │   └── lidar_factor.h
│   │   ├── src/
│   │   └── test/
│   ├── ct_icp/                 # Continuous-Time ICP
│   │   ├── include/ct_icp/
│   │   │   ├── types.h
│   │   │   ├── cost_functions.h
│   │   │   └── ct_icp_registration.h
│   │   ├── src/
│   │   └── test/
│   └── relead/                 # Degeneracy-Aware LIO
│       ├── include/relead/
│       │   ├── types.h
│       │   ├── degeneracy_detection.h
│       │   └── cesikf.h
│       ├── src/
│       └── test/
├── CMakeLists.txt
└── README.md
```

## 各手法の特徴比較

### 点群レジストレーション

| | LiTAMIN2 | A-LOAM | CT-ICP |
|---|---------|--------|--------|
| マッチング | ボクセル正規分布間 | 特徴点 (エッジ/平面) | Point-to-Plane |
| コスト関数 | KLダイバージェンス + ICP | エッジ距離 + 平面距離 | CT Point-to-Plane |
| 最適化 | Gauss-Newton | Ceres (AutoDiff) | Ceres (AutoDiff) |
| 特徴 | 95%点群削減で高速 | 3ステージパイプライン | モーション歪み補償 |
| DoF | 6 | 6 | 12 (begin+end) |

### 退化対策

| | RELEAD | (他手法) |
|---|--------|---------|
| 検知方法 | ヘシアンSVD + 固有値比 | - |
| 対策 | 制約付きESIKF (退化方向の更新を射影) | - |
| IMU統合 | ESIKF prediction + constrained update | - |

## 論文の追加方法

1. `papers/<paper_name>/` ディレクトリを作成
2. `README.md` に論文情報を記載
3. `include/`, `src/`, `test/` に実装
4. トップレベル `CMakeLists.txt` に `add_subdirectory(papers/<paper_name>)` を追加
5. テストが全て通ることを確認

## License

MIT
