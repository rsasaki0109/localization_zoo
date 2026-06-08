#!/usr/bin/env python3
"""手法間の「差」を強調する図を生成する。

生軌跡を並べても優秀な手法はGTにほぼ重なって見分けにくい。そこで GT からの
フレーム毎の位置誤差を可視化する。2 モード:

  heat : 小図グリッド。各軌跡を GT からの偏差で色付け (ズレ大=明色)。共有カラーバー。
  curve: 走行距離に対する位置誤差を全手法重ね描き (log 軸、drift 順の凡例)。

  python3 evaluation/scripts/plot_trajectory_diff.py --mode heat \
    --gt dogfooding_results/gt.txt --dir dogfooding_results \
    --summary /tmp/seq07_grid.json --cols 5 --title "KITTI seq07" \
    --methods KC_LO:KC-LO ... --highlight KC-LO \
    --out docs/assets/grid_seq07_error.png
"""

import argparse
import json
import os

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import numpy as np

BG = '#0d1117'
FG = '#c9d1d9'
MUTED = '#8b949e'
GT_COL = '#6e7681'
HI = '#2dd4bf'


def load_xy(path):
    a = np.loadtxt(path)
    if a.size == 0 or a.size % 12 != 0:
        raise SystemExit(f'{path}: expected KITTI poses (12 values/line), got size {a.size}')
    a = a.reshape(-1, 3, 4)
    return a[:, 0, 3], a[:, 1, 3]


def _norm(s):
    return s.replace('-', '_').lower() if s else s


def load_drift(summary):
    out = {}
    if not summary or not os.path.exists(summary):
        return out
    data = json.load(open(summary))
    results = (data.get('methods') or data.get('results') or data) if isinstance(data, dict) else data
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


def per_frame_error(gx, gy, ex, ey):
    """フレーム整合で GT からの XY ユークリッド誤差 (m) を返す。"""
    m = min(len(gx), len(ex))
    err = np.hypot(ex[:m] - gx[:m], ey[:m] - gy[:m])
    return err, m


def gt_distance(gx, gy):
    d = np.concatenate([[0.0], np.cumsum(np.hypot(np.diff(gx), np.diff(gy)))])
    return d


def collect(args):
    gx, gy = load_xy(args.gt)
    drift = load_drift(args.summary)
    items = []
    for spec in args.methods:
        base, label = spec.split(':', 1)
        path = os.path.join(args.dir, base + '.txt')
        if not os.path.exists(path):
            print('skip (missing):', path)
            continue
        ex, ey = load_xy(path)
        items.append(dict(label=label, ex=ex, ey=ey,
                          drift=drift.get(_norm(label))))
    # drift 昇順 (良い順)
    items.sort(key=lambda d: d['drift'] if isinstance(d['drift'], (int, float)) else float('inf'))
    return gx, gy, items


def render_heat(args):
    gx, gy, items = collect(args)
    n = len(items)
    cols = args.cols
    rows = (n + cols - 1) // cols

    pad = 0.08 * max(np.ptp(gx), np.ptp(gy))
    xlim = (gx.min() - pad, gx.max() + pad)
    ylim = (gy.min() - pad, gy.max() + pad)

    vmax = args.vmax
    cmap = plt.get_cmap('turbo')

    pw = 2.4
    fig, axes = plt.subplots(rows, cols, figsize=(cols * pw, rows * pw * 1.12), dpi=110)
    fig.patch.set_facecolor(BG)
    axes = np.atleast_1d(axes).ravel()

    sm = None
    for i, ax in enumerate(axes):
        ax.set_facecolor(BG)
        ax.set_xticks([]); ax.set_yticks([])
        for s in ax.spines.values():
            s.set_color('#30363d')
        if i >= n:
            ax.axis('off'); continue
        it = items[i]
        err, m = per_frame_error(gx, gy, it['ex'], it['ey'])
        ax.set_aspect('equal'); ax.set_xlim(*xlim); ax.set_ylim(*ylim)
        ax.plot(gx, gy, color=GT_COL, lw=1.1, alpha=0.45, zorder=1)
        pts = np.array([it['ex'][:m], it['ey'][:m]]).T.reshape(-1, 1, 2)
        segs = np.concatenate([pts[:-1], pts[1:]], axis=1)
        lc = LineCollection(segs, cmap=cmap, norm=plt.Normalize(0, vmax), zorder=3)
        lc.set_array(err[:-1]); lc.set_linewidth(2.0)
        sm = ax.add_collection(lc)
        hi = (it['label'] == args.highlight)
        ax.set_title(it['label'], color=(HI if hi else FG), fontsize=11,
                     fontweight='bold', pad=3)
        d = it['drift']
        emax = float(np.max(err)) if len(err) else 0.0
        sub = (f'{d:.3f}%/100m  ·  max {emax:.0f}m' if isinstance(d, (int, float))
               else f'max {emax:.0f}m')
        ax.text(0.5, 0.02, sub, color=MUTED, fontsize=8, ha='center',
                va='bottom', transform=ax.transAxes)

    fig.suptitle(f'Localization Zoo — drift from ground truth on {args.title} (no GT seed)',
                 color=FG, fontsize=15, fontweight='bold', y=0.998)
    fig.text(0.5, 0.965,
             'path colored by distance from ground truth  ·  cool = on-track, warm = drifting',
             color=MUTED, fontsize=10, ha='center')
    fig.tight_layout(rect=[0, 0.05, 1, 0.955])
    # 共有カラーバー
    cax = fig.add_axes([0.30, 0.025, 0.40, 0.018])
    cb = fig.colorbar(sm, cax=cax, orientation='horizontal')
    cb.set_label(f'position error (m), clipped at {vmax:.0f} m', color=MUTED, fontsize=9)
    cb.ax.xaxis.set_tick_params(color=MUTED, labelsize=8)
    plt.setp(plt.getp(cb.ax.axes, 'xticklabels'), color=MUTED)
    cb.outline.set_edgecolor('#30363d')
    fig.savefig(args.out, facecolor=BG)
    print('wrote', args.out, f'({n} methods, {rows}x{cols} heat)')


