from pathlib import Path
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo')
assert (r/'67867d0b_binary.json').exists(),'Wait until the v4 binary is pinned; do not change code during its build'
p=repo/'papers/inten_loam/CMakeLists.txt';s=p.read_text();a=s.index('  include(CheckCXXSymbolExists)');b=s.index('  endif()',a)+len('  endif()\n');s=s[:a]+s[b:];p.write_text(s)
p=repo/'papers/inten_loam/src/inten_loam_paper.cpp';s=p.read_text().replace('#ifdef LZ_HAS_OMP_PAUSE_RESOURCE_ALL\n#include <omp.h>\n#endif\n','');a=s.index('// A quota-limited CPU');b=s.index('Eigen::Matrix4d expSe3(',a);s=s[:a]+s[b:];s=s.replace('  pauseWorkers(params_.accelerated);\n','').replace('  const WorkerPause pause{params_.accelerated};\n','');p.write_text(s)
p=repo/'papers/inten_loam/README.md';s=p.read_text();a=s.index('Where the OpenMP runtime supports soft resource pause');b=s.index('\n\n',a);s=s[:a]+s[b+1:];needle='[Execution protocol and CPU profiles]';i=s.index(needle,s.index('## Optional execution acceleration'));s=s[:i]+'''A later soft-pause scheduling candidate (`fe03ed88`) took 26.305 s against
35.078 s for its frozen control (**1.33x**); the changed default took 32.961 s.
Saved poses still matched. It did not improve on the earlier screening result,
so the soft-pause calls are removed. These single-window timings vary with the
host; final speed ratios use a fresh full control immediately before each
accelerated run. The rejected candidate's raw screening was committed in
`21935329` under
[`candidates/fe03ed88/`](../../experiments/results/paper_pipeline_speed/candidates/fe03ed88/).

'''+s[i:];p.write_text(s)
print('Rejected soft-pause scheduling; numerical pipeline and native queries retained')
