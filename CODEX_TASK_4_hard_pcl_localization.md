# Codex Task: Koide Hard PCL Localization データセットの完走 + 退化調査の続き

> **担当**: Codex(Claude Code セッションからの引き継ぎ、2026-07-16)
> **ブランチ**: `main` 直行(これまでの成果も main に push 済み)
> **前提ドキュメント**: `evaluation/scripts/SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md` / `evaluation/scripts/SETUP_LIDAR_DEGENERACY_BENCHMARK.md`

---

## 背景(3行で)

- ntnu-arl トンネル bag の全軌跡実行で、短窓ヘルスチェックの negative result を覆した: 全手法がペア受理100%のまま、実際は正味変位 24〜115m(真値 ~500m)の巨大ドリフト。X-ICP は退化を 99.8% のフレームで検知できるが回復はできない(詳細: `experiments/results/lidar_degeneracy/tunnel_full/full_trajectory_comparison.md`)
- 次の完走目標としてユーザーが Koide "Hard Point Cloud Localization Dataset" (zenodo 10122133) を指定。GT(TUM形式)と参照地図(.ply)が同梱されており、GT-backed ATE / fixed-map false-lock / triage policy 較正がついに可能になる
- ダウンロードとセットアップは途中まで完了。ROS2 bag の検査から先が未着手

## 環境(重要・再発見に時間を溶かさないこと)

| 項目 | 値 |
|------|-----|
| ビルド環境 | **WSL2 Ubuntu-22.04**(ネイティブ Windows ビルドは不可: vcpkg の PCL は別プロジェクトの MinGW triplet 用) |
| WSL 内クローン | `/root/localization_zoo`(`git clone --no-hardlinks` した作業コピー。**Windows 側 `C:\Users\rsasa\Workspace\old\localization_zoo` が source of truth**。編集は Windows 側 → `cp` で WSL へ → WSL でビルド) |
| 実行方法 | `wsl -d Ubuntu-22.04 -u root -- bash -c "..."` |
| ⚠ quoting 罠 | Git Bash 経由だと `$変数` が wsl 越しで壊れる。**スクリプトをファイルに書いて `bash -c "bash /mnt/c/<path>"` で実行**すること(裸の `/mnt/c/...` 引数は MSYS がパス変換して壊す) |
| Python venv | `/root/lz-venv`(rosbags, ouster-sdk, requirements-lock 済み)。`/root/lz-venv/bin/python3` を使う |
| Ceres | WSL は 2.0.0。`common/include/common/ceres_compat.h` の版判定は修正済み(commit `a3c8b482`)。**戻さないこと** |
| ビルド済みバイナリ | `build/evaluation/{kiss_keyframe_odometry, litamin2_window_odometry, ct_icp_window_odometry, xicp_window_odometry, degen_sense_window_odometry, pcd_dogfooding}` |
| データ(gitignore、WSL のみ) | `data/lidar_degeneracy_datasets/tunnel.bag` (3.47GB) / `dogfooding_results/lidar_degeneracy_tunnel_full/` (3241 frames + imu.csv + frame_timestamps.csv) / `data/hard_pcl_localization/` |

## 現状(2026-07-16 時点)

push 済みコミット(main):

- `a3c8b482` ceres_compat 修正
- `169839a9` トンネル全軌跡ドリフト(ベースライン3手法)
- `82f9686e` X-ICP / DegenSense の GT-free window runner 追加
- `677d1d11` 退化対応手法の結果(検知はできる、回復には別センサが要る)
- `26f44412` Koide データセット SETUP ドキュメント

`data/hard_pcl_localization/`(WSL)の状態:

| ファイル | 状態 |
|---|---|
| gt.zip → `gt/gt/traj_lidar_*.txt` ×8 | ✅ 展開・形式確認済み(TUM: `stamp tx ty tz qx qy qz qw`、epoch秒) |
| map_*.ply ×4 | ✅ ダウンロード済み |
| indoor_easy_01.zip (1.77GB) | ✅ ダウンロード済み(未展開・未検査) |
| indoor_hard_01.zip (1.95GB) | ⚠ **ダウンロード途中で切れている可能性大**(セッション終了時 ~250MB)。サイズが 1,951,298,635 bytes か確認し、違えば `wget -c -O indoor_hard_01.zip "https://zenodo.org/api/records/10122133/files/indoor_hard_01.zip/content"` で再開 |
| 残り9ファイル(indoor_easy_02, indoor_kidnap_01/02, outdoor 6分割) | ❌ 未ダウンロード(URL パターンは同上、一覧は SETUP doc) |

## やること(優先順)

### 1. indoor_hard_01.zip のダウンロード完了確認・再開

上記のとおり。ついでに残りシーケンスもバックグラウンドで落とし始めてよい(合計 ~17.2GB、ディスクは 400GB+ 空き)。

