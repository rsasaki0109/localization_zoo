#pragma once
#include "inten_loam/inten_loam_paper.h"
#include <Eigen/LU>
#include <flann/algorithms/dist.h>
#include <flann/algorithms/kdtree_single_index.h>
#include <flann/util/result_set.h>
#include <map>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
namespace dense_continuation {
using Vec=Eigen::Vector3d;
struct Cell {std::array<int,3> key;int count;Vec point;};
inline std::vector<Cell> voxelize(const std::vector<localization_zoo::inten_loam::PaperFeature>& points,double size) {
  struct Sum {Vec point=Vec::Zero();int count=0;};
  std::map<std::array<int,3>,Sum> sums;
  for(const auto& p:points) {
    std::array<int,3> key;for(int a=0;a<3;++a)key[a]=static_cast<int>(std::floor(p.p[a]/size));
    auto& s=sums[key];s.point+=p.p;++s.count;
  }
  std::vector<Cell> out;for(const auto& entry:sums)out.push_back({entry.first,entry.second.count,entry.second.point/entry.second.count});
  return out;
}
class Index {
 public:
  explicit Index(const std::vector<Cell>& cells):cells_(cells) {
    for(const auto& c:cells)for(int a=0;a<3;++a)coordinates_.push_back(c.point[a]);
    if(!cells.empty()) {
      tree_=std::make_unique<Tree>(flann::Matrix<double>(coordinates_.data(),cells.size(),3),flann::KDTreeSingleIndexParams(15));
      tree_->buildIndex();
    }
  }
  double squaredDistance(const Vec& point) const {
    if(!tree_)return std::numeric_limits<double>::infinity();
    flann::KNNSimpleResultSet<double> result(1);size_t index=0;double distance=0;
    tree_->findNeighbors(result,point.data(),flann::SearchParams(-1,0.f,true));result.copy(&index,&distance,1,true);
    return (point-cells_[index].point).squaredNorm();
  }
 private:
  using Tree=flann::KDTreeSingleIndex<flann::L2_Simple<double>>;
  const std::vector<Cell>& cells_;std::vector<double> coordinates_;std::unique_ptr<Tree> tree_;
};
struct Direction {bool valid=false;double mean=0.;size_t population=0,within_gate=0;};
inline Direction score(const std::vector<Cell>& source,const Index& target,const Eigen::Matrix4d& T,double gate_sq) {
  Direction out;out.population=source.size();if(source.empty())return out;
  double sum=0;
  for(const auto& cell:source) {
    const Vec x=T.block<3,3>(0,0)*cell.point+T.block<3,1>(0,3);
    const double squared=target.squaredDistance(x);
    if(!std::isfinite(squared))return out;
    sum+=std::min(squared,gate_sq);out.within_gate+=squared<gate_sq;
  }
  out.mean=sum/source.size();out.valid=std::isfinite(out.mean);return out;
}
inline bool improves(const Direction& before,const Direction& after) {
  return before.valid&&after.valid&&before.population==after.population&&
    before.mean-after.mean>1e-12+1e-10*std::abs(before.mean);
}
inline bool chooseExtended(const Direction& base_forward,const Direction& base_reverse,
                           const Direction& extended_forward,const Direction& extended_reverse) {
  return improves(base_forward,extended_forward)&&improves(base_reverse,extended_reverse);
}
}
