# Zero-shift majority: native safeguard rejected

The parameter-free safeguard fixes the immediate highway cancellations but fails
its prospective gate. It must not become a production option or start another
full recursive trial. Numerical evidence was committed before this interpretation:
old controls `25043302`, actual-history01 `d2c40347`,02 `e1c35ec1`,08 `c3e330f3`,
and complete report `883aa3d6`.

The frozen [protocol](protocol.json) selects zero shift when zero already has the
existing strict-majority support, otherwise retaining the old maximum-vote rule
and interior/majority fallback. It recomputes the selected facade mask and really
re-solves translation with native re-association and the adaptive prior at the
original unmasked joint estimated rotation. No new numerical threshold, GT
selection, semantic moving-object label, per-frame winner or parameter sweep.
This diagnostic intervention is outside the paper; production code and defaults
are unchanged.

## Complete fixed controls

All8 original-history endpoints are retained, including204/205 rescues and247/433
fallbacks. Every newly exposed immediate length-damage entry from the parent
full00–10 diagnosis is included:01/426,811,838;02/1890,1892,1899,4473; and
08/39,41–46. Each entry-1 is added, deduplicated without dropping any outcome.
That is31 pair endpoints and248 native calls. All8 modes are retained: original
joint; old combined; new combined; old seed-only; old mask-only; new seed-only;
new mask-only; neither translation intervention at original estimated rotation.
Modes use the same extraction, estimated input, profile costs and solver rules.
The unchanged native profile helper is evaluated once per pair and its complete
costs reused in every arm. This is a diagnostic runtime economy, not a speed claim.

Before interpreting any treatment, original/old-consensus poses, full solver
traces, votes, admissions and masks must match the archived controls exactly.
Original cached profile costs also match bit-for-bit. All actual native exits
are zero. Independent checks reconstruct every cost admission, selection, mask,
finite translation update, additive pose update and unchanged estimated rotation.
Native solver output and exact labels are not relaxed; portable offline numeric
comparison uses the inherited relative1e-10/absolute1e-12 bound.

[All31 endpoints](report/all_cases.md) and every factorial metric in
`groups/{old,01,02,08}/result.json` include adverse, neutral and fallback cases.
Length/GT is estimated translation norm divided by offline true step length;
vector error/GT includes direction. A good length alone does not imply a good
vector. These conditional pair replays are not new trajectories or official RTE.

## Results and negatives

The7 immediate01/02 length collapses become non-bad under the inherited20percent
criterion. At01/426, old→new length/GT is0.054142→0.990705 and vector error/GT
1.052997→0.013023. The original-history204 rescue remains: length/GT
1.002040→0.999256 and vector error0.007531→0.007153. Both fallback outputs remain
bitwise original joint.

The full gate still rejects the safeguard:

- Original-history200 length/GT worsens1.002987→1.005762 and vector error
  0.008546→0.010444. At202 length/GT0.952965→0.934458 and vector error
  0.047452→0.065800; at203,0.965060→0.961275 and0.037781→0.041354.
- Actual-history02/1891 is newly bad: length/GT0.980923→0.196111 and vector error
  0.026097→0.803917. Its original joint was already short at0.199499. Old seed-only
  reaches0.981551, but old mask-only reaches only0.198159. Retaining zero majority
  removes a necessary seed correction here, even though it prevents nearby
  cancellations. This is native factorial evidence, not a posthoc pose splice.
- None of the7 immediate08 length-damage entries becomes non-bad. At08/39,
  old=new length/GT0.704652 and vector error1.029104; zero was already the old
  chosen shift. Its original joint length/GT0.801802 has vector error1.122369:
  the inherited history/direction was already poor. At08/41, old→new
  0.679243→0.679195 remains bad. A zero-shift preference cannot explain or cure
  all accepted-refinement length damage.

Thus support for zero shift alone cannot distinguish harmful cancellation from
a necessary correction. Do not relax the predeclared gate after seeing these
outputs. A next proposal must distinguish those estimated-input situations and
keep02/1891 and08 as mandatory adverse controls before any full recursive run.
Physical object motion has not been established. All previous KITTI observations
remain development: original profile07, diagnosis00, inherited DiLO01 prior0.03,
and every later investigation/full run. There is no held-out claim.

## Acquisition, provenance and commands

The opt-in fetcher `--frame INDEX` preserves original frame IDs and obtains only
requested pairs through the official archive/HTTP-range path. Missing/invalid
indices fail, sparse runs avoid downloading the intervening frames, and default
whole-sequence fetching remains available. One owned raw sequence at a time was
used; all30 acquired scan hashes match parent full-run `inputs.json`. Extraction
uses the verbatim pinned runner loader, elevation correction0.17degrees,
max120m, no voxel/deskew and original intensity handling. Complete original/old
solver parity verifies pair independence for these actual histories rather than
assuming it. Raw scans and feature caches are not committed.

Run from a clean study worktree, using the existing recorded Docker dependency
image and pinned libraries. `build.py` records the resolved compiler arguments,
source/binary/library hashes, worker settings and actual exits. Fresh owned
staging paths are required; preserve failed attempts rather than overwrite them.

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_zero_consensus_control/build.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_zero_consensus_control/run_group.py --group old
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_zero_consensus_control/run_group.py --group 01
# Archive and round-trip; commit the complete group; normal git push must succeed.
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_zero_consensus_control/cleanup.py --sequence 01
# Commit and push cleanup BEFORE repeating that lifecycle for02, then08.
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_zero_consensus_control/verify.py --output /tmp/zero-consensus-verification.json
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python -m unittest discover -s tests
```

Each group's `fetch_command.json`, `extract_command.json`, `native_command.json`,
`input.txt`, `offline_truth.json`, `input_sources.json` and archive manifest retain
actual argv/cwd/source/env/exit, exact estimated seeds and hashes. Native input
never contains GT. `cleanup/` verifies remote numerical HEAD and rechecks every
archive byte before deleting only the owned raw sequence; cleanup commits were
pushed before the next acquisition. `checks/storage/` proves immutable committed
evidence hardlink identity; every old logical path/source/build/binary is retained.
`verify.py` reconstructs the report, including the failed adoption gate; successful
verification does not mean the candidate passes. Publication aggregates, Table6,
paper metadata and old full-sequence evidence are unchanged.
