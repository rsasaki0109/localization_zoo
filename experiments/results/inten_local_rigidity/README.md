# Local rigidity and the location of lost support

The two-frame distance witness does not distinguish the earlier reciprocal
rescues from the adverse histories. Removed facade centers have a larger
median discrepancy than kept centers in both. Their signed contributions and
source regions differ. This is a negative result for using this discrepancy
alone to decide which constraints improve ego motion; no selector or new
motion candidate is introduced.

Prospective source/protocol `8af883ce` precedes complete numerical evidence
`7e8ef2ec`; every-case and complete spatial tables `d86b9c01` precede this
interpretation. The [parent support study](../inten_support_information/README.md)
retains all558 exact native outcomes across all31 inherited endpoints/all18
arms. Here those immutable archives are reused without another native solve.
All four predeclared passive arms6/7/14/15 and every joint/executed refinement
iteration remain:540 geometric class histories,42,992 native class observations,
27,360,255 repeated source rows, and3,637,657 removed native-valid residuals.
There are50,776,466 eligible directed geometric relations out of73,428,600
possible ones. First/last pre-update boundaries retain247,032 native-valid
point rows. These are repeated observations, not independent scenes or points.

## Fixed construction and its limits

For each source feature, choose its10 nearest other source features in the
same entire geometric class, sorting exact distance ties by source index.
Ten inherits the original forward-neighbor fanout; it was fixed before this
study, with no sequence-based choice, sweep, threshold or gate. Every case,
arm, class and iteration uses the same construction. Fewer neighbors are used
only when the class has fewer other features. A relation is eligible only if
both endpoints are native-valid; a missing neighbor is not replaced by a
farther valid one. Kept/removed partitions split centers only and keep the
same all-valid neighbor population.

For a relation from source features p_i,p_l to their original first native
anchors q_j(i),q_j(l), record:

`distance(q_j(i),q_j(l)) - distance(p_i,p_l)`.

This equation and neighbor ranking use no estimated/predicted/GT pose,
registration outcome or new optimization. The native assignments themselves
remain conditional on pose-dependent matching. A moving rigid object can
preserve these distances; the mechanism test explicitly demonstrates that
limitation. Cross-object neighbors, sampling, occlusion and discrete anchor
choices can violate them. This is a pose-free equation conditional on frozen
anchors, **not independent static truth or an independent ego-motion witness**.
Intensity has no geometric anchor and remains an unchanged native control.

All iteration aggregates retain counts, signed/absolute/squared sums, mean,
RMS, maximum, normalized length sums and exact per-center score quantiles.
Missing relations and zero-length source relations stay explicit. At every
first/last recorded pre-update boundary, every valid point retains its source
and anchor indices, kept flag, eligible count, discrepancy and signed raw
fixed-input-axis g/H contribution. The original strict native reconstruction
uses rtol1e-10/atol1e-12. The axis is each solver's isolated estimated input
translation; a zero input axis is undefined. These are additive raw partial
derivatives with rotation fixed, not independent solved class effects,
additive Schur terms or physical covariance.

## Rescue and adverse controls

The overview below uses the predeclared primary arm14 joint phase, initial
facade linearization. Each center's score is the mean absolute error over its
eligible directed neighbors; the table gives the median across centers.
Length/GT is unchanged historical context and never enters the witness.

| Inherited control | Unchanged length/GT | Kept median (m) | Removed median (m) | Removed positive gt | Removed negative gt |
|---|---:|---:|---:|---:|---:|
| old/202, rescue |0.973614185|0.0938321054|0.136805516|1.72176970|-0.613969519|
| old/204, rescue |0.985996689|0.113406176|0.162087936|2.14568195|-1.07127335|
| old/247, lost recovery |0.459890548|0.0913314139|0.146574777|1.54384628|-0.318560579|
| 08/39, newly bad length |0.682142975|0.0778109345|0.177262541|0.564135901|-6.31435090|
| 08/45, newly bad length |0.776746136|0.0628724483|0.0955789368|1.07639760|-5.25493393|

A larger removed discrepancy appears in both signs of the motion outcome;
it does not establish a safe removal rule. Positive and negative raw gt
components coexist within a removed class. Their sum alone can conceal
cancellation. Larger geometric inconsistency is not proof of a moving object,
and the most geometrically consistent support need not be the support that
corrects ego motion.

