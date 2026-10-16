# Python bindings (experimental)

A thin pybind11 layer over a representative subset of the C++ methods:

| Python class | C++ class | Style |
|---|---|---|
| `KissICP` | `kiss_icp::KISSICPPipeline` | odometry (`register_frame`) |
| `Litamin2` | `litamin2::LiTAMIN2Registration` | scan-to-target (`set_target` / `align`) |
| `Ndt` | `ndt::NDTRegistration` | scan-to-target (`set_target` / `align`) |
| `Gicp` | `gicp::GICPRegistration` | scan-to-target (`set_target` / `align`) |
| `SumaPaper` | `suma::SuMaPaper` | frame-to-model paper odometry (`process`) |
| `ImlsPaperOdometry` | `imls_slam::ImlsPaperOdometry` | raw-scan sliding-model paper odometry (`process`) |
| `DiloPaperOdometry` | `dilo::DiloPaperOdometry` | range-image paper odometry (`process`) |
| `Litamin2Odometry` | `litamin2::LiTAMIN2Odometry` | Gaussian-map paper odometry (`process`) |

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

## Stateful paper pipelines

Feed scans in acquisition order, in the stored sensor frame. Each object owns
its map/window and predicts motion without a ground-truth seed. Construct a new
object for a new sequence. `Litamin2Odometry` is separate from the existing
scan-to-target `Litamin2` class. SuMa exposes frame-to-model odometry, without
loop closure.

```python
pipelines = {
    "suma": lz.SumaPaper(),
    "imls_slam": lz.ImlsPaperOdometry(),
    "dilo": lz.DiloPaperOdometry(),
    "litamin2": lz.Litamin2Odometry(),
}
for scan in scans:
    for name, pipeline in pipelines.items():
        result = pipeline.process(scan)
        pose = result if name == "dilo" else result.pose
        print(name, pose)  # world <- stored scan frame
```

`pose` returns a copy of the accumulated pose. DiLO also exposes copies of
`window_poses` and `finalized_poses`; its `process` result is the causal pose of
the new scan, while older poses can be refined within the window. Other result
objects expose the native registration/map diagnostics. Empty or nonfinite
arrays raise `ValueError` before advancing these new pipelines.

Parameter classes are `SumaPaperParams`, `ImlsPaperParams`, `DiloPaperParams`
and `Litamin2OdometryParams`. They mirror the native C++ fields and defaults;
pass the parameters at construction. The LiTAMIN2 field `lambda` is named
`lambda_` in Python; `map_mode` uses `Litamin2MapMode.AGGREGATE`, `FIRST` or
`LATEST`. Existing opt-in corrections stay disabled by default, including
SuMa `normalize_bilinear`, IMLS `coarse_frames` and DiLO `degenerate_ratio`.

Run each example on the committed point-cloud fixture, or replace the scan
directory with stored PCD/KITTI XYZI scans:

```bash
python3 python/example_paper_odometry.py suma_paper
python3 python/example_paper_odometry.py imls_slam_paper
python3 python/example_paper_odometry.py dilo_paper
python3 python/example_paper_odometry.py litamin2_paper
cmake --build build --target paper_odometry_reference
ctest --test-dir build -R test_paper_python_bindings --output-on-failure
```

The fixture test compares the Python poses and native diagnostics against the
same C++ paper pipelines, including non-contiguous NumPy inputs. These examples
use native defaults; reproducing a published harness variant also requires its
recorded sensor preprocessing and arguments. They do not replace the stored
benchmark results. [ROS 2 interfaces](../ros2/localization_zoo_ros/README.md).
