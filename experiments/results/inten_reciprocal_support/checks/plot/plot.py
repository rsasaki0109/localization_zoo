from pathlib import Path
import json,hashlib
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
W=Path('/workspace/localization-env/branch-cleanup-20261022/worktree');R=Path('/workspace/localization-env/inten-reciprocal-support-20261022')
new_path=W/'experiments/results/inten_reciprocal_support/report/summary.json'
dense_path=W/'experiments/results/inten_dense_continuation/report/summary.json'
r=json.loads(new_path.read_text());d=json.loads(dense_path.read_text());entries=[]
for g,group in r['groups'].items():
    old={c['frame']:c for c in d['groups'][g]['cases']}
    for c in group['cases']:
        assert all(c['metrics']['0'][k]==old[c['frame']]['baseline_metrics']['0'][k] for k in ('length_ratio','vector_error'))
        entries.append((g,c,old[c['frame']]))
labels=[g+'/'+str(c['frame']) for g,c,_ in entries];x=np.arange(len(entries))
names=['Legacy10','Uniform100','Dense selection','Reciprocal100']
colors=['#555555','#e1812c','#276fbf','#159567']
values=[[c['metrics']['0'],c['metrics']['6'],prior['selected_metrics']['0'],c['metrics']['12']] for _,c,prior in entries]
fig=plt.figure(figsize=(18,12));grid=fig.add_gridspec(3,3,height_ratios=[1.35,1.2,.85],hspace=.5,wspace=.25)
for row,key,title in ((0,'length_ratio','Estimated step / GT step'),(1,'vector_error','Vector error / GT step')):
    ax=fig.add_subplot(grid[row,:])
    if row==0:ax.axhspan(.8,1.2,color='#aaaaaa',alpha=.15);ax.axhline(1,color='gray',lw=.8)
    else:ax.axhline(.2,color='gray',ls='--',lw=.8)
    for i,(name,color) in enumerate(zip(names,colors)):
        ax.plot(x,[v[i][key] for v in values],'.-',label=name,color=color,lw=1,ms=4)
    ax.set_ylabel(title);ax.grid(alpha=.15);ax.set_xticks(x,labels,rotation=60,ha='right',fontsize=8)
    if row==0:
        ax.legend(loc='upper right',fontsize=9)
        for j,(g,c,_) in enumerate(entries):
            if (g=='old' and c['frame']==247) or (g=='08' and c['frame'] in (39,41,42,43,44,45,46)):
                ax.plot(j,c['metrics']['12'][key],'x',color='crimson',ms=8,mew=2)
        j=next(i for i,(g,c,_) in enumerate(entries) if g=='old' and c['frame']==204)
        ax.annotate('202/204 recover; other cases worsen',xy=(j,values[j][3][key]),xytext=(j+1,1.13),fontsize=10,color=colors[3],arrowprops=dict(arrowstyle='->',color=colors[3]))
        ax.set_ylim(.1,1.25)
totals=[r['motion_totals']['0'],r['motion_totals']['6'],d['totals']['selected_metrics']['0'],r['motion_totals']['12']]
for column,(key,title,scale) in enumerate((('length_error','Total absolute length-ratio error',1),('vector_error','Total vector error / GT step',1),('rotation_error_rad','Total rotation error [mrad]',1000))):
    ax=fig.add_subplot(grid[2,column]);height=[v[key]*scale for v in totals]
    ax.bar(range(4),height,color=colors);ax.set_xticks(range(4),names,rotation=25,ha='right',fontsize=9);ax.set_title(title,fontsize=11);ax.grid(axis='y',alpha=.15);ax.set_ylim(0,max(height)*1.18)
    for i,v in enumerate(height):ax.text(i,v+max(height)*.02,f'{v:.3f}',ha='center',fontsize=10)
fig.suptitle('Conditional recoveries trade off across cases; frozen progression guards still fail\nAll 31 inherited endpoints retained; no full-sequence KITTI accuracy or SOTA claim',fontsize=14,y=.985)
fig.savefig(R/'comparison.png',dpi=150,bbox_inches='tight');fig.savefig(R/'comparison.svg',bbox_inches='tight')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
manifest=dict(numerical_commits=['f22127ab','7b398407'],sources={str(p):sha(p) for p in (new_path,dense_path)},artifacts={str(R/name):sha(R/name) for name in ('comparison.png','comparison.svg')})
(R/'plot_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
