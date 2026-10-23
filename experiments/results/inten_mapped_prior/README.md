# Unprojected recursive mapped-motion prior: aborted numerical trial

This initial repository extension was outside the paper and default off.
Native source: `88723d9abc9eabf159502f3de9fd215001821476`.
The [original fixed protocol](protocol.json) required every sequence;
**that completion target was not met**. This trial is retained as a negative
result, not a complete accuracy comparison. No all-sequence ratio is reported.

Full07 completed: reference official KITTI RTE **0.364131%**, candidate
**0.379537%**; ATE **1.461444 → 1.668590 m**. The default-off full07 control
matched the immutable parent in metrics, saved pose/GT bytes and trace body.
See [the committed execution](sequences/07/).

On00, the reference completed, while the candidate aborted with exit **134**
after **2355** frame records (last frame **2354**). The trace has no completion
record or native summary, so it has **no full-sequence RTE**. Its first recorded
nonfinite matrix is S2S input at frame **2189**. PCL eventually rejects an
invalid query point. [Commands and original logs](sequences/00/) and
[geometric diagnosis](sequences/00/geometric_diagnosis/summary.json) preserve
the failed attempt and every rotation-drift measurement.

The raw matrix product used as a seed does not enforce SO(3). A left rigid
solver update cannot remove a seed's non-rigid component. Along00, the final
rotation's Frobenius orthogonality error `||R.T R - I||` grows from
**4.221092e-10** at frame500 to **6.625101e-6** at1000 and **0.039863** at1500.
This numerical feedback failure motivates a separate rigid-pose revision;
that revision must restart every full sequence under a new frozen protocol.
The original KITTI values were selected on07, with00 informing diagnosis;
00 also informed this stability correction. No independent held-out claim.

Raw evidence and cleanup were separately committed and normally pushed before
advancing. [Normal backup receipts](failure_backup_execution/) show preservation
before deleting owned00 scans. Existing studies, binaries, aggregates and
publication values are unchanged. No parameter was retuned during this trial.

[Collection command and failure](collection_execution/), [build and tests](checks/build/),
[CLI parity](checks/cli_preflight/), [binaries/dependencies](binaries.json),
[diagnostic source](diagnose_failure.py). The original collector stops on the
first failed native execution; do not rerun it into these evidence paths.
