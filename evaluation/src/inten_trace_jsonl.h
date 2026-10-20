#pragma once

// Streaming observation export for the evaluation runner, outside the estimator.
#include "inten_loam/inten_loam_paper.h"
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace localization_zoo::evaluation::inten_trace {
using namespace inten_loam;

inline void string(std::ostream& out, const std::string& value) {
  out << '"';
  for (unsigned char c : value) {
    switch (c) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\n': out << "\\n"; break;
      case '\r': out << "\\r"; break;
      case '\t': out << "\\t"; break;
      default:
        if (c < 0x20) {
          const char* hex = "0123456789abcdef";
          out << "\\u00" << hex[c >> 4] << hex[c & 15];
        } else out << static_cast<char>(c);
    }
  }
  out << '"';
}

inline void number(std::ostream& out, double value) {
  if (std::isnan(value)) out << "\"NaN\"";
  else if (std::isinf(value)) out << (value > 0 ? "\"+Infinity\"" : "\"-Infinity\"");
  else out << value;
}

template <class Derived>
void matrix(std::ostream& out, const Eigen::MatrixBase<Derived>& value) {
  out << '[';
  for (Eigen::Index row = 0; row < value.rows(); ++row) {
    for (Eigen::Index col = 0; col < value.cols(); ++col) {
      if (row || col) out << ',';
      number(out, value(row, col));
    }
  }
  out << ']';
}

inline const char* stopName(PaperSolverStop stop) {
  switch (stop) {
    case PaperSolverStop::NotRun: return "not_run";
    case PaperSolverStop::NoIterations: return "no_iterations";
    case PaperSolverStop::InsufficientPairs: return "insufficient_pairs";
    case PaperSolverStop::NonFiniteStep: return "nonfinite_step";
    case PaperSolverStop::Converged: return "converged";
    case PaperSolverStop::IterationLimit: return "iteration_limit";
  }
  throw std::invalid_argument("Unknown InTEn solver stop value");
}

inline std::ostringstream record() {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::setprecision(std::numeric_limits<double>::max_digits10) << std::boolalpha;
  return out;
}

inline void solver(std::ostream& out, const PaperSolverTrace& value) {
  out << "{\"called\":" << value.called << ",\"input\":";
  matrix(out, value.input);
  out << ",\"output\":"; matrix(out, value.output);
  out << ",\"returned_pairs\":" << value.returned_pairs << ",\"stop\":";
  string(out, stopName(value.stop));
  out << ",\"iterations\":[";
  for (size_t i = 0; i < value.iterations.size(); ++i) {
    if (i) out << ',';
    const auto& it = value.iterations[i];
    out << "{\"index\":" << i << ",\"linearization_pose\":";
    matrix(out, it.linearization_pose);
    out << ",\"H\":"; matrix(out, it.H);
    out << ",\"g\":"; matrix(out, it.g);
    out << ",\"delta\":"; matrix(out, it.delta);
    out << ",\"class_pairs\":[" << it.class_pairs[0] << ',' << it.class_pairs[1]
        << ',' << it.class_pairs[2] << ',' << it.class_pairs[3] << ']'
        << ",\"pairs\":" << it.pairs << ",\"weighted_squared_residual\":";
    number(out, it.weighted_squared_residual);
    out << ",\"step_attempted\":" << it.step_attempted
        << ",\"step_finite\":" << it.step_finite
        << ",\"update_applied\":" << it.update_applied << '}';
  }
  out << "]}";
}

inline void params(std::ostream& out, const InTenLoamPaperParams& p) {
  out << '{';
  bool first = true;
#define INTEN_TRACE_PARAM(key) \
  if (!first) out << ','; \
  first = false; out << "\"" #key "\":"; number(out, static_cast<double>(p.key))
  INTEN_TRACE_PARAM(deskew); INTEN_TRACE_PARAM(width); INTEN_TRACE_PARAM(height);
  INTEN_TRACE_PARAM(theta_down_deg); INTEN_TRACE_PARAM(theta_up_deg);
  INTEN_TRACE_PARAM(min_range); INTEN_TRACE_PARAM(max_range);
  INTEN_TRACE_PARAM(segment_columns); INTEN_TRACE_PARAM(segment_bin);
  INTEN_TRACE_PARAM(ground_max_slope); INTEN_TRACE_PARAM(ground_max_fit_error);
  INTEN_TRACE_PARAM(ground_start_below); INTEN_TRACE_PARAM(ground_threshold);
  INTEN_TRACE_PARAM(cluster_angle_deg); INTEN_TRACE_PARAM(min_cluster_points);
  INTEN_TRACE_PARAM(curvature_half_window); INTEN_TRACE_PARAM(edge_threshold);
  INTEN_TRACE_PARAM(facade_threshold); INTEN_TRACE_PARAM(reflector_delta);
  INTEN_TRACE_PARAM(intensity_blocks_u); INTEN_TRACE_PARAM(intensity_blocks_v);
  INTEN_TRACE_PARAM(sectors_per_row); INTEN_TRACE_PARAM(edges_per_sector);
  INTEN_TRACE_PARAM(facades_per_sector); INTEN_TRACE_PARAM(max_ground);
  INTEN_TRACE_PARAM(max_reflectors); out << ",\"use_intensity\":" << p.use_intensity;
  INTEN_TRACE_PARAM(intensity_knot); INTEN_TRACE_PARAM(intensity_weight);
  INTEN_TRACE_PARAM(huber); INTEN_TRACE_PARAM(s2s_gate_sq);
  INTEN_TRACE_PARAM(s2s_iterations); INTEN_TRACE_PARAM(s2m_iterations);
  INTEN_TRACE_PARAM(convergence); out << ",\"use_mapping\":" << p.use_mapping;
  INTEN_TRACE_PARAM(s2m_interval); INTEN_TRACE_PARAM(map_edge_voxel);
  INTEN_TRACE_PARAM(map_plane_voxel); INTEN_TRACE_PARAM(map_intensity_voxel);
  INTEN_TRACE_PARAM(map_radius); INTEN_TRACE_PARAM(temporal_window);
  INTEN_TRACE_PARAM(s2m_gate_sq); INTEN_TRACE_PARAM(s2m_plane_max_dist);
  INTEN_TRACE_PARAM(s2m_plane_min_spread); out << ",\"use_dor\":" << p.use_dor;
  INTEN_TRACE_PARAM(dor_relative_range); INTEN_TRACE_PARAM(dor_object_ratio);
  out << ",\"accelerated\":" << p.accelerated;
  // Omit the disabled extension to preserve the legacy header bytes.
  if (p.s2s_row_support) out << ",\"s2s_row_support\":true";
#undef INTEN_TRACE_PARAM
  out << '}';
}

