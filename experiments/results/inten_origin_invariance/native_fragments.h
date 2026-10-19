#pragma once
namespace native {
using Vec6 = Eigen::Matrix<double,6,1>;
using Mat6 = Eigen::Matrix<double,6,6>;
Eigen::Matrix4d expSe3(const Vec6& d) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  const Eigen::Vector3d w = d.tail<3>();
  if (w.norm() > 1e-12) T.block<3, 3>(0, 0) = Eigen::AngleAxisd(w.norm(), w.normalized()).toRotationMatrix();
  T.block<3, 1>(0, 3) = d.head<3>();
  return T;
}

inline Vec6 solve(const Mat6& H,const Vec6& g) {
struct System { Mat6 H; Vec6 g; };
const System sys{H,g};
    const Vec6 delta = (sys.H + 1e-6 * Mat6::Identity()).ldlt().solve(-sys.g);
return delta;
}
}
