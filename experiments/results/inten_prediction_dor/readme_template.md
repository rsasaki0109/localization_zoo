# Predicted-DOR facade gating: fixed full KITTI evaluation

This evaluates one opt-in repository extension, **not in the paper** and default
off: `--inten-loam-paper-set s2s_predicted_dor_facade=1`. Before normal S2S,
existing DOR evaluates the extracted frame at the initial motion prediction,
using the prior local map and previous full scan. Only source facade queries
whose cluster is marked dynamic are rejected. Extraction, other feature classes,
S2M queries and original post-S2M map insertion retain their existing rules at
the same history. Later reassociation/map/prediction feedback can change all
classes. The existing adaptive S2S prior also changes immediately with facade H.
Mapping and DOR must be enabled; forced-pose diagnostics remain unchanged.

Existing DOR thresholds 0.1/0.5 are reused without fitting. The Boolean candidate
was chosen on current KITTI 01 after committed full-sequence detector evidence
`11cc0aa3` and all matched-seed terminal controls `fe932b7f`. Original profile
selection on 07,00 diagnosis, inherited DiLO 01 prior tuning and all earlier KITTI
exposure are disclosed. No sequence is independent held out. One setting is
frozen across every full 00–10 run; no per-sequence winner is adopted.

Both arms retain mapped-motion prediction, S2S prior 0.03, ground/facade PCA,
acceleration, 120 m range, 0.17 degree elevation and 0.05 m S2M plane check. Plane spread
and row completion remain off. Both use the same freshly compiled native runner;
the default-off arm must reproduce the preceding immutable facade-PCA candidate's
saved pose/GT bytes, metrics and numeric trace body after declared scan-root-only
substitution. The actual native traces are archived unchanged.

@@RESULTS@@

## Reproduce and audit

Run from the repository worktree root. The commands below are the recorded stage
entry points, with complete argv and actual exits in the adjacent receipts.
They refuse existing outputs and are not a single replay command for a completed
checkout. The native producer source, compiler/
link metadata, immutable library/binary SHA256 values, command/cwd/environment
and actual exits are in `full_protocol.json`, `binaries.json`, `checks/` and each
sequence's receipts. Local changed-library tests use actual CTest exits; full
Native and ROS2 CI are required before review.

```bash
python experiments/results/inten_prediction_dor/build_candidate.py
python experiments/results/inten_prediction_dor/run_collection.py
python experiments/results/inten_prediction_dor/report.py --output-dir <fresh-report-dir>
python experiments/results/inten_prediction_dor/import_results.py
```

The native build phase uses source `8ab36c35`; full collection is frozen at
`29bb1fa9`, before its result directories exist. It follows the completed passive
and terminal stages and revalidates their owned 01 acquisition, rather than
silently downloading another copy. The full collector expects their local seal
receipts. Reconstruct those recorded prerequisites in an isolated workspace
before replaying native collection. Rebuilding creates new binary/library hashes
and therefore requires a new committed protocol; do not overwrite these pins or
stored results. The postprocessing protocol records the complete raw snapshot and
the analysis source hashes. Import runs once before its aggregate files exist.

For analysis of the committed results without downloading scans or modifying
aggregates, run only `report.py --output-dir <fresh-report-dir>`. It restores the
lossless traces, verifies the fixed profiles and solver records, and recomputes
all comparison tables and the figure. `summary.json` records the current analysis
source commit, so that provenance field can differ on a later checkout.

The scripts use the recorded managed Ubuntu22.04/GCC11.4/Eigen3.4/PCL1.12
container and byte-pinned unchanged native dependencies; rebuild those matching
dependencies when reconstructing this environment. Host analysis uses the recorded
NumPy/Matplotlib venv, with its PATH prefix and PYTHONDONTWRITEBYTECODE=1. Native
OMP/BLAS threads are fixed. Paired CPU timings are retained raw but never treated
as standalone FPS or used for proxy/default ranking.

Diagnostics predate the new native ABI. Reproduce the passive stage from source
`99b681d4` (corrected collector and analysis), and terminal stage from `782d6b06`,
using their matching headers and original native dependency metadata. Do not
compile those frozen source copies against the final changed private signature.
Their protocols, binaries, native outputs, input seeds/masks and phase hashes are
retained; diagnostic copies never supply full-sequence accuracy. The passive
producer never reads GT. Its `anchored_pose` is sensor-relative; the harness uses
a GT-derived output display anchor. That declared output-coordinate difference
and validated scan-path convention are excluded from passive parity; every
unanchored estimator/solver/count/DOR field matches exactly. GT and the inherited
hand region are offline labels only and never feed either automatic arm.

Official scans are fetched by `evaluation/scripts/fetch_kitti_odometry_sequences.py`
one owned full sequence at a time. Every raw/GT hash is checked against committed
inputs. Lossless archives are byte-roundtrip verified, committed and normally
pushed before deleting owned scans; cleanup is committed/pushed before the next
fetch. Initial metadata-parser exit 1 before native execution, acquisition retries
and CMake-log whitespace gate exit 2 remain recorded. Compilation/acquisition were
not silently rerun after the parser failure; an explicit verified continuation
ran the preserved native producer. CMake stdout is byte-exact gzip plus retained
originals, never sanitized. All native mechanism checks then exited 0.

All historical results, benchmarks, default behavior and canonical publication
numbers are preserved. Immutable historical evidence storage sharing has complete
byte/hash/mode proof; every logical path is retained and old builds/binaries are
untouched. New full variants remain experimental/nondefault/unscored with blank
aggregate timing. Numerical results are committed before this interpretation;
required derived files are regenerated after import.
