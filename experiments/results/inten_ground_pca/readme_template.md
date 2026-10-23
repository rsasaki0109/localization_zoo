# Full iterative S2S ground PCA intervention

This actual odometry comparison follows the fixed-system diagnostics in
[the geometry study](../inten_plane_correspondences/README.md). It tests whether
the nearest-anchor ground PCA normal helps the complete repeated association,
optimization, mapped prediction and map trajectory. A smaller first update
alone was not a trajectory improvement.

## Frozen intervention and provenance

Only `s2s_ground_pca` differs. Ground support must first pass every original
triangle acceptance test. PCA uses exactly the gated native ten-neighbour
query; the plane passes through the original closest point. Collinear or
insufficient PCA support retains the original triangle. Facade, edge and
intensity equations at a fixed pose are unchanged. The unchanged data-adaptive soft prior recomputes its information from the
changed ground equations, including at the first iteration; its ratio and
anchor rule stay fixed. Later support may diverge because motion changes. This repository extension is outside the paper and
default off. It does not infer physical laser rings.

Both arms inherit mapped-motion prediction, the soft translation prior and
acceleration; row completion remains off. Profile selected on07;00 informed
earlier diagnosis. The prior fraction came from DiLO KITTI01 tuning. Ground
PCA was selected from previously committed known-failure first-system controls
on01/02/10, including negative cases on every sequence. The PCA numerical
degeneracy criterion is inherited from that control. All prior outcomes are
development evidence, not held-out accuracy. No retuning after collection
begins, and every full00-10 sequence is run even if07 is negative.

[Protocol](protocol.json), all manifests, native source, tests and collector
are committed before native execution. Reporting sources and the separate
[postprocessing protocol](postprocessing_protocol.json) are committed before
numerical derivation. The forward-information diagnostic was added during
collection as descriptive analysis; it was not registered before acquisition.
[Binary hashes and dependency pins](binaries.json),
[actual configure/build/CTest/link receipts](checks/build/) and runtime capture
identify the executable. A full07 immutable-parent control and every inherited
default-off pair check protect earlier behavior. Each owned official sequence
is verified, archived, round-trip checked, committed and normally pushed before
scan deletion; cleanup is committed and pushed before fetching the next.

## Test failure retained before acquisition

The initial focused CTest exited8: a synthetic-road test required more than30
accepted ground constraints, but that exact-flat fixture supplied2. This was
an unnecessary per-class requirement; the solver cutoff applies to the joint
system. The revised mechanism test adds a small world-fixed road ripple,
requires nonzero ground support, checks actual ground-equation change and exact
non-ground equations/counts at the same seed. Production PCA code and all KITTI
parameters remain unchanged by that test repair. Both the initial failure and
the definitive configure/build/CTest/link exits0 are retained under `checks/`.

## Outcomes

@@RESULTS@@

## Commands and interpretation

From the recorded checkout/environment (the capture receipts preserve actual
cwd, argv, source/binary hashes, thread settings, exit codes and times):

```bash
python3 experiments/results/inten_ground_pca/build.py
python3 experiments/results/inten_ground_pca/collect.py
python3 experiments/results/inten_ground_pca/report.py --output-dir experiments/results/inten_ground_pca/report
python3 experiments/results/inten_ground_pca/import_results.py
python3 experiments/results/inten_ground_pca/finalize_docs.py
```

Official KITTI distance-segment RTE, local step labels, ATE and first-system
controls answer different questions. Error-growth blocks locate drift without
proving surface correctness. Every adverse metric and newly bad local frame is
retained; no per-sequence winner is adopted. Native timings are preserved but
paired runs share CPUs, so no standalone FPS claim is made.
