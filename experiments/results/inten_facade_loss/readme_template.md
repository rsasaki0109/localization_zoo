# S2S facade Cauchy influence: fixed full KITTI evaluation

`--inten-loam-paper-set s2s_facade_cauchy=1` is a repository extension,
**not in the paper and default off**. It replaces only S2S facade Huber IRLS
influence by Cauchy, at the same inherited native scale 0.1 m. Extraction,
correspondence gates, other feature-class loss definitions, S2M and map-insertion
definitions stay fixed. Adaptive S2S prior information changes immediately with
the data system; later reassociation, motion prediction and map feedback can
change every class along the realized trajectory.

For each accepted S2S facade point-to-plane residual `r` at native scale `h`,
the reference uses `w = 1 if abs(r) <= h else h/abs(r)` and the option uses
`w = 1/(1+(r/h)^2)`. Both accumulate `w*J*Jᵀ` and `w*J*r`. No residual is
discarded by this rule, and the stored weighted squared residual remains an
IRLS surrogate rather than a comparable robust loss energy across the arms.

One Boolean at the unchanged scale was selected after committed current01
terminal controls `f52b84a0`, before native implementation and full evaluation.
The positive204 and adverse247/rotation cases are retained below. No scale fit,
sweep, hand region, predicted-DOR source mask or GT seed/input enters the S2S
intervention. The inherited post-S2M DOR and map-insertion logic remains enabled
in both full arms. Original
profile selection on07,00 diagnosis, inherited DiLO01 prior tuning and all prior
KITTI exposure are disclosed. All sequences are development evidence; no
independent held-out or per-sequence adoption claim is made.

Both arms retain mapped-motion prediction, S2S prior0.03, ground/facade PCA,
acceleration,120m range,0.17deg correction and0.05m S2M plane check. Predicted DOR,
plane spread and row completion remain off; original post-S2M DOR remains on.
Both use the same new native binary;
the default-off arm must reproduce the immutable preceding facade-PCA candidate
(also PR162 OFF reference) in saved pose/GT bytes, accuracy metrics and numerical
trace bytes after only the declared scan-root substitution. Original traces are
archived unchanged. Raw paired CPU timing is preserved but is not a standalone
FPS measurement and never selects a default or proxy ranking.

@@RESULTS@@

## Commands and phase-specific reproduction

Run from the worktree root. These are recorded stage entry points, not a single
fresh-clone replay recipe for a completed checkout; scripts refuse existing
outputs. Full argv/cwd/source/environment and actual exit codes accompany each
native execution, build, acquisition, analysis, backup and check.

Use the recorded host analysis venv (on the measured machine,
`/workspace/localization-env/host-venv/bin/python`) with its directory first on
`PATH` and `PYTHONDONTWRITEBYTECODE=1`; keep OMP/BLAS threads at one. The
`analysis_runtime` record pins Python, NumPy and Matplotlib separately from the
native container runtime. Do not substitute a different plotting environment
when checking historical generated figures.

```bash
python experiments/results/inten_facade_loss/resume.py
python experiments/results/inten_facade_loss/seal.py
python experiments/results/inten_facade_loss/build_candidate.py
python experiments/results/inten_facade_loss/run_collection.py
python experiments/results/inten_facade_loss/recovery_collection_backups.py
python experiments/results/inten_facade_loss/report.py --output-dir <fresh-report-dir>
python experiments/results/inten_facade_loss/import_results.py
python experiments/results/inten_facade_loss/run_postprocessing.py
python experiments/results/inten_facade_loss/finalize_docs.py
```

The diagnostic clone and corrected probe use source `0686c89f`, with corrected
analysis `98b9bd5a`. These precede the native parameter ABI extension: reconstruct
those stages with their matching headers and unchanged pinned dependencies,
never compile the old clone against the final changed headers. The terminal
producer receives only official scans and the original estimated relative seed.
Mapping is disabled to isolate S2S; the seed is supplied through the forced
diagnostic interface and is never GT. Its internal observer keeps the public
passive/forced-trace guard intact. The forced first-data count convention is
checked explicitly and separated from the normal caller last count; all other
baseline solver state and iterations must match exactly.

The native implementation is frozen at `bf0d44cc`; full collection source is
`243b14a3` with runtime capture `8923e50d`, before its result directories exist.
Compiler/link flags, actual library/binary hashes and focused CTest exits are in
`binaries.json` and `checks/candidate_build/`. The recorded Ubuntu22.04/GCC11.4/
Eigen3.4/PCL1.12 container reuses byte-pinned unchanged dependencies. Rebuilding
dependencies or paths creates new hashes and requires a new committed protocol;
do not overwrite these pins or stored results. Host NumPy/Matplotlib versions and
the analysis venv/PATH prefix are recorded; native OMP/BLAS threads stay fixed.

`run_postprocessing.py` owns the report/import transaction and commits and pushes
its numerical outputs before `finalize_docs.py` writes the interpretation; do not
run both the individual import and its owner on the same checkout.

For analysis without downloading scans or importing again, run only `report.py`
to a fresh directory. It restores the committed full traces and recomputes all
tables and the figure. Its source-commit provenance field can differ on a later
checkout. `postprocessing_protocol.json` pins the raw collection snapshot and
analysis sources. Import runs once before its new aggregate files exist.

## Failures and preservation

The first official01 acquisition ended early and is retained alongside the
successful retry; all final scan/GT hashes match committed official inputs.
Initial diagnostic source `8f2600a8` rejected forced pose plus public passive
trace before producing any terminal output. The failure is committed before the
corrected internal observer; original binary/object remain unchanged and a
distinct corrected producer is compiled. The first analysis missed the optional
omission of an all-zero prior-information field; its failure and empty partial
outputs are retained before the parser correction. Analysis reuses verified
native output and does not rerun the producer or refetch scans.

After every full native arm and the collector complete, the original collection
wrapper rejects an already existing diagnostic backup archive as its final
storage target. Its actual failure, complete stdout/stderr and original driver
are retained under `collection_wrapper_failure/`. The committed
`recovery_collection_backups.py` archives complete backup receipts to a fresh
`full_backup_execution/`, with its actual exit and byte-roundtrip proof retained.
`backup_execution/` stays byte/hash/mode-identical. This recovery does not rerun
acquisition, native processing, validation or cleanup; collector and wrapper
exit codes are recorded separately.

Official scans use `evaluation/scripts/fetch_kitti_odometry_sequences.py`, one
owned full sequence at a time. Every raw/GT SHA is checked, all raw evidence is
losslessly roundtripped and committed/pushed before scan deletion, and cleanup
is committed/pushed before the next acquisition. Ordered32MiB uncompressed gzip
shards preserve large traces. Original CMake logs, including trailing spaces,
remain byte-exact gzip with private originals and roundtrip hashes.

Immutable historical storage sharing proves every byte/hash/mode and retains all
logical paths; old source/builds/binaries, aggregates, defaults and canonical
publication values remain unchanged. Every new full result is experimental,
nondefault and unscored, with aggregate timing unavailable. Numerical evidence
is committed before prose, the required derived files are regenerated, and full
Native plus ROS2 CI must pass at the exact final head before Ready.
