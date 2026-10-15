# Notes for coding agents

Start with [docs/roadmap.md](docs/roadmap.md): current state, workflow,
plan and open questions. [CONTRIBUTING.md](CONTRIBUTING.md) has the
per-paper checklist and the honesty policy;
[docs/kitti_pitfalls.md](docs/kitti_pitfalls.md) lists traps that already
cost days.

## Build and check

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
python3 -m unittest discover -s tests
python3 evaluation/scripts/validate_showcase.py
python3 evaluation/scripts/generate_leaderboard.py --check
python3 evaluation/scripts/generate_paper_ready_evidence.py --check
```

Building a single target (`cmake --build build --target pcd_dogfooding`,
`--target test_<method>`) is much faster than the full build. Check exit
codes; do not pipe test output into `tail` and read it as a pass.

## Rules

- Every number in the docs comes from a committed result under
  `experiments/results/` or `docs/benchmarks/`, produced by a command that is
  written down.
- One setting per method across sequences; say which sequence a value was
  chosen on. Run every sequence that has a paper value before claiming a
  ratio.
- Behaviour that is not in the paper is opt-in and default off, and the
  method README says it is not in the paper. Stored results must not change.
- Negative results are reported with numbers, not dropped.
- After importing results, regenerate the derived files: `refresh_study_docs.py`,
  `generate_full_variant_table.py`, `generate_paper_ratio_table.py`, and after
  editing `evaluation/data/paper_reported_numbers.json`,
  `generate_reproduction_status.py` and `generate_paper_comparison.py`.
- The README leaderboard block is generated (`generate_leaderboard.py`); its
  rows must match `docs/index.html`. Keep the README short; long material goes
  to `docs/`.
- PR descriptions follow `.github/PULL_REQUEST_TEMPLATE.md`.
- Datasets (`dogfooding_results/`, raw KITTI / NCLT) are not committed; see
  docs/roadmap.md, "Data".
