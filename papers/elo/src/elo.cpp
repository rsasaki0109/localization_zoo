#include "elo/elo.h"

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace localization_zoo {
namespace elo {

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;

Eigen::Matrix3d skew(const Eigen::Vector3d& v) {
  Eigen::Matrix3d m;
  m << 0.0, -v.z(), v.y(),
       v.z(), 0.0, -v.x(),
       -v.y(), v.x(), 0.0;
  return m;
}

Eigen::Matrix3d expSo3(const Eigen::Vector3d& w) {
  const double theta = w.norm();
  if (theta < 1e-12) return Eigen::Matrix3d::Identity() + skew(w);
  const Eigen::Matrix3d K = skew(w / theta);
  return Eigen::Matrix3d::Identity() + std::sin(theta) * K +
         (1.0 - std::cos(theta)) * K * K;
}

Eigen::Matrix4d expSe3(const Eigen::Matrix<double, 6, 1>& xi) {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T.block<3, 3>(0, 0) = expSo3(xi.head<3>());
  T.block<3, 1>(0, 3) = xi.tail<3>();
  return T;
}

bool fitNormal(const std::vector<Eigen::Vector3d>& points,
               const Eigen::Vector3d& orient_reference, int min_neighbors,
               double plane_outlier_threshold, Eigen::Vector3d* normal,
               double* curvature) {
  if (static_cast<int>(points.size()) < min_neighbors) return false;

  auto solve = [](const std::vector<Eigen::Vector3d>& pts, Eigen::Vector3d* n,
                  double* c) {
    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (const auto& p : pts) mean += p;
    mean /= static_cast<double>(pts.size());

    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
    for (const auto& p : pts) {
      const Eigen::Vector3d d = p - mean;
      cov += d * d.transpose();
    }
    cov /= static_cast<double>(pts.size());

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(cov);
    if (es.info() != Eigen::Success) return false;
    const Eigen::Vector3d evals = es.eigenvalues();
    *n = es.eigenvectors().col(0);
    const double sum = evals.sum();
    *c = sum > 1e-12 ? evals(0) / sum : 1.0;
    return n->allFinite();
  };

  Eigen::Vector3d n;
  double c = 1.0;
  if (!solve(points, &n, &c)) return false;

  std::vector<Eigen::Vector3d> inliers;
  inliers.reserve(points.size());
  Eigen::Vector3d mean = Eigen::Vector3d::Zero();
  for (const auto& p : points) mean += p;
  mean /= static_cast<double>(points.size());
  for (const auto& p : points) {
    if (std::abs(n.dot(p - mean)) <= plane_outlier_threshold) {
      inliers.push_back(p);
    }
  }
  if (static_cast<int>(inliers.size()) >= min_neighbors &&
      inliers.size() + 2 < points.size()) {
    if (!solve(inliers, &n, &c)) return false;
  }

  if (n.dot(orient_reference) < 0.0) n = -n;
  *normal = n.normalized();
  *curvature = c;
  return true;
}

}  // namespace

EloPipeline::EloPipeline(const EloParams& params) : params_(params) {
  sri_model_.assign(static_cast<size_t>(params_.sri_width) * params_.sri_height,
                    ModelCell());
  bev_model_.assign(static_cast<size_t>(bevWidth()) * bevHeight(), ModelCell());
}

void EloPipeline::reset() {
  frame_count_ = 0;
  pose_.setIdentity();
  model_pose_.setIdentity();
  last_delta_.setIdentity();
  initialized_ = false;
  std::fill(sri_model_.begin(), sri_model_.end(), ModelCell());
  std::fill(bev_model_.begin(), bev_model_.end(), ModelCell());
}

void EloPipeline::setInitialPose(const Eigen::Matrix4d& pose) {
  pose_ = pose;
  model_pose_ = pose;
}

size_t EloPipeline::nonGroundMapSize() const {
  return static_cast<size_t>(std::count_if(
      sri_model_.begin(), sri_model_.end(),
      [](const ModelCell& c) { return c.valid && c.has_normal; }));
}

size_t EloPipeline::groundMapSize() const {
  return static_cast<size_t>(std::count_if(
      bev_model_.begin(), bev_model_.end(),
      [](const ModelCell& c) { return c.valid; }));
}

int EloPipeline::sriIndex(int row, int col) const {
  col = ((col % params_.sri_width) + params_.sri_width) % params_.sri_width;
  return row * params_.sri_width + col;
}

int EloPipeline::bevWidth() const {
  return std::max(1, static_cast<int>(
                         std::ceil((params_.bev_x_max - params_.bev_x_min) /
                                   params_.bev_resolution)));
}

int EloPipeline::bevHeight() const {
  return std::max(1, static_cast<int>(
                         std::ceil((params_.bev_y_max - params_.bev_y_min) /
                                   params_.bev_resolution)));
}

int EloPipeline::bevIndex(int row, int col) const {
  return row * bevWidth() + col;
}

bool EloPipeline::projectSri(const Eigen::Vector3d& p, int* row, int* col) const {
  const double range = p.norm();
  if (!std::isfinite(range) || range < params_.min_range || range > params_.max_range) {
    return false;
  }
  const double yaw = std::atan2(p.y(), p.x());
  const double pitch = std::asin(std::clamp(p.z() / range, -1.0, 1.0));
  const double fov_up = params_.fov_up_deg * kDegToRad;
  const double fov_down = params_.fov_down_deg * kDegToRad;
  const double fov = fov_up - fov_down;
  if (fov <= 1e-9 || pitch > fov_up || pitch < fov_down) return false;

  const double u = 0.5 * (yaw / kPi + 1.0) * params_.sri_width;
  const double v = (1.0 - (pitch - fov_down) / fov) * params_.sri_height;
  int ci = static_cast<int>(std::floor(u));
  int ri = static_cast<int>(std::floor(v));
  if (ri < 0 || ri >= params_.sri_height) return false;
  ci = ((ci % params_.sri_width) + params_.sri_width) % params_.sri_width;
  *row = ri;
  *col = ci;
  return true;
}

bool EloPipeline::projectBev(const Eigen::Vector3d& p, int* row, int* col) const {
  if (p.x() < params_.bev_x_min || p.x() >= params_.bev_x_max ||
      p.y() < params_.bev_y_min || p.y() >= params_.bev_y_max) {
    return false;
  }
  *col = static_cast<int>(std::floor((p.x() - params_.bev_x_min) /
                                     params_.bev_resolution));
  *row = static_cast<int>(std::floor((p.y() - params_.bev_y_min) /
                                     params_.bev_resolution));
  return *row >= 0 && *row < bevHeight() && *col >= 0 && *col < bevWidth();
}

EloPipeline::FrameData EloPipeline::buildFrame(
    const std::vector<Eigen::Vector3d>& points) const {
  FrameData frame;
  frame.sri.assign(static_cast<size_t>(params_.sri_width) * params_.sri_height,
                   ScanCell());

  for (const auto& p : points) {
    int row = 0, col = 0;
    if (!projectSri(p, &row, &col)) continue;
    const double range = p.norm();
    ScanCell& cell = frame.sri[static_cast<size_t>(sriIndex(row, col))];
    if (!cell.valid || range < cell.range) {
      cell.point = p;
      cell.range = range;
      cell.valid = true;
    }
  }

  segmentGround(&frame);
  computeRangeAdaptiveNormals(&frame);
  collectFramePoints(&frame);
  return frame;
}

void EloPipeline::segmentGround(FrameData* frame) const {
  const double ground_z = -params_.sensor_height;
  const double angle_threshold = params_.ground_angle_threshold_deg * kDegToRad;
  const int H = params_.sri_height;
  const int W = params_.sri_width;

  for (int r = 0; r < H; ++r) {
    for (int c = 0; c < W; ++c) {
      ScanCell& cell = frame->sri[static_cast<size_t>(sriIndex(r, c))];
      if (!cell.valid) continue;
      if (cell.point.z() < ground_z - params_.ground_height_below ||
          cell.point.z() > ground_z + params_.ground_height_above) {
        continue;
      }

      bool has_slope = false;
      bool flat = false;
      for (int dir : {-1, 1}) {
        for (int step = 1; step <= 6; ++step) {
          const int rr = r + dir * step;
          if (rr < 0 || rr >= H) break;
          const ScanCell& nb = frame->sri[static_cast<size_t>(sriIndex(rr, c))];
          if (!nb.valid) continue;
          has_slope = true;
          const Eigen::Vector3d d = nb.point - cell.point;
          const double horiz = std::hypot(d.x(), d.y());
          if (horiz > 1e-6 && std::abs(std::atan2(d.z(), horiz)) < angle_threshold) {
            flat = true;
          }
          break;
        }
      }
      cell.ground = !has_slope || flat;
    }
  }
}

void EloPipeline::computeRangeAdaptiveNormals(FrameData* frame) const {
  const int H = params_.sri_height;
  const int W = params_.sri_width;
  const double fov =
      (params_.fov_up_deg - params_.fov_down_deg) * kDegToRad;

  for (int r = 0; r < H; ++r) {
    for (int c = 0; c < W; ++c) {
      ScanCell& center = frame->sri[static_cast<size_t>(sriIndex(r, c))];
      if (!center.valid || center.ground) continue;

      const double range = std::max(center.range, params_.min_range);
      const int half_x = std::max(
          params_.normal_lx_min,
          std::min(params_.normal_lx_max,
                   static_cast<int>(std::ceil(params_.normal_range_delta /
                                              (range * kPi) * W))));
      const int half_y = std::max(
          params_.normal_ly_min,
          std::min(params_.normal_ly_max,
                   static_cast<int>(std::ceil(params_.normal_range_delta /
                                              (range * fov) * H))));
      std::vector<Eigen::Vector3d> neighbors;
      neighbors.reserve(static_cast<size_t>((2 * half_x + 1) * (2 * half_y + 1)));
      int far_count = 0;
      int valid_count = 0;
      for (int dr = -half_y; dr <= half_y; ++dr) {
        const int rr = r + dr;
        if (rr < 0 || rr >= H) continue;
        for (int dc = -half_x; dc <= half_x; ++dc) {
          const int cc = c + dc;
          const ScanCell& nb =
              frame->sri[static_cast<size_t>(sriIndex(rr, cc))];
          if (!nb.valid || nb.ground) continue;
          ++valid_count;
          if ((nb.point - center.point).norm() >
              params_.normal_range_outlier_threshold) {
            ++far_count;
            continue;
          }
          neighbors.push_back(nb.point);
        }
      }
      if (valid_count > 0 && far_count * 2 > valid_count) continue;

      Eigen::Vector3d normal;
      double curvature = 1.0;
      if (!fitNormal(neighbors, -center.point, params_.normal_min_neighbors,
                     params_.normal_plane_outlier_threshold, &normal,
                     &curvature)) {
        continue;
      }
      center.normal = normal;
      center.curvature = curvature;
      center.has_normal = curvature <= params_.curvature_threshold;
    }
  }
}

void EloPipeline::collectFramePoints(FrameData* frame) const {
  frame->points.clear();
  frame->ground_points = 0;
  frame->non_ground_points = 0;
  frame->points.reserve(frame->sri.size() / 2);
  for (const auto& cell : frame->sri) {
    if (!cell.valid) continue;
    if (cell.ground) {
      FramePoint fp;
      fp.point = cell.point;
      fp.ground = true;
      frame->points.push_back(fp);
      ++frame->ground_points;
    } else if (cell.has_normal) {
      FramePoint fp;
      fp.point = cell.point;
      fp.normal = cell.normal;
      fp.has_normal = true;
      fp.ground = false;
      frame->points.push_back(fp);
      ++frame->non_ground_points;
    }
  }

  if (params_.max_alignment_points > 0 &&
      static_cast<int>(frame->points.size()) > params_.max_alignment_points) {
    std::vector<FramePoint> limited;
    limited.reserve(static_cast<size_t>(params_.max_alignment_points));
    const double step = static_cast<double>(frame->points.size()) /
                        static_cast<double>(params_.max_alignment_points);
    for (int i = 0; i < params_.max_alignment_points; ++i) {
      const size_t idx = std::min(static_cast<size_t>(i * step),
                                  frame->points.size() - 1);
      limited.push_back(frame->points[idx]);
    }
    frame->points.swap(limited);
  }
}

bool EloPipeline::lookupSri(const Eigen::Vector3d& p,
                            Correspondence* corr) const {
  int row = 0, col = 0;
  if (!projectSri(p, &row, &col)) return false;
  const int radius = std::max(0, params_.sri_search_radius_px);
  double best_dist2 = params_.max_sri_correspondence_dist *
                      params_.max_sri_correspondence_dist;
  const ModelCell* best = nullptr;
  for (int dr = -radius; dr <= radius; ++dr) {
    const int rr = row + dr;
    if (rr < 0 || rr >= params_.sri_height) continue;
    for (int dc = -radius; dc <= radius; ++dc) {
      const ModelCell& cell =
          sri_model_[static_cast<size_t>(sriIndex(rr, col + dc))];
      if (!cell.valid || !cell.has_normal) continue;
      const double dist2 = (cell.point - p).squaredNorm();
      if (dist2 < best_dist2) {
        best_dist2 = dist2;
        best = &cell;
      }
    }
  }
  if (best == nullptr) return false;
  corr->point = best->point;
  corr->normal = best->normal;
  corr->ground = false;
  return true;
}

bool EloPipeline::estimateGroundNormal(int row, int col,
                                       Eigen::Vector3d* normal) const {
  struct Candidate {
    double d2 = 0.0;
    Eigen::Vector3d p = Eigen::Vector3d::Zero();
  };
  const int W = bevWidth();
  const int H = bevHeight();
  const int radius = std::max(params_.ground_search_radius_cells,
                              params_.ground_normal_radius_cells);
  std::vector<Candidate> candidates;
  candidates.reserve(static_cast<size_t>((2 * radius + 1) * (2 * radius + 1)));
  const Eigen::Vector2d center_xy(
      params_.bev_x_min + (col + 0.5) * params_.bev_resolution,
      params_.bev_y_min + (row + 0.5) * params_.bev_resolution);
  for (int dr = -radius; dr <= radius; ++dr) {
    const int rr = row + dr;
    if (rr < 0 || rr >= H) continue;
    for (int dc = -radius; dc <= radius; ++dc) {
      const int cc = col + dc;
      if (cc < 0 || cc >= W) continue;
      const ModelCell& cell =
          bev_model_[static_cast<size_t>(bevIndex(rr, cc))];
      if (!cell.valid) continue;
      const Eigen::Vector2d d = cell.point.head<2>() - center_xy;
      candidates.push_back({d.squaredNorm(), cell.point});
    }
  }
  if (static_cast<int>(candidates.size()) < params_.normal_min_neighbors) {
    return false;
  }
  std::sort(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) { return a.d2 < b.d2; });
  std::vector<Eigen::Vector3d> pts;
  const int keep = std::min<int>(static_cast<int>(candidates.size()),
                                 std::max(5, params_.normal_min_neighbors));
  pts.reserve(static_cast<size_t>(keep));
  for (int i = 0; i < keep; ++i) pts.push_back(candidates[static_cast<size_t>(i)].p);
  double curvature = 1.0;
  if (!fitNormal(pts, Eigen::Vector3d::UnitZ(), params_.normal_min_neighbors,
                 params_.normal_plane_outlier_threshold, normal, &curvature)) {
    return false;
  }
  if (normal->z() < 0.0) *normal = -*normal;
  return true;
}

