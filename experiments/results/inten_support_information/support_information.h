#pragma once
#include "mutual.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace support_information {
inline bool selected() {return replay::mode==6 || replay::mode==7 || replay::mode==14 || replay::mode==15;}
// This flag is changed only after the production parallel query/reduction has
// completed. The passive query uses private scratch and never updates a pose.
struct UnfilteredQuery {
  bool original=replay::reciprocal;
  UnfilteredQuery(){replay::reciprocal=false;}
  ~UnfilteredQuery(){replay::reciprocal=original;}
};
struct Frame {
  bool enabled=selected();
  std::array<std::unique_ptr<mutual::Index>,3> backward;
  explicit Frame(const mutual::Frame& f) {
    if(enabled) {
      backward[0]=std::make_unique<mutual::Index>(f.edge);
      backward[1]=std::make_unique<mutual::Index>(f.ground);
      backward[2]=std::make_unique<mutual::Index>(f.facade);
    }
  }
};
template<class Matrix> void array(const Matrix& x) {
  std::cout<<'[';bool comma=false;
  for(int r=0;r<x.rows();++r)for(int c=0;c<x.cols();++c){if(comma)std::cout<<',';comma=true;std::cout<<x(r,c);}
  std::cout<<']';
}
template<class System> void system(const System& s) {
  std::cout<<"{\"H\":";array(s.H);std::cout<<",\"g\":";array(s.g);
  std::cout<<",\"count\":"<<s.n<<",\"cost\":"<<s.weighted_squared_residual<<'}';
}
struct Context {
  const Frame& frame;
  Eigen::Matrix4d pose;
  int iteration;
  bool translation_only;
};
template<class System,class Query>
void inspect(const Context& context,int cls,const std::vector<mutual::Feature>& source,
    const std::vector<mutual::Feature>* target,const Query& query,double huber,bool cauchy=false) {
  if(!context.frame.enabled)return;
  const Eigen::Matrix3d R=context.pose.block<3,3>(0,0);
  const Eigen::Vector3d t=context.pose.block<3,1>(0,3);
  System all,kept,removed;
  all.track_residual=kept.track_residual=removed.track_residual=true;
  struct Row {std::vector<int> indices;bool valid=false,kept=false;};
  std::vector<Row> rows(source.size());
  {
    UnfilteredQuery unfiltered;
    std::vector<float> distances;
    for(size_t i=0;i<source.size();++i) {
      const auto x=Eigen::Vector3d(R*source[i].p+t);
      Eigen::Vector3d a;double residual=0.;auto& row=rows[i];distances.clear();
      row.valid=query(source[i],x,&a,&residual,&row.indices,&distances);
      if(cls==3)row.kept=true;
      else if(!row.indices.empty()) {
        if(!target)throw std::runtime_error("Missing passive geometric target");
        const auto anchor=target->at(row.indices[0]).p;
        const Eigen::Vector3d backward=R.transpose()*(anchor-t);
        row.kept=context.frame.backward.at(cls)->accepts(static_cast<int>(i),backward);
      }
      if(!row.valid)continue;
      if(!x.allFinite()||!a.allFinite()||!std::isfinite(residual))throw std::runtime_error("Nonfinite passive residual");
      all.add(x,a,residual,huber,nullptr,cauchy);
      (row.kept?kept:removed).add(x,a,residual,huber,nullptr,cauchy);
    }
  }
  std::cout<<std::setprecision(17)<<"{\"type\":\"support_query\",\"frame\":"<<replay::frame_number
    <<",\"mode\":"<<replay::mode<<",\"iteration\":"<<context.iteration
    <<",\"translation_only\":"<<(context.translation_only?"true":"false")<<",\"class\":"<<cls<<",\"pose\":";
  array(context.pose);std::cout<<",\"all\":";system(all);std::cout<<",\"kept\":";system(kept);std::cout<<",\"removed\":";system(removed);
  std::cout<<",\"rows\":[";
  for(size_t i=0;i<rows.size();++i){if(i)std::cout<<',';const auto& row=rows[i];
    std::cout<<'['<<(row.valid?"true":"false")<<','<<(row.kept?"true":"false")<<",[";
    for(size_t k=0;k<row.indices.size();++k){if(k)std::cout<<',';std::cout<<row.indices[k];}std::cout<<"]]";}
  std::cout<<"]}\n";
}
}
