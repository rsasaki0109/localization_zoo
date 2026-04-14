# MulRan benchmark prep (LiDAR + `global_pose.csv`)

MulRan sequences ship **Ouster OS1-64** scans as KITTI-style `.bin` (four floats per point: x, y, z, intensity) and **`global_pose.csv`**. Official releases use MulRan **file_player** format: **timestamp + twelve floats** forming a **3×4** row-major pose matrix (`R | t`), not a separate quaternion row. `prepare_mulran_inputs.py` parses that layout and, when LiDAR frame count and `global_pose` row count differ, **matches each scan to the nearest GT timestamp** (same idea as `file_player_mulran`), so short sequences like the public **ParkingLot** sample work.

## 1. Download

**Full sequences (DCC, KAIST, …):** use the official [Download Request Form](https://forms.gle/EmUybUiGc8pR3r7Q6) on [MulRan — Download](https://sites.google.com/view/mulran-pr/download).

**Small public sample (no form):** the **ParkingLot** folder can be fetched with [`gdown`](https://github.com/wkentaro/gdown), for example:

```bash
pip install gdown
mkdir -p data && gdown --folder \
  'https://drive.google.com/drive/folders/1IQZ485H13GcgdsRgEYj0uFF0jlE1j6dW' \
  -O data/mulran_parkinglot
```

Unpack `Ouster.zip` inside that folder if it is still zipped (`unzip -o data/mulran_parkinglot/Ouster.zip`).

Any sequence folder you use must contain:

- `<sequence>/Ouster/*.bin`
- `<sequence>/global_pose.csv`

## 2. Export `dogfooding_results/` + `experiments/reference_data/`

From the repository root:

```bash
python3 evaluation/scripts/prepare_mulran_inputs.py \
  --sequence-root /path/to/MulRan/DCC01 \
  --max-frames 120 \
  --include-full
```

Defaults write:

- `dogfooding_results/mulran_dcc01_120/` — first 120 frames
- `dogfooding_results/mulran_dcc01_full/` — full sequence (when `--include-full`)
- `experiments/reference_data/mulran_dcc01_120_gt.csv` (and `_full` variant)

Use `--lidar-subdir` if your tree uses a different folder name than `Ouster`.

## 3. Smoke benchmark

```bash
./build/evaluation/pcd_dogfooding \
  dogfooding_results/mulran_dcc01_120 \
  experiments/reference_data/mulran_dcc01_120_gt.csv \
  --methods litamin2,ct_icp,gicp,kiss_icp \
  --summary-json /tmp/mulran_smoke.json
```

## 4. Full matrix

After local aggregates look reasonable, point `run_experiment_matrix.py` at manifests under `experiments/pending/` that reference these paths, then promote out of `pending/` when ready.
