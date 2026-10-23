# Which valid constraints reciprocity removes

Reciprocity removes native-valid geometry in both rescued and adverse cases.
The lost signed constraints differ: rescued202/204 lose a backward facade
component, while adverse08 loses a forward facade component. Original247's
late filtered edge component even reverses sign relative to the unfiltered
query at that same pose. A retained-information fraction alone does not
separate recovery from collapse. These are passive equation measurements;
no estimator has improved and the earlier reciprocal/prior candidates remain
rejected.

Complete numerical verification `a2fd354d` and
[all cases and feature contributions](cases.md) `dcfef100` precede this
interpretation. Every inherited endpoint, all earlier arms and both signs
remain. Nothing measured here enters optimization or candidate selection.

## Construction and settings

Source/protocol freeze `31daf928` precedes execution. All31 inherited endpoints
rerun all18 native arms:558 outcomes, every complete original stdout byte,
feature, mask and profile record exactly reproduces the committed prior study.
Four native processes exit0. The passive observer runs at every joint and
executed refinement iteration of unfiltered100 arms6/7 and fixed-reference
reciprocal100 arms14/15. At each actual pose it finishes the original native
query with reciprocity temporarily disabled, then partitions valid rows by the
original reciprocal test. Scratch state is private; actual solver systems,
updates, priors, stopping, consensus and mapping stay unchanged.

Original07/00 geometry, inherited DiLO01 prior0.03, cap100 chosen on original01,
legacy intensity field, native Huber and all gates remain fixed across cases.
Every previously observed KITTI sequence is development; there is no held-out
or SOTA claim. No GT, outcome label, new scans, hand region, threshold, budget,
field or per-case setting enters the native observer.

## Measured support loss and its limits

The complete reconstruction checks42,992 class observations and27,360,255
source-row queries, including repeated observations across arms/iterations.
It distinguishes3,637,657 removed **valid** residuals from8,529,472 reciprocal
rejects that would fail later native gates anyway. These are query counts,
not unique scene-point populations. Every forward population, validity gate,
backward partition, Huber/Jacobian, class H/g/cost, additive partition and actual
solver H/g/count reconstructs at original componentwise rtol1e-10/atol1e-12.

The table uses predeclared arm14's initial pose. Length/GT is the unchanged
parent conditional terminal score, not a new trajectory result. The axis is
that solver's estimated input translation in target coordinates. Longitudinal
information is the centered translation Schur projection, using the inherited
rotation-block regularization1e-6; its all/kept comparison includes the unchanged
intensity class. Native intensity/geometric units are not calibrated physical
covariance.

| Case | Unchanged length/GT | Initial valid geometry removed/all | Initial kept/all Schur | Last kept/all Schur |
|---|---:|---:|---:|---:|
| old/202, rescue |0.973614185|355/617|0.569157979|0.566546583|
| old/204, rescue |0.985996689|356/648|0.595870322|0.639933306|
| old/247, lost recovery |0.459890548|238/472|0.637155545|0.516937248|
| 08/39, newly bad length |0.682142975|319/848|0.716923510|0.722008553|
| 08/45, newly bad length |0.776746136|408/943|0.692297857|0.693850841|
| 02/1891, still collapsed |0.195253604|303/622|0.624142843|0.634853027|

Adverse247/08 can retain a larger initial fraction than rescued202/204. Strong
remaining information does not establish a correct association or motion.
The complete table retains every other case and both consensus histories,
including explicitly unexecuted refinements; every intermediate query and
information spectrum remains in the raw archives and complete report.

Raw axis-gradient projections reveal a different loss direction. At initial
202, facade gt changes0.888587984→-0.219212193 after filtering;204 changes
0.671037122→-0.403371475. At08/39 it changes-5.95953280→-0.209317804, and08/45
-5.42782305→-1.24928672. With rotation fixed, positive gt opposes forward
translation and negative gt favors it under the native minus-gradient update.
Thus the same predicate removes an opposing component in the rescue examples
and a forward component in the adverse examples. These are raw class terms,
not independent solved motion contributions: rotation, other classes and later
associations remain coupled.

The corresponding initial removed axis Htt at08/39 is30.6170672 edge,
0.0211861773 ground and33.5959532 facade;08/45 is34.4412377,0.0221400446 and
47.5407529. Ground loss is small along this particular axis despite dropped
valid ground rows. No intensity row is removed in these examples. At247's
last filtered iteration, the same-pose edge gt changes-0.745024029→0.203276439;
removed edge/facade axis Htt is6.20848883/6.09370992. Earlier unfiltered247 ends
with all/kept Schur49.1868138/27.5625572 at its own final pose, whereas filtered
247 ends23.5822150/12.1905253 at its different final pose. This is evidence of
a different realized association path, not a matched-pose causal contrast
between the two final states.

Only unreduced class H/g/cost are additive; Schur terms are not. Native-valid
residuals do not prove static objects, physically correct correspondence or
full-trajectory causality. There is no new candidate gate to pass or waive.
The earlier failed motion gates, negative field/budget/mask/prior results,
defaults, official GT, stored sequence scores and paper tables stay fixed.
No fraction/reference/mask/budget/field retuning or full candidate trial follows.

## Commands and retained validation failures

Run from the repository root. Complete commands/cwd/environment/source,
compiler, binary/library hashes and actual exits are in `checks/`, `groups/`
and `report/execution/`.

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_support_information/build.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_support_information/run.py
env PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages /opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3 experiments/results/inten_support_information/verify_mixed_parallel.py --output /tmp/inten_support_verification/summary.json
```

Ten native mechanism tests exit0 (`11948146`). Every added raw line and its
insertion ordinal is retained losslessly against the byte-identical committed
parent stream (`80c9ae51`). The complete original global JSON bytes and every
whole worker JSON byte round-trip through the report archive; no numerical
row is omitted or normalized. Original raw logs, inputs and compiled artifacts
remain privately preserved.

Failures remain before their separately frozen verifier-only recoveries:

- Initial exit1 used reflector points to reconstruct an intensity lattice that
  native code builds from the full cloud (`688509d1`). The independently rebuilt
  full-cloud lattice matches every committed native sum and population exactly
  (`d3fe715a`); the native measurements do not change.
- Full-field exit1 (`6dc9b559`) exposes fixed-size Eigen/NumPy arithmetic at
  old247, with maximum H difference2.862e-11. An independent Eigen-only witness
  reproduces native H with exactly zero difference (`35ee7354`).
- The ordered checker still exits1 on08 (`b496ce6a`);08/45's largest H difference
  is9.094e-12. Independent coordinate witnesses identify Eigen's two-double
  packet/one-scalar-tail reduction. A single generic ordered implementation
  matches every witness coordinate, with no case exception or tolerance change.
- A batched reducer retained cumulative buffers through final-row views.
  Its controlled task-worker stop has actual exit-15 and the CLI exit1;
  allocation evidence and before-source survive. Adding only compact copies
  preserves every arithmetic value and passes ownership/bit-equality controls
  (`203d78e3`). It does not change native execution or data.

The frozen original verifier/protocol, complete failed logs, arithmetic
controls and input bindings all remain. The final complete checker exits0 in
all worker processes and globally (`a2fd354d`), using every original population
and strict criterion. This diagnoses validation inputs/arithmetic; it is not
an optimizer repair or a softened acceptance rule.
