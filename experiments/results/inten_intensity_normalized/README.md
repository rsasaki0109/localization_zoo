# Normalized occupied-node intensity: motion gate fails

Normalizing occupied cubic weights removes the legacy stencil-mean jumps on
the positive-weight domain, but does not fix the inherited motion failures.
The complete conditional comparison is negative: bad-length endpoints increase
from 7 to 8, and total length, vector and rotation errors all increase. No full
trajectory trial, accuracy improvement or paper-ratio claim follows.

The source/protocol commits `c0da32e0` and `77c9c129` precede their respective
native executions. Initial execution and failure are preserved in `84c92573`;
complete stable raw observations and reconstructed numbers are committed in
`3055bb2e` before this interpretation. No setting or gate was selected after
seeing the motion outcomes.

## Construction and scope

`--inten-loam-paper-set s2s_intensity_normalized=1`, together with
`--inten-loam-paper-odometry`, enables one repository extension, default off.
It changes only the S2S intensity field:

`I(x) = sum(B_i(x) occupied_i mean_i) / sum(B_i(x) occupied_i)`.

Occupancy and per-node means are fixed at build time. The gradient uses the
quotient rule, including the denominator derivative and inverse knot spacing.
This is a rational field, **not the unchanged cubic field or a paper-specified
sparse completion**. The original count requirement remains: at least four
observed controls. Positive denominator and finite value/gradient are also
required; zero occupied weight is invalid, with no epsilon or extrapolation.
S2M, feature extraction, weights, knot spacing, losses and gates stay unchanged.
The default legacy field and previous scientific results are preserved.

The initial implementation lost numerical precision near a common vanishing
basis factor. The initial independent polynomial checker also suffered
cancellation there and exited 1. All initial binaries, sources, inputs, outputs,
receipts and full failure are retained. The stable implementation cancels a
shared **positive** axis factor analytically before division, and keeps its
exact zero invalid. Independent reconstruction uses nonnegative Bernstein
polynomials. This changes neither the rational field nor its thresholds.
Every case and query was rerun; no favorable subset or new intensity scale.

## Native field and motion outcomes

All 31 inherited endpoints remain: eight original conditional reference
histories and 23 actual rejected recursive-candidate histories from 01/02/08.
All four native arms run at every endpoint: legacy/normalized joint and
legacy/normalized consensus. All 124 calls exit 0; all 62 legacy whole traces,
profiles, masks, votes and outputs reproduce the parent exactly.

The same 8829 reflector paths retain all 208565 real native queries. Every value,
gradient and one-sided limit reconstructs independently; all 26487 interior
finite-difference controls pass. There are 15758 both-valid positive-weight
faces, with no jumps above 1e-8 (maximum 1.110223e-16; maximum gradient jump
3.618455e-15). **This does not establish continuity through coverage holes.**
Of 155 faces with zero limiting occupied weight, five have valid neighboring
queries on both sides but different limiting intensities, maximum 0.169819977.
All five are listed in [cases.md](cases.md). Neighbor coverage also includes
184 mixed and 191 neither-valid pairs; those categories overlap zero-weight
limits. Zero-weight holes are explicitly
outside the prospectively declared positive-denominator continuity domain.

| Complete conditional set | Legacy joint | Normalized joint |
|---|---:|---:|
| Bad-length endpoints (>20% error) | 7 | 8 |
| Bad-vector endpoints (>20% error) | 14 | 14 |
| Sum absolute length-ratio error | 4.741213404 | 5.121149401 |
| Sum normalized vector error | 14.083975645 | 14.443564156 |
| Sum rotation error [rad] | 0.059478496 | 0.059793620 |

These sums are predeclared equal-weight feasibility controls, not sequence
RTE/ATE, trajectory errors or an independent generalization estimate.

Adverse cases are material: actual candidate08/39 becomes newly bad in length,
0.801801894 to 0.796756084 of GT step length. Original01/247 was already bad and
worsens from 0.522727335 to 0.105443532; vector error increases 0.477580491 to
0.895198356 and rotation error 8.113069873 to 12.688331673 mrad. Some cases improve:
original204 length changes 0.607064437 to 0.649309578, but remains bad;205 changes
only 0.372445688 to 0.373229196. The old rescued outputs remain non-bad with
normalized consensus, but that rejected rule's actual02/1892 collapse remains
(0.125854888 to 0.117134244), while the corresponding normalized joint is
1.005968963. Smoothness on covered points does not make that refinement safe.

[cases.md](cases.md) retains every endpoint's four-arm length/vector errors,
rotation errors, bad labels, and every field's positive/zero/invalid coverage.
[report/summary.json](report/summary.json) contains every frozen gate check;
[report/boundaries.json.gz](report/boundaries.json.gz) contains every crossed face.
No gate was relaxed. The construction is not adopted as a collapse remedy and
no fresh full KITTI run is triggered by this failed feasibility gate.

## Provenance and verification

All arms inherit the same profile07 settings, plane check, acceleration, mapped
prediction, translation prior0.03 and ground/facade PCA. The cached replay
changes only extracted-frame and estimated-initial-seed inputs; production
solver equations are unmodified. GT is read only by offline scoring. The primary
pair is normalized joint versus legacy joint; consensus arms are retained
rescue/adverse controls, never per-case alternatives.

The new construction was selected analytically from the committed sparse-field
defect, without a new sequence sweep. Original profile07, diagnosis00, inherited
DiLO01 prior choice, and every later KITTI observation informed development;
there is no held-out claim. No raw scan was acquired or deleted. Feature caches,
source, binary, object and linked-library hashes remain with complete commands.

The actual build/collection/reconstruction subprocess commands are:

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_normalized/build.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_normalized/run.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_normalized/verify.py --output /workspace/localization-env/inten-intensity-normalized-20261022/stable/analysis_stage/summary.json --details /workspace/localization-env/inten-intensity-normalized-20261022/stable/analysis_stage/boundaries.json.gz
```

Full argv/cwd/source hashes/environment, real exit codes, full outputs and
byte-roundtrip receipts are in [checks](checks), [groups](groups), [fields](fields)
and [attempts/initial](attempts/initial). Build/run require the configured existing
container, exact caches and a fresh private stage; they refuse to overwrite
previous producers. Portable reconstruction/tests use committed evidence alone:

```bash
python3 experiments/results/inten_intensity_normalized/verify.py --output /tmp/inten-normalized-check.json --details /tmp/inten-normalized-details.json.gz
python3 -m unittest discover -s tests -p test_inten_intensity_normalized.py -v
```

All 48 native mechanism/regression tests pass. Numerical reconstruction exit 0
means the evidence is consistent; the estimator progression gate is **false**.
The initial failure and its original source snapshot are independently checked.
Frozen source/header snapshots also keep older audits reproducible as production
evolves, without changing their recorded numbers or weakening their byte checks.
No accuracy aggregate is imported; Table6, paper metadata and existing full-run
tables remain unchanged. Full Native/ROS2 CI must pass at the final PR head.
