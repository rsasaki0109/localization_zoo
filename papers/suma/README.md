# SuMa

lightweight `SuMa` 再現版です。生点群を range image に射影して surfel を作り、
sliding window surfel map に対する point-to-plane で pose を更新します。

この実装で入れている要素:

- spherical projection による range image 生成
- 近傍画素差分からの法線推定と surfel 化
- surfel map への point-to-plane scan-to-map
- 直近フレームだけを保持する sliding window map

full `loop closure` や GPU 実装は含めず、`SuMa` の surfel odometry / mapping 部分を
この repo の最小構成に寄せています。
