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
        '    The [weak-translation prediction-prior comparison](../experiments/results/inten_translation_prior/README.md)\n'
        '    also covers every full KITTI00-10 sequence with a fixed new ratio0.03\n'
        '    versus zero, with the preceding mapped-motion seed and acceleration on\n'
        '    in both arms. This outside-paper extension remains default off. Its\n'
        '    quadratic position anchor adds information only in weak sensor-centered\n'
        '    marginal translation directions; S2M and associations are unchanged.\n'
        f"    Geometric mean candidate/reference official RTE is {f(report['geometric_mean_candidate_over_baseline'])}x:\n"
        f"    {len(report['lower_rte_sequences'])} lower, {len(report['higher_rte_sequences'])} higher and {len(report['equal_rte_sequences'])} unchanged sequences.\n"
        '    Failure-sequence RTE: '+outcomes+'.\n'
        '    Sequences still above twice the cited paper RTE: '+(', '.join(report['over_twice_paper_sequences']) or 'none')+'.\n'
        '    Every adverse metric, newly bad local step and error-growth block is\n'
        '    retained. This comparison alone does not establish correspondence\n'
        '    correctness or a general failure remedy; no per-sequence option is\n'
        '    adopted. The original profile was chosen on07, with00 informing\n'
        '    diagnosis. The fraction was borrowed from the existing DiLO KITTI01-tuned\n'
        '    control, fixed before all new InTEn runs and not retuned. Here it is a\n'
        '    soft marginal-information floor, not DiLO hard remapping. Known failures\n'
        '    remain development data, with no independent held-out claim.\n'
        f'    Numerical evidence `{commit[:8]}` precedes this interpretation.\n')
    path=W/'docs/roadmap.md';s=path.read_text()
    # Place this completed result after the preceding study, before item13.
    marker='13. '
    lines=s.splitlines(keepends=True)
    positions=[i for i,line in enumerate(lines) if line.startswith(marker)]
    if len(positions)!=1 or 'weak-translation prediction-prior comparison' in s:
        raise RuntimeError('Unexpected roadmap insertion point')
    lines.insert(positions[0],paragraph+'\n');path.write_text(''.join(lines))
    path=W/'docs/inten_loam_passive_trace.md';s=path.read_text()
    s=s.replace('For S2S, `input` is the previous accepted relative motion, `output` is the',
        'For S2S, `input` is the initial relative prediction (normally the previous\n'
        'accepted S2S motion, or the previous final mapped motion when the opt-in\n'
        '`s2s_mapped_prior` is enabled); `output` is the')
    marker='The stop enum distinguishes no call'
    note=('The optional, default-off `s2s_translation_prior_ratio` adds a quadratic\n'
        'position prior to S2S. When enabled, total `H`, `g` and\n'
        '`weighted_squared_residual` include that prior; class/pair counts and\n'
        'forced diagnostic class systems remain data-only. Nonzero fixed target-axis\n'
        'information is exported as solver `translation_prior_information` (row-major\n'
        '3-by-3); zero/disabled information is omitted, preserving legacy bytes. Its\n'
        'anchor is S2S `input` translation. At an iteration with translation `t`,\n'
        'subtract `J.T * information * J` and `J.T * information * (t-anchor)`\n'
        'with `J=[I,-skew(t)]` to obtain data-only normal equations. A well-conditioned\n'
        'total system containing a motion prior does not establish geometric\n'
        'observability. [Fixed full-sequence experiment](../experiments/results/inten_translation_prior/README.md).\n\n')
    if s.count(marker)!=1:raise RuntimeError('Unexpected trace-doc insertion point')
    path.write_text(s.replace(marker,note+marker))
    for label,script in [('study_docs','refresh_study_docs.py'),('variant_table','generate_full_variant_table.py'),('paper_ratio','generate_paper_ratio_table.py')]:
        if execute(label,[PY,'evaluation/scripts/'+script],stage):
            archival.archive(stage,E/'docs_execution');raise RuntimeError(label+' failed')
    archival.archive(stage,E/'docs_execution')
    subprocess.run(['git','add','-f',str(E),str(W/'papers/inten_loam/README.md'),str(W/'docs')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Report every fixed translation-prior outcome and regenerate study tables'],cwd=W,check=True)
    stage=R/'final_checks_stage'
    checks=[('python',[PY,'-m','unittest','discover','-s','tests']),
        ('showcase',[PY,'evaluation/scripts/validate_showcase.py']),
        ('leaderboard',[PY,'evaluation/scripts/generate_leaderboard.py','--check']),
        ('paper_ready',[PY,'evaluation/scripts/generate_paper_ready_evidence.py','--check']),
        ('preservation',[PY,str(E/'verify.py'),'--output',str(E/'checks/preservation.json')]),
        ('whitespace',['git','diff','--check'])]
    results={label:execute(label,command,stage) for label,command in checks}
    write(stage/'result.json',dict(exit_code=int(any(results.values())),failed=[k for k,v in results.items() if v]))
    archival.archive(stage,E/'checks/final')
    if any(results.values()):raise RuntimeError('Final checks failed; retain all results before fixing')
    subprocess.run(['git','add','-f',str(E/'checks')],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Verify full translation-prior results and preserve prior published evidence'],cwd=W,check=True)


if __name__=='__main__':main()
