"""A ray-cast room using the beam layout of the existing DiLO mechanism test."""
import numpy as np


def moving_room_scans():
    # Sensor geometry comes from papers/dilo/test/test_dilo_paper.cpp::scanAt.
    # This exercises moving output with the native defaults; no method is tuned.
    elevation = np.deg2rad(2 - np.arange(64) * 26.8 / 63)[:, None]
    azimuth = (-np.pi + 2 * np.pi * np.arange(2000) / 2000)[None, :]
    directions = np.stack([
        np.cos(elevation) * np.cos(azimuth),
        np.cos(elevation) * np.sin(azimuth),
        np.broadcast_to(np.sin(elevation), (64, 2000)),
    ], axis=-1).reshape(-1, 3)
    scans = []
    for frame in range(3):
        yaw = 0.02 * frame
        rotation = np.array([[np.cos(yaw), -np.sin(yaw), 0],
                             [np.sin(yaw), np.cos(yaw), 0], [0, 0, 1]])
        origin = np.array([0.3 * frame, 0.1 * frame, 0])
        rays = directions @ rotation.T
        nearest = np.full(len(rays), np.inf)
        for axis, offset in [(2, 1.7), (1, -8), (1, 8), (0, -40), (0, 40)]:
            denominator = rays[:, axis]
            distance = np.divide(-(origin[axis] + offset), denominator,
                                 out=np.full(len(rays), np.inf),
                                 where=np.abs(denominator) > 1e-9)
            nearest = np.minimum(nearest, np.where(distance > 0.5, distance, np.inf))
        valid = nearest < 80
        # Match stored lidar precision so float32 and float64 ROS messages
        # deliver exactly the same coordinates to the native reference.
        points = (directions[valid] * nearest[valid, None]).astype(np.float32).astype(np.float64)
        scans.append((f'synthetic_room_{frame}', np.ascontiguousarray(points)))
    return scans
