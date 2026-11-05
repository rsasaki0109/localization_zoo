# Protocol: ELO with the paper's 0.1 m single-vertex ground BEV

Committed before any full-sequence run of the candidate.

## Question

ELO's Table 6 setting, `dense_elevation_paper_normals_unbiased`, reaches 1.22x
the paper over 00-10. Its gap is spread over the sequences (0.84-1.43x).

One remaining difference from the paper is the ground bird's-eye-view model:

- the dense profile uses 0.2 m cells holding a running mean;
- the paper uses 0.1 m cells holding one vertex, the closest.

Before the three forward-bias fixes, the paper's BEV made the 07 window worse.
With the fixes in place, it no longer did: 0.383 → 0.373 % on frames 0-399 and
0.345 → 0.332 % on 400-799 ([ELO README](../../../papers/elo/README.md#forward-drift-biases-of-the-projective-model)).
It has never been run on full sequences.

Does the paper's BEV, added to the fixed Table 6 setting, change full-sequence
accuracy on every sequence with a paper value?

## Candidate

Table 6's arguments plus `--elo-bev-resolution 0.1 --elo-bev-keep-closest`.

- This moves the port towards the paper; it is not a new mechanism.
- The candidate was chosen from the committed 07 window checks. No value is
  tried here.
- Development sequence: 07. Held out: 00-06 and 08-10.

## Fixed comparison

- Sequences: 00-10.
- Arms (manifests `experiments/elo_kitti_seq_NN_full_paper_bev_matrix.json`):
  - `dense_elevation_paper_normals_unbiased`: the fresh reference;
  - `dense_elevation_paper_normals_unbiased_paper_bev`: the candidate.
- The candidate is marked as an ablation, so Table 6 cannot select it on its
  own.
- One binary, both arms concurrent with one OpenMP thread each, and every
  command recorded by `run_study.py`.

## Metrics and reading rules

- Primary: official KITTI translational RTE (100-800 m), no GT seed. Also
  reported: 100 m RPE, ATE, bad-length steps, and the scale-only and yaw-only
  channels.
- Reference check: the fresh reference is compared with the stored Table 6
  result. The stored runs may have used other thread counts, so a mismatch is
  reported with its size. Interpretation then uses the fresh reference.
- Report per sequence candidate/reference, and the geometric means over 00-10
  and over the held-out sequences.
- Any held-out sequence whose RTE rises by more than 5 % is listed as adverse,
  with its numbers.
- Whatever the result, defaults and Table 6 do not change here. Adopting the
  paper's BEV in Table 6 is the owner's decision.
