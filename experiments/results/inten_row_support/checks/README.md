# Validation records

The first full Python suite failed at
`test_committed_csv_covers_every_ready_default`: the test equated every ready
problem with an adopted variant. This study deliberately leaves its completed
experiments unadopted. The actual exit and lossless output are retained in
[`initial/`](initial/) (evidence commit `cc69a1f0`). Subsequent checks were not
run after that failure.

The contract now compares the declared problem/variant defaults, including
identity and multiplicity, with the marked CSV rows. A regression fixture also
requires a ready experiment with no adoption to retain every variant, with no
marked default and no standalone timing. No experiment setting, native solver,
scientific result or existing default changes to satisfy the test.

The full-suite rerun and remaining documentation/preservation checks are in
[`final/`](final/); exact commands and exits are retained. The preservation
result is [`preservation.json`](preservation.json). Existing focused native
build/test receipts are in [`build/`](build/), and CLI trace parity is in
[`cli_preflight/`](cli_preflight/). Full-project native and ROS CI are required
on the final pull-request head; the focused local build alone is insufficient.
