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

<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 comparison in progress

The full 00 replay of the stored `dense` arguments completes all
4541 frames, with the optional near-range cut still off. The historical
100 m RPE changes from **0.921410%** to
**0.990961%**; ATE changes from **9.833596 m**
to **8.556112 m**. RPE worsens. ATE improves, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction comparison is still running. The cause of this
historical difference and whether extraction affects this cell remain
**unresolved** until that comparison completes. No setting is reselected;
the stored aggregate and published paper comparison are preserved.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `485fa00601644731827dae601eb3b43c7543efeb`.
[Protocol and current scope](../../experiments/results/shared_near_range/README.md)
distinguish these completed runs from a cleaned sequence checkpoint: 00 input
cleanup and the whole-zoo audit are incomplete. Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->
