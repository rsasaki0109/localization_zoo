#!/usr/bin/env python3
"""Single fixed upstream run, with GT intentionally absent from the interface."""
import argparse
import ctypes
import importlib.metadata
import json
from pathlib import Path
import sys
import time
from receipt import save, sha


def main():
    p = argparse.ArgumentParser(__doc__)
    p.add_argument('--package-root', type=Path, required=True)
    p.add_argument('--source-root', type=Path, required=True)
    p.add_argument('--scans', type=Path, required=True)
    p.add_argument('--preprocess-library', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    sys.path.insert(0, str(a.package_root))
    import numpy as np
    from kiss_icp.config import load_config
    from kiss_icp.kiss_icp import KissICP
    assert importlib.metadata.version('kiss-icp') == '1.3.0'
    pins_path = a.source_root/'experiments/results/inten_facade_pca/sequences/01/inputs.json'
    pins = json.loads(pins_path.read_text())['files']
    scans = sorted(a.scans.glob('*.bin'))
    assert [s.name for s in scans] == [f'{i:06d}.bin' for i in range(1101)]
    a.output.mkdir(parents=True, exist_ok=False)
    native = ctypes.CDLL(str(a.preprocess_library)).preprocess
    native.argtypes = [ctypes.POINTER(ctypes.c_float), ctypes.c_size_t]
    native.restype = ctypes.c_size_t
    config = load_config(None)
    assert config.mapping.voxel_size == 1.0
    config.data.max_range = 120.0
    config.data.deskew = False
    config.registration.max_num_threads = 1
    save(a.output/'environment.json', {'config':config.model_dump(),
        'package_version':importlib.metadata.version('kiss-icp'),
        'numpy_version':np.__version__, 'input_manifest_sha256':sha(pins_path),
        'preprocess_library_sha256':sha(a.preprocess_library),
        'package_files':{str(f.relative_to(a.package_root)):sha(f)
                         for folder in ['kiss_icp','kiss_icp-1.3.0.dist-info']
                         for f in sorted((a.package_root/folder).rglob('*'))
                         if f.is_file() and '__pycache__' not in f.parts}})
    odometry = KissICP(config)
    with (a.output/'poses.txt').open('w') as poses, (a.output/'frames.jsonl').open('w') as frames:
        for i, scan in enumerate(scans):
            raw_sha = sha(scan)
            assert raw_sha == pins[scan.name]['sha256'] and scan.stat().st_size == pins[scan.name]['bytes']
            raw = np.fromfile(scan, dtype='<f4').reshape(-1,4)
            count = native(raw.ctypes.data_as(ctypes.POINTER(ctypes.c_float)), len(raw))
            points = np.ascontiguousarray(raw[:count,:3], dtype=np.float64)
            start = time.monotonic()
            _, source = odometry.register_frame(points, np.zeros(0))
            elapsed = time.monotonic()-start
            assert np.isfinite(odometry.last_pose).all()
            poses.write(' '.join(format(x, '.17g') for x in odometry.last_pose[:3].flat)+'\n')
            frames.write(json.dumps({'frame':i,'raw_sha256':raw_sha,'raw_points':len(raw),
                'input_points':count,'input_xyz_sha256':__import__('hashlib').sha256(points.tobytes()).hexdigest(),
                'registration_points':len(source),'seconds':elapsed}, allow_nan=False)+'\n')
            if i % 100 == 0:
                poses.flush(); frames.flush(); print(f'frame {i}', flush=True)
    save(a.output/'completion.json', {'frames':len(scans),'exit_code':0,
        'poses_sha256':sha(a.output/'poses.txt'), 'frames_sha256':sha(a.output/'frames.jsonl')})


if __name__ == '__main__':
    main()
