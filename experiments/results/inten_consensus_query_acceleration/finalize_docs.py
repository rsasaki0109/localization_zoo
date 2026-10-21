"""Run only after report/import numerical evidence has been committed."""
from pathlib import Path
import hashlib
import importlib.util
import json
import os
import shutil
import subprocess
import time

E = Path(__file__).resolve().parent
W = E.parents[2]
R = W.parent
PY = '/workspace/localization-env/host-venv/bin/python'
ENV = dict(os.environ, PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],
           OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', PYTHONDONTWRITEBYTECODE='1',
           MPLCONFIGDIR=str(R/'matplotlib'), XDG_CACHE_HOME=str(R/'cache'))


def read(p): return json.loads(p.read_text())
def write(p,d): p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2,allow_nan=False)+'\n')
def git(*a): return subprocess.check_output(['git',*a],cwd=W,text=True).strip()


def execute(label, command, stage):
    stage.mkdir(parents=True,exist_ok=True)
    started=time.time()
    with (stage/(label+'.stdout.log')).open('xb') as out,(stage/(label+'.stderr.log')).open('xb') as err:
        p=subprocess.run(command,cwd=W,env=ENV,stdout=out,stderr=err)
    write(stage/(label+'_command.json'),dict(command=command,cwd=str(W),source_commit=git('rev-parse','HEAD'),
          started_unix=started,finished_unix=time.time(),exit_code=p.returncode,
          environment={**{k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')},'PATH_prefix':str(Path(PY).parent)}))
    print(label,p.returncode,flush=True)
    return p.returncode


def main():
    if subprocess.check_output(['git','show','HEAD:'+str(Path(__file__).resolve().relative_to(W))],cwd=W)!=Path(__file__).read_bytes():
        raise RuntimeError('Finalize source must be committed before execution')
    if (R/'collector.lock').exists() or git('status','--porcelain'):
        raise RuntimeError('Completed, clean numerical evidence required')
    if subprocess.check_output(['git','show','HEAD:'+str((E/'report/summary.json').relative_to(W))],cwd=W)!=(E/'report/summary.json').read_bytes():
        raise RuntimeError('Numerical report not committed')
    spec=importlib.util.spec_from_file_location('archive',W/'experiments/results/inten_natural_traces/run_sequences.py')
    archival=importlib.util.module_from_spec(spec);spec.loader.exec_module(archival)
    stage=R/'docs_stage'
    if execute('write_readmes',[PY,str(E/'write_docs.py')],stage):
        archival.archive(stage,E/'docs_execution');raise RuntimeError('README writer failed')
    report=read(E/'report/summary.json');commit=git('rev-parse','HEAD')
    def f(v):return f'{v:.6f}'
    outcomes='; '.join(f"{seq} {f(d['modes']['baseline']['kitti_rte_trans_pct'])}% to {f(d['modes']['candidate']['kitti_rte_trans_pct'])}%" for seq,d in report['sequences'].items() if seq in ('01','02','10'))
    paragraph=(
        '    The [recursive fixed-rotation consensus comparison](../experiments/results/inten_consensus_query_acceleration/README.md)\n'
        '    evaluates one default-off Boolean on every full KITTI 00-10 sequence.\n'
        '    The unchanged consensus seed/support rule re-solves translation at the\n'
        '    original estimated rotation; realized map/prediction feedback can change.\n'
        f"    Geometric mean candidate/reference RTE: {f(report['geometric_mean_candidate_over_baseline'])}x;\n"
        f"    {len(report['lower_rte_sequences'])} lower, {len(report['higher_rte_sequences'])} higher, {len(report['equal_rte_sequences'])} equal sequences.\n"
        '    Failure-sequence RTE: '+outcomes+'.\n'
        '    Above twice paper: '+(', '.join(report['over_twice_paper_sequences']) or 'none')+'.\n'
        '    Every adverse metric, new bad frame and error-growth block is retained.\n'
        '    Cached local feasibility was conditional; this full experiment includes\n'
        '    changed history. Local fixedR does not guarantee global mapped rotation.\n'
        '    Original 07/00/DiLO 01 provenance and all past KITTI development disclosed.\n'
        '    No GT input, threshold sweep, retuning or per-sequence adoption.\n'
        f'    Numerical `{commit[:8]}` precedes interpretation.\n')
    path=W/'docs/roadmap.md';s=path.read_text()
    # Place this completed result after the preceding study, before item13.
    marker='13. '
    lines=s.splitlines(keepends=True)
    positions=[i for i,line in enumerate(lines) if line.startswith(marker)]
    if len(positions)!=1 or 'recursive fixed-rotation consensus comparison' in s:
        raise RuntimeError('Unexpected roadmap insertion point')
    lines.insert(positions[0],paragraph+'\n');path.write_text(''.join(lines))
    for label,script in [('study_docs','refresh_study_docs.py'),('variant_table','generate_full_variant_table.py'),('paper_ratio','generate_paper_ratio_table.py')]:
        if execute(label,[PY,'evaluation/scripts/'+script],stage):
            archival.archive(stage,E/'docs_execution');raise RuntimeError(label+' failed')
    # Preserve import bytes after proving only its metadata clock may differ.
    index=W/'experiments/results/index.json'
    saved=subprocess.check_output(['git','show','HEAD:experiments/results/index.json'],cwd=W)
    current=index.read_bytes()
    left,right=json.loads(saved),json.loads(current)
    saved_clock=left.pop('generated_at',None);current_clock=right.pop('generated_at',None)
    if left!=right:raise RuntimeError('Documentation generation changed substantive imported index fields')
    index.write_bytes(saved)
    write(stage/'index_generation_disposition.json',dict(exit_code=0,
        substantive_fields_equal=True,committed_generated_at=saved_clock,
        regenerated_generated_at=current_clock,restored_sha256=hashlib.sha256(saved).hexdigest(),
        regenerated_sha256=hashlib.sha256(current).hexdigest(),
        policy='Retain committed import index bytes after proving only generated_at may differ.'))
    import re
    disposition=[]
    for rel in git('diff','--name-only','--','docs').splitlines():
        original=subprocess.check_output(['git','show','HEAD:'+rel],cwd=W)
        current=(W/rel).read_bytes()
        normalized=lambda raw:re.sub(rb'(?m)^(_Generated at |>[ ]Generated: )\d{4}-\d{2}-\d{2}T[^\s]+',rb'\1CLOCK',raw)
        if original!=current and normalized(original)==normalized(current):
            (W/rel).write_bytes(original)
            disposition.append(dict(path=rel,only_generation_clock_differs=True,old_sha256=hashlib.sha256(original).hexdigest(),generated_sha256=hashlib.sha256(current).hexdigest()))
    write(stage/'generated_docs_disposition.json',dict(exit_code=0,clock_only_restored=disposition))
    archival.archive(stage,E/'docs_execution')
    files=[str(p) for p in E.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    subprocess.run(['git','add','-f','--',*files,str(W/'papers/inten_loam/README.md'),str(W/'docs')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Report full recursive-consensus outcomes and regenerate study tables'],cwd=W,check=True)
    stage=R/'final_checks_stage'
    checks=[('python',[PY,'-m','unittest','discover','-s','tests']),
        ('showcase',[PY,'evaluation/scripts/validate_showcase.py']),
        ('leaderboard',[PY,'evaluation/scripts/generate_leaderboard.py','--check']),
        ('paper_ready',[PY,'evaluation/scripts/generate_paper_ready_evidence.py','--check']),
        ('preservation',[PY,str(E/'verify.py'),'--output',str(E/'checks/preservation.json')]),
        ('whitespace',['git','diff','13ab2f8f0871707060f4923ff3362317280f8af4','--check'])]
    results={label:execute(label,command,stage) for label,command in checks}
    write(stage/'result.json',dict(exit_code=int(any(results.values())),failed=[k for k,v in results.items() if v]))
    archival.archive(stage,E/'checks/final')
    if any(results.values()):raise RuntimeError('Final checks failed; retain all results before fixing')
    subprocess.run(['git','add','-f',str(E/'checks')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Verify full recursive-consensus results and preserve prior evidence'],cwd=W,check=True)


if __name__=='__main__':main()
