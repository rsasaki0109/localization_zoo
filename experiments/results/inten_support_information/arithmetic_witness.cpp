// Independent arithmetic witness at one archived pose; no optimizer or NN search.
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
struct Point {Eigen::Vector3d p;int row;};
using V=Eigen::Matrix<double,6,1>;using M=Eigen::Matrix<double,6,6>;
template<class T>void array(const T& x){std::cout<<'[';bool c=false;for(int r=0;r<x.rows();++r)for(int k=0;k<x.cols();++k){if(c)std::cout<<',';c=true;std::cout<<x(r,k);}std::cout<<']';}
int main(){try{
  int ns,nt;std::cin>>ns>>nt;Eigen::Matrix4d T;for(int k=0;k<16;++k)std::cin>>T(k/4,k%4);
  std::vector<Point> source(ns),target(nt);
  for(auto* set:{&source,&target})for(auto& f:*set){double row,cluster,intensity;std::cin>>f.p.x()>>f.p.y()>>f.p.z()>>row>>cluster>>intensity;f.row=static_cast<int>(row);}
  const Eigen::Matrix3d R=T.block<3,3>(0,0);const Eigen::Vector3d t=T.block<3,1>(0,3);
  M H=M::Zero();V g=V::Zero();int count=0;
  std::cout<<std::setprecision(17);
  for(int i=0;i<ns;++i){int expected,keep,n;std::cin>>expected>>keep>>n;std::vector<int> ids(n);for(auto& k:ids)std::cin>>k;
    const Eigen::Vector3d x=R*source[i].p+t;Eigen::Vector3d gradient=Eigen::Vector3d::Zero();double residual=0.;bool valid=false;
    auto distance=[&](int k){float d=0;for(int a=0;a<3;++a){float v=static_cast<float>(x[a])-static_cast<float>(target[k].p[a]);d+=v*v;}return d;};
    if(n>=2 && distance(ids[0])<=1.){const auto& q=target[ids[0]];int m=-1;
      for(int k=1;k<n && distance(ids[k])<=1.;++k){int dr=std::abs(target[ids[k]].row-q.row);if(dr>=1 && dr<=2){m=ids[k];break;}}
      if(m>=0){const Eigen::Vector3d direction=q.p-target[m].p;
        if(direction.norm()>=1e-3){const Eigen::Vector3d v=direction.normalized(),d=x-q.p;
          const Eigen::Vector3d perpendicular=d-v*v.dot(d);const double r=perpendicular.norm();
          if(r>=1e-9){valid=true;gradient=perpendicular/r;residual=r;}}}}
    if(valid!=bool(expected))throw std::runtime_error("Witness residual validity differs");
    if(valid && keep){V J;J.head<3>()=gradient;J.tail<3>()=x.cross(gradient);double w=std::abs(residual)<=.1?1.:.1/std::abs(residual);H+=w*J*J.transpose();g+=w*J*residual;++count;}
    std::cout<<"{\"type\":\"row\",\"index\":"<<i<<",\"valid\":"<<(valid?"true":"false")<<",\"x\":";array(x);std::cout<<",\"gradient\":";array(gradient);std::cout<<",\"residual\":"<<residual<<"}\n";
  }
  if(!std::cin)throw std::runtime_error("Bad witness input");
  std::cout<<"{\"type\":\"system\",\"count\":"<<count<<",\"H\":";array(H);std::cout<<",\"g\":";array(g);std::cout<<"}\n";
  // Explicit operation-order controls for Eigen's fixed-size reductions.
  Eigen::Vector3d a(1e16,-1e16,1.);std::cout<<"{\"type\":\"sum_control\",\"eigen\":"<<a.sum()<<",\"left\":"<<(a[0]+a[1])+a[2]<<",\"right\":"<<a[0]+(a[1]+a[2])<<"}\n";
  Eigen::Vector3d b(1e8,1.,1.);std::cout<<"{\"type\":\"reduction_controls\",\"dot\":"<<a.dot(Eigen::Vector3d::Ones())
    <<",\"norm2\":"<<b.squaredNorm()<<",\"left_norm2\":"<<(b[0]*b[0]+b[1]*b[1])+b[2]*b[2]
    <<",\"right_norm2\":"<<b[0]*b[0]+(b[1]*b[1]+b[2]*b[2])<<"}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
