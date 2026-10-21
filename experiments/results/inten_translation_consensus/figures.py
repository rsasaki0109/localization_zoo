"""Every inherited endpoint, every factorial arm and every consensus vote."""
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from common import E,R,read,write


def main():
    report=read(E/'analysis/report.json');target=R/'figures';target.mkdir()
    names=['Original','Seed only','Mask only','Seed + mask','Prior rejected mask']
    colors=['#334155','#0d9488','#2563eb','#ea580c','#a855f7']
    fig,axes=plt.subplots(3,1,figsize=(10,9),sharex=True)
    for mode in range(5):
        for ax,key,label in zip(axes,('length_ratio','vector_error','rotation_error_mrad'),
                               ('Translation length / GT','Translation vector error / GT','Rotation error (mrad)')):
            ax.plot(range(8),[c['arms'][mode][key] for c in report['cases']],
                    marker='o',linestyle='--' if mode==4 else '-',color=colors[mode],label=names[mode],alpha=.8)
            ax.set_ylabel(label);ax.grid(alpha=.2)
    axes[0].axhline(1,color='black',linestyle=':',linewidth=1)
    axes[0].legend(ncol=3,fontsize=8)
    axes[-1].set_xticks(range(8),[str(c['frame']) for c in report['cases']])
    axes[-1].set_xlabel('KITTI01 inherited endpoint (categorical spacing; fixed original history)')
    fig.suptitle('Seed correction rescues the collapsed205 pair; rotation still fails the fixed gate')
    fig.tight_layout();fig.savefig(target/'factorial_errors.png',dpi=150);plt.close(fig)
    fig,axes=plt.subplots(2,4,figsize=(14,7),sharex=True)
    for ax,case in zip(axes.flat,report['cases']):
        d=case['decision'];ax.plot([(k-60)*.05 for k in range(121)],d['votes'],color='#2563eb')
        ax.axhline(d['valid_clusters']/2,color='gray',linestyle=':',label='Half the valid clusters')
        ax.axvline(d['shift_m'],color='#ea580c',linestyle='--',label='Applied correction')
        ax.set_title(f"Frame {case['frame']}: "+('used' if d['usable'] else 'fallback'))
        ax.grid(alpha=.2)
    axes[0,0].legend(fontsize=7)
    fig.supxlabel('Additional translation along original prediction (m)')
    fig.supylabel('Clusters admitting each shift (equal votes)')
    fig.suptitle('Every vote curve; no majority or a boundary optimum keeps the original seed and support')
    fig.tight_layout();fig.savefig(target/'consensus_votes.png',dpi=150);plt.close(fig)
    write(target/'metadata.json',dict(numerical_commit=read(E/'postprocessing_protocol.json')['numerical_commit'],
          matplotlib_version=matplotlib.__version__,scope='Every inherited endpoint and all five arms; no new measurement'))


if __name__=='__main__':main()