bool EloPipeline::lookupGround(const Eigen::Vector3d& p,
                               Correspondence* corr) const {
  int row = 0, col = 0;
  if (!projectBev(p, &row, &col)) return false;
  const int W = bevWidth();
  const int H = bevHeight();
  const int radius = std::max(0, params_.ground_search_radius_cells);
  double best_dist2 = params_.max_ground_correspondence_dist *
                      params_.max_ground_correspondence_dist;
  const ModelCell* best = nullptr;
  for (int dr = -radius; dr <= radius; ++dr) {
    const int rr = row + dr;
    if (rr < 0 || rr >= H) continue;
    for (int dc = -radius; dc <= radius; ++dc) {
      const int cc = col + dc;
      if (cc < 0 || cc >= W) continue;
      const ModelCell& cell =
          bev_model_[static_cast<size_t>(bevIndex(rr, cc))];
      if (!cell.valid) continue;
      const double dist2 = (cell.point - p).squaredNorm();
      if (dist2 < best_dist2) {
        best_dist2 = dist2;
        best = &cell;
      }
    }
  }
  if (best == nullptr) return false;

  Eigen::Vector3d normal;
  if (!estimateGroundNormal(row, col, &normal)) return false;
  corr->point = best->point;
  corr->normal = normal;
  corr->ground = true;
  return true;
}

