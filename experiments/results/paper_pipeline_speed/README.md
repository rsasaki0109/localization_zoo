# Paper pipeline speed experiment

The frozen baseline is source `134ead2f21a4611ec328a45aa543b31428d6b92c`.
Its binary SHA-256, input hashes, build command and environment are recorded here.
The three completed CPU profiles were committed in `97cf50af` before describing
these observations or adding the opt-in acceleration candidates.

The KITTI 07 profiles use the first 120 frames, with four OpenMP workers and
one OpenBLAS worker. The selected Mesh-LOAM, IMLS-SLAM and InTEn-LOAM argument
lists are copied from their existing fixed variants; see `protocol.json` and
[`paper_pipeline_speed_kitti07_plan.json`](../../paper_pipeline_speed_kitti07_plan.json).
KITTI 07 is development data for this work, not held out.

The sampling profiles point to facet association in Mesh-LOAM, model projection,
scan normals and list sorting in IMLS-SLAM, and KD-tree construction/query
work in InTEn-LOAM.
`runs/baseline_profile_*/` contains the profiler input, text report, summary,
poses, exact invocation, exit codes and profile-report command. Profiled execution
times are not used to claim speedups.

The unprofiled full-sequence baselines were committed in `b66122bd`. Times
include scan loading and preprocessing (`time_ms`), with the profiler disabled:

| Method | Frames | Time [s] | FPS | Official RTE [%] | ATE [m] |
|---|---:|---:|---:|---:|---:|
| Mesh-LOAM | 1101 | 1417.883 | 0.777 | 0.341905 | 0.855800 |
| IMLS-SLAM | 1101 | 1760.844 | 0.625 | 0.339406 | 0.693085 |
| InTEn-LOAM | 1101 | 602.591 | 1.827 | 0.364131 | 1.461444 |

The first acceleration candidate, source `fe607199`, has completed build,
CTest and short-window screening. The raw screening was committed in
`8479040a` and `8fe732a7`; see
[`candidates/fe607199/screening.json`](candidates/fe607199/screening.json).

| First 120 frames | Frozen control [s] | Changed default [s] | Opt-in [s] | Control / opt-in |
|---|---:|---:|---:|---:|
| Mesh-LOAM | 117.877 | 105.840 | 168.128 | 0.70x |
| IMLS-SLAM | 160.513 | 194.906 | 79.561 | 2.02x |
| InTEn-LOAM | 45.685 | 34.105 | 18.661 | 2.45x |

All saved poses match the control exactly in this window. Timing varies even
between the two default paths; these single window runs guide development and
do not establish a full-sequence speedup. The Mesh tree candidate is a negative
result and is replaced with cached packet filters. IMLS cell-topology caching is
being evaluated next. The target remains open until full-sequence acceleration
and accuracy have been measured.

Acceleration code is opt-in and default off. Historical benchmark aggregates
and paper reference metadata are not changed by this experiment. CLI changes
only parse the option and forward method parameters; neither `common/` nor the
runner loaders change, so the shared-code leaderboard drift audit is outside
this change's scope. CI's existing fixture drift check still applies.

Replay the frozen baseline/profile commands in `baseline_commands.json` and
candidate commands in each run's `command.json`. Their `exit_code` fields and
`candidates/<source>/checks/commands.json` record success directly. The initial
screening driver lost its stdout connection after a successful control run;
its nonzero driver receipt is retained. The resume script verified the completed
run's command/source/binary hash, reused it, and wrote subsequent logs to files.
This transport failure did not turn a failed measurement into a pass.

`helpers/` preserves the executed preparation, capture and profiling scripts.
Commands retain their original workspace paths; replace only workspace/output
prefixes when replaying. The downloader is the repository's
`evaluation/scripts/fetch_kitti_odometry_sequences.py`. Raw scans and executable
binaries are not committed. The first download attempt ended mid-stream; the
successful retry reused completed files. Profiler installation initially used an
unreachable proxy address saved in the existing container; the successful command
forwards the current session proxy for that command only. Both failures and the
successful exits are preserved.
