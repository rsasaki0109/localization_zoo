# MULLS

`MULLS` を意識した軽量 multi-metric LOAM 実装です。

- `A-LOAM` の feature extraction + odometry を front-end に使用
- back-end で `edge + plane + point` の 3 種類の残差を同時最適化
- map は frame history を持つ sliding window として管理

full 論文の全ロバスト化や大規模 map 管理をそのまま移植したものではなく、
`multi-metric scan-to-map` の比較用コアライブラリとして置いています。

## 公式実装との比較 (2026-10-11)

公式 MULLS (YuePanEdward/MULLS) を `evaluation/scripts/run_official_mulls_baseline.py`
で KITTI urban プロファイル (`lo_gflag_list_kitti_urban.txt`: ループ閉じ込みなし、
0.195 deg の鉛直角補正) のまま実行し、このリポジトリと同じ指標で採点した。

| KITTI 07, 公式 KITTI RTE [%] | 値 |
|---|---:|
| 論文 Table II, MULLS-LO(mc) | 0.29 |
| 公式実装 (2 回実行。OpenMP のため実行ごとに少し揺れる) | 0.311 / 0.316 |
| この実装の Table 6 バリアント | 2.627 |

公式コードは論文値を再現する (1.07-1.09x)。Table 6 の 7.62x の差はすべてこの実装側に
ある。この実装は A-LOAM の edge/plane 特徴を使う軽量版で、MULLS の中核である
二重閾値の地面抽出と PCA による ground / facade / pillar / beam / roof / vertex の
分類、それらを使う multi-metric linear least squares ICP を持たない。パラメータ調整
では埋まらない構造上の差なので、差を詰めるにはこれらの再実装が必要になる。

公式実装のビルドには PCL 1.14 / GCC 13 向けの修正が要る (スクリプトの docstring
を参照)。
