#!/usr/bin/env python3
"""Plot this study's committed per-step observations without smoothing."""
import csv
import json
from pathlib import Path
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

study = Path(__file__).resolve().parent
out = Path(sys.argv[1])
summary = json.loads((study/'diagnosis/summary.json').read_text())
fig, axes = plt.subplots(3, 3, figsize=(14, 8), sharex='col', constrained_layout=True)
for col, seq in enumerate(summary['sequences']):
    with (study/'diagnosis'/f"{seq['sequence']}_frames.csv").open() as stream:
        rows = list(csv.DictReader(stream))
    def values(key):
        return np.array([float(r[key]) if r[key] else np.nan for r in rows])
    x = values('frame')
    for key, label, color in (('s2s_length_ratio', 'S2S output', '#e1812c'),
                              ('final_length_ratio', 'Final output', '#3274a1')):
        axes[0,col].plot(x, values(key), label=label, color=color, lw=.7)
    axes[0,col].axhline(1, color='black', lw=.5)
    axes[0,col].set_ylim(0, 1.4)
    axes[0,col].set_title(f"KITTI {seq['sequence']}")
    for stage, color in (('s2s','#e1812c'), ('s2m','#3274a1')):
        axes[1,col].semilogy(x, values(stage+'_condition_last'), label=stage.upper(), color=color, lw=.7)
    axes[2,col].plot(x, values('position_error_m'), color='#3274a1', lw=1)
    for row in range(3):
        for interval in seq['bad_length_ranges']:
            axes[row,col].axvspan(interval['first_frame']-.5,interval['last_frame']+.5,
                                 color='#c44e52',alpha=.12)
        axes[row,col].grid(alpha=.2)
    axes[2,col].set_xlabel('Destination frame (zero based)')
axes[0,0].set_ylabel('Local step length / GT')
axes[1,0].set_ylabel('Last H condition number\n(sensor centred, L=1m)')
axes[2,0].set_ylabel('Position error [m]\n(first-pose alignment)')
axes[0,0].legend(fontsize=8)
axes[1,0].legend(fontsize=8)
out.parent.mkdir(parents=True,exist_ok=True)
fig.savefig(out,dpi=150,metadata={'Software':'Matplotlib '+matplotlib.__version__})
plt.close(fig)
