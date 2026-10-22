# Native intensity-field continuity audit

The legacy query-local mean fill is discontinuous across valid cubic cells.
This is an observed representation defect, not yet a proven cause of trajectory
collapse or evidence that any proposed replacement improves odometry.
Production source, defaults, existing flags and historical results are unchanged.

Protocol/source `ca931064` precedes every native query. Raw native evidence
`49840c6c` and reconstructed results `63951376` precede this interpretation.
The first independent checker exited1 on an empty invalid-gradient array; its
complete failure and the successful corrected reconstruction are retained.
No measurement was repeated or discarded because of its outcome.

## What was measured

The public native `InTenLoamPaper::IntensityGrid` is compiled from the untouched
production translation unit. The passive probe calls `build(previous.all, 0.3)`
with exactly the production S2S index inputs and knot, original feature order and
float intensities. It does not run an optimizer, use GT, change poses or modify
any registration path. It emits lattice-node sums/counts and native values and
gradients, with complete argv/cwd, actual exits, source and binary/object/library
hashes, compiler and dynamic-link information.

All31 inherited endpoints are retained: the original8 conditional reference
histories and the23 actual rejected recursive-candidate01/02/08 histories. For
all8829 reflector primitives originally valid at the joint pose, the audited
straight path translates that query by old-consensus proposal minus joint,
keeping rotation fixed. This is conditional support, not all raw points or all
possible optimizer paths. Each integer cell face strictly crossed on each axis
is tested with lattice offsets1e-3,1e-6,1e-9 and adjacent representable coordinates.
Native cell indices are checked on both sides. Equal-endpoint fallbacks remain
as zero-crossing controls. Source-position rounding at a face is handled by at
most16 extra `nextafter` steps, never by changing the cell definition.

Every native value and gradient is independently reconstructed from tensor
polynomials and archived lattice controls. Every boundary's exact left/right
cell-polynomial limit is compared with native ULP-neighbor values. All17658
endpoint values/coverage and original reflector gradients reproduce the parent
observations within the declared portable tolerance. A central difference at
fixed fractional cell position0.375 uses step1e-5 times knot and never crosses a
cell boundary. Tolerances were frozen before execution; 1e-8 is a reporting
cutoff for jumps, not an estimator threshold or tuned acceptance gate.

## Results and negative controls

- All208565 real-field native samples reconstruct. All26487 within-cell
  derivative controls pass; maximum absolute derivative error is2.912435e-10.
- Of16138 crossed faces,15763 have valid support on both sides,184 on one side
  and191 on neither side. Invalid sides remain explicitly invalid, never zero
  or silently dropped. Of the valid pairs,15428 exceed an absolute intensity
  jump of1e-8; the largest is0.163630951 at original01/205.
- The sparse nonconstant synthetic control has8 occupied nodes on each side
  and a limiting jump0.023746193 on each axis. Dense-ramp and sparse-constant
  controls stay continuous (maximum observed absolute limit difference across
  those controls5.551115e-17). Three-node controls remain invalid.
- Actual candidate02/1892, the previously fully supported false acceptance,
  crosses1144 faces on its original reflector paths:1142 valid pairs,
 1141 above1e-8, maximum0.072155544. This does not establish how much of its
  objective change or motion error is caused by these discontinuities.
- Original01/204 and205, whose old rescues the rejected veto lost, also have
  discontinuities. Zero-displacement original247/433 and actual01/425 are
  retained; their zero crossings do not imply the corresponding field is smooth.

All cases, including neutral controls and support changes:
[cases.md](cases.md). Full per-face limits, gradients, support counts, means and
ULP differences: [report/boundaries.json.gz](report/boundaries.json.gz).
Machine-readable counts and every case's largest witness:
[report/summary.json](report/summary.json).

## Mechanism and limits

Inside one cell, the mean-filled controls are constant and the implemented
analytic gradient is correct; a missing derivative of the within-cell mean is
not the issue. When the cell changes, the mean of occupied stencil nodes can
change. Shared missing lattice nodes then receive different controls. At a cell
face, the departing/entering outer control planes have zero basis weight, but
the shared missing controls have nonzero weight. Their contribution to the
right-minus-left limit is the missing basis mass times the change in stencil
mean. Thus even two valid stencils do not define one globally consistent cubic
field. Merely fixing coverage, gradient scaling or taking smaller within-cell
steps does not establish continuity.

The paper's cubic mechanism motivates the continuity invariant. This does not
establish which sparse-hole construction the paper or its implementation used.
A new consistent-control construction would be an explicitly described repository
extension, opt-in/default-off; the legacy fill and old results must remain.
Before any full fixed-setting KITTI00-10 trial, freeze that construction and its
coverage policy, check these native invariants and all inherited rescue/adverse
controls. Do not relax the rejected consensus or support-veto gates. No new
RTE, paper ratio, speed, physical moving-object or generalization claim follows.

Original profile07, diagnosis00, inherited DiLO01 prior0.03, and ALL later KITTI
observations are development evidence. No held-out claim. No raw scan is acquired
or deleted here; complete retained feature caches are SHA-verified against the
committed parent receipts. Closed immutable evidence is hardlinked only after
byte/blob/hash/mode/ownership/device checks, with every logical path retained;
source, builds, binaries, libraries and indexes remain separate.

## Commands and verification

From the repository root, in the existing configured development environment:

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_continuity/build.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_continuity/run.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_continuity/verify.py --output /tmp/inten-continuity-check.json
/workspace/localization-env/host-venv/bin/python -m unittest discover -s tests -p test_inten_intensity_continuity.py -v
```

Build/run create fresh private stage paths and require a clean frozen checkout;
they intentionally refuse to overwrite retained producers. `run.py` also needs
the exact private full-feature caches named in the parent receipts; restore them
from the parent archives if absent. `verify.py` and the regression tests use
committed evidence alone, check every archive byte, cross-check the parent inputs
and recompute all native observations and complete recorded reports.

[protocol.json](protocol.json) freezes the source, scope and tolerances;
[checks/build](checks/build) retains real compile exits and binary/library hashes;
[groups](groups) retains all native queries, input paths, parent-control values
and actual command exits. Local validation logs retain full Python regression,
document checks and byte-identical regeneration of the published fixed-variant
and paper-ratio tables. Full Native and ROS2 CI are required on the PR's exact
final head before Ready. No accuracy aggregate was imported, so no published
aggregate, Table6, paper metadata or old experiment was regenerated in place.