Eigen::Matrix4d EloPipeline::align(const FrameData& frame,
                                   const Eigen::Matrix4d& init,
                                   EloResult* result) const {
  using Vec6 = Eigen::Matrix<double, 6, 1>;
  using Mat6 = Eigen::Matrix<double, 6, 6>;

  Eigen::Matrix4d T = init;
  int last_corr = 0;
  int last_ground = 0;
  int last_non_ground = 0;
  double last_abs_residual_sum = 0.0;

  for (int iter = 0; iter < params_.max_iterations; ++iter) {
    const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
    const Eigen::Vector3d t = T.block<3, 1>(0, 3);
    Mat6 A = Mat6::Zero();
    Vec6 b = Vec6::Zero();
    int corr_count = 0;
    int ground_count = 0;
    int non_ground_count = 0;
    double abs_residual_sum = 0.0;

    for (const auto& fp : frame.points) {
      const Eigen::Vector3d p_model = R * fp.point + t;
      Correspondence corr;
      const bool ok = fp.ground ? lookupGround(p_model, &corr) : lookupSri(p_model, &corr);
      if (!ok) continue;

      const double residual = corr.normal.dot(p_model - corr.point);
      const double max_dist =
          fp.ground ? params_.max_ground_correspondence_dist
                    : params_.max_sri_correspondence_dist;
      if (std::abs(residual) > max_dist) continue;

      Eigen::Matrix<double, 3, 6> Jp;
      Jp.block<3, 3>(0, 0) = -skew(p_model);
      Jp.block<3, 3>(0, 3) = Eigen::Matrix3d::Identity();
      const Eigen::Matrix<double, 1, 6> J = corr.normal.transpose() * Jp;

      double weight = fp.ground ? params_.ground_weight : params_.non_ground_weight;
      if (params_.robust_scale > 1e-9) {
        const double s = residual / params_.robust_scale;
        weight /= (1.0 + s * s);
      }
      A += weight * J.transpose() * J;
      b -= weight * J.transpose() * residual;

      ++corr_count;
      if (fp.ground) {
        ++ground_count;
      } else {
        ++non_ground_count;
      }
      abs_residual_sum += std::abs(residual);
    }

    last_corr = corr_count;
    last_ground = ground_count;
    last_non_ground = non_ground_count;
    last_abs_residual_sum = abs_residual_sum;
    if (corr_count < params_.min_correspondences) break;

    A += Mat6::Identity() * 1e-9;
    const Vec6 dx = A.ldlt().solve(b);
    if (!dx.allFinite()) break;
    T = expSe3(dx) * T;
    if (result) result->iterations = iter + 1;
    if (dx.norm() < params_.convergence_eps) break;
  }

  if (result) {
    result->num_correspondences = last_corr;
    result->num_ground_correspondences = last_ground;
    result->num_non_ground_correspondences = last_non_ground;
    result->mean_abs_residual =
        last_corr > 0 ? last_abs_residual_sum / static_cast<double>(last_corr) : 0.0;
  }
  return T;
}

