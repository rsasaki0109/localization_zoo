# Localization Zoo

公式OSS実装が存在しないLocalization（自己位置推定）系論文の非公式C++実装集。

## 対象分野

- LiDAR-based Localization (点群マッチング、NDT派生など)
- Visual Localization (Visual Odometry, VPRなど)
- GNSS/INS Integration (タイトカップリング、RTK補正など)
- Sensor Fusion (LiDAR-Camera, LiDAR-IMUなど)
- Map-based Localization (HDマップ、ランドマークベースなど)

## ディレクトリ構成

```
localization_zoo/
├── papers/                  # 各論文の実装
│   └── <paper_name>/
│       ├── README.md        # 論文情報、手法概要、実装メモ
│       ├── CMakeLists.txt
│       ├── include/
│       ├── src/
│       └── test/
├── common/                  # 共通ライブラリ
│   ├── include/common/
│   │   ├── types.h          # 共通データ型 (Pose, PointCloud等)
│   │   ├── math_utils.h     # 数学ユーティリティ
│   │   └── io.h             # データ入出力
│   ├── src/
│   └── CMakeLists.txt
├── CMakeLists.txt           # トップレベルビルド設定
└── README.md
```

## ビルド

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 依存ライブラリ

- C++17
- CMake >= 3.16
- Eigen3
- PCL (Point Cloud Library)
- OpenCV

```bash
# Ubuntu
sudo apt install libeigen3-dev libpcl-dev libopencv-dev
```

## 論文の追加方法

1. `papers/` 以下に論文名のディレクトリを作成
2. `README.md` に論文情報（タイトル、著者、リンク、手法概要）を記載
3. 実装を追加し、トップレベル `CMakeLists.txt` に `add_subdirectory` を追加

## License

MIT
