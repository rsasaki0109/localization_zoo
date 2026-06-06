#!/usr/bin/env python3
"""KITTI軌跡を上面視でアニメーション描画し、README hero 用の GIF/MP4 を生成する。

dogfooding_results/ の KITTI 形式ポーズ (1行12値) を読み込み、GT を背景に複数手法の
軌跡を時間方向に描き進める。ダークテーマ。再利用可能なスクリプトとして配置。

例:
  python3 evaluation/scripts/animate_trajectory.py \
    --gt dogfooding_results/gt.txt \
    --est "KISS-ICP:dogfooding_results/KISS_ICP.txt:#f0883e:0.86" \
          "TrICP-LO:dogfooding_results/TrICP_LO.txt:#a371f7:0.93" \
          "KC-LO:dogfooding_results/KC_LO.txt:#2dd4bf:0.84" \
    --highlight KC-LO --title "KITTI seq00" \
    --gif docs/assets/hero_seq00.gif --mp4 docs/assets/hero_seq00.mp4
"""

import argparse

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

BG = '#0d1117'      # GitHub dark canvas
FG = '#c9d1d9'      # foreground text
GRID = '#21262d'
GT_COL = '#6e7681'  # ground-truth reference


def load_xy(path):
    """KITTI形式ポーズの接地面 (x, y) 列を返す。"""
    a = np.loadtxt(path).reshape(-1, 3, 4)
    return a[:, 0, 3], a[:, 1, 3]


