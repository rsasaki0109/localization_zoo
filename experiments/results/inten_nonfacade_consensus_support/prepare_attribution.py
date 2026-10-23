"""Construct a passive same-library query probe, preserving the rejected guard."""
from common import *
def main():
    source=(E.parent/'inten_consensus_query_acceleration/parity_probe.cpp').read_text()
    start=source.index('    while(std::cin>>frame) {');end=source.index('    if(!std::cin.eof())',start)
    replacement='''    while(std::cin>>frame) {
      Eigen::Matrix4d seed;
      for(int k=0;k<16;++k)if(!(std::cin>>seed(k/4,k%4)))throw std::runtime_error("Invalid estimated query pose");
      const auto& previous=frames.at(frame-1);const auto& current=frames.at(frame);
      InTenLoamPaperParams params;params.s2m_plane_max_dist=.05;params.accelerated=true;
      params.s2s_mapped_prior=true;params.s2s_translation_prior_ratio=.03;
      params.s2s_ground_pca=params.s2s_facade_pca=true;params.use_mapping=false;
      InTenLoamPaper core(params);support::enabled=false;support::observing=false;
      replay::frame=&previous;replay::seed=nullptr;core.process({},{});
      replay::frame=&current;replay::seed=&seed;support::frame=frame;
      support::begin(seed,seed);
      InTenLoamPaperTrace trace;const auto result=core.process({},{},nullptr,&trace);
      support::finish();support::report.query=trace.s2s;
      if(trace.s2s.input!=trace.s2s.output)throw std::runtime_error("Passive query changed pose");
      auto out=serial::record();out<<"{\\\"frame\\\":"<<frame<<",\\\"support\\\":";
      support::emit(out);out<<'}';std::cout<<out.str()<<'\\n'<<std::flush;
      if(!std::cout)throw std::runtime_error("Query output failed");
    }
'''
    source=source[:start]+replacement+source[end:]
    source=source.replace('#include "replay_hooks.h"','#include "replay_hooks.h"\n#include "support_hooks.h"')
    source=source.replace('if(argc!=2)','if(argc!=3)').replace('std::ifstream input(argv[1]);','std::ifstream input(argv[1]);\n    std::ofstream evidence(argv[2]);if(!evidence)throw std::runtime_error("Cannot open query evidence");support::capture=&evidence;')
    source=source.replace('    return 0;','    evidence.close();if(evidence.fail())throw std::runtime_error("Query evidence close failed");\n    return 0;')
    (E/'query_probe.cpp').write_text(source)
    write(E/'attribution_protocol.json',dict(parent_numerical_commit='61ca47d2',purpose='Explain the already rejected fixed-support gate, not a new candidate or relaxed adoption gate. Observe native fresh associations at original joint and unchanged old refined poses. Same object/library/tree/parameters, no optimizer/prior/GT and no altered accepted output.',cases='ALL31 inherited endpoints, BOTH joint/refined query roles, including fallback ties. Do not select only favorable examples.',checks='Original fresh-query class costs/counts must be bitwise the existing original-query guard observation wherever it ran. Every query pose bitwise unchanged, no optimizer step/prior. Reconstruct every H/g/class count/Huber cost from primitives. Re-associated costs have changed supports/counts; never compare their raw sums as an accuracy winner. Report per-class counts, mean losses, common frozen costs, all outcomes.',limits='Associations at different poses are not physical object identities. An improved fresh-query objective is not proof of correct motion. GT remains offline only; this attribution cannot authorize promotion or a full recursive run. Original gate remains failed without amendment.',source_sha256={str(p.relative_to(W)):sha(p) for p in [E/'query_probe.cpp',E/'diagnostic_native.cpp',E/'support_hooks.h',E/'run_attribution.py',E/'common.py']}))
if __name__=='__main__':main()
