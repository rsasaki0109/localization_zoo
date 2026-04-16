# Localization Zoo — 引き継ぎ PLAN（Copilot / Claude / 次のエージェント向け）

> **最終更新: 2026-04-16**
>
> この文書は、次のセッションが **短時間で repo の「いまの真実」** と **踏んではいけない罠** を掴むための handoff である。
> 数値・ready 数・ブランチは **常に Git と `experiments/results/index.json` で再検証すること**（本文に古い数値が残ると危険）。

---

## 0. 最初に読む順（最短ルート）

1. **本ファイルの §1（2026-04-16 スナップショット）**
2. [`experiments/results/index.json`](experiments/results/index.json)（`problems[].status` が真実）
3. [`evaluation/README.md`](evaluation/README.md)（評価の階層・ワンショットコマンド）
4. [`docs/interfaces.md`](docs/interfaces.md)（`pcd_dogfooding` の selector）
5. `git log -5 --oneline` と `git status -sb`

---

## 1. 2026-04-16 スナップショット（Claude 向け・現在の優先情報）

### 1.1 Git / ブランチ

| 項目 | 値（この更新時点） |
|------|---------------------|
| 既定の作業ブランチ | **`main`** |
| 直近の意味のあるコミット例 | `common` のユニットテスト、`evaluation` の評価基盤スクリプト、閾値の明示化 など |
| リモート | `git@github.com:rsasaki0109/localization_zoo.git` |

**注意:** 旧版 PLAN には `wip/profile-expansion-refresh` や Draft PR #1 の記述が残ることがある。実際にどちらで作業するかは **`git branch -a` と GitHub 上の PR 状態** で確認すること。長期の注意事項（HDL-400 / synthetic-time の区別など）は **`main` にマージ後も有効**。

### 1.2 ベンチインデックス（再計算コマンド）

`index.json` は generator が更新する。**本文に書いた件数はスナップショット**。

```bash
python3 - <<'PY'
import json
from collections import Counter
with open("experiments/results/index.json") as f:
    d = json.load(f)
problems = d["problems"]
print("problems", len(problems))
print(dict(Counter(p["status"] for p in problems)))
print("generated_at", d.get("generated_at"))
PY
```

**2026-04-16 時点の例:** `problems` **250** 件、`ready` **248**、`blocked` **1**、`skipped` **1**（再実行で変わる可能性あり）。

### 1.3 テスト基盤（2026-04 強化）

| 内容 | 場所 |
|------|------|
| `common` の数学・I/O テスト | [`common/test/test_math_utils.cpp`](common/test/test_math_utils.cpp), [`common/test/test_io.cpp`](common/test/test_io.cpp) |
| **合格閾値・ゴールデン値の一元化** | [`common/test/test_thresholds.h`](common/test/test_thresholds.h)（`kEpsilon*` / `kGolden*`） |
| KISS-ICP 空初回フレーム | [`papers/kiss_icp/test/test_kiss_icp.cpp`](papers/kiss_icp/test/test_kiss_icp.cpp) |
| CTest 登録 | [`common/CMakeLists.txt`](common/CMakeLists.txt) → `common_math`, `common_io` |

新しい数値テストを書くときは **マジックナンバーを散らさず**、`test_thresholds.h` か各 `papers/*/test` 内の名前付き定数に寄せると、Claude が「何を満たせばパスか」を追いやすい。

### 1.4 評価基盤（evaluation platform）

| 内容 | 場所 |
|------|------|
| 階層の説明（ctest → synthetic → fixture → matrix） | [`evaluation/README.md`](evaluation/README.md) |
| ローカル一括 | [`evaluation/scripts/run_local_evaluation_suite.sh`](evaluation/scripts/run_local_evaluation_suite.sh) |
| CMake ターゲット | `cmake --build build --target eval_local_suite`（[`evaluation/CMakeLists.txt`](evaluation/CMakeLists.txt)） |
| 環境チェック | [`evaluation/scripts/verify_environment.py`](evaluation/scripts/verify_environment.py)（`pcd_dogfooding` / `synthetic_benchmark` / fixture の存在など） |
| CI と同型のスモーク | [`evaluation/scripts/smoke_ci_fixture.sh`](evaluation/scripts/smoke_ci_fixture.sh) |

