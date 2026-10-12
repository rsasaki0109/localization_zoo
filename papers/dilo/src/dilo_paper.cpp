#include "dilo/dilo_paper.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization_zoo {
namespace dilo {

namespace {

constexpr double kPi = 3.14159265358979323846;
// Farid-Simoncelli 5-tap filters (Sec. 4.1).
constexpr double kPre[5] = {0.037659, 0.249153, 0.426375, 0.249153, 0.037659};
constexpr double kDer[5] = {0.109604, 0.276691, 0.0, -0.276691, -0.109604};

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0, -v.z(), v.y(), v.z(), 0, -v.x(), -v.y(), v.x(), 0;
  return m;
}

Eigen::Matrix3d expSO3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K + (1.0 - std::cos(theta)) * K * K;
}

Eigen::Vector3d logSO3(const Eigen::Matrix3d& R) {
  const Eigen::AngleAxisd aa(R);
  return aa.angle() * aa.axis();
}

// Pose increment exp([w, v]) with the translation taken as v (small steps).
Eigen::Matrix4d expPose(const Eigen::Matrix<double, 6, 1>& d) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSO3(d.head<3>());
  T.block<3, 1>(0, 3) = d.tail<3>();
  return T;
}

Eigen::Matrix<double, 6, 1> logPose(const Eigen::Matrix4d& T) {
  Eigen::Matrix<double, 6, 1> d;
  d.head<3>() = logSO3(T.block<3, 3>(0, 0));
  d.tail<3>() = T.block<3, 1>(0, 3);
  return d;
}

Eigen::Matrix4d inversePose(const Eigen::Matrix4d& T) {
  Eigen::Matrix4d inv = Eigen::Matrix4d::Identity();
  inv.block<3, 3>(0, 0) = T.block<3, 3>(0, 0).transpose();
  inv.block<3, 1>(0, 3) = -inv.block<3, 3>(0, 0) * T.block<3, 1>(0, 3);
  return inv;
}

}  // namespace

DiloPaperOdometry::DiloPaperOdometry(const DiloPaperParams& params)
    : params_(params),
      scale_(params.width / (2.0 * kPi)),
      row0_(params.width / (2.0 * kPi) * std::sin(params.top_elevation_deg * kPi / 180.0)) {}

Eigen::Vector3d DiloPaperOdometry::pixelDirection(int row, int col) const {
  const double theta = ((col + 0.5) - params_.width / 2.0) / scale_;
  const double sin_phi = std::clamp((row0_ - (row + 0.5)) / scale_, -1.0, 1.0);
  const double cos_phi = std::sqrt(1.0 - sin_phi * sin_phi);
  return Eigen::Vector3d(cos_phi * std::cos(theta), cos_phi * std::sin(theta), sin_phi);
}

bool DiloPaperOdometry::project(const Eigen::Vector3d& p, int* row, int* col) const {
  const double r = p.norm();
  if (r < 1e-6) return false;
  const double u = scale_ * std::atan2(p.y(), p.x()) + params_.width / 2.0;
  const double v = row0_ - scale_ * (p.z() / r);
  int c = static_cast<int>(std::floor(u));
  const int rr = static_cast<int>(std::floor(v));
  if (rr < 0 || rr >= params_.height) return false;
  c = ((c % params_.width) + params_.width) % params_.width;
  *row = rr;
  *col = c;
  return true;
}