### 2. ROS2 bag 検査

`indoor_easy_01.zip` を展開し、中身(おそらく `metadata.yaml` + `*.db3`)のトピック名・型・レートを確認して **SETUP_HARD_PCL_LOCALIZATION_BENCHMARK.md の「Data Format」節に記録**する。`/root/lz-venv` の `rosbags` は ROS2 bag も読める。確認事項:

- PointCloud2 トピック名、フィールド構成(per-point time はあるか)
- IMU トピック名・レート
- bag の clock domain と GT の epoch 秒が同一か(ずれてたらオフセット推定が必要)
- indoor は Azure Kinect、outdoor は Livox MID360(Livox カスタム msg の可能性あり)

### 3. 抽出パイプライン整備

`evaluation/scripts/extract_ros1_lidar_imu.py` / `extract_ouster_packet_lidar_imu.py` に倣って ROS2 版 extractor を作る(または既存を拡張)。出力は dogfooding PCD layout: `NNNNNNNN/cloud.pcd` + `frame_timestamps.csv` + `imu.csv`。

あわせて **TUM → dogfooding GT CSV コンバータ**(`lidar_pose.x,y,z,roll,pitch,yaw` 形式、`pcd_dogfooding` が読む形式は `evaluation/src/pcd_dogfooding.cpp` の `loadGTPoses` 参照)を用意。

### 4. 最初のベンチマークスライス(GT-backed ATE)

indoor_easy_01(ベースライン)と indoor_hard_01(劣化)で:

```
ベースライン: kiss_keyframe / litamin2 / ct_icp の window runner
退化対応:     xicp_window_odometry / degen_sense_window_odometry(±IMU)
```

今回は GT があるので **ATE/RPE を出す**(window runner は gt_csv を受ける口がある。GT-free 前提で `-` 運用していたので、GT CSV を渡した時の ATE 計算が実装されているか確認し、無ければ足すか `pcd_dogfooding` 経由で走らせる)。結果は `experiments/results/hard_pcl_localization/<sequence>/` に。

### 5. fixed-map false-lock トラック(本命)

同梱の `map_indoor_hard.ply` 等を使い、`pcd_dogfooding --methods fixed_map_ndt` + `--fixed-map-ndt-trace-json` で kidnap シーケンスを流す。既存の publish-guard / trace-sequence verifier(`SETUP_LIDAR_DEGENERACY_BENCHMARK.md` 後半のコマンド群)を **GT エラーラベル付きで**評価するのが目的。NTNU 側では GT が無くて `risk_gt_calibration` が空欄のまま staged されている — ここを埋める:

```bash
python3 evaluation/scripts/calibrate_lidar_degeneracy_risk.py --gt-csv <seq>=<path>
```

### 6. (余力があれば)退化調査の残り

- RELEAD constrained-ESIKF frontend の runner 化(IMU 配線が必要、`papers/relead` + `papers/ct_icp_relead` 参照)
- X-ICP の per-frame localizability を triage policy(`evaluation/config/lidar_degeneracy_triage_policy.json`, v4)にランタイム退化シグナルとして統合。トンネルでベースラインの全ゲートが green のまま 99.8% のフレームに退化が出ていた、という根拠は `tunnel_full/full_trajectory_comparison.md`

## 引き継ぐ知見(再導出しないこと)

1. **短窓ペアヘルスゲートは慢性退化ドリフトに盲目**。200フレーム窓では全手法 100% green、全軌跡では正味変位が真値の 5〜23%
2. **検知と回復は別問題**: X-ICP(絶対閾値の localizability 分類)は 99.8% 検知するが正味 33m。DegenSense の median+MAD は慢性退化で基準線ごと順応して 14% しか出さない(スパイク検知器を慢性環境に使うな)
3. **IMU ブレンドは正味変位を 105→115m に改善する程度**。等速直進は加速度計でも観測不能。回復には退化方向を観測するセンサ(radar/Doppler/ホイール/強度)が要る — ntnu-arl 論文・赤井氏スライド・CUBE-LIO と同じ結論に実データで到達
4. runner の実行時間目安(WSL, 12 core): X-ICP 45s / KISS・LiTAMIN2・CT-ICP 数分 / DegenSense ~16-18分(3241フレーム)

## 参考資料(このタスクの出発点)

- https://github.com/ntnu-arl/lidar_degeneracy_datasets (ICRA2024 "Degradation Resilient LiDAR-Radar-Inertial Odometry")
- https://zenodo.org/records/10122133 (Koide, Hard Point Cloud Localization Dataset)
- 赤井氏 ROBOMECH2026 スライド(Lie群〜Continuous-Time LIO、退化環境でのセンサ融合)
- Panasonic CUBE-LIO(intensity photometric + cubemap で退化対策)
