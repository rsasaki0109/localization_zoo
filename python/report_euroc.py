"""Write an offline EuRoC diagnostic report with Matplotlib figures."""
import argparse
import html
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from evaluate_euroc import evaluate


def render(result, output):
    """Render evaluator output, preserving missing intervals as plot gaps."""
    output = Path(output)
    output.mkdir(parents=True, exist_ok=False)
    samples = result['samples']
    times = [s['time_s'] for s in samples]
    xyz = lambda key: np.array([s[key] if s[key] is not None else [np.nan]*3 for s in samples])
    estimate, reference = xyz('estimate_xyz'), xyz('reference_xyz')
    fig, axes = plt.subplots(2, 2, figsize=(12, 8), layout='constrained')
    for ax, (i, j) in zip(axes[0], [(0, 1), (0, 2)]):
        ax.plot(reference[:, i], reference[:, j], label='GT reference', color='black')
        ax.plot(estimate[:, i], estimate[:, j], '.-', label='VO', color='#0072B2', markersize=3)
        ax.set(xlabel=f'{"XYZ"[i]} (m)', ylabel=f'{"XYZ"[j]} (m)', title='First rectified camera frame')
        ax.set_aspect('equal', adjustable='datalim')
        ax.legend()
    for ax, key, label in [(axes[1, 0], 'translation_error_m', 'Translation error (m)'),
                            (axes[1, 1], 'rotation_error_deg', 'Rotation error (degrees)')]:
        values = [s[key] if s[key] is not None else np.nan for s in samples]
        ax.plot(times, values, '.-', markersize=3)
        ax.set(xlabel='Time since first image (s)', ylabel=label)
        ax.set_xlim(times[0], times[-1] if times[-1] > times[0] else times[0]+1)
        if not any(s[key] is not None for s in samples):
            ax.text(0.5, 0.5, 'No evaluable poses', ha='center', transform=ax.transAxes)
    for ax in axes.flat:
        ax.grid(alpha=0.25)
    fig.savefig(output / 'trajectory-errors.svg')
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(12, 3), layout='constrained')
    categories = [('initialized', 'Initialized', '#009E73'), ('tracking', 'Tracking', '#0072B2'),
                  ('lost', 'Lost', '#D55E00'), ('missing', 'No output', '#777777')]
    for i, (status, label, color) in enumerate(categories):
        stamps = [s['time_s'] for s in samples if s['status'] == status]
        ax.scatter(stamps, [i]*len(stamps), marker='|', color=color)
    no_gt = [s['time_s'] for s in samples if s['reference_xyz'] is None]
    ax.scatter(no_gt, [4]*len(no_gt), marker='|', color='#CC79A7')
    ax.set(yticks=range(5), yticklabels=[c[1] for c in categories]+['No usable GT'],
           xlabel='Time since first image (s)', title='Every input frame, including failures and missing output')
    fig.savefig(output / 'coverage.svg')
    plt.close(fig)
    summary = {k: v for k, v in result.items() if k != 'samples'}
    (output / 'metrics.json').write_text(json.dumps(summary, indent=2, allow_nan=False)+'\n')
    (output / 'samples.json').write_text(json.dumps(samples, indent=2, allow_nan=False)+'\n')
    fields = ['expected_frames', 'output_frames', 'tracked_frames', 'evaluated_frames',
              'tracking_fraction', 'evaluated_fraction', 'translation_rmse_m', 'rotation_rmse_deg', 'complete_output']
    table = ''.join('<tr><th>'+html.escape(k)+'</th><td>'+html.escape(str(summary[k]))+'</td></tr>' for k in fields)
    content = '''<!doctype html><html lang="en"><meta charset="utf-8">
<title>EuRoC stereo VO diagnostics</title>
<style>body{font:16px system-ui;max-width:1200px;margin:2rem auto;padding:0 1rem}img{width:100%}th,td{text-align:left;padding:.3rem 1rem;border-bottom:1px solid #ddd}</style>
<h1>EuRoC stereo VO diagnostics</h1>
<p>First-pose anchor; scale fixed at 1. No global trajectory fit.
Errors cover only evaluable poses. Always read them with full-sequence coverage.</p>
<p>Missing output is not successful tracking. Plot gaps are unavailable data; null metrics mean no evaluable poses.</p>
<table>'''+table+'''</table><img src="trajectory-errors.svg" alt="Trajectory projections and errors over time">
<img src="coverage.svg" alt="Tracking and ground-truth availability for all input frames">
<p><a href="metrics.json">Metrics and input hashes</a> · <a href="samples.json">Per-frame samples</a></p></html>'''
    (output / 'index.html').write_text(content)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mav0', type=Path)
    parser.add_argument('trajectory', type=Path)
    parser.add_argument('output_directory', type=Path)
    parser.add_argument('--max-gt-gap-ns', type=int, default=20000000)
    args = parser.parse_args()
    render(evaluate(args.mav0, args.trajectory, args.max_gt_gap_ns, include_samples=True), args.output_directory)


if __name__ == '__main__':
    main()