inline std::string header(const InTenLoamPaperParams& p, const std::vector<std::string>& argv,
                          size_t input_frames, double min_range, double max_range,
                          double elevation_rad, const Eigen::Matrix4d& sensor_pose,
                          const Eigen::Matrix4d& anchor) {
  auto out = record();
  out << "{\"type\":\"header\",\"schema\":\"localization_zoo.inten_trace\",\"version\":1,"
      << "\"matrix_layout\":\"row_major_flat\",\"perturbation\":\"left_translation_rotation\","
      << "\"s2s_target_frame\":\"previous_sensor\",\"s2m_target_frame\":\"estimator_world\","
      << "\"nonfinite_encoding\":\"NaN,+Infinity,-Infinity strings\","
      << "\"residual_definition\":\"sum_huber_irls_weight_times_squared_residual_at_linearization\","
      << "\"input_frames\":" << input_frames << ",\"argv\":[";
  for (size_t i = 0; i < argv.size(); ++i) {
    if (i) out << ',';
    string(out, argv[i]);
  }
  out << "],\"params\":"; params(out, p);
  out << ",\"loader\":{\"min_range_m\":"; number(out, min_range);
  out << ",\"max_range_m\":"; number(out, max_range);
  out << ",\"elevation_correction_rad\":"; number(out, elevation_rad);
  out << ",\"sensor_pose\":"; matrix(out, sensor_pose);
  out << "},\"output_anchor\":"; matrix(out, anchor);
  out << '}';
  return out.str();
}

inline std::string frame(size_t input_index, const std::string& scan_path,
                         const InTenLoamPaperTrace& value, const InTenLoamPaperResult& result,
                         const Eigen::Matrix4d& anchored_pose) {
  auto out = record();
  out << "{\"type\":\"frame\",\"input_index\":" << input_index << ",\"scan_path\":";
  string(out, scan_path);
  out << ",\"frame_index\":" << value.frame_index << ",\"initialization\":" << value.initialization
      << ",\"features\":{\"edge\":" << result.num_edge << ",\"ground\":" << result.num_ground
      << ",\"facade\":" << result.num_facade << ",\"reflector\":" << result.num_reflector
      << "},\"dor_removed\":" << result.dor_removed << ",\"s2s\":";
  solver(out, value.s2s);
  out << ",\"s2s_fallback\":" << value.s2s_fallback << ",\"accepted_relative\":";
  matrix(out, value.accepted_relative);
  out << ",\"s2m_prediction\":"; matrix(out, value.s2m_prediction);
  out << ",\"s2m\":"; solver(out, value.s2m);
  out << ",\"s2m_accepted\":" << value.s2m_accepted << ",\"pose\":";
  matrix(out, value.pose);
  out << ",\"anchored_pose\":"; matrix(out, anchored_pose);
  out << '}';
  return out.str();
}

inline std::string skipped(size_t input_index, const std::string& scan_path) {
  auto out = record();
  out << "{\"type\":\"skipped_frame\",\"input_index\":" << input_index << ",\"scan_path\":";
  string(out, scan_path);
  out << ",\"reason\":\"empty_cloud\"}";
  return out.str();
}

class Writer {
 public:
  explicit Writer(const std::string& path) : file_(std::fopen(path.c_str(), "wx")) {
    if (!file_) throw std::runtime_error("Cannot create new InTEn trace file: " + path);
  }
  Writer(const Writer&) = delete;
  Writer& operator=(const Writer&) = delete;
  ~Writer() { if (file_) std::fclose(file_); }
  void write(const std::string& line) {
    const std::string bytes = line + '\n';
    if (!file_ || std::fwrite(bytes.data(), 1, bytes.size(), file_) != bytes.size() ||
        std::fflush(file_) != 0)
      throw std::runtime_error("InTEn trace write failed");
  }
  void finish(size_t processed, size_t skipped_frames) {
    write("{\"type\":\"end\",\"processed_frames\":" + std::to_string(processed) +
          ",\"skipped_frames\":" + std::to_string(skipped_frames) + '}');
    std::FILE* closing = file_;
    file_ = nullptr;
    if (std::fclose(closing) != 0) throw std::runtime_error("InTEn trace close failed");
  }
 private:
  std::FILE* file_;
};
}  // namespace localization_zoo::evaluation::inten_trace
