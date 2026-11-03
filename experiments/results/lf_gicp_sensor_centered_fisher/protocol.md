# Protocol: LF-GICP sensor-centred Fisher direction on full KITTI 00-10

Written and committed before any full-sequence run of this study.

## Why

LF-GICP `paper_default` scores 5.616 % official KITTI RTE on sequence 10
against the paper's 0.837 % (6.71x); every other sequence is within
0.65-1.28x. On this host, before writing this protocol, three runs of sequence
10 were made with the same binary
(`build/evaluation/pcd_dogfooding`, SHA-256
`7cbd8e0d737d6a7cb6d0e326a5ccb16ae02d11286d4d34543e228f50a7e50d2c`):

| arm | official RTE [%] | ATE [m] | gated frames |
|---|---:|---:|---:|
| `paper_default` | 5.616436 | 50.981598 | 429 / 1201 |
| `--lf-gicp-sensor-centered-fisher` | 0.993400 | 13.431052 | 446 / 1201 |
| `--lf-gicp-no-mitigation` | 0.830684 | 6.686339 | 0 / 1201 |

The `paper_default` rerun equals the committed result (5.616436 %, ATE
50.981598 m). Summaries, logs and the passive per-frame traces are in
[`development_seq10/`](development_seq10/); `analyze_trace.py` turns a trace
into the per-block table.

Mechanism observed in the traces: the paper linearizes about the world origin
(Eq. 2, `J = [-[Rp + t]x, I]`) and Eq. (7) weights each correspondence by its
information along the weakest eigenvector of that Hessian. A rotation about the
sensor written in the world twist needs a translation part `t x omega`, so its
twist norm grows with the distance from the origin and it becomes the weakest
direction. In frames 0-99 (mean 26 m from the origin) the weakest world
direction is a translation in 98 % of frames; from frame 100 on (110-660 m) it
is a rotation about the sensor in 92-100 % of frames, while the sensor-centred
weakest direction remains a translation in 83-100 %. With the paper form the
largest applied weight reaches 31-119 and half the total weight sits on 9-14 %
of the correspondences (sensor-centred: 11-25 and 28-32 %). The drift blocks
200-400 of the committed diagnostics lie inside the gated frames 110-330 m from
the origin.

Sequence 10 is therefore development data for this study. No sequence is held
out: KITTI 00-10 are all project development data (see the roadmap).

## Fixed setting

One Boolean, no numeric parameter: `--lf-gicp-sensor-centered-fisher`. The
Fisher weights use the weakest eigenvector of `A^T H A`, `A = [[I, 0], [[t]x, I]]`
with `t` the current sensor position, mapped back to the world twist. Gate,
field, weights normalization, Huber, solver parametrization, map and every
paper constant are unchanged. The option is outside the paper's literal
equations, opt-in and default off.

## Arms and sequences

Manifests `experiments/lf_gicp_kitti_seq_NN_full_sensor_fisher_matrix.json`,
NN = 00-10, each with three arms run on the same binary:

- `paper_default`: no arguments (fresh reference);
- `sensor_centered_fisher`: `--lf-gicp-sensor-centered-fisher`;
- `no_mitigation`: `--lf-gicp-no-mitigation` (paper's vanilla VGICP ablation).

`run_study.py` fetches each sequence, records scan hashes, runs the three arms
concurrently with one OpenMP thread each in separate working directories,
stages all run artifacts with saved-pose step diagnostics, re-checks the hashes
and deletes the scans. Every command and exit code goes to the study record.

## What is reported

For every arm and sequence: official KITTI RTE (translation and rotation),
100 m RPE, ATE, frame count, gated frames, FPS; the ratio to the paper's
Table I LF-GICP value; the geometric mean over 00-10; and the counts of
sequences where the candidate is lower / higher / equal relative to the fresh
reference. `no_mitigation` is also compared with the paper's VGICP column.
Every adverse sequence is listed with numbers.

Checks: the fresh `paper_default` must equal the committed historical
`paper_default` result on every sequence (official RTE, 100 m RPE, ATE, frame
count). Any difference is reported, not hidden.

## Decision rules fixed now

- Defaults do not change. Table 6 and `repo_variants` stay on `paper_default`
  whatever the outcome; promoting the opt-in is left to the owner.
- No other value or option is tried in this study. If the candidate is worse
  on some sequences, those numbers are reported as they are.
