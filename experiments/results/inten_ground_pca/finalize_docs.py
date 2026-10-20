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
    a,b=report['sequences']['02']['modes']['baseline'],report['sequences']['02']['modes']['candidate']
    def f(v):return f'{v:.6f}'
    outcomes='; '.join(f"{seq} {f(d['modes']['baseline']['kitti_rte_trans_pct'])}% to {f(d['modes']['candidate']['kitti_rte_trans_pct'])}%" for seq,d in report['sequences'].items() if seq in ('01','02','10'))
    paragraph=(
        '    The [full iterative ground-PCA comparison](../experiments/results/inten_ground_pca/README.md)\n'
        '    uses one fixed setting on every full KITTI00-10 sequence. Only the\n'
        '    normals of already accepted S2S ground triangles change, using the\n'
        '    same gated ten neighbours and original closest-point plane anchor.\n'
        '    PCA degeneracy retains the triangle. Mapped-motion prediction, soft\n'
        '    translation prior0.03 and acceleration are on in both arms; row\n'
        '    completion remains off. This outside-paper extension is default off.\n'
        f"    Geometric mean candidate/reference RTE is {f(report['geometric_mean_candidate_over_baseline'])}x:\n"
        f"    {len(report['lower_rte_sequences'])} lower, {len(report['higher_rte_sequences'])} higher and {len(report['equal_rte_sequences'])} unchanged sequences.\n"
        '    Failure-sequence RTE: '+outcomes+'.\n'
        '    Sequences above twice the cited paper RTE: '+(', '.join(report['over_twice_paper_sequences']) or 'none')+'.\n'
        '    All adverse metrics, new local bad frames and error-growth blocks\n'
        '    are retained. No per-sequence winner is adopted or default promoted.\n'
        '    Known-failure first-system controls motivated this intervention;\n'
        '    all prior outcomes are development evidence, with no independent\n'
        '    held-out claim. Profile chosen on07,00 informed diagnosis; the\n'
        '    inherited prior fraction came from DiLO KITTI01 tuning. This tests\n'
        '    the actual iterative trajectory, not physical-ring correctness.\n'
        f'    Numerical evidence `{commit[:8]}` precedes this interpretation.\n')
    path=W/'docs/roadmap.md';s=path.read_text()
    # Place this completed result after the preceding study, before item13.
    marker='13. '
    lines=s.splitlines(keepends=True)
    positions=[i for i,line in enumerate(lines) if line.startswith(marker)]
    if len(positions)!=1 or 'full iterative ground-PCA comparison' in s:
        raise RuntimeError('Unexpected roadmap insertion point')
    lines.insert(positions[0],paragraph+'\n');path.write_text(''.join(lines))
    path=W/'docs/inten_loam_passive_trace.md';s=path.read_text()
    marker='The stop enum distinguishes no call'
    note=('The optional default-off `s2s_ground_pca` flag is recorded only when\n'
          'enabled. It replaces accepted S2S ground triangle normals with PCA\n'
          'normals of the same gated neighbours through the original nearest\n'
          'point; degenerate PCA retains the triangle. At a fixed pose it changes\n'
          'ground equations without changing accepted counts. Later poses and\n'
          'support may diverge. This repository extension is outside the paper.\n'
          '[Full iterative comparison](../experiments/results/inten_ground_pca/README.md).\n\n')
    if s.count(marker)!=1 or 'optional default-off `s2s_ground_pca`' in s:raise RuntimeError('Unexpected trace-doc insertion point')
    path.write_text(s.replace(marker,note+marker))
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
    archival.archive(stage,E/'docs_execution')
    subprocess.run(['git','add','-f',str(E),str(W/'papers/inten_loam/README.md'),str(W/'docs')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Report every fixed ground-PCA outcome and regenerate study tables'],cwd=W,check=True)
    stage=R/'final_checks_stage'
    checks=[('python',[PY,'-m','unittest','discover','-s','tests']),
        ('showcase',[PY,'evaluation/scripts/validate_showcase.py']),
        ('leaderboard',[PY,'evaluation/scripts/generate_leaderboard.py','--check']),
        ('paper_ready',[PY,'evaluation/scripts/generate_paper_ready_evidence.py','--check']),
        ('preservation',[PY,str(E/'verify.py'),'--output',str(E/'checks/preservation.json')]),
        ('whitespace',['git','diff','e4c1adf3762f7c844846b93464044d2c80d47141','--check'])]
    results={label:execute(label,command,stage) for label,command in checks}
    write(stage/'result.json',dict(exit_code=int(any(results.values())),failed=[k for k,v in results.items() if v]))
    archival.archive(stage,E/'checks/final')
    if any(results.values()):raise RuntimeError('Final checks failed; retain all results before fixing')
    subprocess.run(['git','add','-f',str(E/'checks')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Verify full ground-PCA results and preserve prior published evidence'],cwd=W,check=True)


if __name__=='__main__':main()
