#!/usr/bin/env python3
"""Train OdoNet 1D-CNN on KITTI OXTS and export C++-loadable JSON weights."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset


class OdoNetCnn(nn.Module):
    """Paper-inspired 4×Conv1d + 3×FC (arXiv:2109.03091 Fig. 2)."""

    def __init__(self, window: int = 50, channels: int = 6) -> None:
        super().__init__()
        self.window = window
        self.channels = channels
        self.speed_scale = 30.0

        self.features = nn.Sequential(
            nn.Conv1d(channels, 32, kernel_size=7, padding=3),
            nn.ReLU(inplace=True),
            nn.MaxPool1d(2),
            nn.Conv1d(32, 64, kernel_size=5, padding=2),
            nn.ReLU(inplace=True),
            nn.MaxPool1d(2),
            nn.Conv1d(64, 64, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool1d(2),
            nn.Conv1d(64, 64, kernel_size=3, padding=1),
            nn.ReLU(inplace=True),
            nn.MaxPool1d(2),
        )
        with torch.no_grad():
            dummy = torch.zeros(1, channels, window)
            flat = self.features(dummy).reshape(1, -1).shape[1]
        self.head = nn.Sequential(
            nn.Linear(flat, 128),
            nn.ReLU(inplace=True),
            nn.Linear(128, 64),
            nn.ReLU(inplace=True),
            nn.Linear(64, 1),
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = self.features(x)
        x = x.reshape(x.size(0), -1)
        return self.head(x).squeeze(-1)


def export_model(model: OdoNetCnn, path: Path) -> None:
    convs = []
    pool_kernel = [2, 2, 2, 2]
    conv_modules = [m for m in model.features if isinstance(m, nn.Conv1d)]
    for conv in conv_modules:
        w = conv.weight.detach().cpu().numpy().reshape(-1).tolist()
        b = conv.bias.detach().cpu().numpy().tolist()
        convs.append({
            "in_channels": int(conv.in_channels),
            "out_channels": int(conv.out_channels),
            "kernel_size": int(conv.kernel_size[0]),
            "pool": 2,
            "weight": w,
            "bias": b,
        })

    fcs = []
    for fc in [m for m in model.head if isinstance(m, nn.Linear)]:
        w = fc.weight.detach().cpu().numpy().reshape(-1).tolist()
        b = fc.bias.detach().cpu().numpy().tolist()
        fcs.append({
            "in_features": int(fc.in_features),
            "out_features": int(fc.out_features),
            "weight": w,
            "bias": b,
        })

    payload = {
        "window_size": model.window,
        "channels": model.channels,
        "speed_scale": model.speed_scale,
        "convs": convs,
        "fcs": fcs,
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dataset",
        default="papers/odonet/data/kitti_odonet_dataset.npz",
    )
    parser.add_argument(
        "--output",
        default="papers/odonet/weights/odonet_kitti.json",
    )
    parser.add_argument("--epochs", type=int, default=80)
    parser.add_argument("--batch-size", type=int, default=1024)
    parser.add_argument("--lr", type=float, default=5e-5)
    args = parser.parse_args()

    data = np.load(args.dataset)
    window = int(data["window"])
    channels = int(data["channels"])

    def to_tensor(x: np.ndarray, y: np.ndarray) -> TensorDataset:
        x_t = torch.from_numpy(x.reshape(-1, window, channels)).transpose(1, 2)
        y_t = torch.from_numpy(y / 30.0)
        return TensorDataset(x_t, y_t)

    train_loader = DataLoader(
        to_tensor(data["train_x"], data["train_y"]),
        batch_size=args.batch_size,
        shuffle=True,
    )
    val_x = torch.from_numpy(data["val_x"].reshape(-1, window, channels)).transpose(1, 2)
    val_y = torch.from_numpy(data["val_y"] / 30.0)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = OdoNetCnn(window=window, channels=channels).to(device)
    opt = torch.optim.Adam(model.parameters(), lr=args.lr)
    loss_fn = nn.MSELoss()

    best_val = 1e9
    for epoch in range(1, args.epochs + 1):
        model.train()
        train_loss = 0.0
        n = 0
        for xb, yb in train_loader:
            xb = xb.to(device)
            yb = yb.to(device)
            opt.zero_grad()
            pred = model(xb)
            loss = loss_fn(pred, yb)
            loss.backward()
            opt.step()
            train_loss += float(loss.item()) * xb.size(0)
            n += xb.size(0)
        train_loss /= max(n, 1)

        model.eval()
        with torch.no_grad():
            vp = model(val_x.to(device))
            val_loss = float(loss_fn(vp, val_y.to(device)).item())
            val_mae = float((torch.abs(vp * 30.0 - val_y.to(device) * 30.0)).mean().item())

        if val_loss < best_val:
            best_val = val_loss
            export_model(model, Path(args.output))

        if epoch == 1 or epoch % 10 == 0 or epoch == args.epochs:
            print(f"epoch {epoch:3d} train_mse={train_loss:.6f} val_mse={val_loss:.6f} val_mae={val_mae:.3f} m/s")

    print(f"[done] exported best weights to {args.output} (val_mse={best_val:.6f})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
