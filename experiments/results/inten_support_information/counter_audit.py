"""Split the frozen invalid/non-kept counter by whether backward testing occurred."""
from pathlib import Path
import hashlib,importlib.util,json,lzma
E=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_support_information')
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
spec=importlib.util.spec_from_file_location('counter_report_reader',E/'report_io.py');io=importlib.util.module_from_spec(spec);spec.loader.exec_module(io)
def blank():return dict(source_rows=0,valid_kept=0,valid_removed=0,invalid_kept=0,invalid_tested_reject=0,invalid_without_backward_test=0)
def main(output):
 report=io.read_report();groups={};total=blank()
 for group in ('old','01','02','08'):
  folder=E/'groups'/group;manifest=json.loads((folder/'archive.json').read_text());entry=manifest['files']['support.extra.jsonl'];path=folder/entry['path']
  assert sha(path)==entry['stored_sha256'];digest=hashlib.sha256();size=0;classes=[blank() for _ in range(4)];observations=0
  with lzma.open(path,'rb') as stream:
   for line in stream:
    digest.update(line);size+=len(line);wrapped=json.loads(line);raw=wrapped['raw_line']
    if '"type":"support_query"' not in raw[:80]:continue
    record=json.loads(raw);cls=record['class'];stats=classes[cls];observations+=1
    for valid,kept,indices in record['rows']:
     stats['source_rows']+=1
     if valid:
      if not kept:assert cls<3 and indices
      stats['valid_kept' if kept else 'valid_removed']+=1
     elif kept:stats['invalid_kept']+=1
     else:
      assert cls<3
      stats['invalid_tested_reject' if indices else 'invalid_without_backward_test']+=1
  assert size==entry['bytes'] and digest.hexdigest()==entry['sha256']
  sums={k:sum(c[k] for c in classes) for k in total};old=report['groups'][group]
  assert observations==old['passive_class_queries']
  assert sums['source_rows']==old['counts']['queries']
  assert sums['valid_removed']==old['counts']['valid_removed']
  assert sums['invalid_kept']+sums['invalid_tested_reject']+sums['invalid_without_backward_test']==old['counts']['invalid']
  assert sums['invalid_tested_reject']+sums['invalid_without_backward_test']==old['counts']['invalid_reciprocal_rejections']
  groups[group]=dict(counts=sums,classes=classes,archive_sha256=sha(folder/'archive.json'))
  for k in total:total[k]+=sums[k]
 result=dict(exit_code=0,groups=groups,counts=total,producer_sha256=sha(Path(__file__)),
  observer_sha256=sha(E/'support_information.h'),verified_report_manifest_sha256=sha(E/'report/manifest.json'),
  semantics='The frozen invalid_reciprocal_rejections label is invalid AND not-kept; it includes empty-neighbor rows where no backward predicate ran. Split these from tested rejection without changing any native observation or frozen verifier result.')
 output.write_text(json.dumps(result,indent=2)+'\n');print('Every invalid/non-kept row classified without changing native data or frozen numerical verification.')
if __name__=='__main__':
 import argparse
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);args=p.parse_args();main(args.output)
