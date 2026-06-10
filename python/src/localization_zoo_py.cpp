// Python bindings for a representative subset of Localization Zoo methods.
// Boundary type is an Nx3 float64 numpy array (one point per row).
#include <pybind11/eigen.h>
#include <pybind11/pybind11.h>

#include <vector>

#include "gicp/gicp_registration.h"
#include "kiss_icp/kiss_icp.h"
#include "litamin2/litamin2_registration.h"
#include "ndt/ndt_registration.h"

namespace py = pybind11;

namespace {

using RowMatX3 = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

std::vector<Eigen::Vector3d> toPoints(const Eigen::Ref<const RowMatX3>& array) {
  std::vector<Eigen::Vector3d> points(static_cast<size_t>(array.rows()));
  for (Eigen::Index i = 0; i < array.rows(); ++i) {
    points[static_cast<size_t>(i)] = array.row(i).transpose();
  }
  return points;
}

}  // namespace

PYBIND11_MODULE(localization_zoo, m) {
  m.doc() =
      "Localization Zoo Python bindings (experimental subset).\n"
      "Point clouds are Nx3 float64 numpy arrays; poses are 4x4 numpy arrays.";

  using namespace localization_zoo;

  // ---- KISS-ICP (odometry pipeline) ----
  py::class_<kiss_icp::KISSICPParams>(m, "KissICPParams")
      .def(py::init<>())
      .def_readwrite("voxel_size", &kiss_icp::KISSICPParams::voxel_size)
      .def_readwrite("max_range", &kiss_icp::KISSICPParams::max_range)
      .def_readwrite("min_range", &kiss_icp::KISSICPParams::min_range)
      .def_readwrite("max_points_per_voxel",
                     &kiss_icp::KISSICPParams::max_points_per_voxel)
      .def_readwrite("initial_threshold",
                     &kiss_icp::KISSICPParams::initial_threshold)
      .def_readwrite("max_icp_iterations",
                     &kiss_icp::KISSICPParams::max_icp_iterations)
      .def_readwrite("convergence_criterion",
                     &kiss_icp::KISSICPParams::convergence_criterion)
      .def_readwrite("local_map_radius",
                     &kiss_icp::KISSICPParams::local_map_radius)
      .def_readwrite("map_cleanup_interval",
                     &kiss_icp::KISSICPParams::map_cleanup_interval);

  py::class_<kiss_icp::KISSICPResult>(m, "KissICPResult")
      .def_readonly("pose", &kiss_icp::KISSICPResult::pose)
      .def_readonly("converged", &kiss_icp::KISSICPResult::converged)
      .def_readonly("iterations", &kiss_icp::KISSICPResult::iterations);

  py::class_<kiss_icp::KISSICPPipeline>(m, "KissICP")
      .def(py::init<const kiss_icp::KISSICPParams&>(),
           py::arg("params") = kiss_icp::KISSICPParams())
      .def(
          "register_frame",
          [](kiss_icp::KISSICPPipeline& self,
             const Eigen::Ref<const RowMatX3>& points) {
            return self.registerFrame(toPoints(points));
          },
          py::arg("points"),
          "Register one scan (Nx3 array) and update the internal local map.")
      .def_property_readonly("pose", &kiss_icp::KISSICPPipeline::pose,
                             "Accumulated world-frame pose (4x4).")
      .def_property_readonly("map_size", &kiss_icp::KISSICPPipeline::mapSize);

  // ---- LiTAMIN2 (scan-to-target registration) ----
  py::class_<litamin2::LiTAMIN2Params>(m, "Litamin2Params")
      .def(py::init<>())
      .def_readwrite("voxel_resolution",
                     &litamin2::LiTAMIN2Params::voxel_resolution)
      .def_readwrite("min_points_per_voxel",
                     &litamin2::LiTAMIN2Params::min_points_per_voxel)
      .def_readwrite("max_iterations", &litamin2::LiTAMIN2Params::max_iterations)
      .def_readwrite("use_cov_cost", &litamin2::LiTAMIN2Params::use_cov_cost)
      .def_readwrite("num_threads", &litamin2::LiTAMIN2Params::num_threads);

  py::class_<litamin2::RegistrationResult>(m, "Litamin2Result")
      .def_readonly("transformation",
                    &litamin2::RegistrationResult::transformation)
      .def_readonly("num_iterations",
                    &litamin2::RegistrationResult::num_iterations)
      .def_readonly("final_error", &litamin2::RegistrationResult::final_error)
      .def_readonly("converged", &litamin2::RegistrationResult::converged);

  py::class_<litamin2::LiTAMIN2Registration>(m, "Litamin2")
      .def(py::init<const litamin2::LiTAMIN2Params&>(),
           py::arg("params") = litamin2::LiTAMIN2Params())
      .def(
          "set_target",
          [](litamin2::LiTAMIN2Registration& self,
             const Eigen::Ref<const RowMatX3>& points) {
            self.setTarget(toPoints(points));
          },
          py::arg("points"))
      .def(
          "align",
          [](litamin2::LiTAMIN2Registration& self,
             const Eigen::Ref<const RowMatX3>& points,
             const Eigen::Matrix4d& initial_guess) {
            return self.align(toPoints(points), initial_guess);
          },
          py::arg("points"),
          py::arg("initial_guess") = Eigen::Matrix4d::Identity());

  // ---- NDT (scan-to-target registration) ----
  py::class_<ndt::NDTParams>(m, "NdtParams")
      .def(py::init<>())
      .def_readwrite("resolution", &ndt::NDTParams::resolution)
      .def_readwrite("max_iterations", &ndt::NDTParams::max_iterations)
      .def_readwrite("step_size", &ndt::NDTParams::step_size)
      .def_readwrite("convergence_threshold",
                     &ndt::NDTParams::convergence_threshold)
      .def_readwrite("outlier_ratio", &ndt::NDTParams::outlier_ratio)
      .def_readwrite("min_points_per_cell", &ndt::NDTParams::min_points_per_cell);

  py::class_<ndt::NDTResult>(m, "NdtResult")
      .def_readonly("transformation", &ndt::NDTResult::transformation)
      .def_readonly("score", &ndt::NDTResult::score)
      .def_readonly("iterations", &ndt::NDTResult::iterations)
      .def_readonly("converged", &ndt::NDTResult::converged);

  py::class_<ndt::NDTRegistration>(m, "Ndt")
      .def(py::init<const ndt::NDTParams&>(),
           py::arg("params") = ndt::NDTParams())
      .def(
          "set_target",
          [](ndt::NDTRegistration& self,
             const Eigen::Ref<const RowMatX3>& points) {
            self.setTarget(toPoints(points));
          },
          py::arg("points"))
      .def(
          "align",
          [](ndt::NDTRegistration& self,
             const Eigen::Ref<const RowMatX3>& points,
             const Eigen::Matrix4d& initial_guess) {
            return self.align(toPoints(points), initial_guess);
          },
          py::arg("points"),
          py::arg("initial_guess") = Eigen::Matrix4d::Identity());

  // ---- GICP (scan-to-target registration) ----
  py::class_<gicp::GICPParams>(m, "GicpParams")
      .def(py::init<>())
      .def_readwrite("k_neighbors", &gicp::GICPParams::k_neighbors)
      .def_readwrite("max_correspondence_distance",
                     &gicp::GICPParams::max_correspondence_distance)
      .def_readwrite("max_iterations", &gicp::GICPParams::max_iterations);

  py::class_<gicp::GICPResult>(m, "GicpResult")
      .def_readonly("transformation", &gicp::GICPResult::transformation)
      .def_readonly("fitness", &gicp::GICPResult::fitness)
      .def_readonly("num_iterations", &gicp::GICPResult::num_iterations)
      .def_readonly("num_correspondences",
                    &gicp::GICPResult::num_correspondences)
      .def_readonly("converged", &gicp::GICPResult::converged);

  py::class_<gicp::GICPRegistration>(m, "Gicp")
      .def(py::init<const gicp::GICPParams&>(),
           py::arg("params") = gicp::GICPParams())
      .def(
          "set_target",
          [](gicp::GICPRegistration& self,
             const Eigen::Ref<const RowMatX3>& points) {
            self.setTarget(toPoints(points));
          },
          py::arg("points"))
      .def(
          "align",
          [](gicp::GICPRegistration& self,
             const Eigen::Ref<const RowMatX3>& points,
             const Eigen::Matrix4d& initial_guess) {
            return self.align(toPoints(points), initial_guess);
          },
          py::arg("points"),
          py::arg("initial_guess") = Eigen::Matrix4d::Identity())
      .def_property_readonly("target_size",
                             &gicp::GICPRegistration::targetSize);
}