void EloPipeline::insertSri(std::vector<ModelCell>* model,
                            const Eigen::Vector3d& point,
                            const Eigen::Vector3d& normal, int stamp) {
  int row = 0, col = 0;
  if (!projectSri(point, &row, &col)) return;
  ModelCell& cell = (*model)[static_cast<size_t>(sriIndex(row, col))];
  const double range = point.norm();
  if (!cell.valid || range < cell.range) {
    cell.point = point;
    cell.normal = normal.normalized();
    cell.range = range;
    cell.stamp = stamp;
    cell.count = 1;
    cell.valid = true;
    cell.has_normal = true;
  }
}

void EloPipeline::insertBev(std::vector<ModelCell>* model,
                            const Eigen::Vector3d& point, int stamp, int count) {
  int row = 0, col = 0;
  if (!projectBev(point, &row, &col)) return;
  ModelCell& cell = (*model)[static_cast<size_t>(bevIndex(row, col))];
  if (!cell.valid) {
    cell.point = point;
    cell.range = point.norm();
    cell.stamp = stamp;
    cell.count = std::max(1, count);
    cell.valid = true;
    return;
  }
  const int old_count = std::max(1, cell.count);
  const int new_count = std::max(1, count);
  const int total = std::min(20, old_count + new_count);
  cell.point = (cell.point * static_cast<double>(old_count) +
                point * static_cast<double>(new_count)) /
               static_cast<double>(old_count + new_count);
  cell.range = cell.point.norm();
  cell.stamp = std::max(cell.stamp, stamp);
  cell.count = total;
}