std::shared_ptr<DiloPaperFrame> DiloPaperOdometry::buildFrame(
    const std::vector<Eigen::Vector3d>& scan) const {
  const int H = params_.height, W = params_.width;
  auto frame = std::make_shared<DiloPaperFrame>();
  auto& range = frame->range;
  range.assign(static_cast<size_t>(H) * W, 0.0f);
  // Measured pixels keep their exact point (the pixel-centre ray of a
  // pixel differs from the beam by up to half a pixel, a few centimetres on
  // the ground at 20 m); filled pixels use the pixel-centre ray.
  std::vector<Eigen::Vector3f> measured(static_cast<size_t>(H) * W, Eigen::Vector3f::Zero());
  for (const auto& p : scan) {
    const double r = p.norm();
    if (r < params_.min_range || r > params_.max_range) continue;
    int row, col;
    if (!project(p, &row, &col)) continue;
    float& cell = range[index(row, col)];
    if (cell == 0.0f || r < cell) {
      cell = static_cast<float>(r);
      if (params_.exact_points) measured[index(row, col)] = p.cast<float>();
    }
  }

  // Vertical fill (Sec. 3.2): interpolate across short gaps when the range
  // changes slowly, otherwise take the far range.
  for (int c = 0; c < W; ++c) {
    int last = -1;
    for (int r = 0; r < H; ++r) {
      const float cur = range[index(r, c)];
      if (cur == 0.0f) continue;
      if (last >= 0 && r - last > 1 && r - last - 1 <= params_.max_fill_gap) {
        const float r0 = range[index(last, c)];
        const double slope = std::abs(cur - r0) / (r - last);
        const bool smooth = slope < params_.fill_slope * std::min(r0, cur);
        const Eigen::Vector3f p0 = measured[index(last, c)].squaredNorm() > 0.0f
                                       ? measured[index(last, c)]
                                       : (pixelDirection(last, c) * r0).cast<float>();
        const Eigen::Vector3f p1 = measured[index(r, c)].squaredNorm() > 0.0f
                                       ? measured[index(r, c)]
                                       : (pixelDirection(r, c) * cur).cast<float>();
        for (int k = last + 1; k < r; ++k) {
          const float t = static_cast<float>(k - last) / (r - last);
          if (smooth) {
            // Interpolate the 3D points (a plane stays a plane; linear range
            // interpolation would bend the ground between beams).
            const Eigen::Vector3f p = p0 + t * (p1 - p0);
            range[index(k, c)] = p.norm();
            measured[index(k, c)] = p;
          } else {
            range[index(k, c)] = std::max(r0, cur);
          }
        }
      }
      last = r;
    }
  }

  const size_t n = range.size();
  frame->point.assign(n, Eigen::Vector3f::Zero());
  frame->normal.assign(n, Eigen::Vector3f::Zero());
  frame->planarity.assign(n, 0.0f);
  frame->has_normal.assign(n, 0);
  for (int r = 0; r < H; ++r)
    for (int c = 0; c < W; ++c) {
      const float rg = range[index(r, c)];
      if (rg <= 0.0f) continue;
      const Eigen::Vector3f& m = measured[index(r, c)];
      frame->point[index(r, c)] = m.squaredNorm() > 0.0f ? m : (pixelDirection(r, c) * rg).cast<float>();
    }

  // Separable 5-tap filtering of an image component; false if any support
  // pixel is invalid.
  auto filter = [&](int r, int c, bool along_u, auto value, double* out) {
    double acc = 0.0;
    for (int a = 0; a < 5; ++a)
      for (int b = 0; b < 5; ++b) {
        const int rr = r + a - 2;
        if (rr < 0 || rr >= H) return false;
        const int cc = ((c - (b - 2)) % W + W) % W;
        double v;
        if (!value(rr, cc, &v)) return false;
        // along_u: derivative along columns (b), prefilter along rows (a).
        const double w = along_u ? kPre[a] * kDer[b] : kDer[a] * kPre[b];
        acc += w * v;
      }
    *out = acc;
    return true;
  };
  // Derivative taps index the offset with a sign flip (convolution); for
  // rows use the same convention: row offset a - 2 -> weight kDer[4 - a].
  auto filter_rows = [&](int r, int c, auto value, double* out) {
    double acc = 0.0;
    for (int a = 0; a < 5; ++a)
      for (int b = 0; b < 5; ++b) {
        const int rr = r - (a - 2);
        if (rr < 0 || rr >= H) return false;
        const int cc = ((c + b - 2) % W + W) % W;
        double v;
        if (!value(rr, cc, &v)) return false;
        acc += kDer[a] * kPre[b] * v;
      }
    *out = acc;
    return true;
  };

  for (int r = 2; r < H - 2; ++r) {
    for (int c = 0; c < W; ++c) {
      const float rg = range[index(r, c)];
      if (rg <= 0.0f) continue;
      auto rv = [&](int rr, int cc, double* v) {
        const float x = range[index(rr, cc)];
        if (x <= 0.0f || std::abs(x - rg) > 0.1 * rg + 0.3) return false;
        *v = x;
        return true;
      };
      Eigen::Vector3d nrm;
      if (params_.point_gradient_normals) {
        // Tangents from the same filters applied to the 3D point image: the
        // measured points need not lie on the pixel-centre rays.
        Eigen::Vector3d pu, pv;
        bool ok = true;
        for (int k = 0; k < 3 && ok; ++k) {
          auto pk = [&](int rr, int cc, double* v) {
            double dummy;
            if (!rv(rr, cc, &dummy)) return false;
            *v = frame->point[index(rr, cc)](k);
            return true;
          };
          ok = filter(r, c, true, pk, &pu(k)) && filter_rows(r, c, pk, &pv(k));
        }
        if (!ok) continue;
        nrm = pu.cross(pv);
        if (nrm.norm() < 1e-12) continue;
        nrm.normalize();
        if (nrm.dot(frame->point[index(r, c)].cast<double>()) > 0) nrm = -nrm;  // towards the sensor
        frame->normal[index(r, c)] = nrm.cast<float>();
        frame->has_normal[index(r, c)] = 1;
        continue;
      }
      double ru, rvv;
      if (!filter(r, c, true, rv, &ru) || !filter_rows(r, c, rv, &rvv)) continue;
      const Eigen::Vector3d dir = pixelDirection(r, c);
      const double theta = std::atan2(dir.y(), dir.x());
      const double sin_phi = dir.z(), cos_phi = std::sqrt(std::max(1e-12, 1.0 - sin_phi * sin_phi));
      const Eigen::Vector3d e_r = dir;
      const Eigen::Vector3d e_t(-std::sin(theta), std::cos(theta), 0.0);
      const Eigen::Vector3d e_p(-sin_phi * std::cos(theta), -sin_phi * std::sin(theta), cos_phi);
      // Surface normal of r(theta, phi) (eq. 11) with u = s theta + c_u and
      // v = c_v - s sin(phi): r_theta = s r_u, r_phi = -s cos(phi) r_v.
      nrm = e_r - (scale_ * ru / (rg * cos_phi)) * e_t + (scale_ * cos_phi * rvv / rg) * e_p;
      nrm.normalize();
      frame->normal[index(r, c)] = (-nrm).cast<float>();  // towards the sensor
      frame->has_normal[index(r, c)] = 1;
    }
  }
  // Planarity a = 1 - |dn/du|^2 - |dn/dv|^2.
  for (int r = 2; r < H - 2; ++r) {
    for (int c = 0; c < W; ++c) {
      if (!frame->has_normal[index(r, c)]) continue;
      double I = 0.0;
      bool ok = true;
      for (int k = 0; k < 3 && ok; ++k) {
        auto nv = [&](int rr, int cc, double* v) {
          if (!frame->has_normal[index(rr, cc)]) return false;
          *v = frame->normal[index(rr, cc)](k);
          return true;
        };
        double du, dv;
        ok = filter(r, c, true, nv, &du) && filter_rows(r, c, nv, &dv);
        I += du * du + dv * dv;
      }
      frame->planarity[index(r, c)] = ok ? static_cast<float>(std::clamp(1.0 - I, 0.0, 1.0)) : 0.0f;
    }
  }

  // Plane key points (Sec. 3.6): eleven lists, top K each, duplicates kept.
  std::vector<int> cand;
  for (size_t i = 0; i < n; ++i)
    if (frame->has_normal[i] && frame->planarity[i] > params_.min_planarity) cand.push_back(static_cast<int>(i));
  const int K = params_.keypoints_per_list;
  for (int list = 0; list < 11; ++list) {
    auto score = [&](int i) {
      const Eigen::Vector3d x = frame->point[i].cast<double>();
      const Eigen::Vector3d nn = frame->normal[i].cast<double>();
      const double a = frame->planarity[i];
      if (list < 6) {
        const double t = x.cross(nn)(list / 2);
        return a * (list % 2 == 0 ? t : -t);
      }
      if (list < 10) {
        const double t = nn((list - 6) / 2);
        return a * (list % 2 == 0 ? t : -t);
      }
      return a * nn.z();  // ground (normal towards the sensor points up)
    };
    std::vector<int> order = cand;
    const int k = std::min<int>(K, static_cast<int>(order.size()));
    std::partial_sort(order.begin(), order.begin() + k, order.end(),
                      [&](int a, int b) { return score(a) > score(b); });
    for (int i = 0; i < k; ++i) {
      frame->key_points.push_back(frame->point[order[i]].cast<double>());
      frame->key_normals.push_back(frame->normal[order[i]].cast<double>());
    }
  }
  return frame;
}

