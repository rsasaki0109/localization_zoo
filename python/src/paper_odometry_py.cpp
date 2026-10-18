#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "suma/suma_paper.h"
#include "imls_slam/imls_paper_odometry.h"
#include "dilo/dilo_paper.h"
#include "litamin2/litamin2_odometry.h"

namespace py = pybind11;
using namespace localization_zoo;

namespace {
using Points = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

std::vector<Eigen::Vector3d> checkedPoints(const Eigen::Ref<const Points>& points) {
  if (points.rows() == 0 || !points.allFinite())
    throw py::value_error("points must be a nonempty finite Nx3 array");
  std::vector<Eigen::Vector3d> scan;
  scan.reserve(static_cast<size_t>(points.rows()));
  for (Eigen::Index i = 0; i < points.rows(); ++i)
    scan.push_back(points.row(i).transpose());
  return scan;
}
}  // namespace

void bindPaperOdometry(py::module_& m) {
  using Mode = litamin2::LiTAMIN2OdometryParams::MapMode;
  py::enum_<Mode>(m, "Litamin2MapMode")
      .value("AGGREGATE", Mode::kAggregate)
      .value("FIRST", Mode::kFirst)
      .value("LATEST", Mode::kLatest);

  py::class_<suma::SuMaPaperParams>(m, "SumaPaperParams")
      .def(py::init<>())
      .def_readwrite("width", &suma::SuMaPaperParams::width)
      .def_readwrite("height", &suma::SuMaPaperParams::height)
      .def_readwrite("fov_up_deg", &suma::SuMaPaperParams::fov_up_deg)
      .def_readwrite("fov_down_deg", &suma::SuMaPaperParams::fov_down_deg)
      .def_readwrite("min_depth", &suma::SuMaPaperParams::min_depth)
      .def_readwrite("max_depth", &suma::SuMaPaperParams::max_depth)
      .def_readwrite("model_min_depth", &suma::SuMaPaperParams::model_min_depth)
      .def_readwrite("model_max_depth", &suma::SuMaPaperParams::model_max_depth)
      .def_readwrite("max_iterations", &suma::SuMaPaperParams::max_iterations)
      .def_readwrite("stop_delta", &suma::SuMaPaperParams::stop_delta)
      .def_readwrite("stop_epsilon", &suma::SuMaPaperParams::stop_epsilon)
      .def_readwrite("icp_max_distance", &suma::SuMaPaperParams::icp_max_distance)
      .def_readwrite("icp_max_angle_deg", &suma::SuMaPaperParams::icp_max_angle_deg)
      .def_readwrite("huber_factor", &suma::SuMaPaperParams::huber_factor)
      .def_readwrite("bilinear_sampling", &suma::SuMaPaperParams::bilinear_sampling)
      .def_readwrite("normalize_bilinear", &suma::SuMaPaperParams::normalize_bilinear)
      .def_readwrite("initialize_identity", &suma::SuMaPaperParams::initialize_identity)
      .def_readwrite("degenerate_ratio", &suma::SuMaPaperParams::degenerate_ratio)
      .def_readwrite("coarse_frames", &suma::SuMaPaperParams::coarse_frames)
      .def_readwrite("coarse_iterations", &suma::SuMaPaperParams::coarse_iterations)
      .def_readwrite("coarse_distance_scale", &suma::SuMaPaperParams::coarse_distance_scale)
      .def_readwrite("fallback_mode", &suma::SuMaPaperParams::fallback_mode)
      .def_readwrite("fallback_max_distance", &suma::SuMaPaperParams::fallback_max_distance)
      .def_readwrite("fallback_max_angle_deg", &suma::SuMaPaperParams::fallback_max_angle_deg)
      .def_readwrite("fallback_trans_threshold", &suma::SuMaPaperParams::fallback_trans_threshold)
      .def_readwrite("fallback_rot_threshold", &suma::SuMaPaperParams::fallback_rot_threshold)
      .def_readwrite("min_radius", &suma::SuMaPaperParams::min_radius)
      .def_readwrite("max_radius", &suma::SuMaPaperParams::max_radius)
      .def_readwrite("max_angle_deg", &suma::SuMaPaperParams::max_angle_deg)
      .def_readwrite("map_max_distance", &suma::SuMaPaperParams::map_max_distance)
      .def_readwrite("map_max_angle_deg", &suma::SuMaPaperParams::map_max_angle_deg)
      .def_readwrite("unstable_age", &suma::SuMaPaperParams::unstable_age)
      .def_readwrite("confidence_threshold", &suma::SuMaPaperParams::confidence_threshold)
      .def_readwrite("p_stable", &suma::SuMaPaperParams::p_stable)
      .def_readwrite("p_prior", &suma::SuMaPaperParams::p_prior)
      .def_readwrite("sigma_angle", &suma::SuMaPaperParams::sigma_angle)
      .def_readwrite("sigma_distance", &suma::SuMaPaperParams::sigma_distance)
      .def_readwrite("use_stability", &suma::SuMaPaperParams::use_stability)
      .def_readwrite("gamma", &suma::SuMaPaperParams::gamma)
      .def_readwrite("active_timestamps", &suma::SuMaPaperParams::active_timestamps)
      .def_readwrite("max_confidence", &suma::SuMaPaperParams::max_confidence)
      .def_readwrite("time_init", &suma::SuMaPaperParams::time_init)
      .def_readwrite("p_unstable_init", &suma::SuMaPaperParams::p_unstable_init)
      .def_readwrite("map_extent", &suma::SuMaPaperParams::map_extent)
      .def_readwrite("keep_inactive", &suma::SuMaPaperParams::keep_inactive);

  py::class_<suma::SuMaPaperResult>(m, "SumaPaperResult")
      .def_readonly("pose", &suma::SuMaPaperResult::pose)
      .def_readonly("increment", &suma::SuMaPaperResult::increment)
      .def_readonly("valid_pixels", &suma::SuMaPaperResult::valid_pixels)
      .def_readonly("model_pixels", &suma::SuMaPaperResult::model_pixels)
      .def_readonly("inliers", &suma::SuMaPaperResult::inliers)
      .def_readonly("outliers", &suma::SuMaPaperResult::outliers)
      .def_readonly("iterations", &suma::SuMaPaperResult::iterations)
      .def_readonly("fallback", &suma::SuMaPaperResult::fallback)
      .def_readonly("coarse_start", &suma::SuMaPaperResult::coarse_start)
      .def_readonly("coarse_iterations", &suma::SuMaPaperResult::coarse_iterations)
      .def_readonly("num_surfels", &suma::SuMaPaperResult::num_surfels)
      .def_readonly("new_surfels", &suma::SuMaPaperResult::new_surfels)
      .def_readonly("updated_surfels", &suma::SuMaPaperResult::updated_surfels);

  py::class_<suma::SuMaPaper>(m, "SumaPaper")
      .def(py::init<const suma::SuMaPaperParams&>(),
           py::arg("params") = suma::SuMaPaperParams())
      .def("process",
           [](suma::SuMaPaper& self, const Eigen::Ref<const Points>& points) {
             return self.process(checkedPoints(points));
           }, py::arg("points"), "Process one raw scan in its stored sensor frame.")
      .def_property_readonly("pose", [](const suma::SuMaPaper& self) {
        return Eigen::Matrix4d(self.pose());
      })
      .def("clear", &suma::SuMaPaper::clear);

  py::class_<imls_slam::ImlsPaperParams>(m, "ImlsPaperParams")
      .def(py::init<>())
      .def_readwrite("h", &imls_slam::ImlsPaperParams::h)
      .def_readwrite("r", &imls_slam::ImlsPaperParams::r)
      .def_readwrite("samples_per_list", &imls_slam::ImlsPaperParams::samples_per_list)
      .def_readwrite("window_scans", &imls_slam::ImlsPaperParams::window_scans)
      .def_readwrite("iterations", &imls_slam::ImlsPaperParams::iterations)
      .def_readwrite("scan_normal_radius", &imls_slam::ImlsPaperParams::scan_normal_radius)
      .def_readwrite("scan_normal_max_neighbors", &imls_slam::ImlsPaperParams::scan_normal_max_neighbors)
      .def_readwrite("min_normal_points", &imls_slam::ImlsPaperParams::min_normal_points)
      .def_readwrite("min_range", &imls_slam::ImlsPaperParams::min_range)
      .def_readwrite("max_range", &imls_slam::ImlsPaperParams::max_range)
      .def_readwrite("coarse_frames", &imls_slam::ImlsPaperParams::coarse_frames)
      .def_readwrite("coarse_r", &imls_slam::ImlsPaperParams::coarse_r)
      .def_readwrite("coarse_h", &imls_slam::ImlsPaperParams::coarse_h)
      .def_readwrite("coarse_iterations", &imls_slam::ImlsPaperParams::coarse_iterations);

  py::class_<imls_slam::ImlsPaperResult>(m, "ImlsPaperResult")
      .def_readonly("pose", &imls_slam::ImlsPaperResult::pose)
      .def_readonly("samples", &imls_slam::ImlsPaperResult::samples)
      .def_readonly("model_points", &imls_slam::ImlsPaperResult::model_points)
      .def_readonly("coarse_start", &imls_slam::ImlsPaperResult::coarse_start);

  py::class_<imls_slam::ImlsPaperOdometry>(m, "ImlsPaperOdometry")
      .def(py::init<const imls_slam::ImlsPaperParams&>(),
           py::arg("params") = imls_slam::ImlsPaperParams())
      .def("process",
           [](imls_slam::ImlsPaperOdometry& self, const Eigen::Ref<const Points>& points) {
             return self.process(checkedPoints(points));
           }, py::arg("points"), "Process one raw scan in its stored sensor frame.")
      .def_property_readonly("pose", [](const imls_slam::ImlsPaperOdometry& self) {
        return Eigen::Matrix4d(self.pose());
      });

  py::class_<dilo::DiloPaperParams>(m, "DiloPaperParams")
      .def(py::init<>())
      .def_readwrite("width", &dilo::DiloPaperParams::width)
      .def_readwrite("height", &dilo::DiloPaperParams::height)
      .def_readwrite("top_elevation_deg", &dilo::DiloPaperParams::top_elevation_deg)
      .def_readwrite("min_range", &dilo::DiloPaperParams::min_range)
      .def_readwrite("max_range", &dilo::DiloPaperParams::max_range)
      .def_readwrite("fill_slope", &dilo::DiloPaperParams::fill_slope)
      .def_readwrite("max_fill_gap", &dilo::DiloPaperParams::max_fill_gap)
      .def_readwrite("keypoints_per_list", &dilo::DiloPaperParams::keypoints_per_list)
      .def_readwrite("min_planarity", &dilo::DiloPaperParams::min_planarity)
      .def_readwrite("keypoint_cell_px", &dilo::DiloPaperParams::keypoint_cell_px)
      .def_readwrite("window", &dilo::DiloPaperParams::window)
      .def_readwrite("init_iterations", &dilo::DiloPaperParams::init_iterations)
      .def_readwrite("update_iterations", &dilo::DiloPaperParams::update_iterations)
      .def_readwrite("exact_points", &dilo::DiloPaperParams::exact_points)
      .def_readwrite("point_gradient_normals", &dilo::DiloPaperParams::point_gradient_normals)
      .def_readwrite("planar_prediction", &dilo::DiloPaperParams::planar_prediction)
      .def_readwrite("max_init_deviation_m", &dilo::DiloPaperParams::max_init_deviation_m)
      .def_readwrite("max_init_deviation_rad", &dilo::DiloPaperParams::max_init_deviation_rad)
      .def_readwrite("max_pair_step_m", &dilo::DiloPaperParams::max_pair_step_m)
      .def_readwrite("max_pair_step_rad", &dilo::DiloPaperParams::max_pair_step_rad)
      .def_readwrite("use_edges", &dilo::DiloPaperParams::use_edges)
      .def_readwrite("max_edge_points", &dilo::DiloPaperParams::max_edge_points)
      .def_readwrite("edge_search_px", &dilo::DiloPaperParams::edge_search_px)
      .def_readwrite("harris_k", &dilo::DiloPaperParams::harris_k)
      .def_readwrite("huber_delta", &dilo::DiloPaperParams::huber_delta)
      .def_readwrite("degenerate_ratio", &dilo::DiloPaperParams::degenerate_ratio)
      .def_readwrite("tukey_c", &dilo::DiloPaperParams::tukey_c)
      .def_readwrite("edge_detector", &dilo::DiloPaperParams::edge_detector)
      .def_readwrite("edge_reference_points", &dilo::DiloPaperParams::edge_reference_points)
      .def_readwrite("edge_subpixel", &dilo::DiloPaperParams::edge_subpixel)
      .def_readwrite("edge_row_search", &dilo::DiloPaperParams::edge_row_search)
      .def_readwrite("edge_sum_normal_equations", &dilo::DiloPaperParams::edge_sum_normal_equations)
      .def_readwrite("edge_weight", &dilo::DiloPaperParams::edge_weight)
      .def_readwrite("edge_dofs", &dilo::DiloPaperParams::edge_dofs)
      .def_readwrite("edge_huber_delta", &dilo::DiloPaperParams::edge_huber_delta)
      .def_readwrite("edge_tukey_c", &dilo::DiloPaperParams::edge_tukey_c);

  py::class_<dilo::DiloPaperOdometry>(m, "DiloPaperOdometry")
      .def(py::init<const dilo::DiloPaperParams&>(),
           py::arg("params") = dilo::DiloPaperParams())
      .def("process",
           [](dilo::DiloPaperOdometry& self, const Eigen::Ref<const Points>& points) {
             return self.process(checkedPoints(points));
           }, py::arg("points"), "Process one raw scan in its stored sensor frame.")
      .def_property_readonly("pose", [](const dilo::DiloPaperOdometry& self) {
        const auto window = self.windowPoses();
        return window.empty() ? Eigen::Matrix4d(Eigen::Matrix4d::Identity()) : window.back();
      })
      .def_property_readonly("window_poses", &dilo::DiloPaperOdometry::windowPoses)
      .def_property_readonly("finalized_poses", [](const dilo::DiloPaperOdometry& self) {
        return std::vector<Eigen::Matrix4d>(self.finalizedPoses());
      });

  py::class_<litamin2::LiTAMIN2OdometryParams>(m, "Litamin2OdometryParams")
      .def(py::init<>())
      .def_readwrite("voxel_size", &litamin2::LiTAMIN2OdometryParams::voxel_size)
      .def_readwrite("min_points_per_voxel", &litamin2::LiTAMIN2OdometryParams::min_points_per_voxel)
      .def_readwrite("min_cov_eigenvalue", &litamin2::LiTAMIN2OdometryParams::min_cov_eigenvalue)
      .def_readwrite("lambda_", &litamin2::LiTAMIN2OdometryParams::lambda)
      .def_readwrite("sigma_icp", &litamin2::LiTAMIN2OdometryParams::sigma_icp)
      .def_readwrite("sigma_cov", &litamin2::LiTAMIN2OdometryParams::sigma_cov)
      .def_readwrite("use_cov_cost", &litamin2::LiTAMIN2OdometryParams::use_cov_cost)
      .def_readwrite("max_iterations", &litamin2::LiTAMIN2OdometryParams::max_iterations)
      .def_readwrite("rotation_epsilon", &litamin2::LiTAMIN2OdometryParams::rotation_epsilon)
      .def_readwrite("translation_epsilon", &litamin2::LiTAMIN2OdometryParams::translation_epsilon)
      .def_readwrite("map_mode", &litamin2::LiTAMIN2OdometryParams::map_mode)
      .def_readwrite("map_voxel_size", &litamin2::LiTAMIN2OdometryParams::map_voxel_size)
      .def_readwrite("map_radius", &litamin2::LiTAMIN2OdometryParams::map_radius)
      .def_readwrite("num_threads", &litamin2::LiTAMIN2OdometryParams::num_threads);

  py::class_<litamin2::LiTAMIN2OdometryResult>(m, "Litamin2OdometryResult")
      .def_readonly("pose", &litamin2::LiTAMIN2OdometryResult::pose)
      .def_readonly("iterations", &litamin2::LiTAMIN2OdometryResult::iterations)
      .def_readonly("source_distributions", &litamin2::LiTAMIN2OdometryResult::source_distributions)
      .def_readonly("correspondences", &litamin2::LiTAMIN2OdometryResult::correspondences)
      .def_readonly("map_distributions", &litamin2::LiTAMIN2OdometryResult::map_distributions);

  py::class_<litamin2::LiTAMIN2Odometry>(m, "Litamin2Odometry")
      .def(py::init<const litamin2::LiTAMIN2OdometryParams&>(),
           py::arg("params") = litamin2::LiTAMIN2OdometryParams())
      .def("process",
           [](litamin2::LiTAMIN2Odometry& self, const Eigen::Ref<const Points>& points) {
             return self.process(checkedPoints(points));
           }, py::arg("points"), "Process one raw scan in its stored sensor frame.")
      .def_property_readonly("pose", [](const litamin2::LiTAMIN2Odometry& self) {
        return Eigen::Matrix4d(self.pose());
      });

}