For location, these are the largest-absolute-g removed source-facade cluster
partitions at those same initial boundaries. This descriptive ranking was
chosen after execution to summarize the already named controls; it is not a
selection parameter or a proposed filter. Every cluster of every case/arm
remains in the complete spatial table. Coordinates are source-frame meters;
cluster labels are frame-local native labels, not tracked physical objects.

| Control | Native cluster | Points | Source centroid xyz (m) | Removed gt | Center median discrepancy (m) |
|---|---:|---:|---|---:|---:|
| old/202 |397|4|(-3.63376045, -3.56820554, -1.11650306)|0.284267071|0.0858749218|
| old/204 |600|12|(-6.13900276, -3.77186610, -0.872672205)|0.592858941|0.141436052|
| old/247 |141|35|(4.66148338, -4.81138757, -1.11284503)|0.465714490|0.156524845|
| 08/39 |36|44|(5.45872609, 3.15894228, -1.15665699)|-3.98209389|0.203261587|
| 08/45 |3|33|(-0.110326560, 2.34552087, -0.921694983)|-1.65775595|0.0789391984|

These locate different signed losses near different source regions; they do
not identify an object or prove trajectory causality. In particular the
adverse08/45 region has a small distance discrepancy, overlapping the rescued
old/202 example. No threshold was evaluated, selected or waived.

[Every case/history](cases.md) and [complete table manifest](tables_manifest.json)
retain all other endpoints, classes, intermediate histories and opposing
components. Complete `cases.summary.json` and `locations.md` restore from
`cases.summary.json.xz` and `locations.md.xz`; no row or numeric precision is
dropped. `report/` retains the full original summary and point CSV bytes with
stored/raw hashes. Join a point CSV's `source_index` to the pinned parent's
`support_features` for exact xyz/row/native cluster; spatial aggregates include
centroids and bounds. Unexecuted refinements and empty valid classes remain.

## Reproduction and failures

Actual command/cwd/environment/source/exits and complete stdout/stderr are
under [initial checks](checks/initial/archive.json),
[table checks](checks/tables/archive.json), and
[lossless transport checks](checks/table_transport/archive.json).
The following entry point ran the mechanisms, analysis and full report pack:

```bash
/workspace/localization-env/host-venv/bin/python experiments/results/inten_local_rigidity/execute.py
```

Its recorded analysis command is:

```bash
env PYTHONPATH=/workspace/localization-env/host-venv/lib/python3.12/site-packages /opt/codex/runtimes/codex-primary-runtime/dependencies/python/bin/python3 experiments/results/inten_local_rigidity/analyze.py --output-dir /tmp/localization-zoo-inten-local-rigidity-20261023/output
```

Use a fresh output directory when repeating the analysis; existing raw results
are preserved. Export every committed generated table byte with:

```bash
python3 experiments/results/inten_local_rigidity/pack_tables.py --export --output-dir /tmp/inten_local_rigidity_tables
```

Initial rendering exit1 at02/1890 arm7 refinement's empty edge boundary remains
in [the original failure](checks/empty_boundary_failure/archive.json).
Renderer-only recovery `6404d6bc` explicitly retains the empty observation and
rejects missing nonempty rows. No witness or native measurement changed.
The generated complete tables then exceeded workspace capacity during archive
creation; temporary-link creation also hit ENOSPC. All raw table bytes were
preserved before transport recovery `ed660150`/`d86b9c01`. Complete XZ/CRC64
compression restores every original byte. Past CI proofs were byte/mode
verified before moving to private host storage; original paths remain readable.
Native inputs, binaries, libraries, worktrees and all older results remain.

The eight initial mechanism checks and full analysis/pack exit0. Table recovery
and complete roundtrip exit0. Full-suite and canonical preservation receipts
are recorded separately as checks finish. Final exact-head Native/ROS2 CI,
every essential step and both complete decoded logs are required before Ready.

All previous parameter provenance stays fixed; all observed KITTI is development.
Earlier reciprocal/prior/field/budget/mask gates remain failed. There is no
production/default change, retuning, full candidate run, new trajectory score,
paper-table update or SOTA claim. A future selection rule needs evidence that
separates ego motion from other rigid motion and retains every rescue/adverse
control; this two-frame local distance witness alone does not supply it.
