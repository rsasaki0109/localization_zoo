#!/usr/bin/env python3
"""Smoke-check the Python bindings on a synthetic box-world scan.

Builds a random structured point cloud, applies a known rigid transform, and
verifies that each method recovers it. No dataset required.

Usage:
    cmake -B build -DBUILD_PYTHON_BINDINGS=ON
    cmake --build build -j --target localization_zoo_py
    python3 python/example_synthetic.py
"""
import os
import sys

import numpy as np

sys.path.insert(
    0, os.path.join(os.path.dirname(__file__), "..", "build", "python")
)
import localization_zoo as lz  # noqa: E402


def make_world(rng, n=4000):
    """Points on the walls and floor of a 20 m box corridor."""
    pts = []
    for _ in range(n):
        face = rng.integers(0, 3)
        u, v = rng.uniform(0, 20), rng.uniform(0, 4)
        if face == 0:
            pts.append([u, 0.0, v])
        elif face == 1:
            pts.append([u, 6.0, v])
        else:
            pts.append([u, rng.uniform(0, 6), 0.0])
    return np.asarray(pts) + rng.normal(scale=0.01, size=(n, 3))


def transform(points, T):
    return points @ T[:3, :3].T + T[:3, 3]


def main():
    rng = np.random.default_rng(0)
    target = make_world(rng)

    yaw = np.deg2rad(2.0)
    T_true = np.eye(4)
    T_true[:3, :3] = np.array(
        [
            [np.cos(yaw), -np.sin(yaw), 0.0],
            [np.sin(yaw), np.cos(yaw), 0.0],
            [0.0, 0.0, 1.0],
        ]
    )
    T_true[:3, 3] = [0.3, 0.1, 0.05]
    source = transform(target, np.linalg.inv(T_true))

    litamin2_params = lz.Litamin2Params()
    litamin2_params.voxel_resolution = 0.5  # default 3.0 m is too coarse here

    failures = []
    for name, reg in [
        ("NDT", lz.Ndt()),
        ("GICP", lz.Gicp()),
        ("LiTAMIN2", lz.Litamin2(litamin2_params)),
    ]:
        reg.set_target(target)
        result = reg.align(source)
        err = np.linalg.norm(result.transformation[:3, 3] - T_true[:3, 3])
        status = "OK" if err < 0.1 else "FAIL"
        if status == "FAIL":
            failures.append(name)
        print(f"{name:<10} converged={result.converged} t_err={err:.4f} m  {status}")

    # scan2 = inv(T_true) * world, so the sensor pose after frame 2 is T_true
    odom = lz.KissICP()
    odom.register_frame(target)
    odom.register_frame(source)
    err = np.linalg.norm(odom.pose[:3, 3] - T_true[:3, 3])
    status = "OK" if err < 0.1 else "FAIL"
    if status == "FAIL":
        failures.append("KISS-ICP")
    print(f"{'KISS-ICP':<10} pose t_err={err:.4f} m  {status}")

    if failures:
        raise SystemExit(f"bindings smoke check failed: {', '.join(failures)}")
    print("all bindings OK")


if __name__ == "__main__":
    main()
