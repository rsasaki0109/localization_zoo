// Cached extracted frames and original estimated seeds only. No GT reader.
#include "replay_hooks.h"
#include "support_hooks.h"
#include "inten_loam/detail/s2s_consensus.h"
#include "inten_trace_jsonl.h"
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
using namespace localization_zoo::inten_loam;
namespace serial=localization_zoo::evaluation::inten_trace;
int main(int argc,char** argv) {
  try {
    if(argc!=3) throw std::runtime_error("Expected cached feature file");
    std::ifstream input(argv[1]);
    std::ofstream evidence(argv[2]); if(!evidence)throw std::runtime_error("Cannot open common support evidence");
    support::capture=&evidence;
    if(!input) throw std::runtime_error("Cannot open cached features");
    std::map<int,PaperFrame> frames;
    int frame;
    while(input>>frame) {
      if(frames.count(frame)) throw std::runtime_error("Duplicate frame");
      auto& f=frames[frame];
      for(auto* features:{&f.edge,&f.ground,&f.facade,&f.reflector,&f.all}) {
        size_t count;
        if(!(input>>count)) throw std::runtime_error("Invalid feature count");
        for(size_t k=0;k<count;++k) {
          PaperFeature p;
          if(!(input>>p.p.x()>>p.p.y()>>p.p.z()>>p.row>>p.cluster>>p.intensity)) throw std::runtime_error("Invalid feature");
          features->push_back(p);
        }
      }
    }
    if(!input.eof()||frames.empty()) throw std::runtime_error("Invalid cached input");
    while(std::cin>>frame) {
      Eigen::Matrix4d seed;
      for(int k=0;k<16;++k) if(!(std::cin>>seed(k/4,k%4))) throw std::runtime_error("Invalid estimated seed");
      const auto& previous=frames.at(frame-1);const auto& current=frames.at(frame);
      const auto profiles=detail::clusterTranslationProfiles(previous.all,current,seed,true);
      for(const auto& p:profiles) {
        auto out=serial::record();out<<"{\"type\":\"profile\",\"frame\":"<<frame<<",\"cluster\":"<<p.cluster<<",\"costs_m2\":[";
        for(size_t k=0;k<p.costs.size();++k) {if(k)out<<',';out<<p.costs[k];}
        out<<"]}";std::cout<<out.str()<<'\n';
      }
      replay::profiles=&profiles;support::frame=frame;
      for(int mode=0;mode<3;++mode) {
        InTenLoamPaperParams params;params.s2m_plane_max_dist=.05;params.accelerated=true;
        params.s2s_mapped_prior=true;params.s2s_translation_prior_ratio=.03;
        params.s2s_ground_pca=params.s2s_facade_pca=true;params.use_mapping=false;
        params.s2s_geometry_consensus=mode!=0;
        support::enabled=mode==2;support::report=support::Report();
        InTenLoamPaper core(params);replay::frame=&previous;replay::seed=nullptr;core.process({},{});
        replay::frame=&current;replay::seed=&seed;
        InTenLoamPaperTrace trace;const auto result=core.process({},{},nullptr,&trace);
        auto out=serial::record();out<<"{\"type\":\"terminal\",\"frame\":"<<frame<<",\"mode\":"<<mode<<",\"trace\":"
          <<serial::frame(frame,"cached_estimated_seed_only",trace,result,result.pose);
        out<<",\"support\":";support::emit(out);out<<'}';
        std::cout<<out.str()<<'\n';
      }
      std::cout<<std::flush;if(!std::cout)throw std::runtime_error("Output failed");
    }
    if(!std::cin.eof())throw std::runtime_error("Invalid command");
    evidence.close();if(evidence.fail())throw std::runtime_error("Support evidence close failed");
    return 0;
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
