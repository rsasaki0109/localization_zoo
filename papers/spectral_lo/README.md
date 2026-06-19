# Spectral-LO

周波数領域 (位相相関) LiDAR オドメトリ。**SROM: Simple Real-time Odometry and
Mapping** (phase-only-correlation 系の spectral マッチング, arXiv:2005.02042) の
機構を論文記述から独自再構成したもの。著者コード未公開。

## 動機

ICP / 分布ファクタは点ごとの対応付け (NN 探索) と反復最適化を要する。位相相関は
**FFT のシフト定理** を使い、2 画像間の並進・回転を反復なしの 1 ショットで推定する。
LiDAR スキャンを BEV 画像にすれば、地上車両の平面運動 (x, y, yaw) を周波数領域で
直接求められる。

## アプローチ

ICP を一切使わず、連続フレームの BEV 画像を周波数領域で位置合わせする。

1. **BEV ラスタ化**: スキャンを鳥瞰の密度画像 (N×N, N=2 の冪) に投影し、Hann 窓で
   スペクトル漏れを抑える。
2. **Fourier-Mellin 回転推定**: 各画像の `|FFT|` 振幅スペクトル (並進不変) を
   fftshift + 高域強調し、log-polar 座標へリサンプル。log-polar 画像同士の位相相関
   ピークの **角度成分** が相対 yaw。振幅は点対称なので角度は `[0, π)` で扱う。
3. **de-rotate**: 推定 yaw で現在の点群を回転して再ラスタ化。
4. **Phase-Only Correlation (POC)**: 参照 BEV と de-rotate 後 BEV の正規化相互
   パワースペクトル `R = F₁ F₂* / |F₁ F₂*|` を逆 FFT し、ピーク位置 (放物線
   サブピクセル補間つき) から相対並進 `(dx, dy)` を得る。

FFT は自作の radix-2 Cooley-Tukey (`fft1d`, 2 の冪サイズ)。`z`・roll・pitch は
据え置き、3-DoF 平面運動を合成する。

## パラメータ

- `bev_size` (既定 256): BEV 画像サイズ (2 の冪)。
- `bev_range` (既定 40 m): BEV 半径。セル = 2*bev_range/bev_size ≈ 0.31 m。
- `z_min/z_max` (既定 -3/3 m): BEV に投影する高さ範囲。
- `logpolar_angles/radii` (既定 256): log-polar リサンプルの解像度。
- `max_yaw_deg` (既定 30): フレーム間 yaw の探索上限 (誤マッチ棄却)。
- `keyframe_translation` (既定 0=frame-to-frame): >0 で frame-to-keyframe 化。

## テスト

`test_spectral_lo` の 3 ケース:

1. `FftCorrectness` — 既知 DFT (`[1,1,1,1]→[4,0,0,0]`) と FFT 往復復元。
2. `RecoversTranslation` — 純並進をフレーム間 POC で回復。
3. `RecoversYaw` — 純ヨー回転を Fourier-Mellin で回復。

## KITTI full での所見

KITTI full (no GT seed) では seq00 RPE **10.73%** / seq07 **10.23%**、ATE 128/55 m。
40 m BEV window で RPE は改善し、**~25–27 FPS** で走るが、ATE は 60 m window より
悪化するため精度はまだ低い。

**正直な所見**: ICP-free な BEV 位相相関は反復不要で高速・発散しない (DiLO の 18%
ほど崩れない) が、**粗い**: (1) BEV セル量子化 (~0.31 m) が並進分解能の下限となり
(サブピクセル補間で一部緩和)、(2) frame-to-frame でフレーム毎の量子化・回転誤差が
累積し、(3) **平面 3-DoF 仮定** が KITTI の坂・ピッチを無視するため z/姿勢 drift が
そのまま乗る。spectral/correlation という別ファミリの front-end として残す honest な
fast-but-coarse 結果で、機構の正しさ (FFT・Fourier-Mellin・POC) はユニットテストで
担保している。