int DiloPaperOdometry::accumulate(const DiloPaperFrame& source, const DiloPaperFrame& reference,
                                  const Eigen::Matrix4d& T, int kernel,
                                  Eigen::Matrix<double, 6, 6>* H,
                                  Eigen::Matrix<double, 6, 1>* g) const {
  // T: reference <- source; the increment is applied on the right,
  // T exp(xi), i.e. in the source frame.
  const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  int pairs = 0;
  for (size_t i = 0; i < source.key_points.size(); ++i) {
    const Eigen::Vector3d& ps = source.key_points[i];
    const Eigen::Vector3d x = R * ps + t;
    int row, col;
    if (!project(x, &row, &col)) continue;
    const int idx = index(row, col);
    if (!reference.has_normal[idx]) continue;
    const Eigen::Vector3d xr = reference.point[idx].cast<double>();
    const Eigen::Vector3d nr = reference.normal[idx].cast<double>();
    const double cosw = nr.dot(R * source.key_normals[i]);
    if (cosw <= 0.0) continue;
    const double e = cosw * nr.dot(xr - x);
    double w;
    if (kernel == 0) {
      w = std::abs(e) <= params_.huber_delta ? 1.0 : params_.huber_delta / std::abs(e);
    } else {
      if (std::abs(e) >= params_.tukey_c) continue;
      const double q = 1.0 - (e / params_.tukey_c) * (e / params_.tukey_c);
      w = q * q;
    }
    const Eigen::Vector3d m = R.transpose() * nr;  // reference normal in the source frame
    Eigen::Matrix<double, 6, 1> J;
    J.head<3>() = -cosw * ps.cross(m);
    J.tail<3>() = -cosw * m;
    *H += w * J * J.transpose();
    *g += w * J * e;
    ++pairs;
  }
  return pairs;
}

