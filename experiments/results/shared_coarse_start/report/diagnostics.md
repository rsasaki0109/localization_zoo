| Seq | Coarse path / GT | Raw bad length steps | Coarse bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |
|---|---:|---:|---:|---|---:|---:|
| 00 | 0.996986 | 13/4514 | 13/4514 | 4200-4300 | 131.234 | 7.492 |
| 01 | 1.102829 | 134/1100 | 113/1100 | 800-900 | 193.123 | 160.248 |
| 02 | 1.024914 | 168/4660 | 144/4660 | 2300-2400 | 127.089 | 22.928 |
| 03 | 1.002720 | 1/785 | 2/785 | 600-700 | 83.563 | 2.070 |
| 04 | 1.001324 | 0/270 | 0/270 | 200-270 | 112.094 | 1.479 |
| 05 | 0.998717 | 0/2683 | 0/2683 | 2600-2700 | 102.422 | 3.497 |
| 06 | 1.001468 | 0/1100 | 0/1100 | 900-1000 | 127.875 | 3.467 |
| 07 | 0.997835 | 0/1009 | 0/1009 | 500-600 | 93.673 | 0.507 |
| 08 | 0.997403 | 23/4040 | 18/4040 | 0-100 | 50.648 | 17.318 |
| 09 | 0.999478 | 0/1590 | 0/1590 | 1300-1400 | 144.465 | 4.944 |
| 10 | 0.997566 | 0/1153 | 1/1153 | 700-800 | 127.161 | 2.691 |

Bad length steps use GT motion at least 0.1 m and length error above 20%.
Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.
Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.
These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.
