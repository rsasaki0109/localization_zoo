# F-LOAM

lightweight `F-LOAM` 再現版です。`A-LOAM` の feature-based odometry / mapping を土台に、
入力点群の thinning と mapping update の間引き、map-odom correction の維持を入れて
高速寄りの LOAM pipeline にしています。

この実装で入れている要素:

- stride 間引きした入力点群による feature extraction
- reduced feature budget の `ScanRegistration`
- lightweight `LaserOdometry`
- interval 制御付き `LaserMapping`
- map-odom correction を使った軽量 pose 補正

full original system の全最適化を完全移植したものではなく、この repo の `A-LOAM`
資産を使って `F-LOAM` の fast pipeline 側を小さく再現した版です。
