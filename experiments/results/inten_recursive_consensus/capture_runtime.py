"""Record the actual native container without reading credentials or environment."""
from pathlib import Path
import json
import os
import platform
import subprocess


cpu = next(line.split(':', 1)[1].strip() for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name'))
memory = next(int(line.split()[1])/1024**2 for line in Path('/proc/meminfo').read_text().splitlines() if line.startswith('MemTotal:'))
release = dict(line.split('=', 1) for line in Path('/etc/os-release').read_text().splitlines() if '=' in line)
result = dict(cpu=cpu, logical_cores=os.cpu_count(), affinity_cores=len(os.sched_getaffinity(0)),
    memory_gib=round(memory, 1), os=release['PRETTY_NAME'].strip('"'), kernel=platform.release(),
    architecture=platform.machine(), thread_environment={'OMP_NUM_THREADS': '1', 'OPENBLAS_NUM_THREADS': '1'},
    capture_scope='Shared native container before fixed recursive-consensus controls, before all fixed full paired trials; no standalone FPS claim.',
    source_commit=subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip(),
    versions={key: subprocess.check_output(command, text=True).splitlines()[0] for key, command in
              {'compiler': ['c++', '--version'], 'python': ['python3', '--version']}.items()})
print(json.dumps(result, indent=2))
