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
