// Actual-prefix detector masks and natural seeds only; no GT/hand box is read.
#include "native_loader.h"
#include "inten_trace_jsonl.h"
#include "terminal_hooks.h"
#include <algorithm>
#include <stdexcept>

using namespace localization_zoo::inten_loam;
namespace serial = localization_zoo::evaluation::inten_trace;
struct Scan { std::vector<Eigen::Vector3d> points; std::vector<float> intensity; };
Scan load(const std::string& path) {
  Scan scan;
  for (const auto& p : loadPCDXYZI(path,0.)) { scan.points.push_back(p.point); scan.intensity.push_back(p.intensity); }
  if (scan.points.empty()) throw std::runtime_error("Empty scan");
  if (*std::max_element(scan.intensity.begin(),scan.intensity.end())>1.f)
    for (auto& v:scan.intensity) v/=255.f;
  return scan;
}
int main() {
  try {
    int frame, mode, n;
    std::string previous_path,current_path,last_previous,last_current;
    Scan previous,current;
    while (std::cin>>frame>>mode>>previous_path>>current_path) {
      diagnostic::source_mask.clear(); diagnostic::target_mask.clear();
      if (!(std::cin>>n) || n<0) throw std::runtime_error("Invalid source mask size");
      for (int k=0,c;k<n;++k) { if (!(std::cin>>c)) throw std::runtime_error("Invalid source mask"); diagnostic::source_mask.insert(c); }
      if (!(std::cin>>n) || n<0) throw std::runtime_error("Invalid target mask size");
      for (int k=0,c;k<n;++k) { if (!(std::cin>>c)) throw std::runtime_error("Invalid target mask"); diagnostic::target_mask.insert(c); }
      Eigen::Matrix4d seed;
      for (int k=0;k<16;++k) if (!(std::cin>>seed(k/4,k%4))) throw std::runtime_error("Invalid seed");
      if (previous_path!=last_previous) { previous=load(previous_path); last_previous=previous_path; }
      if (current_path!=last_current) { current=load(current_path); last_current=current_path; }
      InTenLoamPaperParams params;
      params.s2m_plane_max_dist=.05; params.accelerated=true;
      params.s2s_mapped_prior=true; params.s2s_translation_prior_ratio=.03;
      params.s2s_ground_pca=params.s2s_facade_pca=true;
      params.use_mapping=false;
      InTenLoamPaper core(params);
      diagnostic::terminal_mode=mode;
      core.process(previous.points,previous.intensity);
      InTenLoamPaperDiagnostics d; d.forced_pose=&seed;
      core.process(current.points,current.intensity,&d);
      auto out=serial::record();
      out<<"{\"frame\":"<<frame<<",\"mode\":"<<mode<<",\"output\":"; serial::matrix(out,d.s2s_estimate);
      out<<",\"first_systems\":[";
      for (int c=0;c<4;++c) {
        if(c)out<<',';
        out<<"{\"n\":"<<d.s2s_n[c]<<",\"H\":";serial::matrix(out,d.s2s_H[c]);
        out<<",\"g\":";serial::matrix(out,d.s2s_g[c]);out<<'}';
      }
      out<<"]}";std::cout<<out.str()<<'\n'<<std::flush;
      if(!std::cout)throw std::runtime_error("Output failed");
    }
    if(!std::cin.eof())throw std::runtime_error("Invalid input");
    return 0;
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