void EloPipeline::initializeModel(const FrameData& frame) {
  std::fill(sri_model_.begin(), sri_model_.end(), ModelCell());
  std::fill(bev_model_.begin(), bev_model_.end(), ModelCell());
  for (const auto& cell : frame.sri) {
    if (!cell.valid) continue;
    if (cell.ground) {
      insertBev(&bev_model_, cell.point, frame_count_);
    } else if (cell.has_normal) {
      insertSri(&sri_model_, cell.point, cell.normal, frame_count_);
    }
  }
}

void EloPipeline::updateModel(const FrameData& frame,
                              const Eigen::Matrix4d& model_to_current) {
  std::vector<ModelCell> next_sri(sri_model_.size());
  std::vector<ModelCell> next_bev(bev_model_.size());
  const Eigen::Matrix3d R = model_to_current.block<3, 3>(0, 0);
  const Eigen::Vector3d t = model_to_current.block<3, 1>(0, 3);

  for (const auto& cell : sri_model_) {
    if (!cell.valid || !cell.has_normal) continue;
    if (frame_count_ - cell.stamp > params_.model_window_frames) continue;
    insertSri(&next_sri, R * cell.point + t, R * cell.normal, cell.stamp);
  }
  for (const auto& cell : bev_model_) {
    if (!cell.valid) continue;
    if (frame_count_ - cell.stamp > params_.model_window_frames) continue;
    insertBev(&next_bev, R * cell.point + t, cell.stamp, cell.count);
  }

  for (const auto& cell : frame.sri) {
    if (!cell.valid) continue;
    if (cell.ground) {
      insertBev(&next_bev, cell.point, frame_count_);
    } else if (cell.has_normal) {
      insertSri(&next_sri, cell.point, cell.normal, frame_count_);
    }
  }
  sri_model_.swap(next_sri);
  bev_model_.swap(next_bev);
}

