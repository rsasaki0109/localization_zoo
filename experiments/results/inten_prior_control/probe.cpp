// Cached extracted frames and original estimated seeds only. No GT reader.
#include "replay_hooks.h"
#include "inten_loam/detail/s2s_consensus.h"
#include "inten_trace_jsonl.h"
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <set>
#include <iomanip>
using namespace localization_zoo::inten_loam;
namespace serial=localization_zoo::evaluation::inten_trace;
void dumpFeatures(int frame,const PaperFrame& f) {
  std::cout<<std::setprecision(17)<<"{\"type\":\"features\",\"frame\":"<<frame<<",\"classes\":[";
  int cls=0;for(const auto* set:{&f.edge,&f.ground,&f.facade}){if(cls++)std::cout<<',';std::cout<<'[';bool comma=false;
    for(const auto& p:*set){if(comma)std::cout<<',';comma=true;std::cout<<'['<<p.p.x()<<','<<p.p.y()<<','<<p.p.z()<<','<<p.row<<','<<p.cluster<<']';}std::cout<<']';}
  std::cout<<"]}\n";
}
int main(int argc,char** argv) {
  try {
    if(argc!=2) throw std::runtime_error("Expected cached feature file");
    std::ifstream input(argv[1]);
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
    std::set<int> printed;
    while(std::cin>>frame) {
      Eigen::Matrix4d seed;
      for(int k=0;k<16;++k) if(!(std::cin>>seed(k/4,k%4))) throw std::runtime_error("Invalid estimated seed");
      const auto& previous=frames.at(frame-1);const auto& current=frames.at(frame);
      if(printed.insert(frame-1).second)dumpFeatures(frame-1,previous);
      if(printed.insert(frame).second)dumpFeatures(frame,current);
      const auto profiles=detail::clusterTranslationProfiles(previous.all,current,seed,true);
      for(const auto& p:profiles) {
        auto out=serial::record();out<<"{\"type\":\"profile\",\"frame\":"<<frame<<",\"cluster\":"<<p.cluster<<",\"costs_m2\":[";
        for(size_t k=0;k<p.costs.size();++k) {if(k)out<<',';out<<p.costs[k];}
        out<<"]}";std::cout<<out.str()<<'\n';
      }
      Eigen::Matrix3d joint_reference[2],refinement_reference[2];
      bool joint_available[2]={false,false},refinement_available[2]={false,false};
      for(int mode=0;mode<18;++mode) {
        InTenLoamPaperParams params;params.s2m_plane_max_dist=.05;params.accelerated=true;
        params.s2s_mapped_prior=true;params.s2s_translation_prior_ratio=.03;
        params.s2s_ground_pca=params.s2s_facade_pca=true;params.use_mapping=false;
        params.s2s_geometry_consensus=mode%2!=0;
        params.s2s_intensity_normalized=mode<12&&(mode%6==2||mode%6==3);
        params.s2s_intensity_consistent=mode<12&&mode%6>=4;
        params.s2s_iterations=mode<6?10:100;
        replay::frame_number=frame;replay::mode=mode;replay::reciprocal=mode>=12&&mode<16;
        replay::common_prior=mode>=14;
        const int arm=mode%2;
        replay::reference_joint=joint_available[arm]?&joint_reference[arm]:nullptr;
        replay::reference_refinement=refinement_available[arm]?&refinement_reference[arm]:nullptr;
        InTenLoamPaper core(params);replay::frame=&previous;replay::seed=nullptr;core.process({},{});
        replay::frame=&current;replay::seed=&seed;
        InTenLoamPaperTrace trace;const auto result=core.process({},{},nullptr,&trace);
        if(mode==6||mode==7) {
          joint_reference[arm]=trace.s2s.translation_prior_information;
          joint_available[arm]=trace.s2s.called;
          refinement_reference[arm]=trace.consensus.refinement.translation_prior_information;
          refinement_available[arm]=trace.consensus.refinement.called;
        }
        auto out=serial::record();out<<"{\"type\":\"terminal\",\"frame\":"<<frame<<",\"mode\":"<<mode<<",\"trace\":"
          <<serial::frame(frame,"cached_estimated_seed_only",trace,result,result.pose)<<'}';
        std::cout<<out.str()<<'\n';
      }
      std::cout<<std::flush;if(!std::cout)throw std::runtime_error("Output failed");
    }
    if(!std::cin.eof())throw std::runtime_error("Invalid command");
    return 0;
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
