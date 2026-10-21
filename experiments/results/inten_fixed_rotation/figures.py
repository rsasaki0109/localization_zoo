"""All endpoints, all arms, fixed numerical evidence only."""
from pathlib import Path
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
E=Path(__file__).resolve().parent;R=E.parents[2].parent
def main():
    cases=json.loads((E/'analysis/report.json').read_text())['cases']
    folder=R/'figures';folder.mkdir()
    x=np.arange(len(cases));labels=[str(c['frame']) for c in cases]
    fig,axes=plt.subplots(3,1,figsize=(11,10),sharex=True)
    names=['original joint','preceding combined joint','fixed R: original','fixed R: seed','fixed R: mask','fixed R: seed + mask']
    for ax,metric,title in zip(axes,['length_ratio','vector_error','rotation_error_mrad'],['Translation length / GT','Translation-vector error / GT length','Rotation error (mrad)']):
        for mode,name in enumerate(names):
            ax.plot(x,[c['arms'][mode][metric] for c in cases],marker='o',label=name,alpha=.8)
        ax.scatter(x,[c['candidate'][metric] for c in cases],marker='x',s=100,color='black',label='candidate (declared fallback)',zorder=10)
        ax.set_ylabel(title);ax.grid(alpha=.2)
    axes[0].axhline(1,color='gray',ls='--',lw=1)
    axes[0].legend(ncol=2,fontsize=8)
    axes[-1].set_xticks(x,labels);axes[-1].set_xlabel('KITTI01 inherited endpoint (fixed original history)')
    fig.suptitle('Conditional rotation test: every arm and adverse endpoint retained')
    fig.tight_layout();fig.savefig(folder/'all_endpoints.png',dpi=140);plt.close(fig)
if __name__=='__main__':main()
