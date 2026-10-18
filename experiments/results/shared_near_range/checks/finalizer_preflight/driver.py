from pathlib import Path
import hashlib,json,subprocess,sys
E=Path(__file__).resolve().parents[2];sys.path.insert(0,str(E))
from support import *
from finish_docs import MODULES,method_block,historic_nclt
folder=Path(__file__).resolve().parent
paths=[REPO/'papers'/module/'README.md' for module in MODULES.values()]+[E/'README.md',REPO/'docs/roadmap.md']
before={str(p):digest(p) for p in paths}
run('reject_cleaned_partial',[sys.executable,str(E/'finish_docs.py'),'--report',str(E/'checkpoints/04/summary.json'),'--apply'],folder,allowed=(2,))
run('reject_primary_only',[sys.executable,str(E/'finish_docs.py'),'--report',str(E/'sequences/00/audit/report.json'),'--apply'],folder,allowed=(2,))
run('reject_incomplete_controller',[sys.executable,str(E/'finish_when_complete.py')],folder,allowed=(1,))
assert before=={str(p):digest(p) for p in paths}
assert not (E/'completion').exists()
assert not (R/'completion_worker.lock').exists()
checkpoint=json.loads((E/'checkpoints/04/summary.json').read_text())
preview=R/'completion_render_preview';preview.mkdir(exist_ok=True)
rendered=[]
for method in ('CT-ICP','Mesh-LOAM'):
 row=next(r for r in checkpoint['rows'] if r['method']==method)
 block=method_block(method,[row],'PREVIEW_ONLY_REPORT')
 for key in ('stored_rpe_trans_pct','rerun_rpe_trans_pct','stored_ate_m','rerun_ate_m'):
  assert f'{row[key]:.6f}' in block
 assert 'audit' in block and '**1**' in block and 'causes remain unresolved' in block
 target=preview/(method.replace('-','_')+'.md');target.write_text('VALIDATION PREVIEW: committed04 only, not whole-zoo completion.\n'+block)
 rendered.append({'method':method,'file':str(target),'sha256':digest(target),'source_checkpoint':'checkpoints/04/summary.json'})
dilo=historic_nclt('DiLO');assert 'not a controlled cutoff-only pair' in dilo and '54.344593' in dilo and '75.832748' in dilo and 'ATE worsens' in dilo
write(folder/'summary.json',dict(command=[sys.executable,*sys.argv],source_script_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip(),public_document_sha256_before_and_after=before,partial_reports_and_live_controller_rejected=True,public_documents_unchanged=True,completion_output_absent=True,preview_only_actual04_outcomes=rendered,historical_nclt_confound_and_negative_retained=True,exit_code=0,scope='Completion-worker guard and renderer validation from real committed partial evidence; not full KITTI or NCLT validation.'))
print('Partial-data guards rejected finalization; no public docs changed; real04 negative pairs and historical NCLT limits retained.')
