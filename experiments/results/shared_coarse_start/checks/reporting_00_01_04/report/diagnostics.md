| Seq | Coarse path / GT | Raw bad length steps | Coarse bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |
|---|---:|---:|---:|---|---:|---:|
| 00 | 0.996986 | 13/4514 | 13/4514 | 4200-4300 | 131.234 | 7.492 |
| 01 | 1.102829 | 134/1100 | 113/1100 | 800-900 | 193.123 | 160.248 |
| 04 | 1.001324 | 0/270 | 0/270 | 200-270 | 112.094 | 1.479 |

Bad length steps use GT motion at least 0.1 m and length error above 20%.
Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.
Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.
These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.
