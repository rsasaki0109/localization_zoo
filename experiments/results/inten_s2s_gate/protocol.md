# Protocol: InTEn-LOAM 5 m S2S gate with ground without forward

Committed before any full-sequence run of the candidate on 00 or 03-10.

## Question

KITTI 01 is InTEn-LOAM's largest gap: 16.614 % against 0.64 % (25.96x).

- With a GT seed, S2S returns a good step on all but one of 155 inspected pairs
  ([seed interventions](../inten_seed_intervention/README.md)).
- With the natural seed, the step shrinks from frame to frame once it is a few
  percent short. At 215, a seed 0.953 of GT gives an output of 0.793.
- The correct solution therefore has a narrow basin along the forward axis.

S2S keeps a nearest neighbour only within `s2s_gate_sq = 1` (1 m). The paper
leaves this value open. At highway speed (about 2.5 m per scan), a short seed
moves many points out of that gate. The LOAM family uses 5 m (A-LOAM
`DISTANCE_SQ_THRESHOLD = 25`).

Question: does a 5 m S2S gate, together with the merged S2S
ground-without-forward flag, improve full-sequence accuracy on every sequence
with a paper value?

## Candidate (not in the paper, opt-in)

```
--inten-loam-paper-set s2s_gate_sq=25 --inten-loam-paper-set s2s_ground_no_forward=1
```

- `s2s_gate_sq=25` is the LOAM-family value. It was fixed before any run.
- `s2s_ground_no_forward` comes unchanged from `../inten_ground_forward/`.
- Every other argument is that of Table 6's `paper_odometry_plane_check`.

## Development (01 and 02)

Records are in [`development/`](development/). Official KITTI RTE:

| Seq | Reference | 5 m gate | 5 m gate + ground | Ground only (#187) |
|---|---:|---:|---:|---:|
| 01 | 16.614 % | 11.040 % | 9.384 % | 16.636 % |
| 02 | 3.221 % | 3.083 % | 0.811 % | 0.823 % |

After this, gates of 10 m and 20 m (with ground) were run on 01 only, to see
whether the effect saturates: 9.084 % and 9.049 %. They do not change the
choice. The candidate stays at the LOAM value fixed beforehand.

## Fixed comparison

- Sequences: 00-10, every sequence with a paper value. 01 and 02 are rerun.
- Arms (manifests `experiments/inten_loam_kitti_seq_NN_full_s2s_gate_matrix.json`):
  - `paper_odometry_plane_check`: the fresh reference;
  - `paper_odometry_plane_check_gate25_ground_no_forward`: the candidate.
- One binary, both arms concurrent with one OpenMP thread each, and every
  command recorded by `run_study.py`.

## Metrics and reading rules

- Primary: official KITTI translational RTE (100-800 m), no GT seed. Also
  reported: 100 m RPE, ATE and bad-length steps.
- Reference check: the fresh reference must equal the stored Table 6 result.
- Per sequence, report candidate/reference, and candidate against the
  ground-only results of `../inten_ground_forward/`. Report the geometric
  means over 00-10 and over the held-out 00 and 03-10, and the sequences above
  twice the paper.
- Any held-out sequence whose RTE rises by more than 5 % against the
  reference is listed as adverse, with its numbers.
- Whatever the result, defaults and Table 6 do not change. Promotion is the
  owner's decision.
