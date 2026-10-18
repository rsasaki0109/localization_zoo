"""Only apply after checkpoint01 report and full input cleanup are committed."""
from pathlib import Path
import json
import re
import subprocess

repo = Path('/workspace/localization_zoo')
e = repo / 'experiments/results/shared_near_range'
report = e / 'checkpoints/01/summary.json'
rel = report.relative_to(repo).as_posix()
assert report.read_bytes() == subprocess.check_output(['git', 'show', 'HEAD:' + rel], cwd=repo)
x = json.loads(report.read_text())
assert x['completed_sequences'] == ['04', '00', '01'] and not x['whole_zoo_audit_complete']
assert x['exit_code'] == 0
assert all(v == 1 for v in x['audit_exit_codes'].values())
assert all(all(v == 0 for v in c['metric_abs_delta'].values()) and
           all(c['saved_pose_bytes_equal'].values()) for c in x['frozen_comparisons'])
assert {c['method'] for c in x['frozen_comparisons'] if c['sequence'] == '01'} == {'CT-ICP', 'Mesh-LOAM'}
cleanup_path = 'experiments/results/shared_near_range/sequences/01/cleanup.json'
cleanup = json.loads(subprocess.check_output(['git', 'show', 'HEAD:' + cleanup_path], cwd=repo))
assert cleanup['hashes_verified_before_removal'] and not Path(cleanup['owned_input_dir']).exists()
closed_commit = subprocess.check_output(['git', 'log', '-1', '--format=%H', '--', cleanup_path], cwd=repo, text=True).strip()
report_commit = subprocess.check_output(['git', 'log', '-1', '--format=%H', '--', rel], cwd=repo, text=True).strip()

p = e / 'README.md'
s = p.read_text()
a = '''The current [`checkpoints/00/`](checkpoints/00/) covers both committed, cleaned
sequences. Its verification exit 0 does not replace the retained native audit
exits 1.'''
b = '''[`checkpoints/00/`](checkpoints/00/) records the earlier cleaned 04/00 checkpoint.
The current [`checkpoints/01/`](checkpoints/01/) adds the committed, cleaned 01
sequence. Its verification exit 0 does not replace the retained historical
audit exits 1.'''
assert a in s
s = s.replace(a, b)
s = s.replace('## Completed candidate runs: 01; full frozen comparison and cleanup pending',
              '## Completed KITTI checkpoint: 01')
s = s.replace('The current [checkpoint summary](checkpoints/00/summary.json)',
              'The earlier [00 checkpoint summary](checkpoints/00/summary.json)')
a = '''The controller is comparing both historical differences with the frozen
pre-extraction executable. The complete comparison and owned-input cleanup
remain pending, so these candidate runs are **not** added to the cleaned
checkpoint count. The cleaned 04/00 checkpoint remains **34/177** cells.
Attribution of either 01 difference to extraction remains unestablished here.
Stored aggregates, Table 6, paper metadata and defaults are unchanged.'''
b = f'''Both frozen pre-extraction controls now complete with native exits 0. RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. The frozen
historical audit also exits 1. Both historical differences therefore predate
extraction; their original causes remain unresolved.
The [full comparison](sequences/01/frozen_comparison.json),
[commands and exits](sequences/01/frozen_baseline/) and
[hash-verified input cleanup](sequences/01/cleanup.json) were committed at
`{closed_commit}`. The owned scans and alias are removed.

The [current checkpoint](checkpoints/01/summary.json) and
[comparison CSV](checkpoints/01/comparison.csv), committed at `{report_commit}`,
cover **{x['completed_cell_count']}/{x['planned_cell_count']} cells** across 04, 00 and 01:
{x['historical_matching_cell_count']} historical matches and {x['historical_drift_cell_count']} differences. All {len(x['frozen_comparisons'])} frozen controls have
zero recorded metric differences and byte-identical saved poses. All three
historical audits retain exit 1; checkpoint verification exits 0. Remaining
sequences are excluded until their own comparisons and cleanup are committed.
Stored aggregates, Table 6, paper metadata and defaults are unchanged; whole-zoo
coverage remains incomplete. Required checks are retained under
[checks/checkpoint01/](checks/checkpoint01/).'''
assert a in s
p.write_text(s.replace(a, b))

for method in ('ct_icp', 'mesh_loam'):
    p = repo / 'papers' / method / 'README.md'
    text = p.read_text()
    diag = re.search(r'<!-- SHARED_NEAR_RANGE_01_DIAGNOSTICS -->.*?<!-- /SHARED_NEAR_RANGE_01_DIAGNOSTICS -->', text, re.S).group()
    text = text.replace('## Shared-loader replay: KITTI 01 comparison in progress',
                        '## Shared-loader replay: KITTI 01 frozen comparison complete')
    a = '''The complete frozen pre-extraction comparison and verified input cleanup are
pending. Whether this cell changes because of extraction is not established
by the candidate result. Its historical cause remains unresolved. No setting
is reselected, and the near-range cut remains off.'''
    b = '''The frozen pre-extraction replay also completes with native exit 0. RPE,
ATE, frames and official translation/rotation RTE have zero recorded differences
from the extracted implementation, with byte-identical saved poses. This
historical difference predates extraction; its original cause remains unresolved.
The frozen historical audit also exits 1. No setting is reselected, and the
near-range cut remains off.'''
    assert a in text
    text = text.replace(a, b)
    a = '''keep this candidate-only evidence separate from completed sequence checkpoints.'''
    b = f'''now include the full [frozen comparison](../../experiments/results/shared_near_range/sequences/01/frozen_comparison.json)
and [verified input cleanup](../../experiments/results/shared_near_range/sequences/01/cleanup.json),
committed at `{closed_commit}`. The cleaned
[04/00/01 checkpoint](../../experiments/results/shared_near_range/checkpoints/01/summary.json)
remains partial whole-zoo coverage.'''
    assert a in text
    text = text.replace(a, b)
    assert diag in text
    p.write_text(text)

p = repo / 'docs/roadmap.md'
s = p.read_text()
a = '''     The committed, cleaned 04/00 checkpoint now covers 34/177 cells:
     27 historical matches and 7 differences, all reproduced by frozen controls.
     Both historical audits retain exit 1; checkpoint verification exits 0.
     [Checkpoint, CSV and commands](../experiments/results/shared_near_range/checkpoints/00/summary.json).'''
b = f'''     The committed, cleaned 04/00/01 checkpoint now covers {x['completed_cell_count']}/{x['planned_cell_count']} cells:
     {x['historical_matching_cell_count']} historical matches and {x['historical_drift_cell_count']} differences, all reproduced by frozen controls.
     All three historical audits retain exit 1; checkpoint verification exits 0.
     [Checkpoint, CSV and commands](../experiments/results/shared_near_range/checkpoints/01/summary.json).'''
assert a in s
s = s.replace(a, b)
a = '''     are in the method READMEs. Full frozen comparison and input cleanup are
     pending, so the cleaned checkpoint count remains 34/177.'''
b = '''     are in the method READMEs. Both frozen pre-extraction controls have zero
     metric differences and byte-identical poses; the differences predate
     extraction. The frozen historical audit retains exit 1. All input hashes
     were verified before deleting the owned 01 scans and alias.'''
assert a in s
p.write_text(s.replace(a, b))
print('Documented committed, cleaned checkpoint01; persistent diagnostics preserved.')
