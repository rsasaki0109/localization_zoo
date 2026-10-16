# Native paper odometry interfaces

Python and ROS 2 interfaces expose the existing SuMa frame-to-model,
IMLS-SLAM raw-scan model, DiLO range-image and LiTAMIN2 Gaussian-map pipelines.
They preserve native parameter defaults and causal state without ground-truth
seeding or wrapper-side pose accumulation. This is an interface check on the
committed MCD fixture; accuracy conclusions use the existing benchmark evidence.

## Local native/Python verification

Implementation `98c2f56f16397f73c7fe93bc566ac0c3538747db`, based on
`134ead2f21a4611ec328a45aa543b31428d6b92c`. Raw executions were committed in
`502e5c8a` before rendering the table. [Commands and exit codes](local/98c2f56f/commands.json),
[fixture report](local/98c2f56f/paper_python_fixture.json),
[input/module/reference hashes](local/98c2f56f/metadata.json).

| Native paper path | Scans | Maximum Python/C++ pose-element difference | Native diagnostics |
|---|---:|---:|---|
| SuMa | 3 | 0.0 | equal |
| IMLS-SLAM | 3 | 0.0 | equal |
| DiLO | 3 | 0.0 | equal |
| LiTAMIN2 | 3 | 0.0 | equal |

The complete CTest run passes, including the new Python/native fixture test.
The existing Python example and each paper example pass. The binding tests also
exercise Fortran/non-contiguous arrays, independent pose copies, rejected empty
or nonfinite scans, enum fields and the existing disabled opt-in defaults.
The oracle receives the same raw XYZ values and calls the native implementations
directly, with no CLI sensor profile or ground-truth initialization.

DiLO's native default returns identity poses throughout this fixture. Its Python
interface returns the same poses and window state. This fixture provides an API
parity check; motion recovery uses the separate mechanism and KITTI evaluations.
No parameter was selected or retuned using these scans. A separate moving-room
fixture uses the same native DiLO defaults and the sensor beam layout from its
existing mechanism test. It produces non-identity poses and a maximum Python/C++
pose-element difference of 0.0, with equal native diagnostics.
[Committed moving-fixture report](local/cb6a2fb0_python_moving/paper_python_fixture.json),
commands and source guard were recorded in `9175ef7c`. The additional ROS case
checks these moving outputs too, using the same native configuration.


## ROS/native verification

Reliable-input execution at `1398455265a4bcaf3b7683ff77664d998f1754c3`
completed every real-fixture case. Its [report](ci/13984552_ros_reliable/report.json),
[command log](ci/13984552_ros_reliable/job.log) and
[receipt](ci/13984552_ros_reliable/receipt.json) were committed in `ef6115c5`
before the following observations. Both messages preserve the expected frame
names and scan timestamps. The reconstructed matrices include quaternion-to-matrix
rounding; the comparison tolerance is specified by the test.

| Native paper path | Scans | Maximum odometry/C++ matrix difference | Maximum TF/C++ matrix difference |
|---|---:|---:|---:|
| SuMa | 3 | 1.11e-15 | 1.11e-15 |
| IMLS-SLAM | 3 | 4.44e-16 | 4.44e-16 |
| DiLO | 3 | 0 | 0 |
| LiTAMIN2 | 3 | 4.44e-16 | 4.44e-16 |

These are interface parity measurements on one short fixture. DiLO's real-fixture
reference is identity as described above. The additional moving-room case guards
against returning a constant pose, with the same native defaults.

## ROS validation protocol and retained failure

The ROS test publishes the same scans through real node processes and compares
both odometry and TF against a separately built native oracle. It checks world
and sensor frame names, acquisition stamps, scalar float coordinates with either
byte order, and rejected invalid or repeated headers.

The first ROS execution at `98c2f56f` failed with exit code 1. Its
[full job log](ci/98c2f56f_ros_failed/job.log),
[partial report](ci/98c2f56f_ros_failed/partial_report.json) and
[receipt](ci/98c2f56f_ros_failed/receipt.json) remain committed. Before its
per-scan deadline it observed 0/3 odometry outputs for SuMa and 2/3 for IMLS;
DiLO and LiTAMIN2 completed. That attempt used best-effort input and waited only
for the point-cloud input connection. A missing output could reflect input
delivery or an output publisher not yet matched to the observer.

The reliable-input execution above completes every real-fixture case. The new
delivery option stays opt-in; normal sensor subscribers retain best-effort QoS.
Scans, native registration parameters and comparison tolerances are unchanged.
The final test also waits for input, odometry and TF endpoints before replay.
The intervening observer implementation at `021534d5` used a subscription API
absent from ROS 2 Humble and failed before any method comparison; its
[failed job and receipt](ci/021534d5_ros_humble_api_failed/) are retained. The
observer now uses Humble's supported node graph API. The final CI check covers
that observer and the additional moving-room case; the numerical table records
only the already committed execution identified above.

## Reproduction and scope

[Python examples](../../../python/README.md#stateful-paper-pipelines) and
[ROS 2 examples](../../../ros2/localization_zoo_ros/README.md#paper-odometry-nodes)
show each method. Native parameter fields remain available to configure another
sensor; published harness variants still require their recorded preprocessing
and arguments. Each test method uses one unchanged configuration for every scan.

Local commands retain their executed container/workspace prefixes. Replace those
prefixes when replaying; the repository CMake options build the same native
libraries. ROS CI runs the command recorded in `.github/workflows/ci.yml` after
sourcing the installed ROS workspace. Raw scan files and executable binaries are
not added to this results directory.

Core algorithm sources, common code, runner loaders and stored benchmark
aggregates are unchanged. The new binding targets link existing cores; this work
does not change published paper numbers or enable robustness/sensor corrections.
The NCLT real-data sensor-assistant check remains pending separately because its
approved download host is still absent from the enforced environment allowlist.
