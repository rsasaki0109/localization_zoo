from pathlib import Path
import json
R=Path(__file__).resolve().parent
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
rows=[]
for i in range(11):
    seq=f'{i:02d}';old=(W/'experiments/reference_data'/f'kitti_seq_{seq}_full_gt.csv').read_bytes();new=(R/f'regenerated_{seq}.csv').read_bytes()
    rows.append(dict(sequence=seq,byte_identical=old==new,newline_normalized_identical=old.replace(b'\r\n',b'\n')==new.replace(b'\r\n',b'\n'),committed_crlf_lines=old.count(b'\r\n'),regenerated_crlf_lines=new.count(b'\r\n'),committed_lf_count=old.count(b'\n'),regenerated_lf_count=new.count(b'\n')))
report=dict(exit_code=0,sequences=rows,all_text_identical_after_only_crlf_to_lf=all(x['newline_normalized_identical'] for x in rows))
(R/'formatting_report.json').write_text(json.dumps(report,indent=2)+'\n')
assert report['all_text_identical_after_only_crlf_to_lf']
print('Every sequence is text-identical after only CRLF-to-LF normalization.')
