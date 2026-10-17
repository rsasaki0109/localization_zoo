"""Plot a committed full-sequence trace without refitting or rescaling poses."""
from pathlib import Path
import argparse,csv,hashlib,io,json,subprocess,sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

E=Path(__file__).resolve().parent
REPO=E.parents[2]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--sequence',required=True,choices=[f'{i:02d}' for i in range(11)])
    p.add_argument('--output-dir',type=Path,required=True)
    args=p.parse_args()
    revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
    sources={}
    def read(path):
        relative=path.relative_to(REPO).as_posix()
        data=subprocess.check_output(['git','show',f'{revision}:{relative}'],cwd=REPO)
        sources[relative]=hashlib.sha256(data).hexdigest()
        return data
    comparison=json.loads(read(E/'sequences'/args.sequence/'comparison.json'))
    def output(original):
        suffix=original.split('/experiments/results/shared_degeneracy_remap/',1)[1]
        assert '..' not in Path(suffix).parts and not Path(suffix).is_absolute()
        summary=E/suffix
        return summary.parent/(summary.stem+'_native')/'dogfooding_results'
    control_dir=output(comparison['control_summary']);candidate_dir=output(comparison['candidate_summary'])
    gt_name=f'kitti_seq_{args.sequence}_full_evaluated_gt.txt'
    gt_data=read(control_dir/gt_name)
    assert gt_data==read(candidate_dir/gt_name)
    def positions(data):
        values=np.loadtxt(io.BytesIO(data))
        assert values.shape==(comparison['candidate']['frames'],12) and np.isfinite(values).all()
        return values.reshape(-1,3,4)[:,:,3]
    gt=positions(gt_data);control=positions(read(control_dir/'SuMa.txt'));candidate=positions(read(candidate_dir/'SuMa.txt'))
    steps=[np.linalg.norm(np.diff(x,axis=0),axis=1) for x in (gt,control,candidate)]
    errors=[np.linalg.norm(x-gt,axis=1) for x in (control,candidate)]
    frames=np.arange(len(gt));colors=('#444444','#4c78a8','#e45756')
    fig,axes=plt.subplots(2,1,figsize=(10,6),sharex=True,layout='constrained')
    for values,label,color in zip(steps,('GT','Raw defaults','Shared remap 0.03'),colors):
        axes[0].plot(frames[1:],values,label=label,color=color,linewidth=1)
    axes[0].set_ylabel('Successive displacement [m]');axes[0].legend(loc='upper right',frameon=False)
    for values,label,color in zip(errors,('Raw defaults','Shared remap 0.03'),colors[1:]):
        axes[1].plot(frames,values,label=label,color=color,linewidth=1.5)
    axes[1].set_ylabel('Pointwise position error [m]');axes[1].set_xlabel('Pose frame (zero based)')
    for ax in axes:
        ax.grid(alpha=0.2);ax.spines[['top','right']].set_visible(False)
    a=comparison['control']['kitti_rte_trans_pct'];b=comparison['candidate']['kitti_rte_trans_pct']
    fig.suptitle(f'SuMa KITTI {args.sequence}: fixed transfer of local translation remap\nOfficial RTE: raw {a:.3f}% / remap {b:.3f}%',fontsize=12)
    args.output_dir.mkdir(parents=True,exist_ok=True)
    fig.savefig(args.output_dir/'trace.png',dpi=180);fig.savefig(args.output_dir/'trace.svg');plt.close(fig)
    with (args.output_dir/'trace.csv').open('w',newline='') as stream:
        writer=csv.writer(stream);writer.writerow(['frame','gt_step_m','raw_step_m','remap_step_m','raw_position_error_m','remap_position_error_m'])
        for i in frames:
            writer.writerow([int(i),*[float(x[i-1]) if i else '' for x in steps],*[float(x[i]) for x in errors]])
    provenance={'command':[sys.executable,*sys.argv],'cwd':str(Path.cwd()),'input_commit':revision,
      'implementation_commit':comparison['source_commit'],'input_sha256':sources,
      'script_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
      'definitions':{'step':'Euclidean difference of successive saved position vectors; destination frame index.',
                     'position_error':'Euclidean difference from evaluated GT in the saved coordinate frame; no fit or rescaling.',
                     'rte':'Official KITTI RTE copied from the committed full-sequence comparison.'}}
    (args.output_dir/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')

if __name__=='__main__':main()
