# Codex Task 3: 全テストパス確認と実装品質検証

> **担当**: Codex
> **ブランチ**: `wip/profile-expansion-refresh`
> **PR**: Draft PR #1 に積む

---

## 背景（3行で）

- `papers/*/test/` に全 38 実装のユニットテストがある
- 前回の `ctest` 全パスは earlier turn の記録で、Ceres 2.1/2.2 互換修正後の再確認が必要
- 各実装が元論文のアルゴリズムを忠実に再現しているかの deviation メモを文書化する

## やること

### Step 1: ビルド確認

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

全ターゲットのビルドが成功することを確認。エラーがあれば修正する。

### Step 2: 全テスト実行

```bash
ctest --test-dir build --output-on-failure -j$(nproc)
```

全テストがパスすることを確認。失敗があれば:
1. エラー内容を記録
2. Ceres 互換 (`ceres_compat.h`) 関連なら修正
3. テスト自体のバグなら修正
4. 既知の制約（データ依存等）なら skip にする

### Step 3: スクリプト syntax check

```bash
python3 -m py_compile evaluation/scripts/*.py
```

### Step 4: 実装品質検証ドキュメント作成

`docs/implementation_notes.md` を**新規作成**し、以下を記載:

各 method family について:

| セクション | 内容 |
|-----------|------|
| Method name | 手法名と元論文の引用 |
| Source files | `papers/{method}/src/` のファイル一覧 |
| LOC | 行数 |
| Algorithm fidelity | 元論文との一致度 (High / Medium / Low) |
| Known deviations | 簡略化や省略した要素 |
| Test coverage | テスト内容の概要 |
| Benchmark integration | pcd_dogfooding での統合状態 |

#### 記載対象: pcd_dogfooding 統合済みの 27 手法

以下を読んで判断すること:
- `papers/{method}/src/*.cpp` — メインの実装
- `papers/{method}/include/{method}/*.h` — ヘッダー
- `papers/{method}/test/*.cpp` — テスト
- `papers/{method}/CMakeLists.txt` — ビルド設定

#### 判断基準

- **High fidelity**: 元論文の主要アルゴリズムが忠実に実装されている
- **Medium fidelity**: 主要部分は正しいが一部の最適化や補助機能が省略されている
- **Low fidelity**: 大幅な簡略化がある（ただし意図的な場合はその理由も記載）

#### 特に注意して確認するポイント

1. **Ceres 互換**: `ceres_compat.h` を使っているか、古い API が残っていないか
2. **数値安定性**: ゼロ除算、NaN チェック等
3. **スレッドセーフ**: OpenMP 使用箇所の正しさ

## やらないこと

- ベンチマーク実行
- マニフェスト作成/変更
- generated docs の変更
- pcd_dogfooding.cpp への新機能追加

## 参照すべきファイル

- `common/include/common/ceres_compat.h` — Ceres 互換ヘルパー
- `CMakeLists.txt` — 全体ビルド構成
- `papers/*/CMakeLists.txt` — 各実装のビルド設定
- `evaluation/src/pcd_dogfooding.cpp` — 統合状態の確認

## 完了条件

1. `cmake --build build` が全ターゲットでエラーなし
2. `ctest` が全テストパス (またはスキップ理由が明記されている)
3. `python3 -m py_compile evaluation/scripts/*.py` がエラーなし
4. `docs/implementation_notes.md` が存在し、27 手法全てについて記載がある
5. commit して PR #1 に push
