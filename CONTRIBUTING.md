# Contributing to Localization Zoo

Thanks for your interest! This project collects **from-paper C++
reimplementations** of LiDAR localization methods behind a unified API,
benchmarked honestly on KITTI. Contributions of new papers, bug fixes,
and evaluation improvements are all welcome.

## What's in scope

A good paper to reimplement is:

- **Classical / geometric / probabilistic** in mechanism (ICP variants, density
  correlation, robust estimation, surface models, factor graphs, …). Deep /
  neural methods are out of scope.
- **Without public author code** — the whole point is to provide a usable
  reference where none exists. (Methods with good public code may still be added
  as baselines, but the priority is the unpublished ones.)
- **Evaluable on KITTI Odometry** as pure LiDAR odometry, ideally without
  requiring a proprietary sensor or IMU (LIO methods fall back to a
  constant-velocity prior when no IMU is present).

Have one in mind? Open a
[paper request issue](https://github.com/rsasaki0109/localization_zoo/issues/new?template=paper_request.yml).
If it's *your* paper, even better — we'd value your read on the mechanism.

## Honesty policy

**Results are reported as measured.** A reimplementation that turns out to be
near-redundant with point-to-plane, or that is honestly *worse* than KISS-ICP on
clean KITTI, is still a valuable, publishable result here — it tells readers what
the mechanism actually buys. Do not tune to the leaderboard or hide negatives.
We compare every method against a KISS-ICP reference run on the same data.

## Adding a new paper

```bash
mkdir -p papers/your_method/{include/your_method,src,test}
# 1. Write headers, sources, and mechanism unit tests
# 2. Add papers/your_method/CMakeLists.txt
# 3. Add add_subdirectory(papers/your_method) to the top-level CMakeLists.txt
# 4. Wire it into evaluation/ (pcd_dogfooding) for KITTI runs
# 5. Run ctest and keep the full suite passing
```

A complete "one paper unit" PR includes:

1. **≥3 mechanism unit tests** that exercise the *distinctive* idea of the paper
   (not just that ICP converges) — e.g. the kernel/weight behaves as claimed.
2. **KITTI full seq00 / seq07** evaluation with `--no-gt-seed` (no ground-truth
   seeding), using a uniform `--*-dense-profile`.
3. A **leaderboard row** in `README.md` and an entry in `docs/methods.json`.
4. A **module `README.md`** documenting the mechanism, parameters, and the
   measured KITTI results vs the KISS-ICP reference.
5. Honest notes on where it wins, ties, or loses.

## Style

- C++17. Match the surrounding code's naming, comment density, and idioms.
- Keep the ROS-independent core libraries free of ROS dependencies.
- Build from a clean `build/` directory if you hit stale-cache errors.

## Reporting bugs

Use the [bug report template](https://github.com/rsasaki0109/localization_zoo/issues/new?template=bug_report.yml).
Include the failing command, full error output, and your Eigen / PCL / Ceres
versions.

## License

By contributing, you agree that your contributions are licensed under the
project's [MIT License](LICENSE).
