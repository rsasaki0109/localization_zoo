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
