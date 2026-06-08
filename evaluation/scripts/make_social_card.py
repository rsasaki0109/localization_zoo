#!/usr/bin/env python3
"""GitHub social-preview / OG カード (1280x640) を生成する。

左に KITTI seq00 の軌跡 (KC-LO over GT)、右にタイトル・タグライン・統計チップ。
ダークテーマ。Settings -> Social preview にアップロード、explorer の og:image にも使う。

  python3 evaluation/scripts/make_social_card.py \
    --gt dogfooding_results/gt.txt --traj dogfooding_results/KC_LO.txt \
    --out docs/assets/social_card.png
"""

import argparse

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

BG = '#0d1117'
FG = '#c9d1d9'
MUTED = '#8b949e'
GRID = '#21262d'
ACCENT = '#2dd4bf'
GT_COL = '#6e7681'


def load_xy(path):
    a = np.loadtxt(path)
    if a.size == 0 or a.size % 12 != 0:
        raise SystemExit(f'{path}: expected KITTI poses (12 values/line), got size {a.size}')
    a = a.reshape(-1, 3, 4)
    return a[:, 0, 3], a[:, 1, 3]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--gt', required=True)
    ap.add_argument('--traj', required=True)
    ap.add_argument('--out', default='docs/assets/social_card.png')
    args = ap.parse_args()

    gx, gy = load_xy(args.gt)
    tx, ty = load_xy(args.traj)

    dpi = 100
    fig = plt.figure(figsize=(12.8, 6.4), dpi=dpi)
    fig.patch.set_facecolor(BG)

    # --- 左: 軌跡 ---
    ax = fig.add_axes([0.0, 0.0, 0.46, 1.0])
    ax.set_facecolor(BG)
    ax.set_aspect('equal')
    ax.axis('off')
    allx = np.concatenate([gx, tx])
    ally = np.concatenate([gy, ty])
    pad = 0.08 * max(np.ptp(allx), np.ptp(ally))
    ax.set_xlim(allx.min() - pad, allx.max() + pad)
    ax.set_ylim(ally.min() - pad, ally.max() + pad)
    ax.plot(gx, gy, color=GT_COL, lw=1.6, alpha=0.5)
    ax.plot(tx, ty, color=ACCENT, lw=3.4, solid_capstyle='round')
    ax.plot([tx[-1]], [ty[-1]], 'o', color=ACCENT, ms=11, mec=BG, mew=2)

    # --- 右: テキスト ---
    x = 0.50
    fig.text(x, 0.86, 'Localization Zoo', color=FG, fontsize=44,
             fontweight='bold', va='top')
    fig.text(x, 0.715, 'From-paper C++ reimplementations of LiDAR',
             color=FG, fontsize=19, va='top')
    fig.text(x, 0.650, 'localization papers that ship', color=FG,
             fontsize=19, va='top')
    fig.text(x, 0.560, 'no public code', color=ACCENT, fontsize=22,
             fontweight='bold', va='top')
    fig.text(x, 0.475, 'Honestly benchmarked on KITTI Odometry  ·  ROS 2  ·  MIT',
             color=MUTED, fontsize=13.5, va='top')

    # 統計チップ
    chips = [('70+', 'methods'), ('30+', 'from-paper\nreimpls'),
             ('0.514%', 'KITTI seq07\nbest drift')]
    cw, gap = 0.135, 0.018
    cy, ch = 0.20, 0.20
    for i, (big, small) in enumerate(chips):
        cx = x + i * (cw + gap)
        ax2 = fig.add_axes([cx, cy, cw, ch])
        ax2.axis('off')
        ax2.add_patch(plt.Rectangle((0, 0), 1, 1, transform=ax2.transAxes,
                                    facecolor='#161b22', edgecolor=GRID, lw=1.2))
        ax2.text(0.5, 0.66, big, color=ACCENT, fontsize=24, fontweight='bold',
                 ha='center', va='center', transform=ax2.transAxes)
        ax2.text(0.5, 0.24, small, color=MUTED, fontsize=10.5,
                 ha='center', va='center', transform=ax2.transAxes,
                 linespacing=1.1)

    fig.text(x, 0.075, 'github.com/rsasaki0109/localization_zoo',
             color=MUTED, fontsize=13, va='top')

    fig.savefig(args.out, facecolor=BG)
    print('wrote', args.out)


if __name__ == '__main__':
    main()
