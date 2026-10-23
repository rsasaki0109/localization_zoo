# Fixed S2S iteration-budget experiment

Prospective source, cases, controls and gate are frozen in `protocol.json`
before native execution. The primary candidate changes only the existing
iteration cap. Native solver, convergence, damping, prior, correspondence,
loss, S2M and all previous default behavior remain unchanged.

The cap is one computational budget chosen from committed original01
iteration-limit traces. All previously observed KITTI is development. Every
inherited rescue/adverse/fallback endpoint and every prior field/control arm
is retained; no outcome-based field choice or budget sweep is allowed.

An extended budget is outside the paper and explicitly opt-in through
`--inten-loam-paper-set s2s_iterations=100`; default10 remains unchanged.
No new full-sequence accuracy, RTE, paper ratio or SOTA claim follows from
conditional same-history endpoints. Full fixed-setting00–10 is required after
a prospective local gate pass. A gate failure forbids that candidate run.

Commands, from the repository root (each actual argv, cwd, source/environment,
stdin hash, binary/library hash and OS exit is retained in its archive):

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_iteration_budget/build.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_iteration_budget/cli.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_iteration_budget/run.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_iteration_budget/verify.py --output /workspace/localization-env/inten-iteration-budget-20261022/report_stage/summary.json
```

All numerical outputs must be committed before interpretation. Preserve any
failed execution and every negative case before correcting an implementation
or orchestration error; never relax the prospective motion gate.

## Result: original247 recovers; uniform extension fails the full local gate

Numerical `c6832c2a` precedes interpretation. Every31 inherited endpoint runs all12
arms, producing372 outcomes in four native replay processes, all actual exit0.
All186 inherited full traces/profiles match exactly. Every extended joint keeps
its full original joint prefix; early-converged joints stop identically.
Every applied joint/conditional update and stop is independently checked;
maximum relative damped-step backward error1.600121282e-16.

The primary legacy-joint100 arm repairs original247 length/GT
0.522727335→1.023840306 and vector error0.477580491→0.025894853;
rotation8.113069873→2.517761485mrad. Both budgets still reach their iteration
limits here, so no claim that the longer solve is fully converged follows.
Original202 worsens length/GT0.748597638→0.399223855 (still iteration-limited
at100);204 worsens0.607064437→0.398534369 and reports convergence after23
iterations. Thus meeting the numerical convergence condition is not a motion
accuracy witness.205 stays collapsed0.372445688→0.372447045.

| Complete conditional primary comparison | Budget10 | Budget100 |
|---|---:|---:|
| Bad length / vector endpoints | 7 /14 | 7 /13 |
| Sum absolute length-ratio error | 4.741213404 | 4.865650571 |
| Sum vector error /GT step | 14.083975645 | 14.175508759 |
| Sum rotation error [rad] | 0.059478496 | 0.054831082 |

The unchanged bad-length total hides both a recovery and a new failure:
actual08/39 length/GT0.801801894→0.797852053 crosses the frozen20% threshold
(it already had bad vector error in both arms). The full gate fails on this new
label and both worse translation totals. Original consensus rescues stay non-bad,
but rejected consensus still collapses on actual02/1892,0.125854888→0.126206011.
Normalized/fixed-node budget100 controls and every adverse/favorable outcome
remain in [the complete table](cases.md); no field or arm is selected afterward.
No full candidate run, budget sweep, threshold relaxation, paper ratio or default
promotion follows. This is a conditional recovery mechanism, not general accuracy
improvement or SOTA evidence.

Actual production CLI proves default10 and explicit10 have identical frame
outputs; explicit100 appears in the recorded parameter header. Three valid CLI
processes exit0. The intentional fractional-input control is rejected with actual
exit2, retained separately from the successful estimator measurements. Native
mechanism/regression tests57 exit0. The first replay launch stopped before any
native case because CLI evidence was uncommitted; the actual guard failure is
preserved in `checks/orchestration_failure/`. CLI evidence and the stop were
committed before rerunning the unchanged producer; no setting/source/gate changed.

Original01 traces motivated the one tenfold budget before measurement. Every
prior KITTI observation remains development, including original07/00 and the
DiLO01-selected prior0.03. `s2s_iterations=100` is opt-in/outside the paper;
default10, native solver/header, all old results, S2M and paper tables stay fixed.
An independent full-cloud comparison is needed before selecting extended motion;
a longer solve or lower fitted residual alone does not establish its correctness.

Final local verification is committed in `9806d093`, under the sibling
`inten_dense_continuation/checks/`: Python389 tests exit0 (five existing
environment skips), all required documentation checks exit0, and both budget
and dense reports reconstruct and match their committed references under
Haswell and Sandybridge with the original tolerances. Published tables, paper
metadata, fixture golden and every older result are preserved byte-for-byte.
See that study's `checks/final/` and `checks/cpu_report_compare/` for complete
commands, logs, source hashes, reports and actual exit codes.
