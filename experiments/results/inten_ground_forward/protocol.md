# Protocol: InTEn-LOAM S2S ground without forward information

Committed before any full-sequence run of the candidate on 00 or 03-10.

## Question

InTEn-LOAM's KITTI 02 gap (3.221 % vs 0.54 %, 5.97x) comes from steps that are
already too short at the S2S output. At the 02 collapse entries 1876 and 4460
the GT-seeded S2S system has no edge support, and the ground class gives a
backward first-order correction of 3.7-4.3 % of the GT step
([seed interventions](../inten_seed_intervention/README.md)). Flat ground holds
little true forward information. Its many residuals can still add a small
biased forward term, and this term sets the step length where nothing else
constrains it. ELO uses the same idea as an opt-in fix (`ground_constrains_forward`).

Does S2S give better full-sequence accuracy on every KITTI sequence with a
paper value when its ground residuals give no forward information?

## Candidate (not in the paper, opt-in, default off)

`--inten-loam-paper-set s2s_ground_no_forward=1`. For S2S ground residuals, the
translation Jacobian loses its component along the sensor's forward axis in
sensor-centred tangent coordinates (`papers/inten_loam/src/inten_loam_paper.cpp`,
`System::add`). Height, roll, pitch, lateral and yaw information of the ground
is kept. Edges, facades, reflectors, S2M and all gates are unchanged.

## How the setting was chosen (development)

- Development sequences: 01 and 02, chosen because their collapse motivated the
  change.
- Arms: S2S only, or S2S and S2M. Full runs on one binary
  ([`development/`](development/)); official KITTI RTE:

  | Seq | Reference | S2S only | S2S + S2M |
  |---|---:|---:|---:|
  | 01 | 16.614 % | 16.636 % | 16.708 % |
  | 02 | 3.221 % | 0.823 % | 0.855 % |

- S2S only is chosen: it is lower on both sequences and is the smaller change.
- No other value was tried. 01 and 02 are development data and stay labelled
  so in every table.

## Fixed comparison

- Sequences: 00-10, every sequence with a paper value. 01 and 02 are rerun with
  the study binary.
- Arms (manifests `experiments/inten_loam_kitti_seq_NN_full_ground_forward_matrix.json`):
  - `paper_odometry_plane_check`: the unchanged Table 6 setting, rerun as the
    fresh reference;
  - `paper_odometry_plane_check_ground_no_forward`: the same arguments plus
    `--inten-loam-paper-set s2s_ground_no_forward=1`.
- One binary for every run. Both arms of a sequence run concurrently with
  one OpenMP thread each. FPS is therefore not compared.
- Every command, exit code, input hash and timing is recorded by `run_study.py`.

## Metrics and reading rules

- Primary: official KITTI translational RTE (100-800 m), no GT seed.
- Also reported: 100 m RPE, ATE, bad-length steps (`diagnose_kitti_steps.py`).
- Reference check: the fresh reference must equal the stored Table 6 result on
  every sequence. A mismatch is reported and stops interpretation for that
  sequence.
- For each sequence the candidate/reference RTE ratio is reported, and over
  the 11 sequences its geometric mean.
- Also reported: the geometric mean against the paper for both arms, and the
  sequences above twice the paper.
- The held-out reading uses 00 and 03-10 only, since 01/02 chose the setting.
  It reports which of these sequences are lower and which are higher.
- Any sequence whose RTE rises by more than 5 % is listed as adverse, with its
  numbers.
- Whatever the result, defaults, Table 6 (`repo_variants`) and stored results
  do not change. Promotion is the owner's decision.
