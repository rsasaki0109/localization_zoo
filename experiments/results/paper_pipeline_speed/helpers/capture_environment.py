from pathlib import Path
import json,os,subprocess,sys
sys.path.insert(0,'/workspace/localization_zoo/evaluation/scripts')
from capture_benchmark_environment import capture_host
h=capture_host()
h.update({'cpu_quota':Path('/sys/fs/cgroup/cpu.max').read_text().strip(),'memory_limit':Path('/sys/fs/cgroup/memory.max').read_text().strip(),'runtime_environment':{n:os.environ.get(n) for n in ['OMP_NUM_THREADS','OMP_DYNAMIC','OPENBLAS_NUM_THREADS']},'profiler_package_version':subprocess.check_output(['dpkg-query','-W','-f=${Version}','google-perftools'],text=True),'compiler_flags':{'build_type':'Release','cxx_flags':'','cxx_flags_release':'-O3 -DNDEBUG','OpenMP':'-fopenmp'},'base_commit':'134ead2f21a4611ec328a45aa543b31428d6b92c'})
Path('/workspace/localization-env/paper-speed/environment.json').write_text(json.dumps(h,indent=2)+'\n')
print(json.dumps(h,indent=2))