ルート [`README.md`](README.md) の sanity check 付近に、上記ワンショットへのリンクがある。

### 1.5 Claude が次にやるなら（優先度の例）

| 優先度 | 内容 |
|--------|------|
| P0 | `index.json` の `blocked` / `skipped` の理由を [`docs/decisions.md`](docs/decisions.md) または該当 aggregate と突き合わせる |
| P1 | テスト追加時は **閾値を `test_thresholds.h` または論理名付き定数** に集約 |
| P2 | exact **native-time** HDL-400 再現に必要な **ソースアーティファクト** を repo 外前提で整理（下記 §4） |
| P3 | `--manifest` 部分実行後は **`refresh_study_docs.py`** を忘れない（下記 §6） |

---

## 2. 真実源泉の優先順位（不変）

1. [`experiments/results/index.json`](experiments/results/index.json)
2. 各 [`experiments/results/*_matrix.json`](experiments/results/)
3. [`evaluation/scripts/run_experiment_matrix.py`](evaluation/scripts/run_experiment_matrix.py)
4. [`evaluation/src/pcd_dogfooding.cpp`](evaluation/src/pcd_dogfooding.cpp)
5. [`experiments/*_matrix.json`](experiments/)

二次資料: [`docs/experiments.md`](docs/experiments.md), [`docs/decisions.md`](docs/decisions.md), [`docs/interfaces.md`](docs/interfaces.md), `docs/paper_*.md`。

**README の表や件数**は生成物・ナラティブが古いことがある。**index.json を正とする。**

---

## 3. HDL-400 / CT 系で絶対に混同しないこと

### 3.1 二つのベンチは別物

- **reference / native-time スタイル**（ローカルに再現用データがある前提の window）
- **public ROS1 bag + synthetic per-point time** の **public-only** スプリット

後者は **公開データだけで回せる比較**として有効だが、**native per-point `time` の benchmark と同一・exact reproduction ではない**。論文や README で「再現した」と言うときは **どの契約か** を明示する。

### 3.2 aggregate の解釈

- **public ROS1 synthetic-time** の数字と **reference 側 aggregate** は、**同じ「canonical native-time 真値」として結びつけない**。
- ローカルデータの更新・refresh で reference 側の数値は変わりうる。**過去コミットの数字と現在の worktree を混ぜて説明しない。**

### 3.3 言ってよい / 言ってはいけない（要約）

**OK:** public ROS1 synthetic-time を *separate benchmark* として報告する。同じ synthetic-time 契約内での variant 比較。

**NG:** synthetic-time の結果を **original native-time の exact reproduction** と呼ぶ。current reference aggregate を **昔の canonical 唯一の真実** とみなす。

---

## 4. exact native-time provenance（未解決のまま明記）

repo 単体では完結しない。必要になるのは例えば:

- native per-point `time` を保持した元点群
- `hdl_400_ros2` 系のソースアーティファクト相当

**Claude の現実的な一手:** 「主張に必要なソース条件」を [`docs/`](docs/) に明文化し、synthetic-time 前進と混同されないようにする。

---

## 5. 数で見る規模（参考・2026-04-16 時点の例）

旧 PLAN の詳細表と矛盾しないよう、**件数は §1.2 のスクリプトで取り直すこと**。

| 指標 | 例 |
|------|-----|
| `index.json` の `problems` | 250 |
| `ready` / `blocked` / `skipped` | 248 / 1 / 1 |
| CLI method family（`pcd_dogfooding`） | README・`docs/interfaces.md` と整合を取る |

**27 methods** や manifest 数は時期によって変わる。**「27」と書く前に `pcd_dogfooding` の selector と manifest 一覧を grep / 集計する。**

---

## 6. 重要な罠（必読）

### 6.1 `--manifest` 部分実行

`run_experiment_matrix.py --manifest ...` は便利だが、`index.json` を subset で上書きしうる。

対策: 最後に

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

（またはフルマトリクスで再生成）

### 6.2 `blocked` と `skipped`

- **blocked:** 契約・データ上まだ解けない
- **skipped:** manifest はあるがその run で有効結果がない、など

混同しない。

### 6.3 `paper_tracks.md` の FPS

