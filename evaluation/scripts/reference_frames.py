"""Explicit reference-frame conversions; never infer calibration from scores."""
import json
from pathlib import Path

import numpy as np


def load_sensor_to_body(path: Path) -> np.ndarray:
    payload = json.loads(path.read_text())
    matrix = np.asarray(payload['T_body_sensor'], dtype=float)
    if matrix.shape != (4, 4) or not np.isfinite(matrix).all():
        raise ValueError('T_body_sensor must be a finite 4x4 matrix')
    if not np.allclose(matrix[3], [0, 0, 0, 1], rtol=0, atol=1e-10):
        raise ValueError('T_body_sensor must have homogeneous bottom row [0,0,0,1]')
    rotation = matrix[:3, :3]
    if (not np.allclose(rotation.T @ rotation, np.eye(3), rtol=0, atol=1e-6)
            or abs(np.linalg.det(rotation)-1) > 1e-6):
        raise ValueError('T_body_sensor rotation must be proper orthonormal')
    return matrix


def sensor_frame_poses(body_poses, sensor_to_body):
    """T_world_sensor = T_world_body * T_body_sensor."""
    return [pose @ sensor_to_body for pose in body_poses]
