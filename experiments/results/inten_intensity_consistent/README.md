# Fixed-node S2S intensity feasibility experiment

Prospective protocol in `protocol.json`, frozen before native execution.
This repository extension is S2S-only, opt-in and default off; it is not specified
by the paper. Existing defaults, S2M and all stored results are preserved.

All previously exposed KITTI is development. The fixed radius derives from
cubic support, without a parameter sweep. Every inherited endpoint and field
path is included; the motion gate is fixed before execution. No new full
sequence or accuracy improvement is claimed by this local experiment.

Commands (from the repository root, Python environment and Docker command
are recorded in `common.py` and each actual process receipt):

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/build.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/run.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/verify.py --output /workspace/localization-env/inten-intensity-consistent-20261022/report_stage/summary.json --details /workspace/localization-env/inten-intensity-consistent-20261022/report_stage/boundaries.json.gz
```

Build sources, binaries, linked-library hashes, actual argv/cwd/environment,
stdout/stderr and OS exits are retained. All outcomes, including failures, must
be committed before numerical interpretation or README tables.
