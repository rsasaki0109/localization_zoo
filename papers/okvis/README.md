# OKVIS

軽量な `OKVIS` 風プロトタイプです。`IMU preintegration` で current pose を予測しつつ、
fixed-size の local keyframe window に landmark reprojection residual を積んで
毎回詰める compact visual-inertial odometry にしています。

この実装で入れている要素:

- corrected state feedback 付きの IMU propagation
- local window の visual reprojection optimization
- frame 間 IMU pose factor
- small `Ceres` backend

full original の feature tracking / stereo frontend / strict marginalization 全体ではなく、
この repo で breadth を増やすための小さい `OKVIS` 版です。
