# Scan Context

Scan Context (Kim and Kim, IROS 2018) の簡易実装。

## 核心

- 点群を `ring x sector` の極座標グリッドに投影して高さの最大値を記述子化
- row mean の `ring key` で coarse candidate を絞る
- column mean の `sector key` で初期 yaw shift を求める
- circular shift 付き cosine distance で loop candidate を選ぶ

## 実装メモ

- KD-tree ではなく全履歴への線形探索で小さくまとめている
- 候補絞り込みは `ring key` 距離の上位 `N` 件
- 記述子の値は `z + lidar_height` の最大値を使用
