#!/usr/bin/env python3
"""複数手法の 2D 軌跡を 1 枚の小図グリッド (small multiples) にまとめる。

各パネルに 1 手法の上面視軌跡を GT (薄線) の上に重ね、手法名と RPE drift を表示。
ダークテーマ。README 用の「全手法まとめ図」を生成する。

  python3 evaluation/scripts/plot_trajectory_grid.py \
    --gt dogfooding_results/gt.txt --dir dogfooding_results \
    --summary /tmp/seq07_grid.json --cols 5 --title "KITTI seq07" \
    --methods KISS_ICP:KISS-ICP KC_LO:KC-LO TrICP_LO:TrICP-LO ... \
    --out docs/assets/grid_seq07.png
"""

import argparse
import json
import os

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

BG = '#0d1117'
FG = '#c9d1d9'
MUTED = '#8b949e'
GT_COL = '#6e7681'
LINE = '#2dd4bf'
HILINE = '#f0883e'      # 強調手法色


def load_xy(path):
    a = np.loadtxt(path)
    if a.size == 0 or a.size % 12 != 0:
        raise SystemExit(f'{path}: expected KITTI poses (12 values/line), got size {a.size}')
    a = a.reshape(-1, 3, 4)
    return a[:, 0, 3], a[:, 1, 3]


def _norm(s):
    return s.replace('-', '_').lower() if s else s


def load_drift(summary):
    """summary json から表示名 -> RPE drift(%) の対応を作る。"""
    out = {}
    if not summary or not os.path.exists(summary):
        return out
    data = json.load(open(summary))
    if isinstance(data, dict):
        results = data.get('methods') or data.get('results') or data
    else:
        results = data
    if isinstance(results, dict):
        results = results.values()
    for r in results:
        if not isinstance(r, dict):
            continue
        name = r.get('name')
        for k in ('rpe_trans_pct', 'rpe_trans_percent', 'drift_pct', 'drift'):
            if name and k in r and r[k] is not None:
                out[_norm(name)] = r[k]
                break
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--gt', required=True)
    ap.add_argument('--dir', default='dogfooding_results')
    ap.add_argument('--summary', default=None)
    ap.add_argument('--methods', nargs='+', required=True,
                    help='basename:label (txt は <dir>/<basename>.txt)')
    ap.add_argument('--highlight', default=None, help='強調する label')
    ap.add_argument('--cols', type=int, default=5)
    ap.add_argument('--title', default='KITTI')
    ap.add_argument('--out', default='docs/assets/grid.png')
    args = ap.parse_args()

    gx, gy = load_xy(args.gt)
    drift = load_drift(args.summary)

    items = []
    for spec in args.methods:
        base, label = spec.split(':', 1)
        path = os.path.join(args.dir, base + '.txt')
        if not os.path.exists(path):
            print('skip (missing):', path)
            continue
        items.append((label, path))

    n = len(items)
    cols = args.cols
    rows = (n + cols - 1) // cols

    # GT 範囲で全パネル共通スケール (公平比較)
    pad = 0.08 * max(np.ptp(gx), np.ptp(gy))
    xlim = (gx.min() - pad, gx.max() + pad)
    ylim = (gy.min() - pad, gy.max() + pad)

    pw = 2.4
    fig, axes = plt.subplots(rows, cols, figsize=(cols * pw, rows * pw * 1.08),
                             dpi=110)
    fig.patch.set_facecolor(BG)
    axes = np.atleast_1d(axes).ravel()

    for i, ax in enumerate(axes):
        ax.set_facecolor(BG)
        ax.set_xticks([])
        ax.set_yticks([])
        for s in ax.spines.values():
            s.set_color('#30363d')
        if i >= n:
            ax.axis('off')
            continue
        label, path = items[i]
        ex, ey = load_xy(path)
        hi = (label == args.highlight)
        col = HILINE if hi else LINE
        ax.set_aspect('equal')
        ax.set_xlim(*xlim)
        ax.set_ylim(*ylim)
        ax.plot(gx, gy, color=GT_COL, lw=1.3, alpha=0.55, zorder=1)
        ax.plot(ex, ey, color=col, lw=1.8, zorder=2, solid_capstyle='round')
        d = drift.get(_norm(label))
        sub = f'{d:.3f}%/100m' if isinstance(d, (int, float)) else ''
        ax.set_title(label, color=(col if hi else FG), fontsize=11,
                     fontweight='bold', pad=3)
        ax.text(0.5, 0.02, sub, color=MUTED, fontsize=8.5, ha='center',
                va='bottom', transform=ax.transAxes)

    fig.suptitle(
        f'Localization Zoo — 2D trajectories on {args.title} (no GT seed)',
        color=FG, fontsize=15, fontweight='bold', y=0.997)
    fig.text(0.5, 0.965,
             'each panel: estimated path (color) over ground truth (gray)  ·  RPE drift, lower is better',
             color=MUTED, fontsize=10, ha='center')
    fig.tight_layout(rect=[0, 0, 1, 0.955])
    fig.savefig(args.out, facecolor=BG)
    print('wrote', args.out, f'({n} methods, {rows}x{cols})')


if __name__ == '__main__':
    main()
