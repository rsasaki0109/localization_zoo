#pragma once
#include "replay_hooks.h"
#include <Eigen/Geometry>
#include <flann/algorithms/dist.h>
#include <flann/algorithms/kdtree_single_index.h>
#include <flann/util/result_set.h>
#include <array>
#include <memory>
#include <limits>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cmath>
namespace mutual {
using Feature=localization_zoo::inten_loam::PaperFeature;
using Frame=localization_zoo::inten_loam::PaperFrame;
using Vec=Eigen::Vector3d;
class Index {
 public:
  explicit Index(const std::vector<Feature>& points):points_(points) {
    for(const auto& p:points)for(int a=0;a<3;++a){if(!std::isfinite(p.p[a]))throw std::runtime_error("Nonfinite reciprocal feature");coordinates_.push_back(p.p[a]);}
    if(!points.empty()){
      tree_=std::make_unique<Tree>(flann::Matrix<double>(coordinates_.data(),points.size(),3),flann::KDTreeSingleIndexParams(15));tree_->buildIndex();
    }
  }
  int nearest(const Vec& x) const {
    if(!tree_||!x.allFinite())return -1;
    flann::KNNSimpleResultSet<double> result(1);size_t index=0;double distance=0;
    tree_->findNeighbors(result,x.data(),flann::SearchParams(-1,0.f,true));result.copy(&index,&distance,1,true);return static_cast<int>(index);
  }
  bool accepts(int source,const Vec& backward,int* nearest_index=nullptr) const {
    if(source<0||static_cast<size_t>(source)>=points_.size())return false;
    const int index=nearest(backward);if(nearest_index)*nearest_index=index;if(index<0)return false;
    const double best=(points_[index].p-backward).squaredNorm();
    const double original=(points_[source].p-backward).squaredNorm();
    return original<=best+1e-12+1e-10*std::abs(best);
  }
 private:
  using Tree=flann::KDTreeSingleIndex<flann::L2_Simple<double>>;
  const std::vector<Feature>& points_;std::vector<double> coordinates_;std::unique_ptr<Tree> tree_;
};
struct Backward {
  std::array<const std::vector<Feature>*,3> sources;
  std::array<std::unique_ptr<Index>,3> indices;
  bool enabled;
  Backward(const Frame& f,bool active):sources{{&f.edge,&f.ground,&f.facade}},enabled(active) {
    if(enabled)for(int c=0;c<3;++c)indices[c]=std::make_unique<Index>(*sources[c]);
  }
};
class Observation {
 public:
  Observation(const Backward& b,const Eigen::Matrix4d& pose,int iteration,bool translation_only):b_(b),T_(pose),it_(iteration),translation_only_(translation_only) {
    if(b_.enabled)for(int c=0;c<3;++c)rows_[c].resize(b_.sources[c]->size());
  }
  void note(int cls,int source,int anchor) {
    auto& row=rows_.at(cls).at(source);row.visited=true;row.anchor=anchor;
  }
  bool accepts(int cls,int source,const Vec& anchor) {
    auto& row=rows_.at(cls).at(source);
    const Vec backward=T_.block<3,3>(0,0).transpose()*(anchor-T_.block<3,1>(0,3));
    row.accepted=b_.indices.at(cls)->accepts(source,backward,&row.back);
    return row.accepted;
  }
  ~Observation() {
    if(!b_.enabled)return;
    std::cout<<std::setprecision(17)<<"{\"type\":\"reciprocal_iteration\",\"frame\":"<<replay::frame_number<<",\"mode\":"<<replay::mode
      <<",\"translation_only\":"<<(translation_only_?"true":"false")<<",\"iteration\":"<<it_<<",\"pose\":[";
    for(int i=0;i<16;++i){if(i)std::cout<<',';std::cout<<T_(i/4,i%4);}std::cout<<"],\"classes\":[";
    for(int c=0;c<3;++c){if(c)std::cout<<',';std::cout<<'[';bool comma=false;
      for(size_t i=0;i<rows_[c].size();++i){const auto& r=rows_[c][i];if(!r.visited)continue;if(comma)std::cout<<',';comma=true;
        std::cout<<'['<<i<<','<<r.anchor<<','<<r.back<<','<<(r.accepted?"true":"false")<<']';}std::cout<<']';}
    std::cout<<"]}\n";
  }
 private:
  struct Row {bool visited=false,accepted=false;int anchor=-1,back=-1;};
  const Backward& b_;Eigen::Matrix4d T_;int it_;bool translation_only_;
  std::array<std::vector<Row>,3> rows_;
};
}
