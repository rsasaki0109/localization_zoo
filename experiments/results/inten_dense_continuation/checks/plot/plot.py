from pathlib import Path
import json,hashlib
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree');R=Path(__file__).resolve().parent
budget=W/'experiments/results/inten_iteration_budget/report/summary.json';dense=W/'experiments/results/inten_dense_continuation/report/summary.json'
b=json.loads(budget.read_text());d=json.loads(dense.read_text());cases=[(g,c) for g,r in d['groups'].items() for c in r['cases']]
x=np.arange(len(cases));parent={(g,c['frame']):c for g,r in b['groups'].items() for c in r['cases']}
fig=plt.figure(figsize=(15,10),layout='constrained');grid=fig.add_gridspec(3,3,height_ratios=[2,2,1.35])
colors=['#444444','#c96b26','#1767a5'];names=['Legacy10','Uniform100 (gate failed)','Dense selection (gate failed)']
for row,(key,label) in enumerate([('length_ratio','Estimated step / GT step'),('vector_error','Vector error / GT step')]):
 ax=fig.add_subplot(grid[row,:])
 arrays=[[c['baseline_metrics']['0'][key] for _,c in cases],[parent[g,c['frame']]['metrics']['6'][key] for g,c in cases],[c['selected_metrics']['0'][key] for _,c in cases]]
 for a,color,name in zip(arrays,colors,names):ax.plot(x,a,'.-',lw=1,ms=4,color=color,label=name)
 ax.set_ylabel(label);ax.grid(alpha=.2)
 ax.set_xticks(x,[g+'/'+str(c['frame']) for g,c in cases],rotation=60,ha='right',fontsize=8)
 if row==0:
  ax.axhspan(.8,1.2,color='gray',alpha=.10);ax.axhline(1,color='gray',lw=.7);ax.legend(fontsize=8,loc='upper right')
  idx=next(i for i,(g,c) in enumerate(cases) if g=='08' and c['frame']==39)
  ax.plot(idx,arrays[2][idx],marker='x',color='crimson',ms=10,mew=2);ax.annotate('08/39: new bad-length label',xy=(idx,arrays[2][idx]),xytext=(idx-4,.48),fontsize=9,color='crimson',arrowprops={'arrowstyle':'->','color':'crimson'})
 else:ax.axhline(.2,color='gray',lw=.7,ls='--')
for col,(key,label,scale) in enumerate([('length_error','Total absolute length-ratio error',1),('vector_error','Total vector error / GT step',1),('rotation_error_rad','Total rotation error [mrad]',1000)]):
 ax=fig.add_subplot(grid[2,col]);values=[b['motion_totals']['0'][key]*scale,b['motion_totals']['6'][key]*scale,d['totals']['selected_metrics']['0'][key]*scale]
 bars=ax.bar(range(3),values,color=colors);ax.bar_label(bars,fmt='%.3f',padding=3,fontsize=9)
 ax.set_xticks(range(3),['Legacy10','Uniform100','Dense selection'],fontsize=9);ax.set_title(label,fontsize=10);ax.set_ylim(0,max(values)*1.18);ax.grid(axis='y',alpha=.2)
fig.suptitle('Conditional continuation: aggregate errors improve, frozen new-label guard still fails\nAll 31 inherited endpoints retained; no full-sequence KITTI accuracy or SOTA claim',fontsize=12)
for ext in ('png','svg'):fig.savefig(R/('comparison.'+ext),dpi=170)
(R/'plot_manifest.json').write_text(json.dumps(dict(numerical_commits=['c6832c2a','7b398407'],sources={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (budget,dense)},artifacts={str(R/('comparison.'+ext)):hashlib.sha256((R/('comparison.'+ext)).read_bytes()).hexdigest() for ext in ('png','svg')}),indent=2)+'\n')
