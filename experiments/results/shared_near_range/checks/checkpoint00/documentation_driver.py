from pathlib import Path
import json
import re
import subprocess

repo = Path('/workspace/localization_zoo')
e = repo / 'experiments/results/shared_near_range'
path = e / 'checkpoints/00/summary.json'
assert path.read_bytes() == subprocess.check_output(
    ['git', 'show', 'HEAD:' + path.relative_to(repo).as_posix()], cwd=repo)
x = json.loads(path.read_text())
assert x['completed_sequences'] == ['04', '00'] and not x['whole_zoo_audit_complete']
assert x['audit_exit_codes'] == {'04': 1, '00': 1}
assert all(all(v == 0 for v in c['metric_abs_delta'].values()) and
           all(c['saved_pose_bytes_equal'].values()) for c in x['frozen_comparisons'])
assert {c['method'] for c in x['frozen_comparisons'] if c['sequence'] == '00'} == {
    'A-LOAM', 'F-LOAM', 'LeGO-LOAM', 'Mesh-LOAM', 'VLOM'}

p = e / 'README.md'
s = p.read_text()
s = s.replace('## Completed 00 runs; frozen comparison and cleanup pending',
              '## Completed KITTI checkpoint: 00')
a = '''[`checkpoints/04/`](checkpoints/04/) is generated only from committed,
cleaned sequence checkpoints; live 00 outputs are excluded. The checkpoint's
verification exit 0 does not replace the retained native audit exit 1.'''
b = '''[`checkpoints/04/`](checkpoints/04/) records the earlier 04-only checkpoint.
The current [`checkpoints/00/`](checkpoints/00/) covers both committed, cleaned
sequences. Its verification exit 0 does not replace the retained native audit
exits 1.'''
assert a in s
s = s.replace(a, b)
a = '''Only Mesh-LOAM's frozen replay is **still running**; the extraction's effect
on that cell is not yet established. The parent frozen audit has no terminal
exit yet. No setting is changed or historical result replaced.'''
b = '''Mesh-LOAM's frozen replay now also completes with native exit 0. Its RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. Therefore
all five historical 00 differences predate extraction; their original causes
remain unresolved. The parent frozen historical audit exits **1**, retaining
the same differences rather than reporting a clean historical pass.
The complete [frozen comparison](sequences/00/frozen_comparison.json),
[commands and exits](sequences/00/frozen_baseline/) and
[verified input cleanup](sequences/00/cleanup.json) were committed at
`10cf31616365b41878ec9311f0b705e798716c51`. No setting is changed or historical
result replaced.'''
assert a in s
s = s.replace(a, b)
a = '''The official scans remain available for the running frozen comparisons. No 01
inputs are fetched before 00 comparisons and verified cleanup complete.
The completed, cleaned sequence checkpoint count therefore remains 14 cells
(04); these 20 completed candidate runs are reported separately.'''
b = f'''All 00 scan hashes were verified before deleting the owned scans and alias.
Subsequent sequences are excluded until their own comparison and cleanup
checkpoints are committed. The current [checkpoint summary](checkpoints/00/summary.json)
and [comparison CSV](checkpoints/00/comparison.csv), committed at `57fb04d3`,
contain **{x['completed_cell_count']} of {x['planned_cell_count']} cells** across 04 and 00:
{x['historical_matching_cell_count']} historical matches and {x['historical_drift_cell_count']} differences. All {len(x['frozen_comparisons'])} frozen controls have zero
recorded metric differences and byte-identical saved poses. Both historical
sequence audits exit 1; checkpoint verification exits 0. Whole-zoo coverage is
still incomplete.'''
assert a in s
s = s.replace(a, b)
p.write_text(s)

old_footer = '''[Protocol and current scope](../../experiments/results/shared_near_range/README.md)
distinguish these completed runs from a cleaned sequence checkpoint: 00 input
cleanup and the whole-zoo audit are incomplete. Defaults remain off.'''
new_footer = '''The full [frozen comparison](../../experiments/results/shared_near_range/sequences/00/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/00/cleanup.json)
were committed at `10cf31616365b41878ec9311f0b705e798716c51`.
The [committed checkpoint](../../experiments/results/shared_near_range/checkpoints/00/summary.json)
now covers cleaned 04 and 00; the whole-zoo audit remains incomplete.
Defaults remain off.'''
pattern = r'<!-- SHARED_NEAR_RANGE_00_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_00_DIAGNOSTICS -->'
for method in ('aloam', 'floam', 'lego_loam', 'mesh_loam', 'vlom'):
    p = repo / 'papers' / method / 'README.md'
    text = p.read_text()
    diagnostic = re.search(pattern, text, re.S).group()
    assert old_footer in text
    text = text.replace(old_footer, new_footer)
    if method == 'mesh_loam':
        text = text.replace('## Shared-loader replay: KITTI 00 comparison in progress',
                            '## Shared-loader replay: KITTI 00 frozen comparison complete')
        old = '''The frozen pre-extraction comparison is still running. The cause of this
historical difference and whether extraction affects this cell remain
**unresolved** until that comparison completes. No setting is reselected;
the stored aggregate and published paper comparison are preserved.'''
        new = '''The frozen pre-extraction replay also completes with native exit 0. RPE,
ATE, frame count and official translation/rotation RTE have **zero** recorded
differences from the extracted implementation, with byte-identical saved poses.
This historical difference therefore predates extraction; its original cause
remains unresolved. The frozen historical audit also exits 1. No setting is
reselected, and the stored aggregate and published paper comparison are preserved.'''
        assert old in text
        text = text.replace(old, new)
    assert re.search(pattern, text, re.S).group() == diagnostic
    p.write_text(text)

p = repo / 'docs/roadmap.md'
s = p.read_text()
a = '''     A-LOAM, F-LOAM, LeGO-LOAM and VLOM's frozen pre-extraction controls now
     finish with zero metric differences and byte-identical saved poses, so
     their historical differences predate extraction. Mesh-LOAM's comparison
     and input cleanup are still in progress. The implementation is merged'''
b = '''     All five frozen pre-extraction controls now finish with zero metric
     differences and byte-identical saved poses, so their historical differences
     predate extraction; the original causes remain unresolved. Input hashes
     were verified before removing the owned 00 scans. The implementation is merged'''
assert a in s
s = s.replace(a, b)
a = '     00 is not yet a completed sequence checkpoint. The historical NCLT cutoff is retained'
b = f'''     The committed, cleaned 04/00 checkpoint now covers {x['completed_cell_count']}/{x['planned_cell_count']} cells:
     {x['historical_matching_cell_count']} historical matches and {x['historical_drift_cell_count']} differences, all reproduced by frozen controls.
     Both historical audits retain exit 1; checkpoint verification exits 0.
     [Checkpoint, CSV and commands](../experiments/results/shared_near_range/checkpoints/00/summary.json).
     The historical NCLT cutoff is retained'''
assert a in s
p.write_text(s.replace(a, b))
print('Documented committed 04/00 checkpoint; existing diagnostics preserved.')
