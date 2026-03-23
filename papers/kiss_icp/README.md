# KISS-ICP: In Defense of Point-to-Point ICP

## 論文情報
- **著者**: Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Louis Wiesmann, Jens Behley, Cyrill Stachniss
- **会場**: RA-L 2023
- **GitHub**: https://github.com/PRBonn/kiss-icp (MIT License)

## 手法概要
Point-to-Point ICPをシンプルに正しくやるだけで高精度。核心は4つ:
1. ボクセルハッシュマップによる高速近傍探索
2. 適応的対応距離閾値 (前フレームの動きから推定)
3. ロバストカーネル
4. ボクセルサブサンプリング
