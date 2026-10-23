// Passive calls to the unmodified public native IntensityGrid; no optimizer/GT.
#include "inten_loam/inten_loam_paper.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
using namespace localization_zoo::inten_loam;
using V = Eigen::Vector3d;
constexpr double knot = .3;
void vec(const V& x) { std::cout << '[' << x.x() << ',' << x.y() << ',' << x.z() << ']'; }
void sample(const InTenLoamPaper::IntensityGrid& grid, const V& x) {
  double value=0; V gradient=V::Zero();
  bool valid=grid.eval(x,&value,&gradient);
  std::cout << "{\"x\":"; vec(x); std::cout << ",\"valid\":" << (valid?"true":"false") << ",\"value\":";
  if(valid) { if(!std::isfinite(value)||!gradient.allFinite())throw std::runtime_error("Nonfinite native output"); std::cout << value; }
  else std::cout << "null";
  std::cout << ",\"gradient\":"; if(valid)vec(gradient);else std::cout << "null";
  std::cout << '}';
}
void field(const std::string& id,const std::vector<PaperFeature>& cloud) {
  // Diagnostic observation of exact build inputs, in their original summation order.
  std::map<std::array<int,3>,std::pair<double,int>> nodes;
  for(const auto& f:cloud) {
    std::array<int,3> key;
    for(int a=0;a<3;++a)key[a]=static_cast<int>(std::lround(f.p[a]/knot));
    auto& node=nodes[key];node.first+=f.intensity;++node.second;
  }
  std::cout << "{\"type\":\"field\",\"field\":\"" << id << "\",\"knot\":" << knot
            << ",\"features\":" << cloud.size() << ",\"nodes\":[";
  bool comma=false;
  for(const auto& node:nodes) {
    if(comma)std::cout << ',';comma=true;
    const auto& k=node.first;
    std::cout << '[' << k[0] << ',' << k[1] << ',' << k[2] << ',' << node.second.first << ',' << node.second.second << ']';
  }
  std::cout << "]}\n";
}
V side(V x,int axis,int face,int direction,double epsilon) {
  x[axis]=(face+direction*epsilon)*knot;
  if(epsilon==0) {
    const double target=direction<0?-std::numeric_limits<double>::infinity():std::numeric_limits<double>::infinity();
    x[axis]=std::nextafter(x[axis],target);
    for(int n=0;n<16 && static_cast<int>(std::floor(x[axis]/knot))!=(direction<0?face-1:face);++n)
      x[axis]=std::nextafter(x[axis],target);
  }
  if(static_cast<int>(std::floor(x[axis]/knot))!=(direction<0?face-1:face))throw std::runtime_error("Wrong face side");
  return x;
}
void path(const std::string& id,int index,const V& x,const V& delta,const InTenLoamPaper::IntensityGrid& grid) {
  std::cout << "{\"type\":\"path\",\"field\":\"" << id << "\",\"index\":" << index << ",\"start\":";vec(x);
  std::cout << ",\"delta\":";vec(delta);
  std::cout << ",\"endpoints\":[";sample(grid,x);std::cout << ',';sample(grid,x+delta);std::cout << ']';
  // Fixed within-cell derivative control, safely away from any cell face.
  V center;for(int a=0;a<3;++a)center[a]=(std::floor(x[a]/knot)+.375)*knot;
  std::cout << ",\"interior\":[";sample(grid,center);
  for(int a=0;a<3;++a)for(int sign:{-1,1}) {V offset=center;offset[a]+=sign*knot*1e-5;std::cout << ',';sample(grid,offset);}
  std::cout << "],\"boundaries\":[";bool comma=false;
  for(int a=0;a<3;++a) {
    if(delta[a]==0)continue;
    int lo=static_cast<int>(std::floor(std::min(x[a],x[a]+delta[a])/knot));
    int hi=static_cast<int>(std::floor(std::max(x[a],x[a]+delta[a])/knot));
    for(int face=lo+1;face<=hi;++face) {
      double t=(face*knot-x[a])/delta[a];if(t<=0||t>=1)continue;
      V point=x+t*delta;point[a]=face*knot;
      if(comma)std::cout << ',';comma=true;
      std::cout << "{\"axis\":" << a << ",\"face\":" << face << ",\"t\":" << t << ",\"samples\":[";
      bool inner=false;
      for(double eps:{1e-3,1e-6,1e-9,0.})for(int sign:{-1,1}) {
        if(inner)std::cout << ',';inner=true;sample(grid,side(point,a,face,sign,eps));
      }
      std::cout << "]}";
    }
  }
  std::cout << "]}\n";
}
std::map<int,PaperFrame> load(const char* name) {
  std::ifstream input(name);if(!input)throw std::runtime_error("Missing cached features");
  std::map<int,PaperFrame> frames;int index;
  while(input>>index) {
    if(frames.count(index))throw std::runtime_error("Duplicate frame");
    auto& frame=frames[index];
    for(auto* part:{&frame.edge,&frame.ground,&frame.facade,&frame.reflector,&frame.all}) {
      size_t count;if(!(input>>count))throw std::runtime_error("Invalid feature count");
      for(size_t i=0;i<count;++i) {
        PaperFeature f;
        if(!(input>>f.p.x()>>f.p.y()>>f.p.z()>>f.row>>f.cluster>>f.intensity))throw std::runtime_error("Invalid feature");
        part->push_back(f);
      }
    }
  }
  if(!input.eof())throw std::runtime_error("Invalid cache");return frames;
}
int main(int argc,char** argv) {
  try {
    std::cout << std::setprecision(17);
    if(argc==1) {
      for(int axis=0;axis<3;++axis)for(const std::string kind:{"dense_ramp","sparse_ramp","sparse_constant","invalid_three"}) {
        std::vector<PaperFeature> cloud;
        for(int i=-1;i<=3;++i)for(int j=-1;j<=2;++j)for(int k=-1;k<=2;++k) {
          bool sparse=kind!="dense_ramp";
          if(sparse && (k!=0 || (j!=0&&j!=1)))continue;
          if(kind=="invalid_three" && (i!=0 || j!=0))continue;
          PaperFeature f;f.p=V::Zero();f.p[axis]=i*knot;f.p[(axis+1)%3]=j*knot;f.p[(axis+2)%3]=k*knot;
          f.intensity=kind=="sparse_constant"?.4: .2+.05*i+.02*j+.01*k;cloud.push_back(f);
        }
        if(kind=="invalid_three") {cloud.clear();for(int j=0;j<3;++j){PaperFeature f;f.p=V(0,j*knot,0);f.intensity=.2;cloud.push_back(f);}}
        std::string id=kind+"_"+std::to_string(axis);
        field(id,cloud);InTenLoamPaper::IntensityGrid grid;grid.build(cloud,knot);
        V start=V::Constant(.375*knot),delta=V::Zero();start[axis]=.75*knot;delta[axis]=.5*knot;
        path(id,0,start,delta,grid);
      }
    } else if(argc==3) {
      const auto frames=load(argv[1]);std::ifstream inputs(argv[2]);if(!inputs)throw std::runtime_error("Missing paths");
      int index,id,loaded=-1;InTenLoamPaper::IntensityGrid grid;
      while(inputs>>index>>id) {
        V x,delta;for(int a=0;a<3;++a)if(!(inputs>>x[a]))throw std::runtime_error("Invalid start");
        for(int a=0;a<3;++a)if(!(inputs>>delta[a]))throw std::runtime_error("Invalid delta");
        if(index!=loaded) {const auto& cloud=frames.at(index-1).all;field(std::to_string(index),cloud);grid.build(cloud,knot);loaded=index;}
        path(std::to_string(index),id,x,delta,grid);
      }
      if(!inputs.eof())throw std::runtime_error("Invalid path input");
    } else throw std::runtime_error("Expected no args (synthetic), or cache and paths");
    std::cout.flush();if(!std::cout)throw std::runtime_error("Output failed");return 0;
  }catch(const std::exception& e){std::cerr << e.what() << '\n';return 1;}
}
