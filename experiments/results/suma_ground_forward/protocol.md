# Protocol: SuMa ground pixels without forward information

Committed before any full-sequence run of the candidate on 00 or 03-10.

## Question

With the official defaults on raw scans (`paper_odometry`), SuMaPaper fails on
two KITTI sequences: 01 at 13.793 % against 1.7 % in the paper (8.1x), and 02 at
3.512 % against 1.1 % (3.2x). The method README diagnoses 02:

- in corridors the forward direction is nearly unconstrained;
- 73 % of the forward gradient at the GT increment comes from ground pixels;
- the normals of those pixels tilt about 1.3 deg towards the sensor, so their
  small biased forward term sets the step length.

Table 6 therefore takes a second variant on 01/02 (normalised lookup plus a
0.22 deg correction). That second variant costs on the other sequences.
InTEn-LOAM's 02 collapse had the same cause (`../inten_ground_forward/`).

Does one setting that removes the ground pixels' forward information fix 01/02
without costing on the other sequences?

## Candidate (not in the paper, opt-in, default off)

`--suma-paper-set ground_no_forward_vertical_cos=0.866`.

- Ground-like pixels are those whose model normal lies within 30 deg of the
  sensor's vertical axis.
- These pixels give no information along the sensor's forward axis: the
  forward component of their translation Jacobian is removed in
  sensor-centred coordinates (`SuMaPaper::accumulate`).
- They keep height, roll, pitch, lateral and yaw information. All other
  pixels and every gate, lookup and map rule are unchanged, including the
  official GL_LINEAR-style lookup and raw scans without vertical-angle
  correction.
- The 30 deg bound was fixed before any run. No other value was tried.

## How the setting was checked (development)

Development sequences: 01 and 02. Full runs on one binary
([`development/`](development/)), official KITTI RTE:

| Seq | `paper_odometry` | + ground without forward | Paper |
|---|---:|---:|---:|
| 01 | 13.793 % | 1.629 % | 1.7 % |
| 02 | 3.512 % | 1.558 % | 1.1 % |

## Fixed comparison

- Sequences: 00-10, every sequence with a paper value. 01 and 02 are rerun with
  the study binary.
- Arms (manifests `experiments/suma_kitti_seq_NN_full_ground_forward_matrix.json`):
  - `paper_odometry` (`--no-gt-seed --suma-paper`): the fresh reference;
  - `paper_odometry_ground_no_forward`: the same arguments plus
    `--suma-paper-set ground_no_forward_vertical_cos=0.866`.
- One binary for every run. Both arms of a sequence run concurrently with one
  thread each; FPS is not compared.
- Every command, exit code, input hash and timing is recorded by `run_study.py`.

## Metrics and reading rules

- Primary: official KITTI translational RTE (100-800 m), no GT seed.
- Also reported: 100 m RPE, ATE, bad-length steps.
- Reference check: the fresh reference must equal the stored `paper_odometry`
  result on every sequence; a mismatch is reported.
- For each sequence the candidate/reference ratio is reported, and over the 11
  sequences its geometric mean.
- Also reported: the geometric mean against the paper for both arms and for the
  current Table 6 (best of two variants per sequence), and the sequences above
  twice the paper.
- The held-out reading uses 00 and 03-10 only. It reports which of these are
  lower and which are higher.
- Any sequence whose RTE rises by more than 5 % is listed as adverse, with its
  numbers.
- Whatever the result, defaults, Table 6 and stored results do not change.
  The candidate is an ablation variant, so Table 6 cannot select it. Pinning
  SuMa to one setting is the owner's decision.
