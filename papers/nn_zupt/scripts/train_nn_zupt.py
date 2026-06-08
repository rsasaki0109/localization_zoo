#!/usr/bin/env python3
"""Train NN-ZUPT binary CNN; export JSON for C++."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset


class ZuptNet(nn.Module):
    def __init__(self, window: int = 50, channels: int = 6) -> None:
        super().__init__()
        self.window = window
        self.channels = channels
        self.features = nn.Sequential(
            nn.Conv1d(channels, 32, 7, padding=3), nn.ReLU(),
            nn.MaxPool1d(2),
            nn.Conv1d(32, 64, 5, padding=2), nn.ReLU(),
            nn.MaxPool1d(2),
            nn.Conv1d(64, 64, 3, padding=1), nn.ReLU(),
            nn.MaxPool1d(2),
            nn.Conv1d(64, 64, 3, padding=1), nn.ReLU(),
            nn.MaxPool1d(2),
        )
        with torch.no_grad():
            flat = self.features(torch.zeros(1, channels, window)).reshape(1, -1).shape[1]
        self.head = nn.Sequential(
            nn.Linear(flat, 128), nn.ReLU(),
            nn.Linear(128, 64), nn.ReLU(),
            nn.Linear(64, 1),
        )

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        h = self.features(x).reshape(x.size(0), -1)
        return self.head(h).squeeze(-1)


def export_model(model: ZuptNet, path: Path) -> None:
    convs = []
    for conv in [m for m in model.features if isinstance(m, nn.Conv1d)]:
        convs.append({
            "in_channels": int(conv.in_channels),
            "out_channels": int(conv.out_channels),
            "kernel_size": int(conv.kernel_size[0]),
            "pool": 2,
            "weight": conv.weight.detach().cpu().numpy().reshape(-1).tolist(),
            "bias": conv.bias.detach().cpu().numpy().tolist(),
        })
    fcs = []
    for fc in [m for m in model.head if isinstance(m, nn.Linear)]:
        fcs.append({
            "in_features": int(fc.in_features),
            "out_features": int(fc.out_features),
            "weight": fc.weight.detach().cpu().numpy().reshape(-1).tolist(),
            "bias": fc.bias.detach().cpu().numpy().tolist(),
        })
    payload = {
        "window_size": model.window,
        "channels": model.channels,
        "convs": convs,
        "fcs": fcs,
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", default="papers/nn_zupt/data/kitti_nn_zupt_dataset.npz")
    parser.add_argument("--output", default="papers/nn_zupt/weights/nn_zupt_kitti.json")
    parser.add_argument("--epochs", type=int, default=60)
    parser.add_argument("--batch-size", type=int, default=1024)
    parser.add_argument("--lr", type=float, default=5e-5)
    args = parser.parse_args()

    data = np.load(args.dataset)
    w, c = int(data["window"]), int(data["channels"])

    def pack(x, y):
        xt = torch.from_numpy(x.reshape(-1, w, c)).transpose(1, 2)
        yt = torch.from_numpy(y.astype(np.float32))
        return TensorDataset(xt, yt)

    loader = DataLoader(pack(data["train_x"], data["train_y"]),
                        batch_size=args.batch_size, shuffle=True)
    val_x = torch.from_numpy(data["val_x"].reshape(-1, w, c)).transpose(1, 2)
    val_y = torch.from_numpy(data["val_y"].astype(np.float32))

    pos = float(data["train_y"].sum())
    neg = float(len(data["train_y"]) - pos)
    pos_weight = torch.tensor([neg / max(pos, 1.0)])

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = ZuptNet(w, c).to(device)
    opt = torch.optim.Adam(model.parameters(), lr=args.lr)
    loss_fn = nn.BCEWithLogitsLoss(pos_weight=pos_weight.to(device))

    best = 1e9
    for epoch in range(1, args.epochs + 1):
        model.train()
        loss_sum = 0.0
        n = 0
        for xb, yb in loader:
            xb, yb = xb.to(device), yb.to(device)
            opt.zero_grad()
            logits = model(xb)
            loss = loss_fn(logits, yb)
            loss.backward()
            opt.step()
            loss_sum += float(loss.item()) * xb.size(0)
            n += xb.size(0)
        model.eval()
        with torch.no_grad():
            logits = model(val_x.to(device))
            vloss = float(loss_fn(logits, val_y.to(device)).item())
            prob = torch.sigmoid(logits)
            pred = (prob >= 0.5).float()
            acc = float((pred == val_y.to(device)).float().mean().item())
            prec = float((pred[val_y.to(device) == 1].sum() / max(pred.sum(), 1)).item())
            rec = float((pred[val_y.to(device) == 1].sum() / max(val_y.sum(), 1)).item())
        if vloss < best:
            best = vloss
            export_model(model, Path(args.output))
        if epoch == 1 or epoch % 10 == 0 or epoch == args.epochs:
            print(f"epoch {epoch:3d} train={loss_sum/max(n,1):.4f} val={vloss:.4f} "
                  f"acc={acc:.3f} prec={prec:.3f} rec={rec:.3f}")
    print(f"[done] {args.output} best_val={best:.4f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
