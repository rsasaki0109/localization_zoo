# Fixed-Map NDT, KITTI 108-Frame Windows

`fixed_map_ndt` builds one NDT target map from GT/reference poses, then
localizes each live scan against that fixed target. This separates map-based
correction from the previous online-map runners, where map drift followed the
estimated trajectory.

Common settings:

`--ndt-resolution 0.5 --ndt-max-iterations 12 --ndt-map-max-points 120000 --fixed-map-ndt-map-stride 1`

## Seed Source Comparison

| Sequence | Seed source | ATE [m] | RPE trans [%] | FPS | Accepted |
| --- | --- | ---: | ---: | ---: | ---: |
| 02 | GT | 0.002575 | 0.011790 | 32.471013 | 105/107 |
| 02 | Velocity | 67.959083 | 83.426312 | 11.318179 | 0/107 |
| 02 | CT-ICP | 8.356215 | 2.312979 | 11.809493 | 98/107 |
| 05 | GT | 0.002852 | 0.011097 | 35.154042 | 106/107 |
| 05 | Velocity | 55.976082 | 99.994806 | 12.232815 | 0/107 |
| 05 | CT-ICP | 0.567577 | 1.321273 | 8.755050 | 104/107 |
| 08 | GT | 0.020716 | 0.056675 | 45.962452 | 99/107 |
| 08 | Velocity | 22.020911 | 88.921914 | 13.993790 | 0/107 |
| 08 | CT-ICP | 11.631854 | 32.101226 | 21.691205 | 1/107 |

## Scan Context Seed Check

Seed source is Scan Context top-1 place retrieval from the fixed-map scan DB.
Stride 1 includes every reference frame and is an oracle-like wiring smoke
test. Stride 5/10 are sparse-map checks.

| Sequence | Map stride | ATE [m] | RPE trans [%] | FPS | Accepted | SC hits | Mean SC distance |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 02 | 1 | 0.002575 | 0.011790 | 35.399635 | 105/107 | 107/107 | 0.0000 |
| 05 | 1 | 0.002852 | 0.011097 | 32.803283 | 106/107 | 107/107 | -0.0000 |
| 08 | 1 | 0.020716 | 0.056675 | 46.042556 | 99/107 | 107/107 | 0.0000 |
| 02 | 5 | 2.579330 | 3.817213 | 18.286097 | 98/107 | 41/107 | 0.0756 |
| 05 | 5 | 1.729595 | 3.925748 | 20.083523 | 103/107 | 58/107 | 0.0957 |
| 08 | 5 | 1.482620 | 8.798781 | 61.357197 | 59/107 | 37/107 | 0.0686 |
| 02 | 10 | 5.714361 | 7.932532 | 23.325425 | 98/107 | 21/107 | 0.0834 |
| 05 | 10 | 4.353178 | 6.568490 | 20.264291 | 98/107 | 26/107 | 0.0920 |
| 08 | 10 | 3.198984 | 18.308613 | 84.703600 | 30/107 | 19/107 | 0.0754 |

## Scan Context Top-K + NDT Rescoring

Filtered top-K keeps the Scan Context distance threshold at 0.18. It mostly
collapses back to the top-1 result because few sparse-map candidates pass the
threshold. Unfiltered top-K sets the threshold to 0 and lets NDT score choose
among all candidates; this is worse, which means NDT score alone is not a safe
global candidate selector.

