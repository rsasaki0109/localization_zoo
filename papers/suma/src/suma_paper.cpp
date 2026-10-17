#include "suma/suma_paper.h"
#include "common/degeneracy.h"

#include <Eigen/Cholesky>

#include <algorithm>
#include <cmath>
#include <limits>

namespace localization_zoo {
namespace suma {

namespace {

constexpr double kPi = 3.14159265358979323846;

double deg2rad(double deg) { return deg * kPi / 180.0; }

// Bilinear (GL_LINEAR on a rectangle texture with a zero border) or nearest
// sample of a vertex / normal map at continuous image coordinates. Returns
// the interpolated validity (w) of both maps like the official shader.
struct ModelSample {
  Eigen::Vector3d vertex = Eigen::Vector3d::Zero();
  Eigen::Vector3d normal = Eigen::Vector3d::Zero();
  double vertex_w = 0.0;
  double normal_w = 0.0;
};

ModelSample sampleMap(const SuMaPaper::VertexNormalMap& map, double x, double y,
                      bool bilinear, bool normalize) {
  ModelSample s;
  if (!bilinear) {
    const int u = static_cast<int>(std::floor(x));
    const int v = static_cast<int>(std::floor(y));
    if (u < 0 || u >= map.width || v < 0 || v >= map.height) return s;
    const int i = map.index(u, v);
    if (map.vertex_valid[i]) {
      s.vertex = map.vertex[i].cast<double>();
      s.vertex_w = 1.0;
    }
    if (map.normal_valid[i]) {
      s.normal = map.normal[i].cast<double>();
      s.normal_w = 1.0;
    }
    return s;
  }
  const double fx = x - 0.5;
  const double fy = y - 0.5;
  const int u0 = static_cast<int>(std::floor(fx));
  const int v0 = static_cast<int>(std::floor(fy));
  const double ax = fx - u0;
  const double ay = fy - v0;
  for (int dv = 0; dv < 2; ++dv) {
    for (int du = 0; du < 2; ++du) {
      const int u = u0 + du;
      const int v = v0 + dv;
      if (u < 0 || u >= map.width || v < 0 || v >= map.height) continue;
      const double w = (du ? ax : 1.0 - ax) * (dv ? ay : 1.0 - ay);
      if (w <= 0.0) continue;
      const int i = map.index(u, v);
      if (map.vertex_valid[i]) {
        s.vertex += w * map.vertex[i].cast<double>();
        s.vertex_w += w;
      }
      if (map.normal_valid[i]) {
        s.normal += w * map.normal[i].cast<double>();
        s.normal_w += w;
      }
    }
  }
  if (normalize) {
    if (s.vertex_w > 0.0) s.vertex /= s.vertex_w;
    if (s.normal_w > 0.0) s.normal = s.normal.normalized();
  }
  return s;
}

Eigen::Vector3d slerpOfficial(const Eigen::Vector3d& v0, const Eigen::Vector3d& v1,
                              double weight) {
  // update_surfels.vert: "weight is actually (1 - t)", w0 belongs to v0.
  const double c = std::clamp(v0.normalized().dot(v1.normalized()), -1.0, 1.0);
  const double omega = std::acos(c);
  if (omega < 1e-6) return weight * v0 + (1.0 - weight) * v1;
  const double eta = 1.0 / std::sin(omega);
  return eta * std::sin(weight * omega) * v0 +
         eta * std::sin((1.0 - weight) * omega) * v1;
}

}  // namespace

Eigen::Matrix4d se3Exp(const Eigen::Matrix<double, 6, 1>& x) {
  Eigen::Matrix4d result = Eigen::Matrix4d::Identity();
  const Eigen::Vector3d v = x.head<3>();
  const Eigen::Vector3d omega = x.tail<3>();
  const double theta = omega.norm();
  if (theta > 1e-10) {
    Eigen::Matrix3d W;
    W << 0, -omega.z(), omega.y(), omega.z(), 0, -omega.x(), -omega.y(),
        omega.x(), 0;
    const Eigen::Matrix3d W2 = W * W;
    const double alpha = std::sin(theta) / theta;
    const double beta = (1.0 - std::cos(theta)) / (theta * theta);
    const double delta = (theta - std::sin(theta)) / (theta * theta * theta);
    result.topLeftCorner<3, 3>() = Eigen::Matrix3d::Identity() + alpha * W + beta * W2;
    result.block<3, 1>(0, 3) =
        (Eigen::Matrix3d::Identity() + beta * W + delta * W2) * v;
  } else {
    result.block<3, 1>(0, 3) = v;
  }
  return result;
}

void SuMaPaper::VertexNormalMap::reset(int w, int h) {
  width = w;
  height = h;
  const size_t n = static_cast<size_t>(w) * static_cast<size_t>(h);
  vertex.assign(n, Eigen::Vector3f::Zero());
  normal.assign(n, Eigen::Vector3f::Zero());
  vertex_valid.assign(n, 0);
  normal_valid.assign(n, 0);
}

SuMaPaper::SuMaPaper(const SuMaPaperParams& params) : params_(params) {
  last_data_.reset(params_.width, params_.height);
  model_.reset(params_.width, params_.height);
}

void SuMaPaper::clear() {
  surfels_.clear();
  last_data_.reset(params_.width, params_.height);
  model_.reset(params_.width, params_.height);
  pose_.setIdentity();
  last_increment_.setIdentity();
  timestamp_ = 0;
}

double SuMaPaper::pixelSize() const {
  // SurfelMap::setParameters: tangent of half the per-pixel angle.
  const double vfov = params_.fov_up_deg + params_.fov_down_deg;
  const double vpix = std::tan(0.5 * deg2rad(vfov) / params_.height);
  const double hpix = std::tan(0.5 * deg2rad(360.0) / params_.width);
  return std::max(vpix, hpix);
}

double SuMaPaper::logOdds(double p) const { return std::log(p / (1.0 - p)); }

float SuMaPaper::renderConfidenceThreshold() const {
  double ct = params_.confidence_threshold;
  if (timestamp_ < params_.time_init) {
    const double alpha = static_cast<double>(timestamp_) / params_.time_init;
    ct = (1.0 - alpha) * logOdds(params_.p_unstable_init) + alpha * ct;
  }
  return static_cast<float>(ct);
}

bool SuMaPaper::project(const Eigen::Vector3d& p, double* x, double* y) const {
  const double depth = p.norm();
  if (!(depth > 1e-9)) return false;
  const double yaw = std::atan2(p.y(), p.x());
  const double elevation_deg =
      std::asin(std::clamp(p.z() / depth, -1.0, 1.0)) * 180.0 / kPi;
  const double fov = params_.fov_up_deg + params_.fov_down_deg;
  *x = 0.5 * (1.0 - yaw / kPi) * params_.width;
  *y = (params_.fov_up_deg - elevation_deg) / fov * params_.height;
  return *x >= 0.0 && *x < params_.width && *y >= 0.0 && *y < params_.height;
}

SuMaPaper::VertexNormalMap SuMaPaper::computeVertexNormalMap(
    const std::vector<Eigen::Vector3d>& points) const {
  VertexNormalMap map;
  map.reset(params_.width, params_.height);
  std::vector<float> depth_buffer(map.vertex.size(),
                                  std::numeric_limits<float>::infinity());
  for (const auto& p : points) {
    if (!p.allFinite()) continue;
    const double depth = p.norm();
    if (depth < params_.min_depth || depth > params_.max_depth) continue;
    double x = 0.0;
    double y = 0.0;
    if (!project(p, &x, &y)) continue;
    const int u = static_cast<int>(std::floor(x));
    const int v = static_cast<int>(std::floor(y));
    const int i = map.index(u, v);
    if (depth < depth_buffer[i]) {
      depth_buffer[i] = static_cast<float>(depth);
      map.vertex[i] = p.cast<float>();
      map.vertex_valid[i] = 1;
    }
  }
  // gen_normalmap.frag: n = (V(u+1, v) - V) x (V(u, v_up) - V) with wrap in
  // u and no wrap in v; requires the pixel and both neighbours.
  for (int v = 0; v < map.height; ++v) {
    for (int u = 0; u < map.width; ++u) {
      const int i = map.index(u, v);
      if (!map.vertex_valid[i]) continue;
      if (v - 1 < 0) continue;
      const int iu = map.index((u + 1) % map.width, v);
      const int iv = map.index(u, v - 1);
      if (!map.vertex_valid[iu] || !map.vertex_valid[iv]) continue;
      const Eigen::Vector3f du = map.vertex[iu] - map.vertex[i];
      const Eigen::Vector3f dv = map.vertex[iv] - map.vertex[i];
      if (du.norm() < 1e-9f || dv.norm() < 1e-9f) continue;
      const Eigen::Vector3f w = du.normalized().cross(dv.normalized());
      const float len = w.norm();
      if (!(len > 1e-7f)) continue;
      map.normal[i] = w / len;
      map.normal_valid[i] = 1;
    }
  }
  return map;
}

SuMaPaper::VertexNormalMap SuMaPaper::renderModel(
    const Eigen::Matrix4d& T_world_sensor, float confidence_threshold,
    int* rendered_pixels) const {
  VertexNormalMap map;
  map.reset(params_.width, params_.height);
  std::vector<float> zbuffer(map.vertex.size(), std::numeric_limits<float>::infinity());
  const Eigen::Matrix3d R = T_world_sensor.topLeftCorner<3, 3>();
  const Eigen::Vector3d t = T_world_sensor.block<3, 1>(0, 3);
  const Eigen::Matrix3d Rt = R.transpose();
  const int active_from = timestamp_ - params_.active_timestamps;
  const double zrange = params_.model_max_depth - params_.model_min_depth;
  const double W = params_.width;

  auto corner = [&](const Eigen::Vector3d& c, double cx, double* x, double* y,
                    double* z) {
    const double d = c.norm();
    const double yaw = std::atan2(c.y(), c.x());
    const double elev = std::asin(std::clamp(c.z() / d, -1.0, 1.0)) * 180.0 / kPi;
    *x = 0.5 * (1.0 - yaw / kPi) * W;
    if (cx - *x > 0.5 * W) *x += W;
    if (*x - cx > 0.5 * W) *x -= W;
    *y = (params_.fov_up_deg - elev) / (params_.fov_up_deg + params_.fov_down_deg) *
         params_.height;
    *z = (d - params_.model_min_depth) / zrange;
  };

  for (const Surfel& s : surfels_) {
    if (params_.use_stability && !(s.confidence > confidence_threshold)) continue;
    if (s.created < active_from && s.updated < active_from) continue;
    const Eigen::Vector3d p = Rt * (s.position.cast<double>() - t);
    const Eigen::Vector3d n = Rt * s.normal.cast<double>();
    const double d = p.norm();
    if (!(d > 1e-6)) continue;
    if (!(n.dot(-p / d) > 0.01)) continue;
    double cx = 0.0;
    double cy = 0.0;
    if (!project(p, &cx, &cy)) continue;
    const double cz = (d - params_.model_min_depth) / zrange;
    if (cz < 0.0 || cz >= 1.0) continue;

    Eigen::Vector3d u(n.y() - n.z(), -n.x(), n.x());
    if (u.norm() < 1e-9) u = n.unitOrthogonal();
    u.normalize();
    const Eigen::Vector3d vv = n.cross(u).normalized();
    const double r = s.radius;
    const Eigen::Vector3d ru = r * u;
    const Eigen::Vector3d rv = r * vv;
    // Triangle strip (-,-), (+,-), (-,+), (+,+) like render_surfels.geom.
    const Eigen::Vector3d cs[4] = {p - ru - rv, p + ru - rv, p - ru + rv, p + ru + rv};
    const double ts[4][2] = {{-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    double xs[4], ys[4], zs[4];
    for (int k = 0; k < 4; ++k) corner(cs[k], cx, &xs[k], &ys[k], &zs[k]);

    const double minx = std::min({xs[0], xs[1], xs[2], xs[3]});
    const double maxx = std::max({xs[0], xs[1], xs[2], xs[3]});
    const double miny = std::min({ys[0], ys[1], ys[2], ys[3]});
    const double maxy = std::max({ys[0], ys[1], ys[2], ys[3]});
    const int u_begin = std::max(0, static_cast<int>(std::ceil(minx - 0.5)));
    const int u_end = std::min(params_.width - 1, static_cast<int>(std::floor(maxx - 0.5)));
    const int v_begin = std::max(0, static_cast<int>(std::ceil(miny - 0.5)));
    const int v_end = std::min(params_.height - 1, static_cast<int>(std::floor(maxy - 0.5)));
    if (u_begin > u_end || v_begin > v_end) continue;

    const int tris[2][3] = {{0, 1, 2}, {1, 3, 2}};
    const Eigen::Vector3f pf = p.cast<float>();
    const Eigen::Vector3f nf = n.cast<float>();
    for (int pv = v_begin; pv <= v_end; ++pv) {
      for (int pu = u_begin; pu <= u_end; ++pu) {
        const double qx = pu + 0.5;
        const double qy = pv + 0.5;
        for (const auto& tri : tris) {
          const int a = tri[0], b = tri[1], c = tri[2];
          const double det = (xs[b] - xs[a]) * (ys[c] - ys[a]) -
                             (xs[c] - xs[a]) * (ys[b] - ys[a]);
          if (std::abs(det) < 1e-12) continue;
          const double l1 = ((qx - xs[a]) * (ys[c] - ys[a]) - (xs[c] - xs[a]) * (qy - ys[a])) / det;
          const double l2 = ((xs[b] - xs[a]) * (qy - ys[a]) - (qx - xs[a]) * (ys[b] - ys[a])) / det;
          const double l0 = 1.0 - l1 - l2;
          if (l0 < 0.0 || l1 < 0.0 || l2 < 0.0) continue;
          const double tu = l0 * ts[a][0] + l1 * ts[b][0] + l2 * ts[c][0];
          const double tv = l0 * ts[a][1] + l1 * ts[b][1] + l2 * ts[c][1];
          if (tu * tu + tv * tv > 1.0) break;  // fragment discarded
          const double z = l0 * zs[a] + l1 * zs[b] + l2 * zs[c];
          if (z < 0.0 || z > 1.0) break;
          const int i = map.index(pu, pv);
          if (z < zbuffer[i]) {
            zbuffer[i] = static_cast<float>(z);
            map.vertex[i] = pf;
            map.normal[i] = nf;
            map.vertex_valid[i] = 1;
            map.normal_valid[i] = 1;
          }
          break;
        }
      }
    }
  }
  if (rendered_pixels != nullptr) {
    int count = 0;
    for (uint8_t valid : map.vertex_valid) count += valid;
    *rendered_pixels = count;
  }
  return map;
}

void SuMaPaper::accumulate(const VertexNormalMap& data, const VertexNormalMap& model,
                           const Eigen::Matrix4d& pose, double max_distance,
                           double cos_max_angle, Normal6* out) const {
  out->JtJ.setZero();
  out->Jtf.setZero();
  out->error = 0.0;
  out->inliers = 0;
  out->outliers = 0;
  const Eigen::Matrix3d R = pose.topLeftCorner<3, 3>();
  const Eigen::Vector3d t = pose.block<3, 1>(0, 3);
  const double k = params_.huber_factor;
  for (int i = 0; i < static_cast<int>(data.vertex.size()); ++i) {
    if (!data.valid(i)) continue;
    const Eigen::Vector3d v_d = R * data.vertex[i].cast<double>() + t;
    const Eigen::Vector3d n_d = R * data.normal[i].cast<double>();
    double x = 0.0;
    double y = 0.0;
    if (!project(v_d, &x, &y)) continue;
    const ModelSample m = sampleMap(model, x, y, params_.bilinear_sampling,
                                     params_.normalize_bilinear);
    if (!(m.vertex_w + m.normal_w > 1.5)) continue;
    bool inlier = true;
    if ((m.vertex - v_d).norm() > max_distance) inlier = false;
    if (m.normal.dot(n_d) < cos_max_angle) inlier = false;
    const double residual = m.normal.dot(v_d - m.vertex);
    double weight = 1.0;
    if (std::abs(residual) > k) weight = k / std::abs(residual);
    out->error += weight * residual * residual;
    if (!inlier) {
      ++out->outliers;
      continue;
    }
    Eigen::Matrix<double, 6, 1> J;
    J.head<3>() = m.normal;
    J.tail<3>() = v_d.cross(m.normal);
    out->JtJ.noalias() += weight * J * J.transpose();
    out->Jtf.noalias() += weight * residual * J;
    ++out->inliers;
  }
}

Eigen::Matrix4d SuMaPaper::alignProjective(const VertexNormalMap& data,
                                           const VertexNormalMap& model,
                                           const Eigen::Matrix4d& initial,
                                           double max_distance, double max_angle_deg,
                                           IcpStats* stats) const {
  Eigen::Matrix4d T = initial;
  const double cos_max_angle = std::cos(deg2rad(max_angle_deg));
  double last_error = std::numeric_limits<float>::max();
  Normal6 eq;
  eq.JtJ.setZero();
  eq.Jtf.setZero();
  int iterations = 0;
  for (int k = 0; k < params_.max_iterations; ++k) {
    accumulate(data, model, T, max_distance, cos_max_angle, &eq);
    if (eq.inliers < 6) break;
    Eigen::Matrix<double, 6, 1> dx = eq.JtJ.ldlt().solve(-eq.Jtf);
    if (!dx.allFinite()) break;
    if (params_.degenerate_ratio > 0.0)
      math::remapDegenerateTranslation(eq.JtJ.topLeftCorner<3, 3>(), params_.degenerate_ratio, dx.head<3>());
    bool converged = false;
    if (dx.lpNorm<Eigen::Infinity>() < params_.stop_delta) converged = true;
    if (std::abs(eq.Jtf.maxCoeff()) < params_.stop_epsilon) converged = true;
    if (eq.error < last_error && std::abs(eq.error - last_error) < params_.stop_epsilon)
      converged = true;
    T = se3Exp(dx) * T;
    last_error = eq.error;
    ++iterations;
    if (converged) break;
  }
  if (stats != nullptr) {
    stats->inliers = eq.inliers;
    stats->outliers = eq.outliers;
    stats->iterations = iterations;
    stats->error = eq.error;
    stats->JtJ = eq.JtJ;
  }
  return T;
}

void SuMaPaper::updateMap(const VertexNormalMap& data, const Eigen::Matrix4d& T,
                          SuMaPaperResult* result) {
  const size_t npix = data.vertex.size();
  const double pixel_size = pixelSize();
  const double cos_max_angle = std::cos(deg2rad(params_.max_angle_deg));

  // init_radiusConf: radius of a potential new surfel per pixel (eq. 10).
  std::vector<float> radius(npix, 0.0f);
  std::vector<uint8_t> radius_valid(npix, 0);
  for (size_t i = 0; i < npix; ++i) {
    if (!data.valid(static_cast<int>(i))) continue;
    const Eigen::Vector3d v = data.vertex[i].cast<double>();
    const Eigen::Vector3d n = data.normal[i].cast<double>();
    const double d = v.norm();
    const double c = n.dot(-v / d);
    if (!(c > cos_max_angle)) continue;
    radius_valid[i] = 1;
    double r = 1.41 * d * pixel_size / std::clamp(c, 0.5, 1.0);
    radius[i] = static_cast<float>(std::clamp(r, params_.min_radius, params_.max_radius));
  }

  const Eigen::Matrix3d R = T.topLeftCorner<3, 3>();
  const Eigen::Vector3d t = T.block<3, 1>(0, 3);
  const Eigen::Matrix3d Rt = R.transpose();
  const double depth_range = params_.max_depth - params_.min_depth;

  // Local coordinates and pixel of every surfel at the new pose.
  const size_t ns = surfels_.size();
  std::vector<int> pixel(ns, -1);
  std::vector<uint8_t> visible(ns, 0);
  std::vector<int> index_map(npix, -1);
  std::vector<float> index_depth(npix, std::numeric_limits<float>::infinity());
  for (size_t k = 0; k < ns; ++k) {
    const Surfel& s = surfels_[k];
    const Eigen::Vector3d p = Rt * (s.position.cast<double>() - t);
    const Eigen::Vector3d n = (Rt * s.normal.cast<double>()).normalized();
    const double d = p.norm();
    if (!(d > 1e-6)) continue;
    const double facing = n.dot(-p / d);
    double x = 0.0;
    double y = 0.0;
    if (!project(p, &x, &y)) continue;
    const int px = data.index(static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y)));
    const double z = (d - params_.min_depth) / depth_range;
    // update_surfels.vert: inside = x < w, y < h, depth below max_depth.
    if (z < 1.0) {
      pixel[k] = px;
      visible[k] = facing > 0.0;
    }
    // gen_indexmap.vert: depth-buffered nearest surfel per pixel.
    if (facing > 0.01 && z >= 0.0 && z <= 1.0 && z < index_depth[px]) {
      index_depth[px] = static_cast<float>(z);
      index_map[px] = static_cast<int>(k);
    }
  }

  const double log_prior = logOdds(params_.p_prior);
  const double p_unstable = 1.0 - params_.p_stable;
  const double log_unstable = logOdds(p_unstable);
  const double sin_map_angle = std::sin(deg2rad(params_.map_max_angle_deg));
  std::vector<uint8_t> integrated(npix, 0);
  std::vector<uint8_t> keep(ns, 1);
  int updated = 0;
  for (size_t k = 0; k < ns; ++k) {
    Surfel& s = surfels_[k];
    const double old_confidence = s.confidence;
    bool valid = true;
    if (old_confidence < params_.confidence_threshold && params_.use_stability)
      valid = (timestamp_ - s.updated) < params_.unstable_age;
    double update_confidence = log_prior;
    const int px = pixel[k];
    if (px >= 0 && visible[k] && data.valid(px)) {
      const Eigen::Vector3d old_position = s.position.cast<double>();
      const Eigen::Vector3d old_normal = s.normal.cast<double>();
      const Eigen::Vector3d v_global = R * data.vertex[px].cast<double>() + t;
      const Eigen::Vector3d n_global = (R * data.normal[px].cast<double>()).normalized();
      const double distance = std::abs(old_normal.dot(v_global - old_position));
      const double angle = n_global.cross(old_normal).norm();
      const double new_radius = radius[px];
      if (distance < params_.map_max_distance && angle < sin_map_angle) {
        integrated[px] = 1;
        s.updated = timestamp_;
        const double old_radius = s.radius;
        s.radius = static_cast<float>(std::max(std::min(new_radius, old_radius), 0.0));
        valid = true;
        double p = params_.p_stable;
        p *= std::exp(-angle * angle / (params_.sigma_angle * params_.sigma_angle));
        p *= std::exp(-distance * distance /
                      (params_.sigma_distance * params_.sigma_distance));
        p = std::clamp(p, p_unstable, 1.0);
        update_confidence = logOdds(p);
        if (new_radius < old_radius &&
            timestamp_ - s.created < params_.active_timestamps) {
          const double w1 = params_.gamma;
          const double w2 = 1.0 - params_.gamma;
          s.position = (w1 * old_position + w2 * v_global).cast<float>();
          s.normal = slerpOfficial(old_normal, n_global, w1).normalized().cast<float>();
        }
        ++updated;
      } else if (index_map[px] == static_cast<int>(k)) {
        update_confidence = log_unstable;
      }
    }
    if (params_.use_stability) {
      s.confidence = static_cast<float>(std::min(
          old_confidence + update_confidence - log_prior, params_.max_confidence));
      if (s.confidence < log_unstable) valid = false;
    }
    keep[k] = valid;
  }

  // gen_surfels: new surfels for valid pixels without a compatible surfel.
  int created = 0;
  std::vector<Surfel> fresh;
  for (size_t i = 0; i < npix; ++i) {
    if (!data.valid(static_cast<int>(i)) || !radius_valid[i] || integrated[i]) continue;
    const Eigen::Vector3d v = data.vertex[i].cast<double>();
    const Eigen::Vector3d n = data.normal[i].cast<double>().normalized();
    if (!(n.dot(-v / v.norm()) > 0.01)) continue;
    Surfel s;
    s.position = (R * v + t).cast<float>();
    s.normal = (R * n).cast<float>();
    s.radius = radius[i];
    s.confidence = static_cast<float>(log_prior);
    s.created = timestamp_;
    s.updated = timestamp_;
    fresh.push_back(s);
    ++created;
  }

  // copy_surfels: keep valid surfels inside the active area.
  const int active_from = timestamp_ + 1 - params_.active_timestamps;
  std::vector<Surfel> next;
  next.reserve(ns + fresh.size());
  for (size_t k = 0; k < ns; ++k) {
    if (!keep[k]) continue;
    const Surfel& s = surfels_[k];
    if (std::abs(s.position.x() - t.x()) > params_.map_extent ||
        std::abs(s.position.y() - t.y()) > params_.map_extent)
      continue;
    if (!params_.keep_inactive && s.created < active_from && s.updated < active_from)
      continue;
    next.push_back(s);
  }
  next.insert(next.end(), fresh.begin(), fresh.end());
  surfels_.swap(next);

  if (result != nullptr) {
    result->new_surfels = created;
    result->updated_surfels = updated;
    result->num_surfels = surfels_.size();
  }
}

SuMaPaperResult SuMaPaper::process(const aloam::PointCloudConstPtr& cloud) {
  std::vector<Eigen::Vector3d> points;
  if (cloud != nullptr) {
    points.reserve(cloud->size());
    for (const auto& p : cloud->points) points.emplace_back(p.x, p.y, p.z);
  }
  return process(points);
}

SuMaPaperResult SuMaPaper::process(const std::vector<Eigen::Vector3d>& points) {
  SuMaPaperResult result;
  VertexNormalMap data = computeVertexNormalMap(points);
  for (size_t i = 0; i < data.vertex.size(); ++i)
    result.valid_pixels += data.valid(static_cast<int>(i));

  Eigen::Matrix4d increment = Eigen::Matrix4d::Identity();
  if (timestamp_ > 0) {
    model_ = renderModel(pose_, renderConfidenceThreshold(), &result.model_pixels);
    const Eigen::Matrix4d T0 =
        params_.initialize_identity ? Eigen::Matrix4d::Identity() : last_increment_;
    IcpStats stats;
    increment = alignProjective(data, model_, T0, params_.icp_max_distance,
                                params_.icp_max_angle_deg, &stats);
    const Eigen::Matrix4d delta = last_increment_.inverse() * increment;
    const double t_err = delta.block<3, 1>(0, 3).norm();
    const double c = std::clamp(0.5 * (delta.topLeftCorner<3, 3>().trace() - 1.0), -1.0, 1.0);
    const double r_err = std::acos(c);
    if (timestamp_ > 1 && params_.fallback_mode &&
        (t_err > params_.fallback_trans_threshold || r_err > params_.fallback_rot_threshold)) {
      increment = alignProjective(data, last_data_, T0, params_.fallback_max_distance,
                                  params_.fallback_max_angle_deg, &stats);
      result.fallback = true;
    }
    result.inliers = stats.inliers;
    result.outliers = stats.outliers;
    result.iterations = stats.iterations;
    pose_ = pose_ * increment;
    last_increment_ = increment;
  }

  updateMap(data, pose_, &result);
  last_data_ = std::move(data);
  ++timestamp_;

  result.pose = pose_;
  result.increment = increment;
  result.num_surfels = surfels_.size();
  return result;
}

}  // namespace suma
}  // namespace localization_zoo
