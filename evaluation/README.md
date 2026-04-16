# Evaluation platform

This directory holds **benchmark binaries**, **CI / smoke fixtures**, and **Python tooling** for multi-sequence experiments. Use it as the single place to reason about “does the stack still evaluate correctly?”

## Layers (what runs when)

| Layer | What it checks | Typical command |
|--------|----------------|-----------------|
| **1. Unit / library tests** | Algorithms and shared code (`common`, `papers/*/test`) | `cd build && ctest --output-on-failure` |
| **2. Synthetic benchmark** | No external data; compares several methods on generated scans | `./build/evaluation/synthetic_benchmark` |
| **3. PCD dogfooding smoke** | Committed 3-frame MCD slice; exercises `pcd_dogfooding` per method | `bash evaluation/scripts/smoke_ci_fixture.sh` |
| **4. Full experiment matrix** | Large runs, optional local datasets; updates `experiments/results/` | `python3 evaluation/scripts/run_experiment_matrix.py` |

GitHub Actions runs **(1)** then **(3)** after a Release build. **(2)** is fast and useful locally; add it to CI only if runtime stays acceptable.

## One-shot local run

After configuring and building the C++ tree:

```bash
bash evaluation/scripts/run_local_evaluation_suite.sh
```

Options:

- `--skip-ctest` — if you only want benchmarks
- `--skip-synthetic` — if you already ran `synthetic_benchmark`
- `--skip-fixture` — if you only want tests + synthetic

From a configured build directory you can also use:

```bash
cmake --build build --target eval_local_suite
```

## Environment check

```bash
python3 evaluation/scripts/verify_environment.py
```

Expects a built `build/evaluation/pcd_dogfooding`, the `experiments/` tree, and common Python deps (`numpy`, `matplotlib`) for trajectory scripts.

## Layout

| Path | Role |
|------|------|
| `evaluation/src/` | `pcd_dogfooding`, `synthetic_benchmark`, `kitti_eval` sources |
| `evaluation/scripts/` | Matrix runner, doc refresh, dataset helpers, smoke scripts |
| `evaluation/fixtures/` | Small committed PCD + GT for CI (`mcd_kth_smoke`) |
| `experiments/` | Manifests (`*_matrix.json`), aggregated `results/` |

## Trajectory metrics (external poses)

`evaluation/scripts/benchmark.py` computes ATE / RPE and plots comparisons when you have exported pose files. See `--help` and the top-level repository README for examples.

## Heavier docs

- [`scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md`](scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md) — setting up public benchmark windows
- [`docs/interfaces.md`](../docs/interfaces.md) — `pcd_dogfooding` method selectors (generated / maintained with the repo)
