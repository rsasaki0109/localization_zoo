# Protocol: L-LO ground-height-map vertical mode on full KITTI 00/02-10

Written and committed before any full-sequence run of this study.

## Why

The [vertical-channel study](../l_lo_vertical_decomposition/README.md) found
that L-LO's 09/10 gap is vertical: with GT height, pitch and roll the horizontal
error alone is below the paper. The existing frame-to-frame vertical modes drift
in altitude. `pitch_mode=3` (commit `bf851497`, opt-in, **not in the paper**)
estimates height, pitch and roll from the current ground band against a
world-frame ground-height map of the last 30 frames; x, y and yaw are unchanged.

Its constants were fixed in the code before any KITTI run, and none was chosen
on an evaluated error:

- map window: 30 frames;
- cell: 0.5 m;
- Huber: 0.1 m;
- 5 Gauss-Newton iterations;
- at least 50 residuals;
- ground band: the existing `pitch_ground_min/max_range`, 5-25 m.

## Development observation (KITTI 10, before this protocol)

Same binary (`build/evaluation/pcd_dogfooding`, SHA-256
`f11f6032619734bcfe05255ed9d9b4a9e4636cdd9133a984ddfa2addb43ee4d8`), one run each:

| Arm | Official RTE [%] | Horizontal-only | Vertical-only | Tilt-only | ATE [m] |
|---|---:|---:|---:|---:|---:|
| `seq07_tuned` | 4.105241 | 1.536 | 3.558 | 3.414 | 8.63 |
| `seq07_tuned` + `pitch_mode=3` | 2.345403 | 1.603 | 1.641 | 1.665 | 16.94 |

The paper reports 1.90 % on 10. The reference reproduces the stored Table 6 value
(4.105241 %). The candidate still drifts in altitude: estimated height range
33.6 m against 23.8 m in GT. Sequence 10 is development data; no value was
changed after this observation.

## Fixed setting and arms

The manifests are `experiments/l_lo_kitti_seq_NN_full_ground_map_matrix.json`
for NN = 00, 02-10. Sequence 01 is NA in the paper. Each manifest has two arms,
run on the same binary:

- `seq07_tuned`: the Table 6 setting, as a fresh reference;
- `seq07_tuned_ground_map`: the same, plus `--l-lo-set pitch_mode=3`.

`run_study.py` fetches one sequence at a time and records scan hashes before and
after. It runs both arms concurrently, one OpenMP thread each, in separate
working directories. It stages summaries, logs, saved poses and step
diagnostics, and deletes the scans afterwards.

## Reported

For each arm and sequence:

- official KITTI RTE, ratio to the paper's Table I L-LO value, 100 m RPE, ATE,
  frames;
- the horizontal-only and vertical-only channels (as in
  `../gap_decomposition/decompose_table6.py`).

Across sequences:

- the geometric mean over 00/02-10 for both arms;
- how many sequences the candidate makes lower, higher or equal relative to the
  fresh reference, with every adverse sequence listed;
- whether the fresh reference equals the stored Table 6 value on every sequence.

## Fixed now

- Defaults do not change. Table 6 and `repo_variants` stay on `seq07_tuned`
  whatever the outcome; promoting the opt-in is left to the owner.
- No other value or option is tried in this study.
