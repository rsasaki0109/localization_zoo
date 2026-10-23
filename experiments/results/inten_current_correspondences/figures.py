"""Plot committed geometric controls; never read raw scans or change estimator data."""
import os
import tempfile
import json
import numpy as np
from run import E, W, R, module, read, write, sha

os.environ.setdefault('MPLCONFIGDIR', str(R/'matplotlib'))
os.environ.setdefault('XDG_CACHE_HOME', str(R/'cache'))
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt


def main():
    folder = R/'figures'; folder.mkdir()
    archive = module('figures_restore', W/'experiments/results/inten_natural_traces/diagnose.py')
    with tempfile.TemporaryDirectory(dir=R) as temp:
        temp = __import__('pathlib').Path(temp)
        archive.restore(E/'baseline', temp)
        features = {r['frame']: r['features'] for r in map(json.loads, (temp/'features.jsonl').read_text().splitlines())}
        case = next(c for c in read(E/'baseline/report.json')['cases'] if c['frame'] == 204)
        gt = np.array(case['gt_relative']).reshape(4, 4)
        p = np.array(features[203][4])[:, :3]
        q = np.array(features[204][4])[:, :3]@gt[:3, :3].T+gt[:3, 3]
        fig, axes = plt.subplots(2, 1, figsize=(12, 6))
        for ax, (a, b) in zip(axes, [(0, 1), (0, 2)]):
            for points, label, color in [(p, 'Previous full points', 'black'), (q, 'Current full points transformed by GT', 'tab:orange')]:
                mask = ((points[:, 0] >= -12) & (points[:, 0] <= 15) & (points[:, 1] >= -5) & (points[:, 1] <= -2) & (points[:, 2] >= -1.9) & (points[:, 2] <= .5))
                ax.scatter(points[mask, a], points[mask, b], s=2, c=color, alpha=.6, label=label)
            ax.set_xlabel('xyz'[a]+' (m)'); ax.set_ylabel('xyz'[b]+' (m)'); ax.grid(alpha=.3); ax.legend()
        fig.suptitle('KITTI01 frame204: right-side surface displacement after GT alignment')
        fig.tight_layout(); fig.savefig(folder/'scene204.png', dpi=130); plt.close(fig)
    rows = [r for r in map(json.loads, (E/'motion/native.stdout').read_text().splitlines()) if r['frame'] == 204]
    names = ['Right side', 'Right front', 'Right rear', 'Opposite side']
    fig, axes = plt.subplots(1, 2, figsize=(12, 4))
    for row in rows:
        values = np.array(row['profile'])
        for ax, col in zip(axes, [1, 2]):
            ax.plot(values[:, 0], np.sqrt(values[:, col]), label=names[row['region']])
    for ax, title in zip(axes, ['All symmetric NN distances', 'Lowest 80% of pooled NN distances']):
        ax.axvline(0., color='gray', ls='--'); ax.set_xlabel('Additional current-point x translation (m)')
        ax.set_ylabel('NN RMS (m)'); ax.set_title(title); ax.grid(alpha=.3); ax.legend()
    fig.suptitle('KITTI01 frame204: full-point profiles after offline GT transform')
    fig.tight_layout(); fig.savefig(folder/'motion204.png', dpi=130); plt.close(fig)
    write(folder/'provenance.json', dict(exit_code=0, numerical_evidence_commit='59925767',
        inputs={str(p.relative_to(W)): sha(p) for p in [E/'baseline/archive.json', E/'motion/native.stdout']},
        matplotlib=matplotlib.__version__, outputs={p.name: sha(p) for p in folder.glob('*.png')},
        scope='Offline diagnostic plots, not estimator outputs, object annotations or full-sequence scores.'))


if __name__ == '__main__':
    main()
