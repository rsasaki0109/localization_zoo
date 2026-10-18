from pathlib import Path
import json
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo')
# Do not edit code until the preceding build has copied its pinned executable.
assert (r/'3541f773_binary.json').exists()
p=repo/'papers/inten_loam/CMakeLists.txt';s=p.read_text();needle='''  target_link_libraries(inten_loam PUBLIC OpenMP::OpenMP_CXX)
''';assert needle in s;s=s.replace(needle,needle+'''  include(CheckCXXSymbolExists)
  include(CMakePushCheckState)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES OpenMP::OpenMP_CXX)
  check_cxx_symbol_exists(omp_pause_resource_all "omp.h" LZ_HAS_OMP_PAUSE_RESOURCE_ALL)
  cmake_pop_check_state()
  if(LZ_HAS_OMP_PAUSE_RESOURCE_ALL)
    target_compile_definitions(inten_loam PRIVATE LZ_HAS_OMP_PAUSE_RESOURCE_ALL=1)
  endif()
''');p.write_text(s)
p=repo/'papers/inten_loam/src/inten_loam_paper.cpp';s=p.read_text();needle='#include <numeric>\n';assert needle in s;s=s.replace(needle,needle+'''#ifdef LZ_HAS_OMP_PAUSE_RESOURCE_ALL
#include <omp.h>
#endif
''');needle='using Tree = detail::FeatureTree;\n';assert needle in s;s=s.replace(needle,needle+'''
// A quota-limited CPU should run the serial feature/map stages rather than
// keep idle workers spinning. Soft pause is optional in the runtime; it does
// not change the team size, query order or numerical reductions.
void pauseWorkers(bool accelerated) {
#ifdef LZ_HAS_OMP_PAUSE_RESOURCE_ALL
  if (accelerated && !omp_in_parallel()) omp_pause_resource_all(omp_pause_soft);
#else
  (void)accelerated;
#endif
}

struct WorkerPause {
  bool enabled;
  ~WorkerPause() { pauseWorkers(enabled); }
};
''');needle='''  // Angle-based clustering of the non-ground pixels.
''';assert needle in s;s=s.replace(needle,'''  pauseWorkers(params_.accelerated);

'''+needle,1);needle='''  // Intensity block thresholds (median per block).
''';assert needle in s;s=s.replace(needle,'''  pauseWorkers(params_.accelerated);

'''+needle,1)
needle='''int InTenLoamPaper::solveS2S(const PaperFrame& curr, Eigen::Matrix4d* T, ClassSystems* first) const {
''';assert needle in s;s=s.replace(needle,needle+'''  const WorkerPause pause{params_.accelerated};
''');needle='''                             ClassSystems* first) const {
''';assert needle in s;s=s.replace(needle,needle+'''  const WorkerPause pause{params_.accelerated};
''',1)
needle='''    return ix;
''';assert needle in s;s=s.replace(needle,'''    pauseWorkers(params_.accelerated);
'''+needle,1);needle='''    if (params_.use_intensity) map_->local_intensity.build''';assert needle in s;s=s.replace(needle,'''    pauseWorkers(params_.accelerated);
'''+needle,1)
p.write_text(s)
print('Applied optional runtime soft-pause candidate; build and test still required')
