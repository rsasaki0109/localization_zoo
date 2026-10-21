"""Plot every inherited endpoint and every cluster profile; no case selection by outcome."""
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from common import E, R, read, write, module


def main():
    report = read(E/'analysis/report.json')
    target = R/'figures'; target.mkdir()
    x = range(len(report['cases']))
    fig, axes = plt.subplots(3, 1, figsize=(9, 8), sharex=True)
    for mode, color in (('baseline', '#475569'), ('candidate', '#dc2626')):
        for ax, key, label in zip(axes, ('length_ratio', 'vector_error', 'rotation_error_mrad'),
                                  ('Translation length / GT', 'Translation vector error / GT', 'Rotation error (mrad)')):
            ax.plot(x, [c[mode][key] for c in report['cases']], 'o-', color=color,
                    label='Unchanged reference' if mode == 'baseline' else 'Frozen automatic cluster mask')
            ax.set_ylabel(label); ax.grid(alpha=.2)
    axes[0].axhline(1, color='black', linestyle=':', linewidth=1)
    axes[0].legend(fontsize=9)
    axes[-1].set_xticks(list(x), [str(c['frame']) for c in report['cases']])
    axes[-1].set_xlabel('KITTI01 inherited endpoint (categorical spacing; matched natural seed)')
    fig.suptitle('Local improvement does not establish safe static-support selection')
    fig.tight_layout(); fig.savefig(target/'terminal_errors.png', dpi=150); plt.close(fig)
    verification = module('cluster_figure_archive', E/'verify.py')
    native = list(map(json.loads, verification.restored(E/'controls', 'native.stdout.jsonl').splitlines()))
    profiles = [v for v in native if v['type'] == 'profile']
    fig, axes = plt.subplots(2, 4, figsize=(14, 7), sharex=True, sharey=True)
    for ax, case in zip(axes.flat, report['cases']):
        rows = [v for v in profiles if v['frame'] == case['frame'] and v['costs_m2']]
        for excluded, color, label in ((False, '#2563eb', 'Kept'), (True, '#dc2626', 'Excluded')):
            selected = [v for v in rows if v['excluded'] == excluded]
            ax.scatter([(v['best_index']-60)*.05 for v in selected], [v['points'] for v in selected],
                       s=14, alpha=.65, color=color, label=label)
        ax.axvline(-1, color='gray', linestyle=':', linewidth=.8)
        ax.axvline(1, color='gray', linestyle=':', linewidth=.8)
        ax.set_yscale('log'); ax.set_title(f"Frame {case['frame']}"); ax.grid(alpha=.2)
    axes[0, 0].legend(fontsize=8)
    fig.supxlabel('Full-point best shift along seed translation (m)')
    fig.supylabel('Source full points per facade-bearing cluster (log scale)')
    fig.suptitle('All cluster minima; geometric inconsistency is not a semantic motion label')
    fig.tight_layout(); fig.savefig(target/'cluster_minima.png', dpi=150); plt.close(fig)
    write(target/'metadata.json', dict(numerical_commit='58240931bbe1dbd9696a9dea3604708e980467d3',
          matplotlib_version=matplotlib.__version__, source='Every case in analysis/report.json and every controls/native.stdout.jsonl profile',
          no_new_measurement=True))


if __name__ == '__main__':
    main()
