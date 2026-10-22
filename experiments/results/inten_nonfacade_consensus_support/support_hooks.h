#pragma once
// Diagnostic-only common-support veto. No production parameter or API change.
#include "inten_loam/inten_loam_paper.h"
#include "inten_trace_jsonl.h"
#include <array>
#include <ostream>
#include <stdexcept>
namespace support {
using namespace localization_zoo::inten_loam;
namespace serial=localization_zoo::evaluation::inten_trace;
inline bool enabled=false,observing=false;
inline int frame=-1;
inline std::ostream* capture=nullptr;
struct Report {
  bool evaluated=false,allowed=false,complete=true;
  std::array<int,4> counts{};
  std::array<double,4> original{},candidate{};
  Eigen::Vector3d delta=Eigen::Vector3d::Zero();
  PaperSolverTrace query;
};
inline Report report;
inline double rho(double r,double huber) {
  const double a=std::abs(r);
  return a<=huber ? r*r : 2.*huber*a-huber*huber;
}
inline void begin(const Eigen::Matrix4d& original,const Eigen::Matrix4d& candidate) {
  report=Report();report.evaluated=true;
  if(!(original.block<3,3>(0,0).array()==candidate.block<3,3>(0,0).array()).all())
    throw std::runtime_error("Support guard requires identical estimated rotation");
  report.delta=candidate.block<3,1>(0,3)-original.block<3,1>(0,3);
  report.query.called=true;report.query.input=original;observing=true;
}
inline void add(int cls,const char* kind,const Eigen::Vector3d& x,
                const Eigen::Vector3d& a,double r,double changed,double huber,
                bool complete,const Eigen::Vector3d& anchor=Eigen::Vector3d::Zero(),
                const Eigen::Vector3d& direction=Eigen::Vector3d::Zero(),
                double mu=0.,double other_mu=0.,double intensity=0.,double scale=0.) {
  ++report.counts[cls];
  report.original[cls]+=rho(r,huber);
  if(complete)report.candidate[cls]+=rho(changed,huber);
  // Facades are observed for equation reconstruction but never veto by loss/coverage.
  if(cls!=2 && (!complete || !std::isfinite(r) || !std::isfinite(changed)))report.complete=false;
  if(!capture)throw std::runtime_error("Missing common-support output");
  auto out=serial::record();
  out<<"{\"frame\":"<<frame<<",\"class\":"<<cls<<",\"kind\":\""<<kind<<"\",\"x\":";
  serial::matrix(out,x);out<<",\"a\":";serial::matrix(out,a);
  out<<",\"anchor\":";serial::matrix(out,anchor);out<<",\"direction\":";serial::matrix(out,direction);
  out<<",\"r\":"<<r<<",\"changed\":";
  if(complete)out<<changed;else out<<"null";
  out<<",\"huber\":"<<huber<<",\"complete\":"<<(complete?"true":"false")
     <<",\"mu\":"<<mu<<",\"other_mu\":";
  if(complete)out<<other_mu;else out<<"null";
  out<<",\"intensity\":"<<intensity<<",\"scale\":"<<scale<<'}';
  *capture<<out.str()<<'\n';if(!*capture)throw std::runtime_error("Support output failed");
}
inline void line(const Eigen::Vector3d& x,const Eigen::Vector3d& a,double r,double h,
                 const Eigen::Vector3d& anchor,const Eigen::Vector3d& direction) {
  const Eigen::Vector3d d=x+report.delta-anchor;
  const double changed=(d-direction*direction.dot(d)).norm();
  add(0,"line",x,a,r,changed,h,true,anchor,direction);
}
inline void plane(int cls,const Eigen::Vector3d& x,const Eigen::Vector3d& a,
                  double r,double h,const Eigen::Vector3d& anchor) {
  add(cls,"plane",x,a,r,a.dot(x+report.delta-anchor),h,true,anchor);
}
inline void field(const Eigen::Vector3d& x,const Eigen::Vector3d& a,double r,double h,
                  double mu,double other_mu,double intensity,double scale,bool complete) {
  add(3,"field",x,a,r,scale*(other_mu-intensity),h,complete,
      Eigen::Vector3d::Zero(),Eigen::Vector3d::Zero(),mu,other_mu,intensity,scale);
}
inline bool finish() {
  observing=false;
  const int n=report.counts[0]+report.counts[1]+report.counts[3];
  const double a=report.original[0]+report.original[1]+report.original[3];
  const double b=report.candidate[0]+report.candidate[1]+report.candidate[3];
  report.allowed=report.complete && n>0 && std::isfinite(a) && std::isfinite(b) && b<a;
  return report.allowed;
}
inline void emit(std::ostream& out) {
  out<<"{\"evaluated\":"<<(report.evaluated?"true":"false")
     <<",\"allowed\":"<<(report.allowed?"true":"false")
     <<",\"complete\":"<<(report.complete?"true":"false")<<",\"delta\":";
  serial::matrix(out,report.delta);out<<",\"counts\":[";
  for(int c=0;c<4;++c){if(c)out<<',';out<<report.counts[c];}out<<"],\"original_costs\":[";
  for(int c=0;c<4;++c){if(c)out<<',';out<<report.original[c];}out<<"],\"candidate_costs\":[";
  for(int c=0;c<4;++c){if(c)out<<',';out<<report.candidate[c];}out<<"],\"query\":";
  serial::solver(out,report.query);out<<'}';
}
}
