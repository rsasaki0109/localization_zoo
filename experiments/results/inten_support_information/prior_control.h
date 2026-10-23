#pragma once
#include "replay_hooks.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>

namespace prior_control {
inline Eigen::Matrix3d select(const Eigen::Matrix3d& derived, bool enabled,
    bool translation_only, const Eigen::Matrix3d* joint, const Eigen::Matrix3d* refinement) {
  if (!enabled) return derived;
  const auto* reference = translation_only ? refinement : joint;
  if (!reference || !reference->allFinite()) throw std::runtime_error("Missing/nonfinite fixed reference prior");
  return *reference;
}
template<class Matrix> inline void array(const Matrix& x) {
  std::cout<<'[';
  bool comma=false;
  for(int r=0;r<x.rows();++r) for(int c=0;c<x.cols();++c) {
    if(comma)std::cout<<',';comma=true;std::cout<<x(r,c);
  }
  std::cout<<']';
}
inline void observe(int iteration, bool translation_only, const Eigen::Matrix3d& information,
    const Eigen::Vector3d& prediction, const Eigen::Vector3d& t,
    const Eigen::Matrix<double,6,6>& H, const Eigen::Matrix<double,6,1>& g) {
  std::cout<<std::setprecision(17)<<"{\"type\":\"prior_observation\",\"frame\":"<<replay::frame_number
    <<",\"mode\":"<<replay::mode<<",\"iteration\":"<<iteration
    <<",\"translation_only\":"<<(translation_only?"true":"false")<<",\"reference_mode\":"<<(6+replay::mode%2)
    <<",\"information\":";array(information);
  std::cout<<",\"prediction\":";array(prediction);std::cout<<",\"t\":";array(t);
  std::cout<<",\"H_before_prior\":";array(H);std::cout<<",\"g_before_prior\":";array(g);
  std::cout<<"}\n";
}
}
