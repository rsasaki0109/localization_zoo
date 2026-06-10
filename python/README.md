# Python bindings (experimental)

A thin pybind11 layer over a representative subset of the C++ methods:

| Python class | C++ class | Style |
|---|---|---|
| `KissICP` | `kiss_icp::KISSICPPipeline` | odometry (`register_frame`) |
| `Litamin2` | `litamin2::LiTAMIN2Registration` | scan-to-target (`set_target` / `align`) |
| `Ndt` | `ndt::NDTRegistration` | scan-to-target (`set_target` / `align`) |
| `Gicp` | `gicp::GICPRegistration` | scan-to-target (`set_target` / `align`) |

Point clouds are Nx3 `float64` numpy arrays; poses and initial guesses are
4x4 numpy arrays.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_PYTHON_BINDINGS=ON
cmake --build build -j"$(nproc)" --target localization_zoo_py
```

pybind11 is found via `find_package` when installed, otherwise fetched
automatically at configure time. The module is written to
`build/python/localization_zoo.cpython-*.so`.

## Use

```python
import sys
sys.path.insert(0, "build/python")

import numpy as np
import localization_zoo as lz

# Odometry: feed scans, read the accumulated pose
odom = lz.KissICP()
for scan in scans:                      # scan: Nx3 float64 array
    result = odom.register_frame(scan)
print(odom.pose)                        # 4x4 world-frame pose

# Pairwise registration
reg = lz.Ndt()
reg.set_target(target_points)
result = reg.align(source_points, initial_guess=np.eye(4))
print(result.transformation, result.converged)
```

Parameters mirror the C++ structs:

```python
params = lz.KissICPParams()
params.voxel_size = 0.4
odom = lz.KissICP(params)
```

A runnable end-to-end check lives in
[`example_synthetic.py`](example_synthetic.py) (no dataset needed):

```bash
python3 python/example_synthetic.py
```