def render_curve(args):
    gx, gy, items = collect(args)
    dist = gt_distance(gx, gy)
    cmap = plt.get_cmap('turbo')
    n = len(items)

    fig, ax = plt.subplots(figsize=(10.5, 5.6), dpi=120)
    fig.patch.set_facecolor(BG); ax.set_facecolor(BG)
    for s in ax.spines.values():
        s.set_color('#30363d')
    ax.tick_params(colors=MUTED, labelsize=9)
    ax.grid(True, which='both', color='#1b2129', lw=0.6)

    for rank, it in enumerate(items):
        err, m = per_frame_error(gx, gy, it['ex'], it['ey'])
        hi = (it['label'] == args.highlight)
        col = HI if hi else cmap(0.12 + 0.76 * rank / max(1, n - 1))
        lw = 2.6 if hi else 1.4
        z = 10 if hi else 3
        d = it['drift']
        lbl = f"{it['label']}  {d:.3f}%" if isinstance(d, (int, float)) else it['label']
        ax.plot(dist[:m], np.clip(err, 1e-3, None), color=col, lw=lw, zorder=z,
                label=lbl, solid_capstyle='round')

    ax.set_yscale('log')
    ax.set_xlabel('distance traveled along ground truth (m)', color=FG, fontsize=11)
    ax.set_ylabel('position error from GT (m, log)', color=FG, fontsize=11)
    ax.set_title(f'Localization Zoo — error growth on {args.title} (no GT seed)',
                 color=FG, fontsize=14, fontweight='bold', pad=10)
    leg = ax.legend(loc='upper left', fontsize=8.2, ncol=2, frameon=True,
                    facecolor='#161b22', edgecolor='#30363d', labelcolor=FG,
                    title='RPE drift %/100m (lower better)')
    plt.setp(leg.get_title(), color=MUTED, fontsize=8.5)
    fig.text(0.5, 0.005, 'github.com/rsasaki0109/localization_zoo',
             color=MUTED, fontsize=8.5, ha='center')
    fig.tight_layout(rect=[0, 0.02, 1, 1])
    fig.savefig(args.out, facecolor=BG)
    print('wrote', args.out, f'({n} methods curve)')


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--mode', choices=['heat', 'curve'], default='heat')
    ap.add_argument('--gt', required=True)
    ap.add_argument('--dir', default='dogfooding_results')
    ap.add_argument('--summary', default=None)
    ap.add_argument('--methods', nargs='+', required=True)
    ap.add_argument('--highlight', default=None)
    ap.add_argument('--cols', type=int, default=5)
    ap.add_argument('--vmax', type=float, default=15.0, help='heat: 色のクリップ上限 (m)')
    ap.add_argument('--title', default='KITTI')
    ap.add_argument('--out', default='docs/assets/grid_diff.png')
    args = ap.parse_args()
    if args.mode == 'heat':
        render_heat(args)
    else:
        render_curve(args)


if __name__ == '__main__':
    main()
