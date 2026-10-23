# Fixed-node S2S intensity feasibility experiment

Prospective protocol in `protocol.json`, frozen before native execution.
This repository extension is S2S-only, opt-in and default off; it is not specified
by the paper. Existing defaults, S2M and all stored results are preserved.

All previously exposed KITTI is development. The fixed radius derives from
cubic support, without a parameter sweep. Every inherited endpoint and field
path is included; the motion gate is fixed before execution. No new full
sequence or accuracy improvement is claimed by this local experiment.

Commands (from the repository root, Python environment and Docker command
are recorded in `common.py` and each actual process receipt):

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/build.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/run.py
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_intensity_consistent/verify.py --output /workspace/localization-env/inten-intensity-consistent-20261022/report_stage/summary.json --details /workspace/localization-env/inten-intensity-consistent-20261022/report_stage/boundaries.json.gz
```

Build sources, binaries, linked-library hashes, actual argv/cwd/environment,
stdout/stderr and OS exits are retained. All outcomes, including failures, must
be committed before numerical interpretation or README tables.

## Result: fixed field, failed motion progression gate

Numerical commit `6eec3472` precedes this interpretation. All31 inherited endpoints
produce all186 endpoint/arm outcomes in four replay subprocesses (actual exit0).
Five field-probe subprocesses also exit0. All124 complete legacy/normalized
control traces and profiles reproduce the committed previous experiment exactly.
No endpoint, mixed/invalid field sample or adverse case is excluded.

All208565 real native queries on8829 paths reconstruct independently, including
26487 interior derivative controls. At all15763 both-valid faces, no value or
gradient limit jump exceeds1e-8 (maxima1.110223025e-16 and1.942890293e-16).
All five previous zero-weight holes now have equal limits from valid neighboring
cells. Count coverage still leaves184 mixed and191 neither-valid faces; this is
not a claim of global validity. Original observations are never overwritten and
missing values are independent of query cell, cache population and thread order.

| Complete conditional primary comparison | Legacy joint | Fixed-node joint |
|---|---:|---:|
| Bad length / vector endpoints | 7 /14 | 7 /14 |
| Sum absolute length-ratio error | 4.741213404 | 4.804046127 |
| Sum vector error /GT step | 14.083975645 | 14.133971193 |
| Sum rotation error [rad] | 0.059478496 | 0.058919262 |
| original01/247 length/GT | 0.522727335 | 0.464794342 |
| original01/204 length/GT | 0.607064437 | 0.606566380 |
| original01/205 length/GT | 0.372445688 | 0.371744129 |
| actual08/39 length/GT | 0.801801894 | 0.801362892 |

There are no newly bad labels in either paired joint or consensus arm; all
original consensus rescues stay non-bad. However total length and vector errors
worsen, failing both predeclared improvement requirements. Original247 remains
a severe collapse and worsens; rejected consensus still collapses on actual02/1892
(length/GT0.125854888→0.124100291). The lower rotation total and smoother field do
not justify adopting this as a motion fix. No full candidate run, trajectory
accuracy claim, paper ratio or default change follows. The earlier normalized
candidate and all its negatives remain separately preserved.

[Every endpoint/arm and every former zero-weight hole](cases.md) is tabulated.
The complete raw output, byte-roundtrip manifests, frozen source snapshots,
binary/library hashes, commands, OS exits and independently rebuilt report are
under `groups/`, `fields/`, `checks/build/` and `report/`. Native mechanism and
regression tests55 passed with actual exit0. The local protocol is frozen at
`b5c7e208`; build evidence at `d86516ae`; numerical evidence at `6eec3472`.

The original profile07, diagnosis00, inherited DiLO01 prior0.03 and every later
KITTI observation are development. Radius three is derived analytically from
four-control cubic support, without sequence tuning or threshold sweeps.
`--inten-loam-paper-set s2s_intensity_consistent=1` is outside the paper,
default off and S2S-only; enabling it together with normalized mode throws
before updates. S2M, old defaults, old results and Table6 remain unchanged.
The unresolved failure now needs a motion/geometry explanation; smoothing this
field alone did not recover the underestimated seed on original247.

## Integration validation

Complete local Python regressions377 return0 (five existing environment skips).
Independent recorded-report checks under both Haswell and Sandybridge kernels
return0 with all original tolerances, metrics and gate decisions unchanged.
Showcase, leaderboard and paper-ready checks return0. Full-variant and paper-ratio
regeneration is byte-identical to the committed canonical tables. Paper-number
metadata, fixture golden values, landing page and old experiment bytes match
parent `c9fb225b`. Actual commands, complete logs and hashes are committed in
`checks/final/`. The comparison-plot command and artifact hashes are retained in
`checks/plot/`; its only numerical source is committed `6eec3472`.

Full Native/ROS2 CI must pass on the final review head before this PR is marked
ready. Passing integration checks do not turn the failed motion gate into a pass.
