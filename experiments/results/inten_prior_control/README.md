# Fixed-reference prior control (prospective)

This distinct controlled experiment follows the rejected reciprocal-support trial.
It separates changing sparse associations from the consequent change in the
first-normal-system-derived weak translation prior. The protocol and source are
frozen before native execution; there are no new motion results yet.

All inherited endpoints and every earlier arm remain. Reference information is
computed per endpoint from the same original estimated inputs by the unchanged
unfiltered rule, separately for the joint and consensus-refinement phases. It is
not a case-selected constant. The prior center remains the corresponding solve's
original estimated translation. No GT or outcome enters the native intervention.
Common-information unfiltered negative controls must be exact.

The cap comes from original01; the prior fraction from DiLO01; original07/00 and
all later KITTI remain development. No held-out, full-trajectory, paper or SOTA
claim is supported. This is a native feasibility harness outside the paper, not
a production option; existing defaults, scores and prior failed gates remain.

Commands (each command's actual exit and full outputs will be archived):

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_prior_control/build.py
/workspace/localization-env/host-venv/bin/python experiments/results/inten_prior_control/run.py
env PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages /opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3 experiments/results/inten_prior_control/verify.py --output /tmp/inten_prior_control.json
```

See `protocol.json` for the fixed reference rule, complete controls, inherited
progression conditions, exact source/input pins, archive format and failure policy.
