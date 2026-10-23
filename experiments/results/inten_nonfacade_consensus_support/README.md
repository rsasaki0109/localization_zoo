# Non-facade common-support veto: rejected

A native veto based on non-facade robust loss does not reliably distinguish a
necessary motion correction from a harmful one. It suppresses known rescues and
still accepts a fully supported collapse. The frozen gate fails; this study adds
no production option or recursive trial. Numerical evidence precedes prose:
`61ca47d2` (all guard controls), `3e90a147` (all passive attribution), `82c98752`
(complete report). Previous zero-majority results and their rejection are preserved.

## Prospective rule and complete controls

The [protocol](protocol.json), committed before measurements, compares the guarded
proposal against the original joint estimate at every inherited endpoint and
requires preservation of the earlier rescues/fallbacks. It is a new hypothesis;
the previous zero-majority gate is not amended. All 31 endpoints from that study
are included: original-history 190/200/202/203/204/205/247/433, every new immediate
01/02/08 length-damage entry, and deduplicated preceding controls. All actual
estimated seeds and complete previous/current feature caches are unchanged.

The native process first computes the unchanged original joint estimate and old
consensus refinement. Before accepting the refinement, it queries native residuals
at the original final joint pose, freezes those line/plane associations, and
measures the proposal against exactly that support. Point-to-line distance is
recomputed nonlinearly; plane normals/anchors stay fixed. Original reflector
source features query the same previous-frame intensity field at the proposal.
Any missing candidate field sample vetoes the proposal. The sum of true Huber
loss over edge, ground and reflector features must strictly decrease. Facades
and the prediction prior do not participate in this decision. The Huber scale
and intensity scale are inherited; no new numeric threshold or GT decision.

This is an explicitly enabled diagnostic intervention outside the paper. Native
rotation stays the original estimated rotation. All old profile, voting, mask,
refinement, finite/pair, and majority/interior fallback rules remain. The passive
query cannot take an optimization step or change the estimate. Serial observation
keeps the original feature order and built native trees; normal controls keep
their acceleration path. Full profiles are computed once and reused identically.

All **93 terminal calls** retain exact original/old-consensus controls, including
full solver traces, feature counts, poses, profiles, votes and masks. Every
primitive, H/g system, IRLS objective, Huber loss and native decision is independently
reconstructed. All native commands exited 0. [All 31 outcomes](report/all_cases.md),
per-class costs and every primitive remain committed, including 13 accepted
proposals, 15 vetoes and 3 untouched fallbacks. These are conditional pair results,
not full trajectories, official RTE, a speed claim or a paper ratio.

## Negative results and why the objective fails

- Original-history 204 loses its rescue: old consensus length/GT 1.002040 becomes
  0.607064 after the veto. At 205, 0.954865 becomes 0.372446. Necessary corrections
  at 200/202/203 are also suppressed; 190's rescue and original 247/433 fallbacks
  remain. Actual-history 02/1891 also loses its old rescue, 0.980923 to 0.199499.
- At **02/1892**, complete original support remains available, yet the guard accepts
  a collapse: length/GT 1.015129 to 0.125855, vector error/GT 0.022428 to 0.874305.
  Native non-facade Huber loss decreases **3.143803 to 3.113702**. Edge loss rises
  0.037364 to 0.367419 and ground loss 0.300246 to 0.350088, but reflector loss
  falls 2.806192 to 2.396195 and outweighs both. Thus the counterexample does not
  depend on silently losing support. Feature classes are not verified static
  objects or independent motion truth.
- Several immediate 01/02 collapses are vetoed, including 01/426, whose guarded
  output returns to original length/GT 0.966919 from old 0.054142. Other adverse
  and 08 tradeoff cases remain. At 08/39 the guard accepts length/GT 0.704652
  versus original 0.801802, while vector error improves but remains bad.

The [separately frozen attribution](attribution_protocol.json) follows the rejected
trial without changing its decision or gate. It makes **62 passive native queries**:
both original and old-proposal poses at every endpoint, including fallback ties.
It uses the same pinned native object, changes no optimizer step, and retains all
fresh supports, counts, losses and equations. Original-query costs/counts match
the guard observations bit-for-bit.

At original-history 204, the proposal's edge loss is 6.685981 under the frozen
original associations but 1.810439 after fresh association; edge counts change
66 to 59. At 205 these losses are 9.323254 and 1.251646, with counts 88 to 64.
This demonstrates the dependence on original associations; changed raw sums
are not an accuracy comparison because support also changes. Even fresh 204
edge loss per pair increases 0.025678 to 0.030685 versus the original estimate,
although the correction rescues motion. At actual 02/1891 there are no edge pairs
at either pose; at 1892 the fresh proposal has no edge pairs versus 2 originally.
A fresh-query total cannot silently treat those disappearing constraints as a win.

Next investigate independent longitudinal information, support changes and intensity
ambiguity in the accepted reference failures. Another aggregate-loss or zero-vote
heuristic is not justified by these results. Physical moving objects have not been
established. Every past KITTI observation remains development: original profile07,
diagnosis00, borrowed DiLO01 prior0.03 and all later studies/full runs. There is no
held-out claim. These diagnostics do not close full01 recovery or10 gradual drift.

## Reproduction and storage

No new raw scans were acquired. Private retained feature caches are verified
against committed parent hashes before use; original/old native parity verifies
actual histories. Raw scans and feature caches are not committed. Every old
source, build, binary and evidence path is preserved. `checks/storage/` records
byte/hash/mode/ownership/device proof for hardlinking only closed immutable evidence
into a separate worktree; source and build files remain separate.

Run the scripts from a clean study worktree with fresh owned staging paths and
the recorded existing Docker/dependency image. Preserve failed runs rather than
overwrite them. Build and native command receipts retain actual argv/cwd/source,
environment, exits, input hashes and binary/object/library hashes.

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_nonfacade_consensus_support/build.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_nonfacade_consensus_support/run.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_nonfacade_consensus_support/run_attribution.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_nonfacade_consensus_support/verify.py --output /tmp/nonfacade-support-verification.json
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python -m unittest discover -s tests
```

`groups/` and `attribution/` archive every result and command with byte round-trip
checks. Missing-field proposal costs are explicitly **partial available sums**,
never complete comparison costs; the guard rejects such cases. Verification exit0
means the evidence and failed gate reconstruct, not that adoption passed. Offline
metric comparison uses relative1e-10/absolute1e-12 bounds; native poses, control
traces and decision labels remain exact. Independent H/g reconstruction also
allows absolute1e-10 accumulation roundoff, separate from any accuracy criterion.
Production defaults, old options, publication aggregates, Table6 and paper metadata
remain unchanged.