表の FPS は **「Best FPS」** であり、**default プロファイルの FPS ではない**。default の FPS は aggregate JSON や [`docs/paper_assets.md`](docs/paper_assets.md) を見る。

### 6.4 `dogfooding_results/` は原則 commit しない

大容量 raw は `.gitignore`。commit するのは manifest、aggregate、generator、小さい fixture、生成 docs など。

### 6.5 `hdl_graph_slam` full-sequence の例外ポリシー

長時間・重いシーケンスは **truthful に skipped / default-only** とする方針が README・decisions にある。**軽量プロファイルを後付けして「全部 ready」と見せない。**

---

## 7. 作業プレイブック

### 7.1 ローカル検証（推奨フロー）

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j"$(nproc)"
bash evaluation/scripts/run_local_evaluation_suite.sh
# または
cd build && ctest --output-on-failure
bash ../evaluation/scripts/smoke_ci_fixture.sh
```

### 7.2 Python スクリプトの構文チェック

```bash
python3 -m py_compile evaluation/scripts/run_experiment_matrix.py
```

### 7.3 ドキュメント再生成

```bash
python3 evaluation/scripts/refresh_study_docs.py
```

---

## 8. 最初に開くファイル（実装・実験）

| 目的 | ファイル |
|------|----------|
| 集計の全体像 | `experiments/results/index.json` |
| 走らせ方 | `evaluation/scripts/run_experiment_matrix.py` |
| 手法の入口 | `evaluation/src/pcd_dogfooding.cpp` |
| ROS1 抽出・時間モード | `evaluation/scripts/extract_ros1_lidar_imu.py` |
| public window のセットアップ | `evaluation/scripts/SETUP_PUBLIC_BENCHMARK_WINDOWS.md` |
| CT 系 manifest 例 | `experiments/*ct_*hdl_400*.json` |

---

## 9. 付録 A — 旧版 PLAN（2026-04-14 前後）からの履歴メモ

以下は **当時のブランチ `wip/profile-expansion-refresh`、Draft PR #1、dirty worktree** についての詳細記録だった。**現在の `main` の HEAD や PR 状態とは一致しない場合がある** が、次の **プロヴナンス** として残す。

- **`e4d9af4` 前後:** public ROS1 HDL-400 synthetic-time を native-time とは別 manifest に分離、`extract_ros1_lidar_imu.py` に `--time-mode` など。
- **その上のコミット群:** profile 拡張、CLINS public 証拠、ナラティブ refresh、`hdl_graph_slam` full-sequence ポリシー文書化、など。
- **dirty worktree にあった複数バッチ**（CT-ICP public の性能優先 default、新規 method の profile expansion、generated paper assets）は、**いまでは branch に載った可能性が高い** — **逐一 `git log` で確認**。

**一括 `git checkout -- .` で消さない**（複数意図のバッチが混ざるため）。

---

## 10. 付録 B — 旧 §「数で見る現在地」の表（2026-04-14 時点の参考値）

当時の `index.json` 集計として **250 manifest 相当 / 248 ready / 1 blocked / 1 skipped** などが記録されていた。**現在値は §1.2 のスクリプトで確認すること。**

HDL-400 の内訳（reference vs public ROS1 synthetic-time）、CT 系 manifest 数、`pcd_dogfooding` の method 一覧は **時期で変動**する。README の固定リストを鵜呑みにしない。

---

## 11. 付録 C — 検証ログの例（再現用コマンド）

```bash
git status --short
git log --oneline -10
ctest --test-dir build --output-on-failure -j"$(nproc)"   # build 済みなら
bash evaluation/scripts/smoke_ci_fixture.sh
python3 evaluation/scripts/verify_environment.py
```

---

## 12. Claude への最終メッセージ（繰り返し）

1. **`index.json` と Git が真実** — PLAN 本文の数値は古くなる。
2. **public ROS1 synthetic-time と native-time exact reproduction は別問題**。
3. **`paper_tracks.md` の FPS を default と誤読しない**。
4. **部分 `--manifest` のあと `refresh_study_docs.py`**。
5. **テストの合格条件は `test_thresholds.h` のような名前付き定数に寄せる**と、次のエージェントが追いやすい。

---

*End of PLAN.md — 更新時は日付と §1 を必ず直すこと。*