def parse_est(spec):
    """'name:path:color:drift' を分解 (color/drift は省略可)。"""
    parts = spec.split(':')
    name, path = parts[0], parts[1]
    color = parts[2] if len(parts) > 2 and parts[2] else '#58a6ff'
    drift = parts[3] if len(parts) > 3 and parts[3] else None
    return name, path, color, drift


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--gt', required=True)
    ap.add_argument('--est', nargs='+', required=True,
                    help='name:path[:color[:drift%%]]')
    ap.add_argument('--highlight', default=None, help='強調する手法名')
    ap.add_argument('--title', default='KITTI')
    ap.add_argument('--gif', default='hero.gif')
    ap.add_argument('--mp4', default=None)
    ap.add_argument('--frames', type=int, default=150, help='アニメ駒数')
    ap.add_argument('--fps', type=int, default=20)
    ap.add_argument('--width', type=int, default=900)
    args = ap.parse_args()

    gx, gy = load_xy(args.gt)
    n = len(gx)
    methods = []
    for spec in args.est:
        name, path, color, drift = parse_est(spec)
        ex, ey = load_xy(path)
        m = min(len(ex), n)
        methods.append(dict(name=name, x=ex[:m], y=ey[:m], color=color,
                            drift=drift, hi=(name == args.highlight)))

    # 図のセットアップ -------------------------------------------------------
    dpi = 100
    w_in = args.width / dpi
    h_in = w_in * 0.62
    fig = plt.figure(figsize=(w_in, h_in), dpi=dpi)
    fig.patch.set_facecolor(BG)
    ax = fig.add_axes([0.04, 0.05, 0.7, 0.9])
    ax.set_facecolor(BG)
    ax.set_aspect('equal')
    for s in ax.spines.values():
        s.set_visible(False)
    ax.tick_params(colors=GRID, labelsize=7)
    ax.grid(True, color=GRID, lw=0.5)

    allx = np.concatenate([gx] + [m['x'] for m in methods])
    ally = np.concatenate([gy] + [m['y'] for m in methods])
    pad = 0.06 * max(np.ptp(allx), np.ptp(ally))
    ax.set_xlim(allx.min() - pad, allx.max() + pad)
    ax.set_ylim(ally.min() - pad, ally.max() + pad)

    # 背景に GT 全体を薄く
    ax.plot(gx, gy, color=GT_COL, lw=1.0, alpha=0.35, zorder=1)

    # 動的アーティスト
    gt_line, = ax.plot([], [], color=GT_COL, lw=1.6, alpha=0.9, zorder=2,
                       label='Ground truth')
    # drift 降順 (悪い順) に太→細の入れ子バンドで描き、全色が縁取りで見えるようにする。
    # 最良 (=最前面・最細) を最後に重ね、強調手法はヘッドマーカーで主張する。
    draw_order = sorted(range(len(methods)),
                        key=lambda i: float(methods[i]['drift']) if methods[i]['drift'] else 0,
                        reverse=True)
    nm = len(methods)
    lines = [None] * len(methods)
    heads = [None] * len(methods)
    for rank, i in enumerate(draw_order):
        m = methods[i]
        lw = 4.2 - 1.6 * rank / max(1, nm - 1)        # 背面ほど太い
        z = 3 + rank
        ln, = ax.plot([], [], color=m['color'], lw=lw, zorder=z,
                      solid_capstyle='round', alpha=0.95)
        hd, = ax.plot([], [], 'o', color=m['color'],
                      ms=9 if m['hi'] else 5, zorder=20 + rank,
                      mec=BG, mew=1.4)
        lines[i] = ln
        heads[i] = hd

    # テキスト / ブランディング --------------------------------------------
    fig.text(0.04, 0.95, 'Localization Zoo', color=FG, fontsize=16,
             fontweight='bold', va='top')
    fig.text(0.04, 0.905, f'{args.title}  ·  top-down  ·  no GT seed',
             color=GT_COL, fontsize=9, va='top')

    # 右側の凡例パネル
    lx = 0.76
    fig.text(lx, 0.88, 'RPE drift  (%/100 m)', color=FG, fontsize=9,
             fontweight='bold', va='top')
    fig.text(lx, 0.855, 'lower is better ↓', color=GT_COL, fontsize=7.5, va='top')
    yy = 0.80
    fig.text(lx, yy, '— Ground truth', color=GT_COL, fontsize=9, va='top')
    yy -= 0.05
    # drift 昇順で並べる
    order = sorted(range(len(methods)),
                   key=lambda i: float(methods[i]['drift']) if methods[i]['drift'] else 9e9)
    for i in order:
        m = methods[i]
        label = m['name']
        val = f"  {m['drift']}%" if m['drift'] else ''
        weight = 'bold' if m['hi'] else 'normal'
        fig.text(lx, yy, f"— {label}{val}", color=m['color'], fontsize=10.5,
                 fontweight=weight, va='top')
        if m['hi']:
            fig.text(lx + 0.005, yy - 0.032, 'leaderboard seq07 #1',
                     color=m['color'], fontsize=6.8, va='top', style='italic')
            yy -= 0.028
        yy -= 0.052
    fig.text(lx, 0.10, 'github.com/rsasaki0109/\nlocalization_zoo',
             color=GT_COL, fontsize=8, va='top')
    progress = fig.text(lx, 0.04, '', color=GT_COL, fontsize=8, va='top')

    # アニメ ----------------------------------------------------------------
    hold = max(6, args.fps // 2)              # 末尾の静止駒
    total = args.frames + hold
    steps = np.linspace(1, n, args.frames).astype(int)

    def update(f):
        idx = steps[min(f, args.frames - 1)]
        gt_line.set_data(gx[:idx], gy[:idx])
        for m, ln, hd in zip(methods, lines, heads):
            k = min(idx, len(m['x']))
            ln.set_data(m['x'][:k], m['y'][:k])
            if k > 0:
                hd.set_data([m['x'][k - 1]], [m['y'][k - 1]])
        progress.set_text(f'frame {idx:>4d} / {n}')
        return [gt_line, *lines, *heads, progress]

    anim = animation.FuncAnimation(fig, update, frames=total,
                                   interval=1000 / args.fps, blit=False)

    print(f'writing {args.gif} ...')
    anim.save(args.gif, writer=animation.PillowWriter(fps=args.fps))
    if args.mp4:
        print(f'writing {args.mp4} ...')
        try:
            anim.save(args.mp4, writer=animation.FFMpegWriter(
                fps=args.fps, bitrate=2400,
                extra_args=['-pix_fmt', 'yuv420p']))
        except Exception as e:  # noqa
            print('mp4 skipped:', e)
    print('done')


if __name__ == '__main__':
    main()
