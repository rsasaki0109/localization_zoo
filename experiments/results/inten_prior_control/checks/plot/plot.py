from pathlib import Path
import json,hashlib,shutil,sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree')
E=W/'experiments/results/inten_prior_control'
R=Path(__file__).resolve().parent
source=E/'report/summary.json';report=json.loads(source.read_text())
cases=[(g,c) for g,v in report['groups'].items() for c in v['cases']]
x=np.arange(len(cases));labels=[f"{g}/{c['frame']}" for g,c in cases]
fig,axes=plt.subplots(3,1,figsize=(16,9),sharex=True)
panels=[('length_error',1.,'Conditional length error change'),('vector_error',1.,'Conditional vector error change'),('rotation_error_rad',1000.,'Rotation error change [mrad]')]
for ax,(key,scale,label) in zip(axes,panels):
    delta=np.array([scale*(c['metrics']['14'][key]-c['metrics']['12'][key]) for _,c in cases])
    ax.axhline(0,color='#334155',linewidth=1,linestyle='--')
    ax.axvspan(13.5,21.5,color='#e2e8f0',alpha=.75)
    ax.plot(x,delta,color='#0369a1',marker='o',markersize=5,linewidth=1.3)
    ax.set_ylabel(label,fontsize=10);ax.grid(axis='y',alpha=.2)
    ax.set_xlim(-.6,len(cases)-.4)
    for boundary in (7.5,13.5,21.5):ax.axvline(boundary,color='#94a3b8',linewidth=.7)
axes[0].set_title('Common information minus own information: only the 02 history changes',loc='left',fontsize=12)
axes[-1].set_xticks(x,labels,rotation=65,ha='right',fontsize=8)
axes[-1].set_xlabel('Every inherited development history/frame; negative changes mean lower error')
fig.suptitle('Fixed-reference prior control — progression gate FAILED',fontsize=17,y=.97)
base=report['motion_totals']['0']['length_error'];candidate=report['motion_totals']['14']['length_error']
fig.text(.5,.915,f'Total conditional length error: original {base:.6f} → common-prior candidate {candidate:.6f}; 247 / 08 remain adverse.',ha='center',fontsize=11)
fig.text(.5,.015,'23 complete joint/consensus trace identities; all 434 inherited controls exact. Conditional study, not recursive trajectory accuracy or SOTA.',ha='center',fontsize=10)
fig.subplots_adjust(top=.865,bottom=.17,left=.09,right=.98,hspace=.25)
for suffix in ('png','svg'):fig.savefig(R/('comparison.'+suffix),dpi=160)
plt.close(fig)
out=Path('/tmp/localization-zoo-inten-prior-control-20261023/plot_stage');out.mkdir(exist_ok=True)
shutil.copyfile(Path(__file__),out/'plot.py')
manifest=dict(numerical_source_commit='a358a572',attribution_source_commit='2eca2585',
    input_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
    outputs={p.name:dict(path=str(p),sha256=hashlib.sha256(p.read_bytes()).hexdigest()) for p in (R/'comparison.png',R/'comparison.svg')})
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
