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
scan normals and list sorting in IMLS-SLAM, and tree construction in InTEn-LOAM.
`runs/baseline_profile_*/` contains the profiler input, text report, summary,
poses, exact invocation, exit codes and profile-report command. Profiled execution
times are not used to claim speedups.

The unprofiled full-sequence baseline measurements are in progress. Candidate
acceleration code is opt-in and default off; equivalence tests and paired full
sequence speed/accuracy validation remain pending. Historical benchmark
aggregates and paper reference metadata are not changed by this experiment.

`helpers/` preserves the executed preparation, capture and profiling scripts.
Commands retain their original workspace paths; replace only workspace/output
prefixes when replaying. The downloader is the repository's
`evaluation/scripts/fetch_kitti_odometry_sequences.py`. Raw scans and executable
binaries are not committed. The first download attempt ended mid-stream; the
successful retry reused completed files. Profiler installation initially used an
unreachable proxy address saved in the existing container; the successful command
forwards the current session proxy for that command only. Both failures and the
successful exits are preserved.
