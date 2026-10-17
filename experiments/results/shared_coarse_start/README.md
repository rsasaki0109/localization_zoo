# Shared initial coarse registration

The second shared-robustness component in roadmap item 6 shares the startup
stage policy between IMLS-SLAM and opt-in SuMa. Correspondences, residuals,
normal gates and numerical stopping remain specific to each method; this is
not a shared numerical aligner. Both native defaults remain off.

[Protocol](protocol.json) fixes the SuMa arguments before execution. The first
registration and coarse iteration budget derive from the existing IMLS-SLAM
04 choice. The distance multiplier derives from its coarse/fine radius ratio;
SuMa keeps its own Huber residual and projective associations. No SuMa sweep
or new held-out claim is made: its prior full KITTI results were already known.
The previous SuMa translation-remap experiment is not combined with this one.

Full KITTI 00–10 evaluation and affected-method compatibility checks will be
recorded here, with commands and actual exits. Experimental results are not
promoted to the published leaderboard, Table 6 or paper-reference metadata.
Near-range sharing and NCLT remain separate work. The NCLT host remains absent
from the enforced policy despite earlier approval; this work does not retry
that approval request or substitute synthetic scans for real validation.

Single-variant matrix aggregates retain the runner's automatic adoption label.
That label ranks only the experimental variant; the separately measured raw
control determines any transfer recommendation, and defaults stay off.
