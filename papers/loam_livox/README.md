# LOAM Livox

Livox 系ソリッドステート LiDAR を意識した LOAM 風パイプラインの簡易実装。

## 核心

- 非反復スキャンを azimuth sector に分割して pseudo scan line を作る
- sector ごとの曲率から edge / plane 特徴を抽出
- 下流の odometry / mapping は `A-LOAM` の scan-to-scan / scan-to-map を再利用

## 実装メモ

- Livox 独自ドライバ依存は入れず、通常の `PointXYZI` 点群を入力にする
- front-FOV の点群でも使いやすいように azimuth 並びで特徴を選ぶ
- 正式な Livox Feature Extraction の完全再現ではなく、小さく試せる派生版
