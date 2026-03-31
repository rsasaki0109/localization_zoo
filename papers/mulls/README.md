# MULLS

`MULLS` を意識した軽量 multi-metric LOAM 実装です。

- `A-LOAM` の feature extraction + odometry を front-end に使用
- back-end で `edge + plane + point` の 3 種類の残差を同時最適化
- map は frame history を持つ sliding window として管理

full 論文の全ロバスト化や大規模 map 管理をそのまま移植したものではなく、
`multi-metric scan-to-map` の比較用コアライブラリとして置いています。