std::vector<Eigen::Matrix4d> DiloPaperOdometry::windowPoses() const {
  std::vector<Eigen::Matrix4d> out;
  for (const auto& f : window_) out.push_back(f->pose);
  return out;
}

Eigen::Matrix4d DiloPaperOdometry::process(const std::vector<Eigen::Vector3d>& scan) {
  auto frame = buildFrame(scan);
  if (window_.empty()) {
    frame->pose = Eigen::Matrix4d::Identity();
    window_.push_back(frame);
    return frame->pose;
  }
  const Eigen::Matrix4d previous = window_.back()->pose;
  Eigen::Matrix4d prediction = last_delta_;
  if (params_.planar_prediction) {
    // Keep only the planar part of the last motion (x, y, yaw): the vertical
    // part of a frame-to-frame estimate is the noisiest and a pitch error
    // moves the projective association on the ground.
    const double yaw = std::atan2(last_delta_(1, 0), last_delta_(0, 0));
    prediction = Eigen::Matrix4d::Identity();
    prediction.block<3, 3>(0, 0) = Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()).toRotationMatrix();
    prediction(0, 3) = last_delta_(0, 3);
    prediction(1, 3) = last_delta_(1, 3);
  }
  frame->pose = previous * prediction;

  // Init: register the new frame against every queued frame (Huber).
  for (int it = 0; it < params_.init_iterations; ++it) {
    Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
    Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
    int pairs = 0;
    for (const auto& ref : window_)
      pairs += accumulate(*frame, *ref, inversePose(ref->pose) * frame->pose, 0, &H, &g);
    if (pairs < 6) break;
    const Eigen::Matrix<double, 6, 1> d = H.ldlt().solve(-g);
    if (!d.allFinite()) break;
    frame->pose = frame->pose * expPose(d);
  }
  window_.push_back(frame);

  // Update (Algorithm 1): pairwise Tukey steps, then the window poses are
  // averaged in the log map, with the oldest frame fixed.
  const int M = static_cast<int>(window_.size());
  if (M >= 3) {
    for (int it = 0; it < params_.update_iterations; ++it) {
      // rel[a][b]: measured pose of b in the frame of a.
      std::vector<std::vector<Eigen::Matrix4d>> rel(M, std::vector<Eigen::Matrix4d>(M, Eigen::Matrix4d::Identity()));
      for (int a = 0; a < M; ++a)
        for (int b = a + 1; b < M; ++b) {
          // Newer frame b registered into the older frame a.
          Eigen::Matrix4d T = inversePose(window_[a]->pose) * window_[b]->pose;
          Eigen::Matrix<double, 6, 6> H = Eigen::Matrix<double, 6, 6>::Zero();
          Eigen::Matrix<double, 6, 1> g = Eigen::Matrix<double, 6, 1>::Zero();
          if (accumulate(*window_[b], *window_[a], T, 1, &H, &g) >= 6) {
            const Eigen::Matrix<double, 6, 1> d = H.ldlt().solve(-g);
            if (d.allFinite()) T = T * expPose(d);
          }
          rel[a][b] = T;
          rel[b][a] = inversePose(T);
        }
      std::vector<Eigen::Matrix4d> next(M);
      next[0] = window_[0]->pose;
      for (int k = 1; k < M; ++k) {
        const Eigen::Matrix4d& cur = window_[k]->pose;
        const Eigen::Matrix4d cur_inv = inversePose(cur);
        Eigen::Matrix<double, 6, 1> mean = Eigen::Matrix<double, 6, 1>::Zero();
        for (int i = 0; i < M; ++i) {
          const Eigen::Matrix4d candidate = window_[0]->pose * rel[0][i] * rel[i][k];
          mean += logPose(cur_inv * candidate);
        }
        next[k] = cur * expPose(mean / M);
      }
      for (int k = 1; k < M; ++k) window_[k]->pose = next[k];
    }
  }

  const Eigen::Matrix4d causal = window_.back()->pose;
  last_delta_ = inversePose(window_[M - 2]->pose) * causal;
  while (static_cast<int>(window_.size()) > params_.window + 1) {
    finalized_.push_back(window_.front()->pose);
    window_.pop_front();
  }
  return causal;
}

}  // namespace dilo
}  // namespace localization_zoo
