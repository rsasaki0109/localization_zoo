"""Interpret the committed complete factorial, without changing the fixed gate."""
from common import E,W,read,git


def table(report,key,title):
    names=['Original','Seed only','Mask only','Seed + mask','Prior rejected mask']
    out=[f'| Endpoint | {" | ".join(names)} |','|---:|'+'---:|'*5]
    for c in report['cases']:
        out.append('| '+str(c['frame'])+' | '+' | '.join(f'{a[key]:.6f}' for a in c['arms'])+' |')
    return title+'\n\n'+'\n'.join(out)+'\n'


def main():
    post=read(E/'postprocessing_protocol.json');commit=post['numerical_commit']
    assert git('show',commit+':experiments/results/inten_translation_consensus/analysis/report.json')==(E/'analysis/report.json').read_text().strip()
    report=read(E/'analysis/report.json');assert not report['candidate_progression_condition_passed']
    length=table(report,'length_ratio','### Translation length / GT')
    vector=table(report,'vector_error','### Translation vector error / GT')
    rotation=table(report,'rotation_error_mrad','### Rotation error (mrad)')
    votes=['| Endpoint | Applied shift (m) | Peak support / valid clusters | Excluded clusters | Used / fallback |',
           '|---:|---:|---:|---:|---|']
    for c in report['cases']:
        d=c['decision'];votes.append(f"| {c['frame']} | {d['shift_m']:.2f} | {max(d['votes'])}/{d['valid_clusters']} | {len(d['excluded_clusters'])} | {'Used' if d['usable'] else 'Fallback'} |")
    text='''# InTEn-LOAM: shared-translation consensus factorial

Shared geometric correction can rescue the already collapsed205 terminal pair,
and the majority check avoids the preceding247 filtering regression. **The fixed
combined mechanism still fails its predeclared progression condition because204
rotation worsens. It does not advance to a production accuracy candidate.**
No threshold or acceptance condition is changed after these results.

This is a completed conditional S2S diagnosis at every inherited endpoint, not a
recursive01 trajectory, official RTE, paper ratio, or proof of moving/static truth.
Production source/API/defaults and every historical result remain unchanged.

Numerical snapshot `'''+commit+'''` precedes this interpretation. The full
[report](analysis/report.json), [frozen protocol](protocol.json), all vote curves,
native traces and actual first-query residuals retain every result.

## Frozen mechanism and controls

Reuse every preceding full-point cluster profile at its original natural estimated
seed. A cluster admits every shift whose trimmed squared NN cost is within0.1²m²
of its own minimum. That band uses the unchanged native Huber0.1m scale with a new
cost interpretation; it is not a paper rule. Give each cluster one vote at every
admissible shift. A plane with ambiguous translation can admit multiple shifts.
Choose maximum votes, breaking ties toward zero then negative motion. Require a
strict majority and an interior grid optimum. Otherwise keep the original seed
and all source facade support. Unavailable profiles are kept and do not vote.

For usable consensus, add the selected displacement along the archived original
seed translation direction, preserving rotation. Exclude source facade clusters
that do not admit that common shift. Grid(-3..3m,0.05m) and80percent point trimming
stay identical to the previous diagnostic. No hand region, GT, parameter sweep,
per-case winner or recomputation after the seed change is used.

All inherited endpoints190/200/202/203/204/205/247/433 and the complete2×2 are
retained: original, corrected seed only, consensus mask only, corrected seed with
mask. The fifth arm exactly repeats the preceding rejected mask as a negative
and producer-parity control. Only the combined arm was eligible for progression.

The original profile was selected on07, diagnosed on00, and inherits the prior
fraction0.03 chosen on DiLO01. Every prior KITTI observation is development;
there is no independent held-out claim. Both previous PCA extensions remain on;
Cauchy, predicted-DOR, row completion and map-plane spread remain off.

The seed is also the center of the native adaptive translation prior. Therefore
seed correction changes the prior center, first associations and potentially
every data class; it is not merely a different initialization. Masking changes
the data Hessian and adaptive-prior information. Other parameters stay fixed.

## Every outcome

'''+length+'\n'+vector+'\n'+rotation+'''
At205, original length/GT0.372446 becomes0.936491 with seed correction alone,
0.394997 with support removal alone, and0.955054 with both. Combined vector error
falls0.627582→0.045419 and rotation2.419936→1.798922mrad. Under this original
history, the seed/prior-center intervention is the larger conditional rescue.
This does not certify a physical object label or a recovered recursive trajectory.

At204, original length/GT0.607064 becomes0.907108 with seed only,1.003770 with
mask only, and1.004613 with both. Combined vector error falls0.392957→0.009234,
but rotation worsens1.716533→2.958909mrad. At190 combined rotation also worsens,
2.320873→2.629751mrad. Every adverse arm/metric is retained above.

At247 only54 of134 valid clusters admit the most-supported shift, so no strict
majority is found. The new arms preserve the original0.522727 length/GT instead
of the preceding rejected mask's0.046618. This avoids additional harm but does
not repair the original failure. At433 the vote optimum lies on the search
boundary; fallback similarly retains the original result and forfeits the
preceding mask's local improvement.

Combined length and vector errors are lower at6 endpoints and equal at2, with
no higher values; combined rotation is lower at4, higher at2 and equal at2.
These selected development endpoint counts are not a sequence accuracy score.

![All factorial errors](figures/factorial_errors.png)

### Every consensus decision

'''+ '\n'.join(votes)+'''

![All consensus vote curves](figures/consensus_votes.png)

## Predeclared progression condition and limits

Before output, the protocol required: lower204 length and vector errors, no
higher204 rotation, no higher205/247 length or vector errors, and no new20percent
bad-length label among these endpoints. The204 rotation condition fails. Retain
that negative result rather than relaxing the condition to promote a favorable
translation outcome. No full-sequence candidate or default is introduced here.

Cluster majority is not object majority or static truth. Fragmentation, repeated
geometry, one-way trimming, occlusion, point sampling and aperture can bias votes;
independent motions may dominate. The original prediction determines the search
direction/range, and these cached profiles do not change after correction. Only
one translation direction is corrected. These controls establish conditional
mechanism effects; they cannot establish full01 recovery or explain10 drift.

The next mechanism must retain the demonstrated seed-rescue effect while addressing
the rotation/support tradeoff. A future setting must be frozen opt-in/default-off,
evaluated with one configuration on every full00–10 sequence, and retain all
regressions before any official RTE or paper-ratio claim.

## Numerical integrity and reproduction

All40 terminal registrations finish with native exit0. Every original and prior
rejected-mask output, first H/g/count and full solver iteration/state matches the
corresponding committed preceding arm exactly. Within each fixed seed, masked
non-facade first systems remain exact and facade queries are ordered subsets.
Every actually accepted first-query residual in every arm is retained; independent
reconstruction gives maximum relative H error1.558953e-16 and g error0. The first
joint Hessian including the adaptive prior is reconstructed; maximum relative
error1.629545e-15. Every finite damped update passes backward-error checks,
maximum5.343795e-17. Forced returned counts are independently checked as first
class-count sums. Majority/boundary/empty/tie/ambiguous-profile mechanism tests
and the independent equation analysis retain actual zero exits.

Use an isolated producer checkout `99062a08` with the compiler, pinned libraries
and existing development container specified in [controls](controls/), then run:

```bash
PYTHONDONTWRITEBYTECODE=1 /workspace/localization-env/host-venv/bin/python experiments/results/inten_translation_consensus/execute.py
```

The command restores losslessly verified committed extracted frames and profiles;
it fetches no raw scans and reads no GT in consensus or native input. GT is read
only by offline error analysis. Feature serialization exactly matches the prior
producer. Source commits, cwd/environment, compiler/link commands, binary/object/
static-library hashes, linked-library metadata, input hashes, stdin, full traces,
actual query logs and actual exits are retained with byte-roundtrip archives.
The isolated diagnostic source is an exact declared transformation of the archived
native source; no production source is modified. Portable evidence checks use
that snapshot, while full local checks also verify live production pins and the
entire historical diff.

Fresh native checks, Python discovery and publication/preservation checks are
recorded at `completion/`. The roadmap's study/variant/paper-ratio generators are
rerun, restoring only proved generation-clock differences. No historical result,
primary Table6, root README/index or paper-number JSON is promoted by this trial.
'''
    (E/'README.md').write_text(text)
    method=W/'papers/inten_loam/README.md';before=method.read_text()
    assert '## Shared-translation consensus factorial' not in before
    addition='''\n## Shared-translation consensus factorial (outside paper; not adopted)

The [fixed consensus diagnostic](../../experiments/results/inten_translation_consensus/README.md)
compares original, corrected-seed-only, consensus-mask-only and both, with the
preceding rejected mask as a fifth control. Every inherited current01 endpoint
uses one frozen rule, no GT/region in decisions, and no per-case winner. Numerical
commit `'''+commit[:8]+'''` precedes interpretation. All40 terminal calls and every
first-query residual are retained; original and prior negative controls reproduce
exactly. All prior KITTI is development, not independent held out.

'''+length+'\n'+rotation+'''
At205, seed correction alone reaches0.936491 length/GT, versus0.394997 for mask
alone and0.955054 combined, from0.372446 original. Combined vector error/GT falls
0.627582→0.045419. At247 the majority fallback keeps0.522727 rather than the prior
mask's0.046618, avoiding extra harm without repairing the original failure.
At433 a boundary optimum falls back. Combined length/vector error counts are
6 lower/2 equal; rotation4 lower/2 higher/2 equal.

The fixed progression condition fails because204 rotation worsens
1.716533→2.958909mrad despite length/GT0.607064→1.004613. At190 rotation also
worsens2.320873→2.629751mrad. No condition is relaxed after output, no production
option/default is added, and no full-sequence accuracy is claimed. A corrected
seed also moves the adaptive translation-prior center; this is conditional
seed/prior/support sensitivity, not recursive01 recovery or10 drift resolution.
'''
    method.write_text(before+addition)
    roadmap=W/'docs/roadmap.md';old=roadmap.read_text();lines=old.splitlines(keepends=True)
    positions=[i for i,line in enumerate(lines) if line.startswith('13. ')];assert len(positions)==1
    lines.insert(positions[0],'''    The [shared-translation consensus factorial](../experiments/results/inten_translation_consensus/README.md)
    separates seed correction from support removal at every inherited01 endpoint,
    with the previous rejected mask as an exact negative control. All40 terminal
    calls and actual first-query equations are retained. At205 seed-only length/GT
    is0.936491, mask-only0.394997 and combined0.955054, from0.372446 original.
    A majority fallback avoids extra247 harm but leaves its original failure.
    Combined translation errors improve at6 endpoints and tie at2;204 rotation
    worsens1.716533→2.958909mrad, failing the predeclared progression condition.
    No gate retuning, production option or full-sequence accuracy claim follows.
    Seed rescue is conditional on fixed original history and also changes the
    prior center. Next address the rotation/support tradeoff while retaining this
    effect. Full01 recovery and10 drift remain open. Numerical `'''+commit[:8]+'''`
    precedes interpretation; every prior KITTI observation remains development.

''')
    roadmap.write_text(''.join(lines))


if __name__=='__main__':main()
