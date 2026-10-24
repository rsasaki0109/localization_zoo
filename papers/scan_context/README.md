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

## 検証と負の結果

[全区間の検索評価](../../docs/place_recognition_benchmark.md)は KITTI
00/02/05/07/08 の保存済みクエリから再評価し、F1max・適合率100%時の再現率・
top-1 再現率が既存結果と一致しました。記述子の再実行ではなく再スコアです。

[公式 KISS-Matcher による候補の幾何検証](../../experiments/results/imu_dr_readme_and_retrieval/README.md)
では、事前に固定した KITTI 00 の4クエリを各モードで評価しました。
検索候補のうち3件は再訪半径外でした。正しい再訪 3500→507 でも並進誤差は
KISS-Matcher 0.727 m、Quatro モード 0.742 m で、事前の 0.5 m 基準を
超えました。全8ケースとも登録の成功基準を満たしませんでした。
これは検索スコアで閾値選別していない小規模診断で、全区間のループ採択精度では
ありません。失敗例と実行コマンドはリンク先に保存しています。

同じ粗い推定値へ固定設定の任意 ICP 補正を加えた追試も 0/8 のままでした。
正しい再訪は両モードとも 0.229° / 0.730 m に収束し、並進基準を超えています。
収束の有無をそのまま正しいループの採択基準にすることはできません。
