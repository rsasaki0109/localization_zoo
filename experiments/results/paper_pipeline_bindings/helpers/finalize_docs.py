from pathlib import Path
import json,subprocess
repo=Path('/workspace/localization_zoo');e=repo/'experiments/results/paper_pipeline_bindings'
def committed(p):
 assert subprocess.check_output(['git','show','HEAD:'+str(p.relative_to(repo))],cwd=repo)==p.read_bytes()
ros=e/'ci/13984552_ros_reliable/report.json';moving=e/'local/cb6a2fb0_python_moving/paper_python_fixture.json'
committed(ros);committed(moving);data=json.loads(ros.read_text());extra=json.loads(moving.read_text())['methods']['dilo_synthetic'];assert extra['nonidentity_native_output'] and extra['diagnostics_equal']
names={'suma_paper':'SuMa','imls_slam_paper':'IMLS-SLAM','dilo_paper':'DiLO','litamin2_paper':'LiTAMIN2'}
rows=['| Native paper path | Scans | Maximum odometry/C++ matrix difference | Maximum TF/C++ matrix difference |','|---|---:|---:|---:|']
for method,v in data['methods'].items():
 rows.append(f"| {names[method]} | {v['frames']} | {v['max_abs_odom_delta']:.3g} | {v['max_abs_tf_delta']:.3g} |")
p=e/'README.md';text=p.read_text();a='No parameter was selected or retuned using these scans.'
text=text.replace(a,a+f''' A separate moving-room
fixture uses the same native DiLO defaults and the sensor beam layout from its
existing mechanism test. It produces non-identity poses and a maximum Python/C++
pose-element difference of {extra['max_abs_pose_delta']:.1f}, with equal native diagnostics.
[Committed moving-fixture report](local/cb6a2fb0_python_moving/paper_python_fixture.json),
commands and source guard were recorded in `9175ef7c`. The additional ROS case
checks these moving outputs too, using the same native configuration.
''')
marker='## ROS validation protocol and retained failure\n';block='''## ROS/native verification

Reliable-input execution at `1398455265a4bcaf3b7683ff77664d998f1754c3`
completed every real-fixture case. Its [report](ci/13984552_ros_reliable/report.json),
[command log](ci/13984552_ros_reliable/job.log) and
[receipt](ci/13984552_ros_reliable/receipt.json) were committed in `ef6115c5`
before the following observations. Both messages preserve the expected frame
names and scan timestamps. The reconstructed matrices include quaternion-to-matrix
rounding; the comparison tolerance is specified by the test.

'''+ '\n'.join(rows)+'''

These are interface parity measurements on one short fixture. DiLO's real-fixture
reference is identity as described above. The additional moving-room case guards
against returning a constant pose, with the same native defaults.

'''
assert marker in text;text=text.replace(marker,block+marker)
old='''The revised protocol explicitly selects reliable input and waits for input,
odometry and TF endpoints before replay. The new delivery option stays opt-in;
normal sensor subscribers retain best-effort QoS. Scans, native registration
parameters and comparison tolerances are unchanged. The complete revised ROS
verification is pending; partial results above do not close the check.'''
new='''The reliable-input execution above completes every real-fixture case. The new
delivery option stays opt-in; normal sensor subscribers retain best-effort QoS.
Scans, native registration parameters and comparison tolerances are unchanged.
The final test also waits for input, odometry and TF endpoints before replay.
The intervening observer implementation at `021534d5` used a subscription API
absent from ROS 2 Humble and failed before any method comparison; its
[failed job and receipt](ci/021534d5_ros_humble_api_failed/) are retained. The
observer now uses Humble's supported node graph API. The final CI check covers
that observer and the additional moving-room case; the numerical table records
only the already committed execution identified above.'''
assert old in text;text=text.replace(old,new);p.write_text(text)
p=repo/'docs/roadmap.md';text=p.read_text();a=text.index('7. **Bindings.**');b=text.index('\n### Phase 3:',a)
text=text[:a]+'''7. **Completed: paper-pipeline bindings and ROS 2 nodes.** Python now exposes
   SuMa frame-to-model, IMLS-SLAM raw-scan model, DiLO range-image and LiTAMIN2
   Gaussian-map odometry, alongside the existing registration bindings.
   - [Python examples](../python/README.md#stateful-paper-pipelines) and
     [ROS 2 examples](../ros2/localization_zoo_ros/README.md#paper-odometry-nodes)
     cover each method. Separate node/class names preserve existing interfaces.
   - Native defaults and opt-in correction defaults are preserved; no sensor
     profile or ground-truth seed is silently added. ROS output uses acquisition
     stamps and the stored scan frame; the input QoS is configurable explicitly.
   - Fixture checks compare Python diagnostics/poses and real ROS odometry/TF
     with native C++. The identity DiLO result on the real fixture and the
     incomplete best-effort ROS trial remain documented; a moving synthetic
     fixture also exercises non-identity DiLO output with the same defaults.
   - [Commands, committed parity reports, failures and scope](../experiments/results/paper_pipeline_bindings/README.md).
''' +text[b:];p.write_text(text)
for method,observed in [('suma',0),('imls_slam',2)]:
 p=repo/'papers'/method/'README.md';text=p.read_text();marker='[Native fixture checks](../../experiments/results/paper_pipeline_bindings/README.md).\n';assert marker in text
 note=f'''\nThe initial ROS interface trial observed {observed}/3 odometry messages before its
per-scan deadline with best-effort input. The recorded reliable-input trial
completed 3/3 and matched the native pipeline; commands and both trials are
linked above. This short interface check retains the benchmark evidence below.\n''';p.write_text(text.replace(marker,marker+note,1))
p=repo/'papers/dilo/README.md';text=p.read_text();marker='[Native fixture checks](../../experiments/results/paper_pipeline_bindings/README.md).\n';note='''\nThe native-default interface fixture returns identity poses for all 3 scans.
Python reproduces those poses and window state. A separate moving-room fixture
keeps the same native defaults and verifies non-identity Python/C++ output;
the ROS test includes it too. Reports and the fixture's limits are linked above.\n''';p.write_text(text.replace(marker,marker+note,1))
print('Rendered documentation from committed native, moving-room and ROS records.')