| Sequence | Map stride | Top-K | Threshold | ATE [m] | RPE trans [%] | FPS | NDT candidate evals |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 02 | 5 | 5 | 0.18 | 2.627187 | 3.817213 | 18.160845 | 41 |
| 05 | 5 | 5 | 0.18 | 1.729596 | 3.925751 | 20.672483 | 58 |
| 08 | 5 | 5 | 0.18 | 1.503362 | 8.874181 | 53.282819 | 39 |
| 02 | 5 | 10 | 0.18 | 2.627187 | 3.817213 | 18.615474 | 41 |
| 05 | 5 | 10 | 0.18 | 1.729596 | 3.925751 | 20.192084 | 58 |
| 08 | 5 | 10 | 0.18 | 1.503362 | 8.874181 | 60.584155 | 39 |
| 02 | 10 | 5 | 0.18 | 5.714361 | 7.932532 | 23.287327 | 21 |
| 05 | 10 | 5 | 0.18 | 4.353178 | 6.568490 | 20.295504 | 26 |
| 08 | 10 | 5 | 0.18 | 3.938975 | 22.226139 | 93.490143 | 19 |
| 02 | 10 | 10 | 0.18 | 5.714361 | 7.932532 | 23.113832 | 21 |
| 05 | 10 | 10 | 0.18 | 4.353178 | 6.568490 | 20.561907 | 26 |
| 08 | 10 | 10 | 0.18 | 3.938975 | 22.226139 | 90.908652 | 19 |
| 02 | 5 | 5 | 0 | 33.491484 | 61.876153 | 3.997357 | 535 |
| 05 | 5 | 5 | 0 | 17.019407 | 45.503335 | 4.141273 | 535 |
| 08 | 5 | 5 | 0 | 3.901821 | 15.027487 | 25.869846 | 535 |
| 02 | 5 | 10 | 0 | 46.456756 | 90.005672 | 2.163810 | 1070 |
| 05 | 5 | 10 | 0 | 38.673947 | 92.938338 | 2.147296 | 1070 |
| 08 | 5 | 10 | 0 | 8.542529 | 30.906088 | 18.524126 | 1070 |
| 02 | 10 | 5 | 0 | 46.357718 | 85.905549 | 5.364486 | 535 |
| 05 | 10 | 5 | 0 | 23.095312 | 96.319662 | 4.916312 | 535 |
| 08 | 10 | 5 | 0 | 9.153156 | 56.963713 | 51.926106 | 535 |
| 02 | 10 | 10 | 0 | 53.369916 | 93.675518 | 2.736847 | 1070 |
| 05 | 10 | 10 | 0 | 53.797970 | 136.834989 | 2.530695 | 1070 |
| 08 | 10 | 10 | 0 | 13.407426 | 72.870297 | 36.887518 | 1070 |

## GT Perturbation Check

Seed source is GT with local-frame perturbation. This is a fixed-map basin
sanity check, not a production prior.

| Sequence | Perturbation | ATE [m] | RPE trans [%] | FPS |
| --- | --- | ---: | ---: | ---: |
| 02 | x=0.25m | 0.162598 | 0.231189 | 13.557666 |
| 02 | x=0.50m | 0.488097 | 0.692309 | 12.730725 |
| 02 | yaw=20deg | 0.049391 | 26.907662 | 13.364114 |
| 05 | x=0.25m | 0.157550 | 0.393286 | 14.375750 |
| 05 | x=0.50m | 0.482807 | 0.114922 | 13.380492 |
| 05 | yaw=20deg | 0.036739 | 34.129183 | 13.015659 |
| 08 | x=0.25m | 0.230769 | 0.956785 | 16.808856 |
| 08 | x=0.50m | 0.494619 | 2.078420 | 20.536798 |
| 08 | yaw=20deg | 0.145423 | 28.306570 | 24.972881 |

## Readout

The fixed-map path itself works: GT-seeded fixed-map NDT reaches mm-to-cm ATE
on seq02/05/08, and a 0.5m GT translation perturbation is still pulled back to
roughly 0.49m ATE on these 108-frame windows.

CT-ICP remains outside the useful initialization basin on seq02 and seq08.
Fixed-map NDT with CT-ICP seed improves seq02 only marginally
(8.381m online hybrid to 8.356m fixed-map), improves seq05 slightly
(0.583m CT-ICP prior to 0.568m), and does not rescue seq08.

This means the current blocker is not online-map drift anymore. It is the prior:
CT-ICP no-GT odometry must be made substantially more accurate, or a global
initializer must provide translation hypotheses closer to the fixed-map NDT
basin.

Scan Context is now wired as a seed source. With every reference frame in the
DB it recovers the GT-seeded fixed-map result, which validates the adapter. With
sparse map strides of 5/10, top-1 retrieval alone is not accurate enough for the
NDT basin. The next useful step is top-K Scan Context hypotheses plus NDT
rescoring, instead of committing to only the top-1 place candidate.

Top-K with thresholded Scan Context candidates does not materially improve the
sparse-map result; top-K without the Scan Context threshold is much worse. NDT
score alone is therefore not calibrated enough for global hypothesis selection.
The next initializer experiment needs either a stronger geometric verifier
(FPFH/RANSAC, TEASER++, or branch-and-bound) or a sequential place-recognition
prior, not just more Scan Context candidates.
