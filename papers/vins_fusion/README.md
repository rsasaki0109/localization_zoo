# VINS-Fusion

軽量な `VINS-Fusion` 風プロトタイプです。既知 landmark の reprojection と
`IMU preintegration` を frame 間拘束として使う compact visual-inertial odometry にしています。

この実装で入れている要素:

- `IMU preintegration` による raw pose prediction
- keyframe ごとの landmark reprojection residual
- small `Ceres` pose graph による visual-inertial optimization
- fixed-size keyframe window

full original の feature tracking、stereo / loop closure / map reuse をそのまま
移植したものではなく、この repo で dogfooding しやすい最小版です。
