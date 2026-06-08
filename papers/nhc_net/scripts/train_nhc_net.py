#!/usr/bin/env python3
"""Train VMSC CNN + adaptive NHC head; export JSON for C++."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import numpy as np
import torch
import torch.nn as nn
from torch.utils.data import DataLoader, TensorDataset


class VmscNet(nn.Module):
    def __init__(self, window: int = 50, channels: int = 6, num_classes: int = 4) -> None:
        super().__init__()
        self.window = window
        self.channels = channels
        self.num_classes = num_classes
        self.nhc_scale = 2.0

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
        self.shared = nn.Sequential(
            nn.Linear(flat, 128), nn.ReLU(),
            nn.Linear(128, 64), nn.ReLU(),
        )
        self.class_head = nn.Linear(64, num_classes)
        self.nhc_head = nn.Linear(64, 2)

    def forward(self, x: torch.Tensor) -> tuple[torch.Tensor, torch.Tensor]:
        h = self.features(x).reshape(x.size(0), -1)
        h = self.shared(h)
        return self.class_head(h), self.nhc_head(h)


def export_model(model: VmscNet, path: Path) -> None:
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
    shared_fcs = []
    for fc in [m for m in model.shared if isinstance(m, nn.Linear)]:
        shared_fcs.append({
            "in_features": int(fc.in_features),
            "out_features": int(fc.out_features),
            "weight": fc.weight.detach().cpu().numpy().reshape(-1).tolist(),
            "bias": fc.bias.detach().cpu().numpy().tolist(),
        })

    def head_dict(fc: nn.Linear) -> dict:
        return {
            "in_features": int(fc.in_features),
            "out_features": int(fc.out_features),
            "weight": fc.weight.detach().cpu().numpy().reshape(-1).tolist(),
            "bias": fc.bias.detach().cpu().numpy().tolist(),
        }

    payload = {
        "window_size": model.window,
        "channels": model.channels,
        "num_classes": model.num_classes,
        "nhc_scale": model.nhc_scale,
        "convs": convs,
        "shared_fcs": shared_fcs,
        "class_head": head_dict(model.class_head),
        "nhc_head": head_dict(model.nhc_head),
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload) + "\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", default="papers/nhc_net/data/kitti_nhc_net_dataset.npz")
    parser.add_argument("--output", default="papers/nhc_net/weights/nhc_net_kitti.json")
    parser.add_argument("--epochs", type=int, default=60)
    parser.add_argument("--batch-size", type=int, default=1024)
    parser.add_argument("--lr", type=float, default=5e-5)
    args = parser.parse_args()

    data = np.load(args.dataset)
    w, c, k = int(data["window"]), int(data["channels"]), int(data["num_classes"])

    def pack(x, yc, yn):
        xt = torch.from_numpy(x.reshape(-1, w, c)).transpose(1, 2)
        yct = torch.from_numpy(yc)
        ynt = torch.from_numpy(yn / 2.0)
        return TensorDataset(xt, yct, ynt)

    loader = DataLoader(pack(data["train_x"], data["train_cls"], data["train_nhc"]),
                        batch_size=args.batch_size, shuffle=True)
    val_x = torch.from_numpy(data["val_x"].reshape(-1, w, c)).transpose(1, 2)
    val_cls = torch.from_numpy(data["val_cls"])
    val_nhc = torch.from_numpy(data["val_nhc"] / 2.0)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = VmscNet(w, c, k).to(device)
    opt = torch.optim.Adam(model.parameters(), lr=args.lr)
    ce = nn.CrossEntropyLoss()
    mse = nn.MSELoss()

    best = 1e9
    for epoch in range(1, args.epochs + 1):
        model.train()
        loss_sum = 0.0
        n = 0
        for xb, ycb, ynb in loader:
            xb, ycb, ynb = xb.to(device), ycb.to(device), ynb.to(device)
            opt.zero_grad()
            logits, nhc = model(xb)
            loss = ce(logits, ycb) + mse(nhc, ynb)
            loss.backward()
            opt.step()
            loss_sum += float(loss.item()) * xb.size(0)
            n += xb.size(0)
        model.eval()
        with torch.no_grad():
            logits, nhc = model(val_x.to(device))
            vloss = float((ce(logits, val_cls.to(device)) + mse(nhc, val_nhc.to(device))).item())
            acc = float((logits.argmax(1) == val_cls.to(device)).float().mean().item())
            nhc_mae = float((torch.abs(nhc * 2.0 - val_nhc.to(device) * 2.0)).mean().item())
        if vloss < best:
            best = vloss
            export_model(model, Path(args.output))
        if epoch == 1 or epoch % 10 == 0 or epoch == args.epochs:
            print(f"epoch {epoch:3d} train={loss_sum/max(n,1):.4f} val={vloss:.4f} acc={acc:.3f} nhc_mae={nhc_mae:.3f} m/s")
    print(f"[done] {args.output} best_val={best:.4f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
