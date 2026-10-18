| Seq | Remap path / GT | Raw bad length steps | Remap bad length steps | Largest growth pose interval | GT travel [m] | Position-error increase [m] |
|---|---:|---:|---:|---|---:|---:|
| 00 | 0.996997 | 13/4514 | 13/4514 | 4200-4300 | 131.234 | 7.225 |
| 01 | 0.031934 | 134/1100 | 1100/1100 | 600-700 | 269.598 | 268.474 |
| 02 | 0.741858 | 168/4660 | 2040/4660 | 700-800 | 127.784 | 108.485 |
| 03 | 1.017108 | 1/785 | 218/785 | 500-600 | 76.797 | 14.231 |
| 04 | 0.025261 | 0/270 | 270/270 | 100-200 | 144.369 | 142.520 |

Bad length steps use GT motion at least 0.1 m and length error above 20%.
Pose intervals use zero-based indices and the nonoverlapping blocks from block_frames=100.
Growth is the positive end-minus-start pointwise position-error change, with no pose refit or rescaling.
These saved-pose diagnostics are distinct from official KITTI RTE and do not establish a solver-level cause.
