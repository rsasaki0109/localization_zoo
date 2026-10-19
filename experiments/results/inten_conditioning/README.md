# InTEn-LOAM: offline normal-equation conditioning

Roadmap item 12 needs an explicit unit and coordinate convention for passive
solver observations. This tool reads stored traces only; it cannot supply a pose,
alter a solver gate or select a registration setting.

## Convention

The native left tangent mixes translation in metres and rotation in radians.
Its rotation is about the target-coordinate origin. Direct raw S2S/S2M spectra
therefore confound geometry with units and world origin.

For the recorded linearization pose `T=(R,t)` define
`q=[u_sensor,L*omega_sensor]` and `A=Ad(T) diag(I,I/L)`:
`A=[[R,[t]x R/L],[0,R/L]]`. Then `Hq=A.T H A` and `gq=A.T g`
describe motion about the current sensor centre in its axes. This first-order
coordinate change adds no solve and does not re-express a finite update.

`--rotation-length-m` is required; no physical scale is universally correct.
The [protocol](protocol.json) selects **1 m** before these fixture analyses and
applies it unchanged to every tuple, without KITTI failure-sequence selection.
The relative numerical rank cutoff is **1e-9** times the largest absolute
eigenvalue. Scale and cutoff can change condition/rank; neither gates an estimator.

The existing structural checker validates the full trace/run/exit tuple first.
The analysis streams every iteration, preserves distinct input/call indices,
then rechecks input hashes before publishing into a new directory. Archived scan
paths are never opened. Hashes and matching argv alone do not prove execution
authenticity; preserve the collector's binary/source/scan evidence separately.

## Committed fixture results

Source/protocol `5576c48b7bc7bbab46ecb53bacb1119f7df3ff10` precede execution.
Raw evidence `6f96bf4702f2cb6a923a99ac85031bf559ebedce` precedes this prose.
All **7** complete positive tuples from the existing legacy, skipped-input and
acceleration compatibility corpus are included; every analysis exits **0**.

| Stored tuple | S2S iterations | S2M iterations | Unavailable spectra |
|---|---:|---:|---:|
| legacy_default | 20 | 16 | 0 |
| legacy_plane_check | 20 | 16 | 0 |
| skipped_input | 10 | 5 | 0 |
| paper_default__default | 20 | 16 | 0 |
| paper_default__accelerated | 20 | 16 | 0 |
| paper_odometry_plane_check__default | 20 | 16 | 0 |
| paper_odometry_plane_check__accelerated | 20 | 16 | 0 |

All **231** records have numerical rank **6** at the declared scale/cutoff.
These short fixtures show no rank deficit under that convention; this does not
establish correct associations/motion or explain KITTI failures. No native
command, accuracy measurement or timing comparison was rerun. The original
default/plane-check/acceleration settings and their provenance are preserved.

[Every original/scaled H, gradient, delta, spectrum and decision](evidence/),
[actual commands/exits](evidence/commands.json),
[tuple hashes and stage coverage](evidence/summary.json) and
[lossless imports](evidence/import.json) are retained. Initialization, uncalled
stages and loader skips do not become solved identities.

## Unavailable systems and verification

No damping is added and no eigenvalue is clipped. Only roundoff-sized
antisymmetry is averaged. Nonfinite H, nonrigid poses, excessive asymmetry,
material indefiniteness and arithmetic failure have explicit statuses.
Rank below **6** gives a null condition number, including all-zero H.
Tagged nonfinite gradients and failed deltas are retained, never zero-filled.
A finite H spectrum alone does not establish a successful step.

[Source checks](checks/source/) pass **18** tests in normal and optimized Python,
including the existing stream checker. Independent scalar point/plane Jacobians
test shifted/rotated world origins and the transformed gradient. Analytic systems
verify scaling, quadratic-form preservation, planar/zero rank and indefiniteness.
Corrupt/incomplete runs, changed second-pass bytes and existing output are rejected.

[Full Python, documentation and preservation checks](checks/docs/) retain actual
commands/exits and complete logs. This separate checkout has no built evaluator;
its native evaluation guards are checked by the final-head CI build.

## Reproduce

```bash
trace_run=experiments/results/inten_trace_export/evidence/fixture/paper_default/candidate_on
python3 evaluation/scripts/analyze_inten_conditioning.py \
  --trace "$trace_run/trace.jsonl" \
  --summary "$trace_run/summary.json" \
  --receipt "$trace_run/command.json" \
  --rotation-length-m 1 --rank-rtol 1e-9 \
  --output-dir /tmp/fresh-inten-conditioning
```

The output directory must not exist. Read `iterations.jsonl` with
`summary.json`. A zero analysis exit means complete validated reporting,
not acceptable motion or a positive conditioning finding.

Spectra depend on L, residual definitions/weights, sample geometry/counts and
numerical precision. Sensor centring removes an origin convention but cannot
recover precision lost in the recorded native matrix. This does not validate
GT indexing or binary/source/scan authenticity. Full continuous KITTI failure
traces and their solver-level cause remain pending. The active frozen whole-zoo
replay and its owned scans are untouched; current-main acceleration/trace
compatibility remains separate from that replay.
