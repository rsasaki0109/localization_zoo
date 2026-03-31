# BALM2

lightweight `BALM2` 再現版です。`A-LOAM` の front-end で特徴点と初期 pose を作り、
直近 keyframe window の pose を `Ceres` でまとめて再最適化します。

この実装で入れている要素:

- edge / plane feature に対する multi-pose residual
- 直近 `N` keyframe の local bundle adjustment
- raw odometry 相対変換を使う pose prior
- 小さい window を毎フレーム relinearize する online backend

full `point cluster + second-order solver` の完全移植ではありません。
この repo では、`BALM2` の「複数 pose を geometric residual でまとめて詰める」
部分を、既存 `A-LOAM` 資産の上で小さく使える形に寄せています。
