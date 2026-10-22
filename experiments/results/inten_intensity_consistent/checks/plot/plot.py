from pathlib import Path
import json,hashlib
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree');R=Path(__file__).resolve().parent
source=W/'experiments/results/inten_intensity_consistent/report/summary.json'
s=json.loads(source.read_text());cases=[(g,c) for g,r in s['groups'].items() for c in r['cases']]
fig,axes=plt.subplots(3,1,figsize=(15,9),sharex=True,layout='constrained')
x=np.arange(len(cases));colors=['#444444','#c26926','#1269a6']
for ax,(key,label,scale) in zip(axes,[('length_ratio','Estimated length / GT step',1),('vector_error','Vector error / GT step',1),('rotation_error_rad','Rotation error [mrad]',1000)]):
 for arm,name,color in zip((0,2,4),('Legacy joint','Previous normalized (rejected)','Fixed-node (motion gate failed)'),colors):
  ax.plot(x,[c['metrics'][str(arm)][key]*scale for _,c in cases],'.-',label=name,color=color,linewidth=1,markersize=4)
 ax.set_ylabel(label);ax.grid(alpha=.2)
 if key=='length_ratio':ax.axhspan(.8,1.2,color='gray',alpha=.10);ax.axhline(1,color='gray',lw=.7)
 if key=='vector_error':ax.axhline(.2,color='gray',lw=.7,ls='--')
axes[0].legend(loc='upper right',fontsize=8)
axes[-1].set_xticks(x,[g+'/'+str(c['frame']) for g,c in cases],rotation=65,ha='right',fontsize=8)
axes[-1].set_xlabel('Every inherited conditional history / endpoint (same estimated initial seed)')
fig.suptitle('Fixed-node intensity: covered-field jumps removed; total motion errors still worsen\n31 conditional endpoints, all retained — not full-sequence KITTI scores',fontsize=12)
for ext in ('png','svg'):fig.savefig(R/('comparison.'+ext),dpi=170)
(R/'plot_manifest.json').write_text(json.dumps(dict(numerical_commit='6eec3472',source=str(source),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),artifacts={str(R/('comparison.'+ext)):hashlib.sha256((R/('comparison.'+ext)).read_bytes()).hexdigest() for ext in ('png','svg')}),indent=2)+'\n')