EloResult EloPipeline::registerFrame(const std::vector<Eigen::Vector3d>& points) {
  EloResult result;
  FrameData frame = buildFrame(points);
  result.pose = pose_;
  result.num_ground_points = frame.ground_points;
  result.num_non_ground_points = frame.non_ground_points;

  if (frame.points.empty()) {
    return result;
  }

  if (!initialized_) {
    initializeModel(frame);
    initialized_ = true;
    model_pose_ = pose_;
    result.pose = pose_;
    result.converged = true;
    result.num_correspondences = static_cast<int>(frame.points.size());
    ++frame_count_;
    return result;
  }

  const Eigen::Matrix4d predicted_pose = pose_ * last_delta_;
  const Eigen::Matrix4d init_current_to_model = model_pose_.inverse() * predicted_pose;
  const Eigen::Matrix4d current_to_model = align(frame, init_current_to_model, &result);
  const Eigen::Matrix4d new_pose = model_pose_ * current_to_model;

  result.pose = new_pose;
  result.converged = new_pose.allFinite() &&
                     result.num_correspondences >= params_.min_correspondences;

  const Eigen::Matrix4d model_to_current = current_to_model.inverse();
  updateModel(frame, model_to_current);
  last_delta_ = pose_.inverse() * new_pose;
  pose_ = new_pose;
  model_pose_ = new_pose;
  ++frame_count_;
  return result;
}

}  // namespace elo
}  // namespace localization_zoo
