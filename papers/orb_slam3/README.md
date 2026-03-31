# ORB-SLAM3

軽量な `ORB-SLAM3` 風プロトタイプです。known landmark の reprojection、`IMU preintegration`
による frame 間拘束、landmark overlap を使う compact loop closure を 1 つの
keyframe graph に載せています。

この実装で入れている要素:

- visual-inertial keyframe graph
- current frame の visual pose refinement
- landmark overlap ベースの loop candidate 選択
- small `Ceres` backend

full original の ORB frontend、BoW database、map merging 全体ではなく、
この repo の breadth を増やすための compact `ORB-SLAM3` 版です。
