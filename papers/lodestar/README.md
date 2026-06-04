# LODESTAR (from-paper reimplementation)

**LODESTAR: Degeneracy-Aware LiDAR-Inertial Odometry with Adaptive Schmidt-Kalman
Filter and Data Exploitation** — Eungchang Mason Lee, Kevin Christiansen Marsim,
Hyun Myung (KAIST), IEEE RA-L 2025, [arXiv:2511.09142](https://arxiv.org/abs/2511.09142).

著者コードは未公開。本実装は論文記述からの再現であり、純 LiDAR scope。

## 何を再現したか

LODESTAR は退化環境 (長い廊下・高高度飛行など LiDAR 計測が不均衡/疎になる場面)
での状態推定の悪条件化に、二つのモジュールで対処する。

1. **DA-ASKF (退化対応適応 Schmidt-Kalman フィルタ)**
   - registration の Hessian `H` を固有分解し、条件数 `χ(H)=λmax/λmin` を計算。
     `χ` は計測の疎さと不均衡の両方を捉える。
   - 滑動窓の過去状態を、退化度に応じて **active / fixed** に適応分類。
     `λ_k < λmax / T_χ` の方向を退化方向とみなす。
   - **Schmidt-Kalman 更新**: fixed 状態の Kalman ゲインを 0 にして更新しないが、
     その共分散を通して active 状態を拘束する ("lodestar" = 北極星のような
     基準アンカー)。退化方向の解はアンカー (本実装では等速予測) へ固定される。

2. **DA-DE (退化対応データ活用)**
   - (i) **剪定**: 局所化寄与 `Ω=|n·v_ℓ|` が `T_loc=cos(35°)` 未満の曖昧な計測を
     current/active 状態の更新から除外。
   - (ii) 剪定後 Jacobian の SVD で退化方向 (最小特異ベクトル) を同定。
   - (iii) **補償**: 退化方向に寄与する fixed 状態の計測を `χ(H̄) ≤ T_χ` になるまで
     逐次追加して悪条件を補償。

### パラメータ (論文値)

| 記号 | 値 | 役割 |
|---|---|---|
| `T_χ` | 1.5 | 退化判定の条件数しきい値 |
| `T_loc` | cos(35°)≈0.819 | 剪定の局所化寄与しきい値 |
| `s_a` | 2 | active 過去状態数 |
| `s_f` | 2 | fixed 過去状態数 |

## scope と近似

- **純 LiDAR / 等速 fallback**: KITTI は IMU を持たないため、IMU 予測は滑動窓の
  active 過去状態を平均した等速 (constant-velocity) 予測で代替する。論文の
  「fixed 状態 anchor」は本実装では等速予測として表現し、退化方向の解を予測へ
  Schmidt-Kalman 的に固定する。LIO 用の IMU 前積分バックエンド・完全な多状態
  共分散伝播・SubT-MRS/HILTI での評価は範囲外。
- **`T_χ` の適用**: 論文の 1.5 は DA-DE 正規化後 Jacobian に対する値。本実装は
  raw point-to-plane Hessian の各固有方向 (`λ_k < λmax / T_χ`) に適用する。
  対称シーンでは `χ≈1` で非退化、廊下では `x` 軸が `λ≈0` で退化と判定される
  (ユニットテストで検証)。実 KITTI 向けには profile で `t_chi` を上書きする。
- **DA-DE 補償**: 退化方向への fixed 計測の逐次追加は、退化方向アンカー注入
  (DA-ASKF) に統合して表現する。剪定 (step i) は退化フレームでのみ実行する。

## ビルドとテスト

```
cmake --build build --target test_lodestar
ctest --test-dir build -R test_lodestar
```

ユニットテスト:
- `RecoversKnownTranslation`: 良条件シーンで既知の並進を回復。
- `DetectsDegenerateCorridorNotClosedBox`: 廊下は退化判定 (条件数大・anchor 注入)、
  対称な閉じた部屋は非退化。
- `AnchorsDegenerateAxisToPrediction`: 退化軸では LiDAR が拘束できないが、滑動窓の
  等速予測 anchor が解を前進方向へ固定する。
