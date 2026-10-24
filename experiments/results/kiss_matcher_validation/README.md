# Official KISS-Matcher pair validation

[Protocol, complete results, negatives and reproduction](../../../docs/kiss_matcher_validation.md).

One fixed configuration per variant was committed before execution. The initial
real-data diagnostic passes 4/9 cases for KISS-Matcher and 6/9 for its Quatro
mode; the same-protocol provenance rerun passes 6/9 for each. The hard
3072→3112 pair fails at all tested yaws in both executions. The initial full
solver translation errors on it are 21.790–29.163 m. Quatro also fails the
arbitrary full-rotation synthetic control (168.522° / 8.366 m initially), outside
its yaw-only assumption. These cases and run variability are not tuned away.

The initial backend-hash audit failure is retained. `run_with_backend_provenance/`
hashes the actual native extension and passes the saved-output audit. This is
an external official-package pair diagnostic, not a Zoo paper implementation,
full-sequence odometry result, or separate Quatro++/KISS-SLAM evaluation.
