# Codex Task: native-time provenance 要件の明文化

> **担当**: Codex
> **ブランチ**: `wip/profile-expansion-refresh`
> **PR**: Draft PR #1 に積む

---

## 背景（3行で）

- この repo の HDL-400 ベンチマークには「本物のタイムスタンプ付き点群 (native-time)」と「公開 bag から疑似生成したタイムスタンプ (synthetic-time)」の2系統がある
- synthetic-time での実験は既に動いているが、native-time の exact reproduction に何が必要かが**どこにも明文化されていない**
- これを `docs/native_time_provenance.md` として書き下ろす

## やること

`docs/native_time_provenance.md` を**新規作成**し、以下の内容を書く。

### 1. 用語定義

以下の3つの用語を定義すること:

| 用語 | 意味 |
|------|------|
| native per-point time | センサーが記録した各点の発射時刻。PointCloud2 の `time` フィールドに入っている |
| synthetic time | 公開 ROS1 bag に `time` フィールドが無いため、`extract_ros1_lidar_imu.py --time-mode index\|azimuth` で後付けした疑似時刻 |
| reference aggregate | `experiments/results/ct_lio_reference_profile_matrix.json` 等に入っている、native-time 由来のベンチマーク数値 |

### 2. 現状のデータフロー図（テキストで）

```
hdl_400_ros2 bag (native time 付き)
  └─ extract → PCD (time フィールド保持)
       └─ pcd_dogfooding → reference aggregate (native-time truth)

hdl_400 公開 ROS1 bag (time フィールド無し)
  └─ extract_ros1_lidar_imu.py --time-mode index
       └─ PCD (synthetic time 付き)
            └─ pcd_dogfooding → public synthetic-time aggregate
```

この図をベースに、2系統が**別物**であることを視覚的に示す。

### 3. exact native-time reproduction に必要な source artifact 条件

以下を箇条書きで列挙する:

- **必須 artifact**: `hdl_400_ros2` bag もしくはそこから抽出された、各点に native `time` フィールドを持つ PCD シーケンス
- **必須 artifact の所在**: `../lidarloc_ws/data/official/hdl_localization/hdl_400_ros2`（manifest 内の `source_bag` 参照）
- **GT trajectory**: `experiments/reference_data/hdl_400_public_reference.csv`（これは repo 内に存在する）
- **抽出スクリプト**: `evaluation/scripts/extract_ros1_lidar_imu.py` の `--time-mode preserve` モード（ただし native time が無い bag では失敗する）
- **検証方法**: 抽出した PCD の `time` フィールドが全点で非ゼロ・単調増加であること

### 4. synthetic-time でできること/できないことの表

| | synthetic-time | native-time |
|---|---|---|
| non-CT 系 (GICP, KISS-ICP 等) のベンチマーク | OK | OK |
| CT 系 (CT-LIO, CT-ICP) の variant 間比較 | OK（同条件内での相対比較） | OK |
| CT 系の stored reference aggregate との一致 | NG | OK |
| "exact reproduction" の主張 | NG | OK |
| 公開データだけで実行 | OK | NG（非公開 bag が必要） |

### 5. blocking 状態の記述

現在 `experiments/ct_lio_public_readiness_matrix.json` が `blocked` である理由を引用し、この文書と紐づける:

> `blocker`: "A reference-based CT-LIO comparison now exists separately, but this repository still does not ship an independently curated GT CSV aligned to the extracted HDL-400 LiDAR+IMU sequence."

### 6. 解除条件チェックリスト

```markdown
- [ ] `hdl_400_ros2` bag または native-time PCD が入手可能な場所に置かれている
- [ ] `extract_ros1_lidar_imu.py --time-mode preserve` で抽出が成功する
- [ ] 抽出 PCD の `time` フィールドが全点で有効値を持つ
- [ ] `pcd_dogfooding` で CT-LIO / CT-ICP を実行し、stored reference aggregate と ATE が ±5% 以内で一致する
- [ ] 上記の再現手順が本文書に記載されている
```

## やらないこと

- synthetic-time 側の変更
- manifest / aggregate JSON の変更
- `README.md` や `paper_*` docs の変更
- 実際のベンチマーク実行

## 参照すべきファイル（読むだけ）

| ファイル | 見るポイント |
|----------|-------------|
| `experiments/ct_lio_public_readiness_matrix.json` | `blocker` / `next_step` の文言 |
| `experiments/results/ct_lio_reference_profile_matrix.json` | `dataset.source_bag` のパス |
| `evaluation/scripts/extract_ros1_lidar_imu.py` | `--time-mode` の選択肢と `PACKED_POINT_DTYPE` |
| `evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md` | synthetic time の注意書き |
| `docs/interfaces.md` | Summary JSON Contract の構造 |
| `PLAN.md` §5, §8 | 「言ってよいこと/いけないこと」の基準 |

## 完了条件

1. `docs/native_time_provenance.md` が存在する
2. 上記セクション 1-6 が全て含まれている
3. `python3 evaluation/scripts/refresh_study_docs.py` を実行しても壊れない（この文書は手書き docs なので refresh 対象外のはず。念のため確認）
4. commit して PR #1 に push する
