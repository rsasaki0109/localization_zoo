# LINS

lightweight `LINS` 再現版です。`CESIKF` による iterated filter update を中心に、
local LiDAR map から point-to-plane correspondence を生成して状態を更新します。

この実装で入れている要素:

- IMU sample 列の逐次 propagation
- LiDAR scan の voxel downsample
- local map に対する point-to-plane correspondence
- iterated error-state Kalman update
- keyframe ベースの sliding-window local map

full robocentric formulation や original system の全工夫を完全移植したものではなく、
この repo の `RELEAD/CESIKF` 資産を使って `LINS` の軽量 state-estimator 部分を
小さく再現した版です。
