#!/usr/bin/env python3
"""Plot committed local-step evidence, including adverse controls."""
import csv
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

root=Path(__file__).resolve().parent
with (root/'report/steps.csv').open() as f:
    steps=list(csv.DictReader(f))
with (root/'report/paired.csv').open() as f:
    pairs=list(csv.DictReader(f))
fig,axes=plt.subplots(2,1,figsize=(10,7),layout='constrained')
for method,label,color in [('inten_fixed','Frozen InTEn full run','#b53b32'),
                           ('kiss_same_input','Independent KISS full run','#147d64')]:
    rows=[r for r in steps if r['method']==method and 190<=int(r['frame'])<=260]
    axes[0].plot([int(r['frame']) for r in rows],
                 [float(r['estimate_length_m'])/float(r['gt_length_m']) for r in rows],
                 label=label,color=color)
for mode,label,color in [('native','Native seed replay','#b53b32'),
                          ('kiss_current','Independent current seed','#147d64'),
                          ('kiss_previous','Independent previous seed','#4265b0')]:
    rows=[r for r in pairs if r['mode']==mode and 190<=int(r['frame'])<=260]
    axes[1].plot([int(r['frame']) for r in rows],[float(r['output_length_ratio']) for r in rows],
                 label=label,color=color,alpha=.85)
for ax in axes:
    ax.axhline(1,color='black',lw=.7)
    ax.axhline(.8,color='gray',ls=':',lw=.8,label='20% short-step threshold')
    ax.set_ylabel('Estimated / GT step length')
    ax.set_xlabel('KITTI 01 destination frame (zero-based)')
    ax.legend(fontsize=8,loc='lower right')
    ax.grid(alpha=.15)
axes[0].set_title('Full causal trajectories: same raw scans and input policy')
axes[1].set_title('Isolated InTEn scan-to-scan output: no full-trajectory claim')
fig.suptitle('Onset diagnosis; additional controls 420 and 433 are retained in paired.csv',fontsize=11)
fig.savefig(root/'report/onset.png',dpi=140,metadata={'Software':'Matplotlib'})
plt.close(fig)
