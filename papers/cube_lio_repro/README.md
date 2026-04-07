# CUBE-LIO (slide 準拠) — 再現スコープ

参照: [劉 陽, 第31回ロボティクスシンポジア スライド](https://www.docswell.com/s/scomup/59N8N9-2026-03-22-173102)（ツイート `https://x.com/scomup/status/2035684276591882292` からのリンク）。

## このディレクトリで「再現」しているもの

- **キューブマップ投影**: LiDAR 点をセンサ中心の単位方向に射影し、6 面の画像平面へ割り当てる（スライド p.10–11 の考え方）。
- **反射強度画像**: 各面でセルごとの最大反射強度（疎点群のラスタ化）。
- **IGM (intensity gradient magnitude)**: ガウシアン平滑化後の Sobel 勾配ノルム（スライド p.13）。
- **Semi-dense 特徴**: IGM が上位分位を超えるピクセル集合（スライド p.13 の high-response 選択の近似）。

## まだ含めないもの（本番 CUBE-LIO 全体）

- **光度残差と幾何残差の同時最適化**、**IMU / ESIKF**、ENWIDE 等データセット上の ATE 再現（スライド p.7, 15–18）。これは ICRA 2026 論文＋実装公開待ち、または別パッケージへの統合が必要です。

## デモの実行

```bash
python3 evaluation/scripts/cube_lio_cubemap_demo.py \
  --pcd dogfooding_results/mcd_kth_day_06_108/00000000/cloud.pcd \
  --output-dir /tmp/cube_lio_demo \
  --size 512
```

6 面を 1 枚にまとめる（俯瞰用）:

```bash
python3 evaluation/scripts/cube_lio_cubemap_demo.py \
  --pcd dogfooding_results/mcd_kth_day_06_108/00000000/cloud.pcd \
  --output-dir /tmp/cube_lio_demo \
  --montage --no-per-face-pngs
```

出力: 各面の `face_*_intensity.png`, `face_*_igm.png`（既定）、または `--montage` で `cubemap_montage_intensity.png` / `cubemap_montage_igm.png`、`features.csv`。

## 公開 MCD での `pcd_dogfooding` スモーク

```bash
./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh 30
```

`dogfooding_results/mcd_kth_day_06_108` に `imu.csv` がある場合のみ `ct_lio` も実行します。

```bash
./evaluation/scripts/smoke_mcd_pcd_dogfooding.sh all 20   # KTH + NTU + TUHH
```

## シーケンス全体の cubemap（軽量: モンタージュのみ）

```bash
python3 evaluation/scripts/cube_lio_cubemap_demo.py \
  --sequence-pcd-dir dogfooding_results/mcd_kth_day_06_108 \
  --output-dir /tmp/cube_kth_run \
  --max-sequence-frames 20 \
  --size 256
```

各フレームは `output_dir/00000000/` のように分かれ、`cubemap_montage_*.png` と `features.csv` が入ります。12 枚の面 PNG も欲しい場合は `--sequence-include-face-pngs`。

後段ツール向けに **JSON** でも特徴を出す: **`--export-features-json`**（`features.json`）。

## CI 用の超小さな MCD 切片

`evaluation/fixtures/mcd_kth_smoke/`（3 フレーム、コミット想定）を使った回帰は `evaluation/scripts/smoke_ci_fixture.sh`。GitHub Actions の `build-and-test` ジョブからも呼ばれます。
