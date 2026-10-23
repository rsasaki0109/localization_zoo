#include "native_loader.h"
#include "inten_loam/inten_loam_paper.h"
#include <iomanip>
#include <stdexcept>
using namespace localization_zoo::inten_loam;
int main(int argc,char** argv) {try {
  if(argc!=3) throw std::runtime_error("Expected scan directory and feature output");
  std::ofstream out(argv[2]);if(!out)throw std::runtime_error("Cannot open features");
  out<<std::setprecision(17);
  InTenLoamPaperParams params;params.s2m_plane_max_dist=.05;params.accelerated=true;
  params.s2s_mapped_prior=true;params.s2s_translation_prior_ratio=.03;
  params.s2s_ground_pca=params.s2s_facade_pca=true;
  InTenLoamPaper core(params);int frame;int previous=-1;
  while(std::cin>>frame) {
    if(frame<=previous)throw std::runtime_error("Expected increasing original IDs");previous=frame;
    char name[32];std::snprintf(name,sizeof(name),"%06d.bin",frame);
    const auto raw=loadPCDXYZI((fs::path(argv[1])/name).string(),0.0);
    if(raw.empty())throw std::runtime_error("Empty scan");
    std::vector<Eigen::Vector3d> points;std::vector<float> intensity;
    for(const auto& p:raw){points.push_back(p.point);intensity.push_back(p.intensity);}
    if(*std::max_element(intensity.begin(),intensity.end())>1.0f)for(auto& v:intensity)v/=255.0f;
    const auto f=core.extractFeatures(points,intensity);out<<frame<<'\n';
    for(const auto* set:{&f.edge,&f.ground,&f.facade,&f.reflector,&f.all}) {
      out<<set->size()<<'\n';
      for(const auto& p:*set) out<<p.p.x()<<' '<<p.p.y()<<' '<<p.p.z()<<' '<<p.row<<' '<<p.cluster<<' '<<p.intensity<<'\n';
    }
  }
  if(!std::cin.eof())throw std::runtime_error("Invalid frame input");
  out.close();if(out.fail())throw std::runtime_error("Feature output failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
