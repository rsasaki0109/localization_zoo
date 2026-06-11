/// PCD連番データでの Dogfooding ツール
///
/// 使い方:
///   ./pcd_dogfooding <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]
///   Methods include litamin2,gicp,small_gicp,voxel_gicp,ndt,fixed_map_ndt,kiss_icp,genz_icp,adaptive_icp,d2lio,ct_voxelmap,cube_lio,r_voxelmap,degen_sense,vibration_lio,id_lio,rf_lio,bievr_lio,ua_lio,damm_loam,lodestar,terrain_rbf_lio,lidar_iba,dali_slam,intensity_flow,svn_icp,pcr_dat,small_mighty,m_gclo,quadric_lo,dilo,nhc_lio,student_t_lo,spectral_lo,gmm_lo,gnc_lo,mcc_lo,imls_slam,mesh_loam,elo,tc_lvgf,opl_lvio,v_loam15,tc_vlo,ad_vlo,tc_mvlo,tricp_lo,kc_lo,i_loam,pl_loam,inten_loam,mcgicp,icpsc,vlom,odonet,nhc_net,nn_zupt,dlo,dlio,aloam,floam,lego_loam,mulls,ct_icp,ct_icp_ndt,ct_icp_ndt_keyframe,ct_lio,xicp,fast_lio2,hdl_graph_slam,vgicp_slam,suma,balm2,isc_loam,loam_livox,lio_sam,lins,fast_lio_slam,point_lio,rko_lio,fr_lio,pg_lio,clins.
///
/// pcd_dir: 00000000/cloud.pcd, 00000001/cloud.pcd, ... が並ぶディレクトリ
/// gt_csv:  lidar_pose.x,y,z,roll,pitch,yaw を含むCSV

#include "gicp/gicp_registration.h"
#include "kiss_icp/kiss_icp.h"
#include "genz_icp/genz_icp.h"
#include "adaptive_icp/adaptive_icp.h"
#include "d2lio/d2lio.h"
#include "ct_voxelmap/ct_voxelmap.h"
#include "r_voxelmap/r_voxelmap.h"
#include "damm_loam/damm_loam.h"
#include "lodestar/lodestar.h"
#include "terrain_rbf_lio/terrain_rbf_lio.h"
#include "lidar_iba/lidar_iba.h"
#include "dali_slam/dali_slam.h"
#include "intensity_flow/intensity_flow.h"
#include "svn_icp/svn_icp.h"
#include "pcr_dat/pcr_dat.h"
#include "small_mighty/small_mighty.h"
#include "m_gclo/m_gclo.h"
#include "quadric_lo/quadric_lo.h"
#include "dilo/dilo.h"
#include "nhc_lio/nhc_lio.h"
#include "student_t_lo/student_t_lo.h"
#include "spectral_lo/spectral_lo.h"
#include "gmm_lo/gmm_lo.h"
#include "gnc_lo/gnc_lo.h"
#include "mcc_lo/mcc_lo.h"
#include "imls_slam/imls_slam.h"
#include "mesh_loam/mesh_loam.h"
#include "elo/elo.h"
#include "tc_lvgf/tc_lvgf.h"
#include "opl_lvio/opl_lvio.h"
#include "v_loam15/v_loam15.h"
#include "tc_vlo/tc_vlo.h"
#include "ad_vlo/ad_vlo.h"
#include "tc_mvlo/tc_mvlo.h"
#include "tricp_lo/tricp_lo.h"
#include "kc_lo/kc_lo.h"
#include "i_loam/i_loam.h"
#include "pl_loam/pl_loam.h"
#include "inten_loam/inten_loam.h"
#include "mcgicp/mcgicp.h"
#include "icpsc/icpsc.h"
#include "vlom/vlom.h"
#include "odonet/odonet.h"
#include "nhc_net/nhc_net.h"
#include "nn_zupt/nn_zupt.h"
#include "degen_sense/degen_sense.h"
#include "vibration_lio/vibration_lio.h"
#include "id_lio/id_lio.h"
#include "rf_lio/rf_lio.h"
#include "bievr_lio/bievr_lio.h"
#include "ua_lio/ua_lio.h"
#include "cube_lio/cube_lio.h"
#include "rko_lio/rko_lio.h"
#include "fr_lio/fr_lio.h"
#include "pg_lio/pg_lio.h"
#include "litamin2/litamin2_registration.h"
#include "ndt/ndt_registration.h"
#include "scan_context/scan_context.h"
#include "small_gicp/small_gicp_registration.h"
#include "voxel_gicp/voxel_gicp_registration.h"
#include "aloam/scan_registration.h"
#include "aloam/laser_odometry.h"
#include "aloam/laser_mapping.h"
#include "floam/floam.h"
#include "dlo/dlo.h"
#include "dlio/dlio.h"
#include "lego_loam/lego_loam.h"
#include "mulls/mulls.h"
#include "ct_icp/ct_icp_registration.h"
#include "ct_lio/ct_lio_registration.h"
#include "xicp/xicp_registration.h"
#include "fast_lio2/fast_lio2.h"
#include "hdl_graph_slam/hdl_graph_slam.h"
#include "vgicp_slam/vgicp_slam.h"
#include "suma/suma.h"
#include "balm2/balm2.h"
#include "isc_loam/isc_loam.h"
#include "loam_livox/loam_livox.h"
#include "lio_sam/lio_sam.h"
#include "lins/lins.h"
#include "fast_lio_slam/fast_lio_slam.h"
#include "point_lio/point_lio.h"
#include "clins/clins.h"

#define PCL_NO_PRECOMPILE
#include <pcl/io/pcd_io.h>
#include <pcl/PCLPointCloud2.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/conversions.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <Eigen/Core>
#include <Eigen/Geometry>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::high_resolution_clock;

struct GTPose {
  double timestamp;
  double x, y, z, roll, pitch, yaw;

  Eigen::Matrix4d toMatrix() const {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    Eigen::AngleAxisd rx(roll, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd ry(pitch, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd rz(yaw, Eigen::Vector3d::UnitZ());
    T.block<3, 3>(0, 0) = (rz * ry * rx).toRotationMatrix();
    T(0, 3) = x; T(1, 3) = y; T(2, 3) = z;
    return T;
  }
};

struct ImuSampleCsv {
  double timestamp = 0.0;
  Eigen::Vector3d gyro = Eigen::Vector3d::Zero();
  Eigen::Vector3d accel = Eigen::Vector3d::Zero();
};

enum class FrameTimestampSource {
  kFrameTimestampCsv,
  kGraphTrajectory,
  kSampledGT,
};

struct FrameGapStats {
  bool valid = false;
  double min_gap = 0.0;
  double max_gap = 0.0;
  double mean_gap = 0.0;
  double median_gap = 0.0;
};

struct EIGEN_ALIGN16 PointXYZITime {
  PCL_ADD_POINT4D;
  float intensity;
  float time;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

POINT_CLOUD_REGISTER_POINT_STRUCT(
    PointXYZITime,
    (float, x, x)
    (float, y, y)
    (float, z, z)
    (float, intensity, intensity)
    (float, time, time))

struct LoadedScan {
  std::vector<Eigen::Vector3d> points;
  std::vector<double> relative_times;
  bool has_per_point_time = false;
};

struct LoadedXYZI {
  Eigen::Vector3d point = Eigen::Vector3d::Zero();
  float intensity = 0.0f;
};

std::string trimCsvToken(std::string token) {
  while (!token.empty() &&
         (token.back() == '\r' || token.back() == '\n' || token.back() == ' ' ||
          token.back() == '\t')) {
    token.pop_back();
  }
  size_t begin = 0;
  while (begin < token.size() &&
         (token[begin] == ' ' || token[begin] == '\t')) {
    begin++;
  }
  return token.substr(begin);
}

std::string normalizeMethodId(std::string method) {
  method = trimCsvToken(method);
  for (char& c : method) {
    if (c == '-') c = '_';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return method;
}

std::vector<std::string> splitMethodList(const std::string& csv) {
  std::vector<std::string> methods;
  std::istringstream ss(csv);
  std::string token;
  while (std::getline(ss, token, ',')) {
    token = normalizeMethodId(token);
    if (!token.empty()) methods.push_back(token);
  }
  return methods;
}

bool isSupportedMethod(const std::string& method) {
  return method == "litamin2" || method == "gicp" || method == "ndt" ||
         method == "fixed_map_ndt" || method == "kiss_icp" || method == "genz_icp" ||
         method == "adaptive_icp" ||
         method == "small_gicp" ||
         method == "voxel_gicp" || method == "aloam" || method == "floam" ||
         method == "dlo" || method == "dlio" || method == "lego_loam" ||
         method == "mulls" || method == "ct_lio" || method == "ct_icp" ||
         method == "ct_icp_ndt" || method == "ct_icp_ndt_keyframe" ||
         method == "xicp" || method == "fast_lio2" ||
         method == "hdl_graph_slam" || method == "vgicp_slam" ||
         method == "suma" || method == "balm2" || method == "isc_loam" ||
         method == "loam_livox" || method == "lio_sam" || method == "lins" ||
         method == "fast_lio_slam" || method == "point_lio" ||
         method == "rko_lio" || method == "fr_lio" || method == "pg_lio" ||
         method == "d2lio" ||
         method == "ct_voxelmap" || method == "cube_lio" ||
         method == "r_voxelmap" || method == "degen_sense" ||
         method == "vibration_lio" || method == "id_lio" ||
         method == "rf_lio" || method == "bievr_lio" ||
         method == "ua_lio" || method == "damm_loam" ||
         method == "lodestar" || method == "terrain_rbf_lio" ||
         method == "lidar_iba" || method == "dali_slam" ||
         method == "intensity_flow" || method == "svn_icp" ||
         method == "pcr_dat" || method == "small_mighty" ||
         method == "m_gclo" || method == "quadric_lo" ||
         method == "dilo" || method == "nhc_lio" ||
         method == "student_t_lo" || method == "spectral_lo" ||
         method == "gmm_lo" || method == "gnc_lo" || method == "mcc_lo" ||
         method == "imls_slam" || method == "mesh_loam" || method == "elo" ||
         method == "tc_lvgf" || method == "opl_lvio" ||
         method == "v_loam15" || method == "tc_vlo" ||
         method == "ad_vlo" || method == "tc_mvlo" ||
         method == "tricp_lo" || method == "kc_lo" ||
         method == "i_loam" || method == "pl_loam" || method == "inten_loam" ||
         method == "mcgicp" ||          method == "icpsc" || method == "vlom" ||
         method == "odonet" || method == "nhc_net" || method == "nn_zupt" ||
         method == "clins";
}

bool isMethodEnabled(const std::vector<std::string>& methods,
                     const std::string& method) {
  return std::find(methods.begin(), methods.end(), method) != methods.end();
}

std::vector<GTPose> loadGTPoses(const std::string& csv_path) {
  std::vector<GTPose> poses;
  std::ifstream file(csv_path);
  std::string line;
  std::getline(file, line);  // skip header

  // ヘッダからカラムインデックスを取得
  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_ts = -1, idx_x = -1, idx_y = -1, idx_z = -1;
  int idx_roll = -1, idx_pitch = -1, idx_yaw = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "timestamp") idx_ts = i;
    if (cols[i] == "lidar_pose.x") idx_x = i;
    if (cols[i] == "lidar_pose.y") idx_y = i;
    if (cols[i] == "lidar_pose.z") idx_z = i;
    if (cols[i] == "lidar_pose.roll") idx_roll = i;
    if (cols[i] == "lidar_pose.pitch") idx_pitch = i;
    if (cols[i] == "lidar_pose.yaw") idx_yaw = i;
  }

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));

    if (idx_x >= 0 && idx_x < (int)vals.size()) {
      GTPose gp;
      gp.timestamp = idx_ts >= 0 ? std::stod(vals[idx_ts]) : poses.size();
      gp.x = std::stod(vals[idx_x]);
      gp.y = std::stod(vals[idx_y]);
      gp.z = std::stod(vals[idx_z]);
      gp.roll = std::stod(vals[idx_roll]);
      gp.pitch = std::stod(vals[idx_pitch]);
      gp.yaw = std::stod(vals[idx_yaw]);
      poses.push_back(gp);
    }
  }
  return poses;
}

std::vector<ImuSampleCsv> loadImuCsv(const std::string& csv_path) {
  std::vector<ImuSampleCsv> samples;
  std::ifstream file(csv_path);
  if (!file.is_open()) return samples;

  std::string line;
  if (!std::getline(file, line)) return samples;

  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_stamp = -1;
  int idx_gx = -1, idx_gy = -1, idx_gz = -1;
  int idx_ax = -1, idx_ay = -1, idx_az = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "stamp") idx_stamp = i;
    if (cols[i] == "angular_vel.x") idx_gx = i;
    if (cols[i] == "angular_vel.y") idx_gy = i;
    if (cols[i] == "angular_vel.z") idx_gz = i;
    if (cols[i] == "linear_acc.x") idx_ax = i;
    if (cols[i] == "linear_acc.y") idx_ay = i;
    if (cols[i] == "linear_acc.z") idx_az = i;
  }

  while (std::getline(file, line)) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));

    if (idx_stamp < 0 || idx_az < 0 || idx_stamp >= static_cast<int>(vals.size()) ||
        idx_az >= static_cast<int>(vals.size())) {
      continue;
    }

    ImuSampleCsv sample;
    sample.timestamp = std::stod(vals[idx_stamp]);
    sample.gyro = Eigen::Vector3d(std::stod(vals[idx_gx]), std::stod(vals[idx_gy]),
                                  std::stod(vals[idx_gz]));
    sample.accel = Eigen::Vector3d(std::stod(vals[idx_ax]), std::stod(vals[idx_ay]),
                                   std::stod(vals[idx_az]));
    samples.push_back(sample);
  }

  return samples;
}

std::vector<Eigen::Vector3d> loadPCD(const std::string& path, double leaf = 0.5) {
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
  if (pcl::io::loadPCDFile<pcl::PointXYZ>(path, *cloud) == -1) return {};

  if (!(leaf > 1e-9)) {
    std::vector<Eigen::Vector3d> points;
    points.reserve(cloud->size());
    for (auto& p : cloud->points) {
      double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
      if (r > 1.0 && r < 80.0) points.emplace_back(p.x, p.y, p.z);
    }
    return points;
  }

  // ダウンサンプリング
  pcl::VoxelGrid<pcl::PointXYZ> vg;
  vg.setInputCloud(cloud);
  vg.setLeafSize(leaf, leaf, leaf);
  pcl::PointCloud<pcl::PointXYZ> filtered;
  vg.filter(filtered);

  std::vector<Eigen::Vector3d> points;
  for (auto& p : filtered) {
    double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r > 1.0 && r < 80.0)
      points.emplace_back(p.x, p.y, p.z);
  }
  return points;
}

std::vector<Eigen::Vector3d> limitPoints(const std::vector<Eigen::Vector3d>& points,
                                         size_t max_points) {
  if (points.size() <= max_points) return points;
  std::vector<Eigen::Vector3d> limited;
  limited.reserve(max_points);
  double step = static_cast<double>(points.size()) / max_points;
  for (size_t i = 0; i < max_points; i++) {
    size_t idx = std::min(static_cast<size_t>(i * step), points.size() - 1);
    limited.push_back(points[idx]);
  }
  return limited;
}

std::vector<LoadedXYZI> loadPCDXYZI(const std::string& path, double leaf = 0.5) {
  pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);
  if (pcl::io::loadPCDFile<pcl::PointXYZI>(path, *cloud) == -1) return {};

  pcl::PointCloud<pcl::PointXYZI>::Ptr selected = cloud;
  pcl::PointCloud<pcl::PointXYZI> filtered;
  if (leaf > 1e-9) {
    pcl::VoxelGrid<pcl::PointXYZI> vg;
    vg.setInputCloud(cloud);
    vg.setLeafSize(leaf, leaf, leaf);
    vg.setDownsampleAllData(true);
    vg.filter(filtered);
    selected.reset(new pcl::PointCloud<pcl::PointXYZI>(filtered));
  }

  std::vector<LoadedXYZI> points;
  points.reserve(selected->size());
  for (const auto& p : selected->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
        !std::isfinite(p.z) || !std::isfinite(p.intensity)) {
      continue;
    }
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r <= 1.0 || r >= 80.0) continue;
    points.push_back({Eigen::Vector3d(p.x, p.y, p.z), p.intensity});
  }
  return points;
}

std::vector<LoadedXYZI> limitLoadedXYZI(const std::vector<LoadedXYZI>& points,
                                        size_t max_points) {
  if (points.size() <= max_points) return points;
  std::vector<LoadedXYZI> limited;
  limited.reserve(max_points);
  double step = static_cast<double>(points.size()) / max_points;
  for (size_t i = 0; i < max_points; i++) {
    size_t idx = std::min(static_cast<size_t>(i * step), points.size() - 1);
    limited.push_back(points[idx]);
  }
  return limited;
}

void normalizeRelativeTimes(std::vector<double>& relative_times) {
  if (relative_times.empty()) return;

  auto [min_it, max_it] =
      std::minmax_element(relative_times.begin(), relative_times.end());
  double min_time = *min_it;
  double max_time = *max_it;
  if (max_time <= min_time + 1e-9) {
    const size_t denom = relative_times.size() > 1 ? relative_times.size() - 1 : 1;
    for (size_t i = 0; i < relative_times.size(); i++) {
      relative_times[i] = static_cast<double>(i) / denom;
    }
    return;
  }

  for (double& t : relative_times) {
    t = (t - min_time) / (max_time - min_time);
  }
}

LoadedScan limitLoadedScan(const LoadedScan& scan, size_t max_points) {
  if (scan.points.size() <= max_points) return scan;

  LoadedScan limited;
  limited.has_per_point_time = scan.has_per_point_time;
  limited.points.reserve(max_points);
  if (scan.has_per_point_time) limited.relative_times.reserve(max_points);

  double step = static_cast<double>(scan.points.size()) / max_points;
  for (size_t i = 0; i < max_points; i++) {
    size_t idx = std::min(static_cast<size_t>(i * step), scan.points.size() - 1);
    limited.points.push_back(scan.points[idx]);
    if (scan.has_per_point_time) {
      limited.relative_times.push_back(scan.relative_times[idx]);
    }
  }
  return limited;
}

LoadedScan loadTimedPCD(const std::string& path, double leaf = 0.5) {
  LoadedScan scan;

  pcl::PCLPointCloud2 raw_cloud;
  if (pcl::io::loadPCDFile(path, raw_cloud) == -1) return scan;

  bool has_time_field = false;
  for (const auto& field : raw_cloud.fields) {
    if (field.name == "time") {
      has_time_field = true;
      break;
    }
  }
  if (!has_time_field) {
    scan.points = loadPCD(path, leaf);
    return scan;
  }

  pcl::PointCloud<PointXYZITime>::Ptr cloud(new pcl::PointCloud<PointXYZITime>);
  pcl::fromPCLPointCloud2(raw_cloud, *cloud);

  pcl::PointCloud<PointXYZITime>::Ptr filtered(new pcl::PointCloud<PointXYZITime>);
  pcl::VoxelGrid<PointXYZITime> vg;
  vg.setInputCloud(cloud);
  vg.setLeafSize(leaf, leaf, leaf);
  vg.setDownsampleAllData(true);
  vg.filter(*filtered);

  scan.points.reserve(filtered->size());
  scan.relative_times.reserve(filtered->size());
  for (const auto& p : filtered->points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
        !std::isfinite(p.time)) {
      continue;
    }
    double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (r <= 1.0 || r >= 80.0) continue;

    scan.points.emplace_back(p.x, p.y, p.z);
    scan.relative_times.push_back(p.time);
  }

  scan.has_per_point_time = !scan.relative_times.empty();
  normalizeRelativeTimes(scan.relative_times);
  return scan;
}

pcl::PointCloud<pcl::PointXYZI>::Ptr toPclXYZICloud(
    const std::vector<Eigen::Vector3d>& points) {
  auto cloud = pcl::PointCloud<pcl::PointXYZI>::Ptr(
      new pcl::PointCloud<pcl::PointXYZI>);
  cloud->reserve(points.size());
  for (const auto& p : points) {
    pcl::PointXYZI pt;
    pt.x = static_cast<float>(p.x());
    pt.y = static_cast<float>(p.y());
    pt.z = static_cast<float>(p.z());
    pt.intensity = 0.0f;
    cloud->push_back(pt);
  }
  return cloud;
}

pcl::PointCloud<pcl::PointXYZI>::Ptr toPclXYZICloud(
    const std::vector<LoadedXYZI>& points) {
  auto cloud = pcl::PointCloud<pcl::PointXYZI>::Ptr(
      new pcl::PointCloud<pcl::PointXYZI>);
  cloud->reserve(points.size());
  for (const auto& p : points) {
    pcl::PointXYZI pt;
    pt.x = static_cast<float>(p.point.x());
    pt.y = static_cast<float>(p.point.y());
    pt.z = static_cast<float>(p.point.z());
    pt.intensity = p.intensity;
    cloud->push_back(pt);
  }
  return cloud;
}

std::vector<std::string> listPCDDirs(const std::string& dir) {
  std::vector<std::string> dirs;
  for (auto& entry : fs::directory_iterator(dir)) {
    if (entry.is_directory()) {
      auto pcd_path = entry.path() / "cloud.pcd";
      if (fs::exists(pcd_path)) dirs.push_back(entry.path().string());
    }
  }
  std::sort(dirs.begin(), dirs.end());
  return dirs;
}

std::vector<double> loadFrameTimestampsFromGraph(
    const std::vector<std::string>& pcd_dirs) {
  std::vector<double> timestamps;
  timestamps.reserve(pcd_dirs.size());

  for (const auto& dir : pcd_dirs) {
    fs::path trajectory_csv = fs::path(dir) / "trajectory.csv";
    if (!fs::exists(trajectory_csv)) {
      return {};
    }

    std::ifstream file(trajectory_csv);
    std::string line;
    if (!std::getline(file, line) || !std::getline(file, line)) {
      return {};
    }

    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));
    if (vals.size() < 2) return {};
    timestamps.push_back(std::stod(vals[1]));
  }

  return timestamps;
}

std::vector<double> loadFrameTimestampsFromCsv(const std::string& dir,
                                               size_t expected_frames) {
  fs::path csv_path = fs::path(dir) / "frame_timestamps.csv";
  if (!fs::exists(csv_path)) return {};

  std::ifstream file(csv_path);
  std::string line;
  if (!std::getline(file, line)) return {};

  std::istringstream header_ss(line);
  std::string col;
  std::vector<std::string> cols;
  while (std::getline(header_ss, col, ',')) cols.push_back(trimCsvToken(col));

  int idx_timestamp = -1;
  for (size_t i = 0; i < cols.size(); i++) {
    if (cols[i] == "timestamp") {
      idx_timestamp = static_cast<int>(i);
      break;
    }
  }
  if (idx_timestamp < 0) idx_timestamp = cols.size() > 1 ? 1 : 0;

  std::vector<double> timestamps;
  timestamps.reserve(expected_frames);
  while (std::getline(file, line) && timestamps.size() < expected_frames) {
    std::istringstream ss(line);
    std::vector<std::string> vals;
    std::string token;
    while (std::getline(ss, token, ',')) vals.push_back(trimCsvToken(token));
    if (idx_timestamp >= static_cast<int>(vals.size())) continue;
    timestamps.push_back(std::stod(vals[idx_timestamp]));
  }

  if (timestamps.size() != expected_frames) return {};
  return timestamps;
}

std::string resolveKittiRawRgbPath(const std::string& rgb_root,
                                   const std::string& camera_subdir,
                                   int global_frame_index) {
  char name[32];
  std::snprintf(name, sizeof(name), "%010d.png", global_frame_index);
  return (fs::path(rgb_root) / camera_subdir / name).string();
}

FrameGapStats computeFrameGapStats(const std::vector<double>& frame_timestamps) {
  FrameGapStats stats;
  if (frame_timestamps.size() < 2) return stats;

  std::vector<double> gaps;
  gaps.reserve(frame_timestamps.size() - 1);
  for (size_t i = 1; i < frame_timestamps.size(); i++) {
    gaps.push_back(std::max(frame_timestamps[i] - frame_timestamps[i - 1], 0.0));
  }

  auto sorted_gaps = gaps;
  std::sort(sorted_gaps.begin(), sorted_gaps.end());

  stats.valid = true;
  stats.min_gap = *std::min_element(gaps.begin(), gaps.end());
  stats.max_gap = *std::max_element(gaps.begin(), gaps.end());
  double sum = 0.0;
  for (double gap : gaps) sum += gap;
  stats.mean_gap = sum / gaps.size();
  const size_t mid = sorted_gaps.size() / 2;
  if (sorted_gaps.size() % 2 == 0) {
    stats.median_gap = 0.5 * (sorted_gaps[mid - 1] + sorted_gaps[mid]);
  } else {
    stats.median_gap = sorted_gaps[mid];
  }
  return stats;
}

std::string frameTimestampSourceName(FrameTimestampSource source) {
  switch (source) {
    case FrameTimestampSource::kFrameTimestampCsv:
      return "frame_timestamps.csv";
    case FrameTimestampSource::kGraphTrajectory:
      return "graph/trajectory.csv";
    case FrameTimestampSource::kSampledGT:
      return "sampled GT timestamps";
  }
  return "unknown";
}

std::string jsonEscape(const std::string& input) {
  std::string escaped;
  escaped.reserve(input.size() + 8);
  for (char c : input) {
    switch (c) {
      case '\\':
        escaped += "\\\\";
        break;
      case '"':
        escaped += "\\\"";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}

double computeATE(const std::vector<Eigen::Matrix4d>& est,
                  const std::vector<Eigen::Matrix4d>& gt) {
  int n = std::min(est.size(), gt.size());
  if (n == 0) return 0;
  double sum = 0;
  for (int i = 0; i < n; i++) {
    double e = (est[i].block<3, 1>(0, 3) - gt[i].block<3, 1>(0, 3)).norm();
    sum += e * e;
  }
  return std::sqrt(sum / n);
}

struct RPEMetrics {
  bool available = false;
  double trans_pct = 0.0;
  double rot_deg_per_m = 0.0;
};

// KITTI-style benchmarks use 100 m segments when the trajectory is long enough.
// For short public windows (e.g. MCD ~15 m), scale the segment length down so RPE
// is still defined without changing behavior on sequences >= 100 m.
double rpeSegmentLengthM(double trajectory_length_m) {
  if (trajectory_length_m <= 0.0) return 0.0;
  constexpr double kKitBench = 100.0;
  if (trajectory_length_m >= kKitBench) return kKitBench;
  const double half = 0.5 * trajectory_length_m;
  constexpr double kMinSeg = 3.0;
  double seg = std::max(half, kMinSeg);
  seg = std::min(seg, trajectory_length_m * 0.95);
  return seg;
}

RPEMetrics computeRPE(const std::vector<Eigen::Matrix4d>& est,
                      const std::vector<Eigen::Matrix4d>& gt,
                      double segment_length_m = 100.0) {
  const int n = std::min(est.size(), gt.size());
  if (n < 2 || segment_length_m <= 0.0) return {};

  std::vector<double> trans_errs;
  std::vector<double> rot_errs;
  for (int i = 0; i < n; i++) {
    double dist = 0.0;
    int j = i + 1;
    while (j < n) {
      dist += (gt[j].block<3, 1>(0, 3) - gt[j - 1].block<3, 1>(0, 3)).norm();
      if (dist >= segment_length_m) break;
      ++j;
    }
    if (j >= n || dist <= 1e-9) break;

    const Eigen::Matrix4d dT_est = est[i].inverse() * est[j];
    const Eigen::Matrix4d dT_gt = gt[i].inverse() * gt[j];
    const Eigen::Matrix4d T_err = dT_gt.inverse() * dT_est;

    const double t_err = T_err.block<3, 1>(0, 3).norm();
    const Eigen::Matrix3d R_err = T_err.block<3, 3>(0, 0);
    const double trace_term =
        std::clamp((R_err.trace() - 1.0) / 2.0, -1.0, 1.0);
    const double r_err_rad = std::acos(trace_term);

    trans_errs.push_back(t_err / dist * 100.0);
    rot_errs.push_back(r_err_rad / dist * 180.0 / M_PI);
  }

  if (trans_errs.empty()) return {};

  RPEMetrics metrics;
  metrics.available = true;
  for (double value : trans_errs) metrics.trans_pct += value;
  for (double value : rot_errs) metrics.rot_deg_per_m += value;
  metrics.trans_pct /= trans_errs.size();
  metrics.rot_deg_per_m /= rot_errs.size();
  return metrics;
}

int frameToGTIndex(size_t frame_idx, size_t total_pcd_frames, size_t num_gt_poses) {
  if (num_gt_poses == 0) return 0;
  if (num_gt_poses == total_pcd_frames) {
    return std::min(static_cast<int>(frame_idx), static_cast<int>(num_gt_poses) - 1);
  }
  int gt_idx = static_cast<int>(
      static_cast<double>(frame_idx) / std::max<size_t>(total_pcd_frames, 1) * num_gt_poses);
  return std::min(gt_idx, static_cast<int>(num_gt_poses) - 1);
}

std::vector<double> sampleFrameTimestamps(const std::vector<GTPose>& gt_poses_raw,
                                          size_t num_frames,
                                          size_t total_pcd_frames) {
  std::vector<double> timestamps;
  timestamps.reserve(num_frames);
  for (size_t i = 0; i < num_frames; i++) {
    int gt_idx = frameToGTIndex(i, total_pcd_frames, gt_poses_raw.size());
    timestamps.push_back(gt_poses_raw[gt_idx].timestamp);
  }
  return timestamps;
}

std::vector<Eigen::Matrix4d> sampleGTPoseMatrices(
    const std::vector<GTPose>& gt_poses_raw,
    const std::vector<double>& frame_timestamps) {
  std::vector<Eigen::Matrix4d> gt;
  if (gt_poses_raw.empty()) return gt;

  std::vector<double> gt_times;
  gt_times.reserve(gt_poses_raw.size());
  for (const auto& gp : gt_poses_raw) gt_times.push_back(gp.timestamp);

  Eigen::Matrix4d T0_inv = gt_poses_raw.front().toMatrix().inverse();
  gt.reserve(frame_timestamps.size());
  for (double ts : frame_timestamps) {
    auto it = std::lower_bound(gt_times.begin(), gt_times.end(), ts);
    size_t idx = 0;
    if (it == gt_times.end()) {
      idx = gt_times.size() - 1;
    } else if (it == gt_times.begin()) {
      idx = 0;
    } else {
      size_t hi = static_cast<size_t>(it - gt_times.begin());
      size_t lo = hi - 1;
      idx = std::abs(gt_times[hi] - ts) < std::abs(gt_times[lo] - ts) ? hi : lo;
    }
    gt.push_back(T0_inv * gt_poses_raw[idx].toMatrix());
  }
  return gt;
}

std::vector<localization_zoo::imu_preintegration::ImuSample> selectImuWindow(
    const std::vector<ImuSampleCsv>& imu_samples, double start_time, double end_time) {
  std::vector<localization_zoo::imu_preintegration::ImuSample> out;
  for (const auto& sample : imu_samples) {
    if (sample.timestamp < start_time) continue;
    if (sample.timestamp > end_time) break;

    localization_zoo::imu_preintegration::ImuSample imu;
    imu.timestamp = sample.timestamp;
    imu.gyro = sample.gyro;
    imu.accel = sample.accel;
    out.push_back(imu);
  }
  return out;
}

// ============================================================

struct MethodResult {
  std::string name;
  std::vector<Eigen::Matrix4d> poses;
  double time_ms = 0;
  double ate = 0;
  double rpe_trans_pct = 0;
  double rpe_rot_deg_per_m = 0;
  bool has_rpe = false;
  bool skipped = false;
  std::string status = "ok";
  std::string note;
  // Fraction of GT-seeded frames whose registration was rejected by the
  // weak-update gate and rolled back to the seed pose. <0 means N/A (the
  // method ran without a per-frame GT seed). Quantifies GT dependence.
  double seed_fallback_rate_pct = -1.0;
};

MethodResult makeSkippedResult(const std::string& name, const std::string& note) {
  MethodResult res;
  res.name = name;
  res.skipped = true;
  res.status = "skipped";
  res.note = note;
  return res;
}

struct SeedPerturbation {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double yaw_rad = 0.0;

  bool enabled() const {
    constexpr double kEps = 1e-12;
    return std::abs(x) > kEps || std::abs(y) > kEps ||
           std::abs(z) > kEps || std::abs(yaw_rad) > kEps;
  }
};

Eigen::Matrix4d seedPerturbationMatrix(const SeedPerturbation& perturb) {
  Eigen::Matrix4d delta = Eigen::Matrix4d::Identity();
  delta.block<3, 3>(0, 0) =
      Eigen::AngleAxisd(perturb.yaw_rad, Eigen::Vector3d::UnitZ())
          .toRotationMatrix();
  delta(0, 3) = perturb.x;
  delta(1, 3) = perturb.y;
  delta(2, 3) = perturb.z;
  return delta;
}

Eigen::Matrix4d applySeedPerturbation(const Eigen::Matrix4d& seed,
                                      const SeedPerturbation& perturb) {
  if (!perturb.enabled()) {
    return seed;
  }
  return seed * seedPerturbationMatrix(perturb);
}

bool isFiniteMatrix(const Eigen::Matrix4d& T) {
  for (int r = 0; r < T.rows(); ++r) {
    for (int c = 0; c < T.cols(); ++c) {
      if (!std::isfinite(T(r, c))) {
        return false;
      }
    }
  }
  return true;
}

std::string seedPerturbationNote(const SeedPerturbation& perturb) {
  if (!perturb.enabled()) {
    return "";
  }
  std::ostringstream ss;
  ss << " Seed perturbation applied in local seed frame: dx="
     << std::fixed << std::setprecision(3) << perturb.x
     << "m dy=" << perturb.y
     << "m dz=" << perturb.z
     << "m yaw=" << perturb.yaw_rad * 180.0 / M_PI << "deg.";
  return ss.str();
}

struct LiTAMIN2DogfoodingOptions {
  double voxel_resolution = 2.0;
  int min_points_per_voxel = 1;
  int max_iterations = 6;
  bool use_cov_cost = true;
  int num_threads =
      static_cast<int>(std::max(1u, std::thread::hardware_concurrency() / 2));
  size_t max_source_points = 2500;
  size_t map_max_points = 45000;
  size_t refresh_interval = 3;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  SeedPerturbation seed_perturbation;
};

struct GICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  int k_neighbors = 8;
  double max_correspondence_distance = 2.5;
  int max_iterations = 8;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  SeedPerturbation seed_perturbation;
};

struct SmallGICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  double voxel_resolution = 1.0;
  int k_neighbors = 12;
  double max_correspondence_distance = 4.0;
  int max_correspondences = 256;
  int max_iterations = 20;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  SeedPerturbation seed_perturbation;
};

struct VoxelGICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  double voxel_resolution = 1.0;
  int min_points_per_voxel = 1;
  double max_correspondence_distance = 4.0;
  int max_iterations = 20;
  double covariance_regularization = 1e-3;
  double fitness_epsilon = 1e-6;
  double rotation_epsilon = 1e-4;
  double translation_epsilon = 1e-4;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  SeedPerturbation seed_perturbation;
};

struct ALOAMDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;
};

struct FLOAMDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;

  size_t input_point_stride = 2;
  int mapping_update_interval = 2;
  bool enable_mapping = true;

  float curvature_threshold = 0.08f;
  int max_corner_sharp = 1;
  int max_corner_less_sharp = 10;
  int max_surf_flat = 2;
  float less_flat_leaf_size = 0.3f;

  double odom_distance_sq_threshold = 16.0;
  int odom_outer_iters = 1;
  int odom_ceres_iters = 3;

  double map_line_resolution = 0.6;
  double map_plane_resolution = 1.0;
  int map_outer_iters = 1;
  int map_ceres_iters = 3;
  double map_knn_max_dist_sq = 4.0;
};

struct ILoamDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  size_t input_point_stride = 2;

  float curvature_threshold = 0.08f;
  int max_corner_sharp = 1;
  int max_corner_less_sharp = 10;
  int max_surf_flat = 2;
  float less_flat_leaf_size = 0.3f;

  double odom_distance_sq_threshold = 16.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  bool enable_mapping = true;
  int mapping_update_interval = 1;
  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;

  // I-LOAM 強度拡張
  bool use_intensity_weight = true;
  bool use_intensity_correspondence = true;
  double intensity_sigma = 0.15;
  double intensity_corr_weight = 1.0;
};

struct PlLoamDogfoodingOptions {
  size_t input_stride = 2;
  int max_point_features = 280;
  int max_line_features = 64;
  int patch_radius = 4;
  bool use_depth_prior = true;
  bool use_line_features = true;
  bool use_scale_correction = true;
  double depth_prior_weight = 1.0;
  int ceres_max_iterations = 12;
  /// KITTI Raw RGB root (drive_*_sync dir). Empty → pseudo-image.
  std::string rgb_image_root;
  std::string rgb_camera_subdir = "image_02/data";
  bool use_rgb = false;
  bool rgb_half_res = true;
};

struct InTenLoamDogfoodingOptions {
  size_t input_stride = 2;
  int cyl_width = 1024;
  int cyl_height = 64;
  bool use_intensity_registration = true;
  int max_edge_features = 200;
  int max_surface_features = 400;
  int max_reflector_features = 120;
  double intensity_weight = 1.0;
  bool enable_tvf = true;
  bool enable_dor = true;
  bool enable_mapping = true;
  double tvf_voxel_size = 1.0;
  int tvf_min_observations = 3;
  double dor_range_delta_thresh = 0.6;
  double map_voxel_size = 1.0;
  double local_map_radius = 80.0;
  int mapping_keyframe_interval = 2;
};

struct IcpscLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  int cyl_width = 1024;
  int cyl_height = 64;
  int num_rings = 20;
  int num_sectors = 60;
  double intensity_sigma = 0.15;
  double adaptive_alpha = 1.0;
  bool enable_intensity = true;
  double voxel_size = 1.0;
  int max_iterations = 15;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct VlomDogfoodingOptions {
  size_t input_point_stride = 2;
  size_t visual_input_stride = 2;
  int max_point_features = 280;
  int max_line_features = 64;
  bool enable_visual_bootstrap = true;
  bool enable_scale_correction = true;
  int scale_correction_interval = 5;
  double mad_outlier_k = 2.5;
  bool enable_mapping = true;

  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;
  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;
  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;
  std::string rgb_image_root;
  std::string rgb_camera_subdir = "image_02/data";
  bool use_rgb = false;
  bool rgb_half_res = true;
};

struct McGicpLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double intensity_scale = 2.0;
  double normal_epsilon = 1e-3;
  double intensity_epsilon = 0.05;
  bool enable_intensity = true;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct LeGOLOAMDogfoodingOptions {
  int n_scans = 64;
  int ground_scan_limit = 6;
  int num_subregions = 6;
  int neighbor_window = 5;
  float minimum_range = 1.0f;
  float maximum_range = 100.0f;
  float sensor_height = 1.8f;
  float ground_height_tolerance = 0.4f;
  float sensor_mount_angle_deg = 0.0f;
  float ground_angle_threshold_deg = 10.0f;
  float scan_period = 0.1f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
  int map_outer_iters = 2;
  int map_ceres_iters = 4;
  double map_huber_loss_s = 0.1;
  int map_knn = 5;
  double map_knn_max_dist_sq = 1.0;
  double map_edge_eigenvalue_ratio = 3.0;
  double map_plane_threshold = 0.2;
};

struct MULLSDogfoodingOptions {
  int n_scans = 64;
  float scan_period = 0.1f;
  float minimum_range = 1.0f;
  float curvature_threshold = 0.1f;
  int max_corner_sharp = 2;
  int max_corner_less_sharp = 20;
  int max_surf_flat = 4;
  float less_flat_leaf_size = 0.2f;

  double odom_distance_sq_threshold = 25.0;
  double odom_nearby_scan = 2.5;
  int odom_outer_iters = 2;
  int odom_ceres_iters = 4;
  double odom_huber_loss_s = 0.1;

  double line_resolution = 0.4;
  double plane_resolution = 0.8;
  double point_resolution = 1.0;
  int mulls_map_iters = 2;
  int mulls_ceres_iters = 4;
  double mulls_huber_loss_s = 0.1;
  int mulls_knn = 5;
  double mulls_knn_max_dist_sq = 1.0;
  double mulls_edge_eigenvalue_ratio = 3.0;
  double mulls_plane_threshold = 0.2;
  double mulls_point_neighbor_max_dist_sq = 4.0;
  double mulls_line_weight = 1.0;
  double mulls_plane_weight = 1.0;
  double mulls_point_weight = 0.3;
  int mulls_full_downsample_rate = 5;
  int mulls_max_frames_in_map = 30;
};

struct NDTDogfoodingOptions {
  double source_voxel_size = 0.75;
  size_t max_source_points = 2000;
  double resolution = 1.5;
  int max_iterations = 5;
  double step_size = 0.2;
  double convergence_threshold = 1e-4;
  int min_points_per_cell = 1;
  size_t map_max_points = 22000;
  size_t refresh_interval = 8;
  double map_radius = 35.0;
  double max_seed_translation_delta = 1.5;
  double max_seed_rotation_delta_rad = 0.2;
  SeedPerturbation seed_perturbation;
};

struct KISSICPDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  double initial_threshold = 1.5;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct GenZICPDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  double initial_threshold = 1.5;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double planarity_threshold = 0.55;
  int normal_min_neighbors = 5;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct AdaptiveICPDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  double initial_threshold = 1.5;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  int coarse_max_iterations = 8;
  double planarity_threshold = 0.5;
  int normal_min_neighbors = 5;
  double density_percentile = 5.0;
  double reliable_translation_tau = 1.5;
  double motion_decay = 1.5;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct RKOLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  double initial_threshold = 1.5;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
  double gyro_bias_gain = 0.3;
};

struct FRLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 0.5;
  double grid_size = 2.0;
  double lidar_range = 40.0;
  double map_lambda = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 20;
  double max_correspondence_dist = 1.5;
  int max_subframes = 4;
  double esks_gain = 0.5;
  int min_valid_matches = 30;
};

struct PGLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  int range_image_width = 512;
  int range_image_height = 64;
  int max_patches = 48;
  double photometric_weight = 1.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct D2LIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double planarity_threshold = 0.5;
  double base_threshold = 1.0;
  double max_threshold = 3.0;
  double cauchy_scale = 1.0;
  double degeneracy_ratio = 0.05;
  double imu_prior_weight = 1.0;
  double gyro_bias_gain = 0.3;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct CTVoxelMapDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int voxel_min_points = 6;
  double planarity_ratio = 0.1;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  double sigma_depth = 0.02;
  double sigma_bearing = 0.0015;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct RVoxelMapDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int voxel_min_points = 6;
  double planarity_ratio = 0.1;
  double inlier_dist = 0.1;
  int max_depth = 2;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct DammLoamDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  double vertical_cos = 0.966;
  double horizontal_cos = 0.34;
  double ground_z = 0.0;
  double edge_weight = 0.5;
  double degeneracy_ratio = 0.05;
  double degeneracy_boost = 4.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct LodestarDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  double t_chi = 3.0;          // raw Hessian 向けに論文値 1.5 から緩める
  double anchor_strength = 1.0;
  int active_window = 2;
  bool enable_data_exploitation = true;
  double t_loc = 0.819;        // cos(35°)
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct TerrainRbfDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  double terrain_cell_size = 2.0;
  double terrain_sigma = 3.0;
  double terrain_ground_band = 1.0;
  double terrain_weight = 5.0;  // z-drift 抑制 soft 拘束 (opt-in、dogfooding は有効)
  int terrain_warmup = 10;
  double sensor_height_alpha = 0.05;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct LidarIbaDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  bool enable_ba = true;
  int keyframe_interval = 2;
  int window_size = 5;
  int ba_iterations = 4;
  double plane_voxel = 2.0;
  int max_landmarks = 150;
  double lidar_sigma = 0.05;
  double pose_prior_weight = 20.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct DaliSlamDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  // KITTI(IMU無)既定: deskew は off。azimuth ベースの per-point 時刻復元はこの
  // harness のデータと不整合で、CV deskew が距離誤差を増幅し長系列(seq00)で発散
  // する。退化対応 remap (DALI の主寄与) は維持。--dali-slam-... で deskew を有効化可。
  bool enable_deskew = false;
  bool spline_quadratic = false;
  double degeneracy_ratio = 0.02;
  bool enable_degeneracy = true;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct IntensityFlowDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_icp_iterations = 30;
  double initial_threshold = 2.0;
  bool enable_gradient_flow = true;
  double gf_radius = 1.0;
  int gf_num_bins = 10;
  double gf_keep_ratio = 0.5;
  bool enable_intensity = true;
  double intensity_sigma = 0.2;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct SvnIcpDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  int num_particles = 12;
  int svn_iterations = 4;
  double step_size = 1.0;
  double lidar_sigma = 0.1;
  double prior_precision = 1.0;
  double init_spread_rot = 0.01;
  double init_spread_trans = 0.05;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct PcrDatDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  int distribution_min_points = 8;
  double distribution_planarity = 0.1;
  double distance_planarity = 0.4;
  double distance_sigma = 0.1;
  double distribution_regularization = 0.01;
  double distance_weight = 1.0;
  double distribution_weight = 1.0;
  double robust_scale = 1.0;
  double prior_precision = 0.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct SmallMightyDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double planar_min = 0.4;
  double edge_min = 0.5;
  double contribution_gain = 1.0;
  double contribution_cap = 3.0;
  double plane_weight = 1.0;
  double edge_weight = 0.5;
  double robust_scale = 1.0;
  double prior_precision = 0.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct MGcloDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  double ground_normal_threshold = 0.85;
  double ground_height_offset = -0.5;
  double ground_weight = 2.0;
  double nonground_weight = 1.0;
  double ground_planarity = 0.3;
  double distribution_regularization = 0.01;
  double uncertainty_range_ref = 40.0;
  double robust_scale = 1.0;
  double prior_precision = 0.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct QuadricLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int plane_min_neighbors = 5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  int quadric_min_neighbors = 14;
  double planarity_threshold = 0.4;
  double quadric_weight = 1.0;
  double min_grad_norm = 1e-3;
  double robust_scale = 1.0;
  double prior_precision = 0.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct DiloDogfoodingOptions {
  // SRI は密な点群から構築するため既定で間引かない (0=全点)。GN ソースは DiLO 内部で
  // 別途間引く (gn_voxel_size)。
  double source_voxel_size = 0.0;
  size_t max_source_points = 200000;
  double gn_voxel_size = 0.5;
  int sri_height = 64;
  int sri_width = 1024;
  double fov_up_deg = 2.0;
  double fov_down_deg = -24.8;
  int max_iterations = 30;
  double initial_threshold = 1.0;
  double robust_scale = 0.5;
  double keyframe_translation = 2.0;
  double keyframe_rotation_deg = 10.0;
};

struct NhcLioDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  bool enable_nhc = true;
  double nhc_weight = 5.0;
  double nhc_yaw_ref = 0.03;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct StudentTLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double initial_threshold = 2.0;
  int max_iterations = 20;
  bool enable_student_t = true;
  double student_t_dof = 5.0;
  double scale_init = 0.5;
  bool estimate_scale = true;
  double scale_floor = 0.05;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct SpectralLoDogfoodingOptions {
  double source_voxel_size = 0.0;  // full cloud (BEV ラスタに密度が要る)
  size_t max_source_points = 200000;
  int bev_size = 256;
  double bev_range = 60.0;
  double max_range = 80.0;
  double z_min = -3.0;
  double z_max = 3.0;
  int logpolar_angles = 256;
  int logpolar_radii = 256;
  double max_yaw_deg = 30.0;
  double keyframe_translation = 0.0;
};

struct GmmLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  int max_iterations = 20;
  double sigma_init = 1.2;
  double sigma_final = 0.25;
  double outlier_weight = 0.1;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct GncLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double gnc_truncation = 0.3;
  double gnc_factor = 1.4;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct MccLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  double mcc_sigma = 0.3;
  bool mcc_adaptive_sigma = true;
  double mcc_sigma_floor = 0.3;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct ImlsSlamDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double normal_radius = 0.0;
  double imls_h = 0.5;
  double imls_support_radius = 0.0;
  double max_correspondence_dist = 1.0;
  int max_iterations = 20;
  int samples_per_axis = 100;
  bool use_observability_sampling = true;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct MeshLoamDogfoodingOptions {
  double source_voxel_size = 0.25;
  size_t max_source_points = 8000;
  double voxel_size = 0.1;
  double imls_h = 0.05;
  double lambda_n = 0.2;
  double curvature_threshold = 0.1;
  double cos_gate = 0.98;
  double search_radius = 0.2;
  double max_correspondence_dist = 0.5;
  int max_iterations = 30;
  int mesh_update_interval = 5;
  double local_map_radius = 80.0;
};

struct EloDogfoodingOptions {
  double source_voxel_size = 0.2;
  size_t max_source_points = 50000;
  int sri_width = 1024;
  int sri_height = 80;
  double bev_resolution = 0.25;
  double sensor_height = 1.73;
  int max_iterations = 18;
  double max_sri_correspondence_dist = 1.0;
  double max_ground_correspondence_dist = 1.0;
  double ground_weight = 5.0;
  double non_ground_weight = 1.0;
  int max_alignment_points = 12000;
  int model_window_frames = 100;
};

struct TcLvgfDogfoodingOptions {
  double source_voxel_size = 0.45;
  size_t max_source_points = 5500;
  double voxel_size = 1.0;
  double registration_voxel_size = 0.9;
  int max_registration_points = 4200;
  int image_width = 1024;
  int image_height = 64;
  int max_icp_iterations = 24;
  double max_correspondence_dist = 2.0;
  int min_visual_lines = 8;
  int max_lidar_lines = 180;
  int max_visual_lines = 120;
  double line_weight = 0.2;
  double direction_weight = 0.05;
  double fusion_pixel_gate = 8.0;
  double local_map_radius = 70.0;
  int map_cleanup_interval = 4;
};

struct OplLvioDogfoodingOptions {
  double source_voxel_size = 0.45;
  size_t max_source_points = 5500;
  double voxel_size = 1.0;
  double registration_voxel_size = 0.9;
  int max_registration_points = 4200;
  int image_width = 1024;
  int image_height = 64;
  int max_icp_iterations = 24;
  double max_correspondence_dist = 2.0;
  int min_visual_points = 20;
  int max_visual_points = 320;
  int min_visual_lines = 8;
  int max_lidar_lines = 180;
  int max_visual_lines = 120;
  double visual_point_weight = 0.08;
  double line_weight = 0.0;
  double direction_weight = 0.0;
  double helmert_min_scale = 0.25;
  double helmert_max_scale = 4.0;
  double fusion_pixel_gate = 8.0;
  double local_map_radius = 70.0;
  int map_cleanup_interval = 4;
};

struct VisualLidarAdapterDogfoodingOptions {
  double source_voxel_size = 0.45;
  size_t max_source_points = 5500;
  double voxel_size = 1.0;
  double registration_voxel_size = 0.9;
  int max_registration_points = 4200;
  int image_width = 1024;
  int image_height = 64;
  int max_icp_iterations = 24;
  double max_correspondence_dist = 2.0;
  int min_visual_points = 12;
  int max_visual_points = 300;
  int min_visual_lines = 4;
  int max_lidar_lines = 180;
  int max_visual_lines = 120;
  double local_map_radius = 70.0;
  int map_cleanup_interval = 4;
};

bool applyVisualLidarAdapterProfileArg(
    const std::string& arg, const std::string& flag_prefix,
    VisualLidarAdapterDogfoodingOptions* options) {
  if (arg == "--" + flag_prefix + "-fast-profile") {
    options->source_voxel_size = 0.55;
    options->max_source_points = 3500;
    options->voxel_size = 1.1;
    options->registration_voxel_size = 1.0;
    options->max_registration_points = 2800;
    options->image_width = 768;
    options->image_height = 64;
    options->max_icp_iterations = 18;
    options->max_lidar_lines = 120;
    options->max_visual_lines = 80;
    options->local_map_radius = 55.0;
    options->map_cleanup_interval = 3;
    return true;
  }
  if (arg == "--" + flag_prefix + "-dense-profile") {
    options->source_voxel_size = 0.35;
    options->max_source_points = 7000;
    options->voxel_size = 0.8;
    options->registration_voxel_size = 0.7;
    options->max_registration_points = 5500;
    options->image_width = 1024;
    options->image_height = 80;
    options->max_icp_iterations = 32;
    options->max_visual_points = 420;
    options->max_lidar_lines = 220;
    options->max_visual_lines = 160;
    options->local_map_radius = 80.0;
    options->map_cleanup_interval = 5;
    return true;
  }
  return false;
}

struct TricpLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int normal_min_neighbors = 5;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  int max_iterations = 20;
  bool auto_overlap = true;
  double overlap_ratio = 0.9;
  double min_overlap = 0.8;
  double frmsd_lambda = 1.5;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct KcLoDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 20;
  int max_iterations = 20;
  double kc_sigma = 0.5;
  double kc_sigma_init = 2.0;
  double kc_anneal = 0.7;
  double kc_radius_factor = 3.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct DegenSenseDogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double planarity_threshold = 0.5;
  double initial_threshold = 1.5;
  int warmup_frames = 10;
  double mad_k = 3.0;
  double gyro_bias_gain = 0.3;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct VibrationLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double planarity_threshold = 0.5;
  double max_correspondence_dist = 2.0;
  double sigma_depth = 0.02;
  double sigma_bearing = 0.0015;
  double gamma = 0.1;
  double scan_period = 0.1;
  double gyro_bias_gain = 0.3;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct IDLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  int range_image_width = 1024;
  int range_image_height = 64;
  double new_object_margin = 1.0;
  double disappearance_margin = 1.0;
  double dynamic_weight = 0.15;
  int delayed_removal_frames = 3;
  double gyro_bias_gain = 0.2;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct RFLIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  int range_image_width = 1024;
  int range_image_height = 64;
  double new_object_margin = 1.0;
  double disappearance_margin = 1.0;
  double dynamic_weight = 0.10;
  int delayed_removal_frames = 1;
  double foreground_margin = 0.9;
  double max_removal_fraction = 0.30;
  int min_keep_points = 1000;
  double gyro_bias_gain = 0.2;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct BievrLIODogfoodingOptions {
  double source_voxel_size = 0.4;
  size_t max_source_points = 6000;
  double voxel_size = 1.0;
  int voxel_min_points = 8;
  double planarity_ratio = 0.15;
  double pixel_res = 0.25;
  int max_icp_iterations = 30;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct UALIODogfoodingOptions {
  double source_voxel_size = 0.5;
  size_t max_source_points = 4500;
  double voxel_size = 1.0;
  int voxel_min_points = 6;
  int max_icp_iterations = 30;
  double max_correspondence_dist = 2.0;
  bool ground_constraint = true;
  double ground_weight = 0.5;
  double gyro_bias_gain = 0.3;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct CubeLIODogfoodingOptions {
  double input_voxel_size = 0.4;
  size_t max_input_points = 60000;
  double voxel_size = 1.0;
  int max_points_per_voxel = 12;
  int max_icp_iterations = 30;
  double planarity_threshold = 0.5;
  double initial_threshold = 1.5;
  int cubemap_size = 256;
  double gaussian_sigma = 1.0;
  double semi_dense_quantile = 0.6;
  double intensity_sigma = 30.0;
  double local_map_radius = 60.0;
  int map_cleanup_interval = 4;
};

struct CTICPDogfoodingOptions {
  double source_voxel_size = 0.75;
  size_t max_source_points = 500;
  double voxel_resolution = 1.5;
  int max_iterations = 8;
  int ceres_max_iterations = 1;
  double planarity_threshold = 0.08;
  double keypoint_voxel_size = 1.25;
  int max_frames_in_map = 8;
  // <=0 leaves the algorithm-level default of 100 m^2 unchanged. Override via
  // --ct-icp-max-correspondence-distance for paper-style tighter outlier
  // rejection (paper uses 1-2 m^2, i.e. 1-1.4 m linear).
  double max_correspondence_dist = -1.0;
  int knn = -1;  // <=0 keeps algorithm default (5).
  // Architecture-level extensions enabled by --ct-icp-paper-arch.
  bool multi_scale_correspondences = false;
  bool use_normal_cholesky_solver = false;
  // LiTAMIN2/X-ICP 風の "refinement-acceptance gate" + velocity-model bootstrap。
  // CT-ICP は素朴な constant-velocity 外挿だと急カーブで発散するため、結果が
  // 大きく seed から逸れた場合は予測値にロールバックして発散を抑える。
  bool refinement_gate = false;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
  // Seed source for the gate: when false (default), body-frame constant velocity
  // from (T_prev_prev_world, T_prev_world); when true, CT-ICP's own (begin_pose,
  // end_pose) intra-scan delta is extrapolated one scan period forward.
  bool native_ct_icp_seed = false;

  // Paper weight scheme (Gap B+D+E+F bundle).
  // 個別フラグで isolation ablation 可能。
  double power_planarity = 1.0;
  double weight_alpha = 1.0;
  double weight_neighborhood = 0.0;
  double min_planarity_floor = 0.0;
  bool flat_regularizer_weight = false;
  // 既定 -1 で CTICPParams::cauchy_loss_param=0.5 を維持。
  // paper 既定は 0.1。
  double cauchy_loss_param = -1.0;
  // <=0 で N_corr 全数 (現状)。正値で sqrt(min(N_corr, cap) * β)。
  int regularizer_n_cap = 0;

  // Pick 2 / Gap A: closest-neighbor reference + larger PCA neighborhood.
  bool use_closest_neighbor_reference = false;
  int pca_neighbor_count = 0;  // 0 で knn と同じ。paper は 20。
  // Pick 2 / Gap C: min-distance voxel insertion. 単位 m。0 で無効。paper は 0.1。
  double min_distance_between_points = 0.0;

  // Coarse-to-fine 2-phase schedule.
  bool coarse_to_fine = false;
  int coarse_iterations = 3;
  int coarse_search_radius = 2;
  double coarse_planarity_threshold = 0.05;
  double coarse_cauchy_sigma_mult = 2.0;

  // Continuous-time trajectory regularization weights. <0 leaves the
  // algorithm-level defaults (all 0.001) unchanged. These tie the current
  // frame's begin pose to the previous end pose (location/orientation) and
  // enforce constant velocity across the scan, so they trade local
  // registration freedom for trajectory smoothness (drift control).
  double location_consistency_weight = -1.0;
  double orientation_consistency_weight = -1.0;
  double constant_velocity_weight = -1.0;
};

struct CTICPNDTHybridOptions {
  int keyframe_interval = 5;
  double max_correction_translation_delta = 0.5;
  double max_correction_rotation_delta_rad = 0.35;
  // <=0 disables score gating because NDT score scale is dataset/profile
  // dependent in this dogfooding implementation.
  double max_ndt_score = -1.0;
};

struct FixedMapNDTOptions {
  int map_stride = 1;
  std::string seed_source = "gt";  // gt, velocity, ct_icp, scan_context
  double scan_context_distance_threshold = 0.18;
  int scan_context_top_k = 1;
  std::string trace_json_path;
  int publish_min_stable_frames = 5;
  int publish_max_hold_frames = 3;
  int scan_context_relock_min_confirmations = 2;
  double scan_context_relock_max_distance = 0.05;
  double scan_context_relock_max_ndt_score = 1.2;
  double scan_context_relock_max_score_delta = 0.25;
};

struct FixedMapNDTTraceFrame {
  size_t frame_index = 0;
  std::string decision = "unprocessed";
  std::string seed_source;
  bool accepted = false;
  bool seed_fallback = false;
  bool scan_context_hit = false;
  int scan_context_candidate_count = 0;
  int scan_context_candidates_evaluated = 0;
  int selected_candidate_index = -1;
  double scan_context_front_distance = std::numeric_limits<double>::quiet_NaN();
  double selected_candidate_distance = std::numeric_limits<double>::quiet_NaN();
  double selected_candidate_yaw_offset_rad = std::numeric_limits<double>::quiet_NaN();
  double ndt_score = std::numeric_limits<double>::quiet_NaN();
  int ndt_iterations = 0;
  bool ndt_converged = false;
  double seed_translation_error_m = std::numeric_limits<double>::quiet_NaN();
  double seed_rotation_error_rad = std::numeric_limits<double>::quiet_NaN();
  double refined_translation_error_m = std::numeric_limits<double>::quiet_NaN();
  double refined_rotation_error_rad = std::numeric_limits<double>::quiet_NaN();
  double final_translation_error_m = std::numeric_limits<double>::quiet_NaN();
  double final_rotation_error_rad = std::numeric_limits<double>::quiet_NaN();
  double correction_translation_delta_m = std::numeric_limits<double>::quiet_NaN();
  double correction_rotation_delta_rad = std::numeric_limits<double>::quiet_NaN();
  double final_step_m = std::numeric_limits<double>::quiet_NaN();
  double gt_step_m = std::numeric_limits<double>::quiet_NaN();
  std::string publish_action = "not_evaluated";
  std::string publish_state = "not_evaluated";
  std::string publish_gate_reason;
  std::string gt_safety_state = "not_evaluated";
  bool allow_pose_publish = false;
  bool has_pose_output = false;
  bool runtime_refinement_candidate = false;
  bool relock_candidate = false;
  bool relock_sequence_compatible = false;
  bool gt_wrong_pose = false;
  bool gt_recovery_transition = false;
  bool gt_unsafe_transition = false;
  int runtime_refinement_streak = 0;
  int relock_streak = 0;
  int relock_candidate_index_delta = 0;
  int relock_frame_gap = 0;
  int publish_hold_age_frames = 0;
  double published_translation_error_m = std::numeric_limits<double>::quiet_NaN();
  std::string relock_rejected_reason;
  Eigen::Matrix4d seed_pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d refined_pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d final_pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d gt_pose = Eigen::Matrix4d::Identity();
  Eigen::Matrix4d published_pose = Eigen::Matrix4d::Identity();
};

struct FixedMapNDTPublishPolicyState {
  int runtime_refinement_streak = 0;
  int hold_streak = 0;
  bool has_last_published_pose = false;
  size_t last_published_frame = 0;
  Eigen::Matrix4d last_published_pose = Eigen::Matrix4d::Identity();
  int scan_context_relock_streak = 0;
  bool has_last_scan_context_relock = false;
  size_t last_scan_context_relock_frame = 0;
  int last_scan_context_relock_candidate_index = -1;
  double last_scan_context_relock_score =
      std::numeric_limits<double>::quiet_NaN();
};

MethodResult runCTICP(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const CTICPDogfoodingOptions& options,
                      bool gt_seed);

struct DLODofeedingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
  int gicp_k_neighbors = 15;
  double gicp_max_correspondence_distance = 4.0;
  int gicp_max_iterations = 25;
  double gicp_rotation_epsilon = 1e-4;
  double gicp_translation_epsilon = 1e-4;
  double gicp_covariance_regularization = 1e-3;
  double gicp_fitness_epsilon = 1e-6;
};

struct DLIODogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  double keyframe_translation_threshold = 0.6;
  double keyframe_rotation_threshold_rad = 8.0 * 3.14159265358979323846 / 180.0;
  int max_keyframes_in_map = 30;
  int gicp_k_neighbors = 15;
  double gicp_max_correspondence_distance = 4.0;
  int gicp_max_iterations = 25;
  double gicp_rotation_epsilon = 1e-4;
  double gicp_translation_epsilon = 1e-4;
  double gicp_covariance_regularization = 1e-3;
  double gicp_fitness_epsilon = 1e-6;
  double imu_rotation_fusion_weight = 0.15;
  double imu_translation_fusion_weight = 0.2;
  double imu_velocity_fusion_weight = 0.2;
  double lidar_confidence_correspondence_scale = 100.0;
};

struct XICPDogfoodingOptions {
  double source_voxel_size = 1.0;
  size_t max_source_points = 2500;
  int max_iterations = 30;
  double convergence_threshold = 1e-5;
  double kappa_f = 0.1736;
  double kappa_s = 0.707;
  int k_neighbors = 10;
  double max_correspondence_distance = 3.0;
  size_t map_max_points = 40000;
  size_t refresh_interval = 4;
  double map_radius = 45.0;
  double max_seed_translation_delta = 2.0;
  double max_seed_rotation_delta_rad = 0.25;
};

struct FastLio2DogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  int max_iterations = 8;
  int ceres_max_iterations = 6;
  double voxel_resolution = 1.0;
  double keypoint_voxel_size = 0.5;
  int knn = 5;
  double max_correspondence_distance = 3.0;
  double planarity_threshold = 0.1;
  int max_frames_in_map = 30;
};

struct HdlGraphSlamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.7;
  double map_voxel_size = 1.0;
  int keyframe_stride = 1;
  int max_keyframes_in_submap = 30;
  size_t loop_stride = 3;
  bool enable_loop_closure = true;
  bool enable_floor_constraint = true;
};

struct VgicpSlamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.4;
  double map_voxel_size = 0.6;
  int keyframe_stride = 1;
  int max_keyframes_in_submap = 30;
  size_t loop_stride = 3;
  bool enable_loop_closure = true;
};

struct SuMaDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 80.0;
  double surfel_resolution = 0.4;
  int surfel_stride = 1;
  int max_iterations = 4;
  int max_frames_in_map = 20;
};

struct Balm2DogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double corner_resolution = 0.3;
  double surface_resolution = 0.6;
  int corner_stride = 2;
  int surface_stride = 3;
  int window_size = 5;
  int num_outer_iterations = 2;
  int ceres_max_iterations = 6;
};

struct IscLoamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  int keyframe_stride = 1;
  size_t loop_stride = 3;
  bool enable_loop_closure = true;
};

struct LoamLivoxDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  float less_flat_leaf_size = 0.2f;
  int odom_outer_iters = 2;
  int map_outer_iters = 2;
  double map_line_resolution = 0.4;
  double map_plane_resolution = 0.8;
};

struct LioSamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  int keyframe_stride = 1;
  bool enable_loop_closure = true;
  bool enable_imu_rotation_prior = true;
};

struct LINSDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.5;
  double map_voxel_size = 0.8;
  int max_correspondences = 200;
  int max_keyframes_in_map = 30;
};

struct FastLioSlamDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  int max_iterations = 8;
  int ceres_max_iterations = 6;
  double voxel_resolution = 1.0;
  double keypoint_voxel_size = 0.5;
  int max_frames_in_map = 30;
  int keyframe_stride = 1;
  size_t loop_stride = 4;
  bool enable_loop_closure = true;
};

struct PointLioDogfoodingOptions {
  double input_voxel_size = 0.5;
  size_t max_input_points = 6000;
  double min_range = 1.0;
  double max_range = 100.0;
  double registration_voxel_size = 0.4;
  double map_voxel_size = 0.7;
  int max_correspondences = 250;
  int max_keyframes_in_map = 30;
};

struct CLINSDogfoodingOptions {
  double source_voxel_size = 0.75;
  size_t max_source_points = 500;
};

/// ワールド座標に変換
std::vector<Eigen::Vector3d> transformPoints(
    const std::vector<Eigen::Vector3d>& pts, const Eigen::Matrix4d& T) {
  Eigen::Matrix3d R = T.block<3, 3>(0, 0);
  Eigen::Vector3d t = T.block<3, 1>(0, 3);
  std::vector<Eigen::Vector3d> out;
  out.reserve(pts.size());
  for (auto& p : pts) out.push_back(R * p + t);
  return out;
}

void compactPointMap(std::vector<Eigen::Vector3d>& map_points,
                     size_t max_points) {
  if (map_points.size() <= max_points) return;
  const size_t tail_keep = std::min(map_points.size(), max_points * 3 / 4);
  const size_t head_keep = max_points - tail_keep;

  std::vector<Eigen::Vector3d> compacted;
  compacted.reserve(max_points);
  if (head_keep > 0) {
    const size_t head_size = map_points.size() - tail_keep;
    if (head_size > 0) {
      const double step = static_cast<double>(head_size) / head_keep;
      for (size_t i = 0; i < head_keep; i++) {
        size_t idx = std::min(static_cast<size_t>(i * step), head_size - 1);
        compacted.push_back(map_points[idx]);
      }
    }
  }
  compacted.insert(compacted.end(), map_points.end() - tail_keep, map_points.end());
  map_points.swap(compacted);
}

void prunePointMapByRadius(std::vector<Eigen::Vector3d>& map_points,
                           const Eigen::Vector3d& center,
                           double radius) {
  if (radius <= 0.0 || map_points.empty()) return;

  const double radius_sq = radius * radius;
  std::vector<Eigen::Vector3d> filtered;
  filtered.reserve(map_points.size());
  for (const auto& point : map_points) {
    if ((point - center).squaredNorm() <= radius_sq) {
      filtered.push_back(point);
    }
  }
  map_points.swap(filtered);
}

void addPointsToMap(std::vector<Eigen::Vector3d>& map_points,
                    const std::vector<Eigen::Vector3d>& local_points,
                    const Eigen::Matrix4d& pose,
                    size_t max_points,
                    double local_map_radius = 0.0) {
  auto points_world = transformPoints(local_points, pose);
  map_points.insert(map_points.end(), points_world.begin(), points_world.end());
  prunePointMapByRadius(map_points, pose.block<3, 1>(0, 3), local_map_radius);
  compactPointMap(map_points, max_points);
}

Eigen::Matrix4d anchorRelativePose(const Eigen::Matrix4d& anchor_pose,
                                   const Eigen::Matrix4d& relative_pose) {
  return anchor_pose * relative_pose;
}

bool shouldRefreshTargetMap(size_t frame_index, size_t refresh_interval) {
  return frame_index <= 1 || refresh_interval <= 1 ||
         (frame_index % refresh_interval) == 0;
}

double poseTranslationDelta(const Eigen::Matrix4d& lhs,
                            const Eigen::Matrix4d& rhs) {
  return (lhs.block<3, 1>(0, 3) - rhs.block<3, 1>(0, 3)).norm();
}

double poseRotationDelta(const Eigen::Matrix4d& lhs,
                         const Eigen::Matrix4d& rhs) {
  const Eigen::Matrix3d dR =
      lhs.block<3, 3>(0, 0).transpose() * rhs.block<3, 3>(0, 0);
  Eigen::AngleAxisd aa(dR);
  if (!std::isfinite(aa.angle())) {
    return std::numeric_limits<double>::infinity();
  }
  return std::abs(aa.angle());
}

bool isReasonableRefinement(const Eigen::Matrix4d& refined_pose,
                            const Eigen::Matrix4d& seed_pose,
                            double max_translation_delta,
                            double max_rotation_delta_rad) {
  return poseTranslationDelta(refined_pose, seed_pose) <= max_translation_delta &&
         poseRotationDelta(refined_pose, seed_pose) <= max_rotation_delta_rad;
}

double poseTranslationErrorToGT(const Eigen::Matrix4d& pose,
                                const std::vector<Eigen::Matrix4d>& gt,
                                size_t frame_index) {
  if (frame_index >= gt.size() || !isFiniteMatrix(pose)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return poseTranslationDelta(pose, gt[frame_index]);
}

double poseRotationErrorToGT(const Eigen::Matrix4d& pose,
                             const std::vector<Eigen::Matrix4d>& gt,
                             size_t frame_index) {
  if (frame_index >= gt.size() || !isFiniteMatrix(pose)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return poseRotationDelta(pose, gt[frame_index]);
}

void finalizeFixedMapNDTTraceFrame(FixedMapNDTTraceFrame& frame,
                                   const std::vector<Eigen::Matrix4d>& gt,
                                   const Eigen::Matrix4d* previous_final_pose) {
  const size_t i = frame.frame_index;
  if (i < gt.size()) {
    frame.gt_pose = gt[i];
  }
  frame.seed_translation_error_m =
      poseTranslationErrorToGT(frame.seed_pose, gt, i);
  frame.seed_rotation_error_rad = poseRotationErrorToGT(frame.seed_pose, gt, i);
  frame.refined_translation_error_m =
      poseTranslationErrorToGT(frame.refined_pose, gt, i);
  frame.refined_rotation_error_rad =
      poseRotationErrorToGT(frame.refined_pose, gt, i);
  frame.final_translation_error_m =
      poseTranslationErrorToGT(frame.final_pose, gt, i);
  frame.final_rotation_error_rad =
      poseRotationErrorToGT(frame.final_pose, gt, i);
  frame.correction_translation_delta_m =
      poseTranslationDelta(frame.final_pose, frame.seed_pose);
  frame.correction_rotation_delta_rad =
      poseRotationDelta(frame.final_pose, frame.seed_pose);
  if (previous_final_pose != nullptr) {
    frame.final_step_m = poseTranslationDelta(frame.final_pose, *previous_final_pose);
  }
  if (i > 0 && i < gt.size()) {
    frame.gt_step_m = poseTranslationDelta(gt[i], gt[i - 1]);
  }
}

bool fixedMapNDTTraceHasWrongPose(const FixedMapNDTTraceFrame& frame) {
  return !std::isfinite(frame.final_translation_error_m) ||
         frame.final_translation_error_m > 1.0;
}

double finiteRatio(double numerator, double denominator) {
  if (!std::isfinite(numerator) || !std::isfinite(denominator) ||
      denominator <= 1e-9) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return numerator / denominator;
}

void annotateFixedMapNDTGTSafety(FixedMapNDTTraceFrame& frame,
                                 const FixedMapNDTTraceFrame* previous_frame) {
  frame.gt_wrong_pose = fixedMapNDTTraceHasWrongPose(frame);
  const double step_ratio = finiteRatio(frame.final_step_m, frame.gt_step_m);
  const double previous_error =
      previous_frame ? previous_frame->final_translation_error_m
                     : std::numeric_limits<double>::quiet_NaN();
  const double transition_error_delta =
      (std::isfinite(previous_error) &&
       std::isfinite(frame.final_translation_error_m))
          ? previous_error - frame.final_translation_error_m
          : std::numeric_limits<double>::quiet_NaN();
  const bool is_jump = std::isfinite(step_ratio) && step_ratio > 3.0;
  frame.gt_recovery_transition =
      is_jump && std::isfinite(transition_error_delta) &&
      transition_error_delta >= 1.0 &&
      std::isfinite(frame.final_translation_error_m) &&
      frame.final_translation_error_m <= 1.0;
  frame.gt_unsafe_transition =
      is_jump && !frame.gt_recovery_transition &&
      (!std::isfinite(frame.final_translation_error_m) ||
       frame.final_translation_error_m > 1.0 ||
       (std::isfinite(transition_error_delta) &&
        transition_error_delta < 0.0));

  if (frame.gt_unsafe_transition) {
    frame.gt_safety_state = "unsafe_transition";
  } else if (frame.gt_recovery_transition) {
    frame.gt_safety_state = "recovery_transition";
  } else if (frame.gt_wrong_pose) {
    frame.gt_safety_state = "wrong_pose";
  } else {
    frame.gt_safety_state = "inside_publish_envelope";
  }
}

bool fixedMapNDTRuntimeRefinementCandidate(const FixedMapNDTTraceFrame& frame) {
  return frame.accepted && !frame.seed_fallback &&
         isFiniteMatrix(frame.final_pose) && std::isfinite(frame.ndt_score) &&
         (frame.ndt_converged || frame.ndt_iterations >= 2);
}

bool fixedMapNDTScanContextRelockCandidate(
    FixedMapNDTTraceFrame& frame,
    const FixedMapNDTOptions& options) {
  frame.relock_candidate = false;
  if (options.seed_source != "scan_context") {
    frame.relock_rejected_reason = "not_scan_context_seed";
    return false;
  }
  if (!frame.runtime_refinement_candidate) {
    frame.relock_rejected_reason = "not_runtime_refinement_candidate";
    return false;
  }
  if (frame.selected_candidate_index < 0) {
    frame.relock_rejected_reason = "missing_selected_global_candidate";
    return false;
  }
  if (!std::isfinite(frame.selected_candidate_distance) ||
      frame.selected_candidate_distance >
          options.scan_context_relock_max_distance) {
    frame.relock_rejected_reason = "scan_context_distance_too_large";
    return false;
  }
  if (!std::isfinite(frame.ndt_score) ||
      frame.ndt_score > options.scan_context_relock_max_ndt_score) {
    frame.relock_rejected_reason = "ndt_score_too_large_for_relock";
    return false;
  }
  frame.relock_candidate = true;
  frame.relock_rejected_reason.clear();
  return true;
}

void updateFixedMapNDTScanContextRelockState(
    FixedMapNDTTraceFrame& frame,
    const FixedMapNDTOptions& options,
    FixedMapNDTPublishPolicyState& state) {
  if (!fixedMapNDTScanContextRelockCandidate(frame, options)) {
    frame.relock_streak = state.scan_context_relock_streak;
    return;
  }

  bool compatible = true;
  if (state.has_last_scan_context_relock) {
    const int index_delta =
        frame.selected_candidate_index -
        state.last_scan_context_relock_candidate_index;
    const int frame_gap =
        static_cast<int>(frame.frame_index -
                         state.last_scan_context_relock_frame);
    const int expected_frame_gap =
        index_delta * std::max(1, options.map_stride);
    const int tolerance = std::max(1, std::max(1, options.map_stride) / 2);
    const double score_delta =
        std::isfinite(state.last_scan_context_relock_score)
            ? std::abs(frame.ndt_score - state.last_scan_context_relock_score)
            : 0.0;
    frame.relock_candidate_index_delta = index_delta;
    frame.relock_frame_gap = frame_gap;
    compatible =
        index_delta > 0 &&
        std::abs(frame_gap - expected_frame_gap) <= tolerance &&
        score_delta <= options.scan_context_relock_max_score_delta;
  }

  frame.relock_sequence_compatible = compatible;
  if (compatible) {
    state.scan_context_relock_streak =
        state.has_last_scan_context_relock
            ? state.scan_context_relock_streak + 1
            : 1;
  } else {
    state.scan_context_relock_streak = 1;
    frame.relock_rejected_reason = "relock_sequence_incompatible_reset";
  }
  state.has_last_scan_context_relock = true;
  state.last_scan_context_relock_frame = frame.frame_index;
  state.last_scan_context_relock_candidate_index =
      frame.selected_candidate_index;
  state.last_scan_context_relock_score = frame.ndt_score;
  frame.relock_streak = state.scan_context_relock_streak;
}

void returnUnknownFixedMapNDTPublish(FixedMapNDTTraceFrame& frame,
                                     FixedMapNDTPublishPolicyState& state,
                                     const std::string& publish_state,
                                     const std::string& reason) {
  frame.publish_action = "return_unknown";
  frame.publish_state = publish_state;
  frame.publish_gate_reason = reason;
  frame.allow_pose_publish = false;
  frame.has_pose_output = false;
  frame.publish_hold_age_frames = 0;
  state.hold_streak = 0;
}

void annotateFixedMapNDTPublishPolicy(
    FixedMapNDTTraceFrame& frame,
    const FixedMapNDTOptions& options,
    FixedMapNDTPublishPolicyState& state,
    const FixedMapNDTTraceFrame* previous_frame) {
  annotateFixedMapNDTGTSafety(frame, previous_frame);
  frame.runtime_refinement_candidate =
      fixedMapNDTRuntimeRefinementCandidate(frame);

  if (frame.decision == "anchor") {
    state.runtime_refinement_streak = 0;
    returnUnknownFixedMapNDTPublish(
        frame, state, "anchor_frame",
        "initial anchor is not a runtime publish proof");
    return;
  }

  if (options.seed_source == "gt") {
    state.runtime_refinement_streak = 0;
    returnUnknownFixedMapNDTPublish(
        frame, state, "lab_only_oracle_seed",
        "GT seed validates the localizer but is not deployable");
    return;
  }

  if (!frame.runtime_refinement_candidate) {
    state.runtime_refinement_streak = 0;
    const std::string publish_state =
        frame.seed_fallback ? "seed_fallback_unknown"
                            : "refinement_rejected_unknown";
    const std::string reason =
        frame.seed_fallback
            ? "seed fallback has no fixed-map verification"
            : "NDT refinement was not accepted by runtime gates";
    if (options.seed_source != "scan_context" &&
        state.has_last_published_pose &&
        state.hold_streak < std::max(0, options.publish_max_hold_frames)) {
      ++state.hold_streak;
      frame.publish_action = "hold_last_pose";
      frame.publish_state = publish_state;
      frame.publish_gate_reason = reason;
      frame.allow_pose_publish = true;
      frame.has_pose_output = true;
      frame.publish_hold_age_frames =
          static_cast<int>(frame.frame_index - state.last_published_frame);
      frame.published_pose = state.last_published_pose;
      frame.published_translation_error_m =
          poseTranslationDelta(frame.published_pose, frame.gt_pose);
    } else {
      returnUnknownFixedMapNDTPublish(frame, state, publish_state, reason);
    }
    return;
  }

  ++state.runtime_refinement_streak;
  frame.runtime_refinement_streak = state.runtime_refinement_streak;
  if (options.seed_source == "scan_context") {
    updateFixedMapNDTScanContextRelockState(frame, options, state);
    if (!frame.relock_candidate) {
      returnUnknownFixedMapNDTPublish(
          frame, state, "relock_candidate_rejected",
          frame.relock_rejected_reason.empty()
              ? "Scan Context candidate failed relock gates"
              : frame.relock_rejected_reason);
      return;
    }
    if (!frame.relock_sequence_compatible ||
        frame.relock_streak <
            std::max(1, options.scan_context_relock_min_confirmations)) {
      returnUnknownFixedMapNDTPublish(
          frame, state, "warming_up_relock_sequence",
          "Scan Context relock needs another sequence-compatible confirmation");
      return;
    }

    state.hold_streak = 0;
    state.has_last_published_pose = true;
    state.last_published_frame = frame.frame_index;
    state.last_published_pose = frame.final_pose;
    frame.publish_action = "publish_pose";
    frame.publish_state = "scan_context_relock_verified";
    frame.publish_gate_reason =
        "Scan Context descriptor, NDT score, and relock sequence are consistent";
    frame.allow_pose_publish = true;
    frame.has_pose_output = true;
    frame.publish_hold_age_frames = 0;
    frame.published_pose = frame.final_pose;
    frame.published_translation_error_m = frame.final_translation_error_m;
    return;
  }

  if (state.runtime_refinement_streak <
      std::max(1, options.publish_min_stable_frames)) {
    returnUnknownFixedMapNDTPublish(
        frame, state, "warming_up_stable_refinement",
        "runtime refinement streak is shorter than the publish threshold");
    return;
  }

  state.hold_streak = 0;
  state.has_last_published_pose = true;
  state.last_published_frame = frame.frame_index;
  state.last_published_pose = frame.final_pose;
  frame.publish_action = "publish_pose";
  frame.publish_state = "runtime_stable_refinement";
  frame.publish_gate_reason =
      "accepted runtime refinement passed the stable publish threshold";
  frame.allow_pose_publish = true;
  frame.has_pose_output = true;
  frame.publish_hold_age_frames = 0;
  frame.published_pose = frame.final_pose;
  frame.published_translation_error_m = frame.final_translation_error_m;
}

void writeFixedMapNDTTraceJson(const std::string& path,
                               const FixedMapNDTOptions& options,
                               size_t num_frames,
                               size_t map_points,
                               const std::vector<FixedMapNDTTraceFrame>& frames) {
  if (path.empty()) return;
  const fs::path out_path(path);
  if (!out_path.parent_path().empty()) {
    fs::create_directories(out_path.parent_path());
  }

  auto write_number = [](std::ostream& out, double value) {
    if (std::isfinite(value)) {
      out << std::fixed << std::setprecision(9) << value;
    } else {
      out << "null";
    }
  };
  auto write_bool = [](std::ostream& out, bool value) {
    out << (value ? "true" : "false");
  };
  auto write_pose = [&write_number](std::ostream& out,
                                    const Eigen::Matrix4d& pose) {
    out << "[";
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (r != 0 || c != 0) out << ", ";
        write_number(out, pose(r, c));
      }
    }
    out << "]";
  };

  std::ofstream out(path);
  out << "{\n";
  out << "  \"trace_version\": \"fixed_map_ndt_trace_v2\",\n";
  out << "  \"seed_source\": \"" << jsonEscape(options.seed_source) << "\",\n";
  out << "  \"map_stride\": " << std::max(1, options.map_stride) << ",\n";
  out << "  \"scan_context_distance_threshold\": ";
  write_number(out, options.scan_context_distance_threshold);
  out << ",\n";
  out << "  \"scan_context_top_k\": "
      << std::max(1, options.scan_context_top_k) << ",\n";
  out << "  \"publish_policy\": {\n";
  out << "    \"policy_version\": \"fixed_map_ndt_runtime_publish_policy_v1\",\n";
  out << "    \"gt_safety_label_version\": \"fixed_map_ndt_trace_gt_safety_v1\",\n";
  out << "    \"publish_error_m\": 1.000000000,\n";
  out << "    \"jump_step_ratio\": 3.000000000,\n";
  out << "    \"recovery_error_drop_m\": 1.000000000,\n";
  out << "    \"min_stable_frames\": "
      << std::max(1, options.publish_min_stable_frames) << ",\n";
  out << "    \"max_hold_frames\": "
      << std::max(0, options.publish_max_hold_frames) << ",\n";
  out << "    \"scan_context_relock_min_confirmations\": "
      << std::max(1, options.scan_context_relock_min_confirmations) << ",\n";
  out << "    \"scan_context_relock_max_distance\": ";
  write_number(out, options.scan_context_relock_max_distance);
  out << ",\n";
  out << "    \"scan_context_relock_max_ndt_score\": ";
  write_number(out, options.scan_context_relock_max_ndt_score);
  out << ",\n";
  out << "    \"scan_context_relock_max_score_delta\": ";
  write_number(out, options.scan_context_relock_max_score_delta);
  out << ",\n";
  out << "    \"scan_context_publish_requires_relock_sequence_verifier\": true\n";
  out << "  },\n";
  out << "  \"num_frames\": " << num_frames << ",\n";
  out << "  \"map_points\": " << map_points << ",\n";
  out << "  \"frames\": [\n";
  for (size_t i = 0; i < frames.size(); i++) {
    const auto& frame = frames[i];
    out << "    {\n";
    out << "      \"frame_index\": " << frame.frame_index << ",\n";
    out << "      \"decision\": \"" << jsonEscape(frame.decision) << "\",\n";
    out << "      \"seed_source\": \"" << jsonEscape(frame.seed_source) << "\",\n";
    out << "      \"accepted\": ";
    write_bool(out, frame.accepted);
    out << ",\n";
    out << "      \"seed_fallback\": ";
    write_bool(out, frame.seed_fallback);
    out << ",\n";
    out << "      \"scan_context_hit\": ";
    write_bool(out, frame.scan_context_hit);
    out << ",\n";
    out << "      \"scan_context_candidate_count\": "
        << frame.scan_context_candidate_count << ",\n";
    out << "      \"scan_context_candidates_evaluated\": "
        << frame.scan_context_candidates_evaluated << ",\n";
    out << "      \"selected_candidate_index\": "
        << frame.selected_candidate_index << ",\n";
    out << "      \"scan_context_front_distance\": ";
    write_number(out, frame.scan_context_front_distance);
    out << ",\n";
    out << "      \"selected_candidate_distance\": ";
    write_number(out, frame.selected_candidate_distance);
    out << ",\n";
    out << "      \"selected_candidate_yaw_offset_rad\": ";
    write_number(out, frame.selected_candidate_yaw_offset_rad);
    out << ",\n";
    out << "      \"ndt_score\": ";
    write_number(out, frame.ndt_score);
    out << ",\n";
    out << "      \"ndt_iterations\": " << frame.ndt_iterations << ",\n";
    out << "      \"ndt_converged\": ";
    write_bool(out, frame.ndt_converged);
    out << ",\n";
    out << "      \"publish_action\": \""
        << jsonEscape(frame.publish_action) << "\",\n";
    out << "      \"publish_state\": \""
        << jsonEscape(frame.publish_state) << "\",\n";
    out << "      \"publish_gate_reason\": \""
        << jsonEscape(frame.publish_gate_reason) << "\",\n";
    out << "      \"allow_pose_publish\": ";
    write_bool(out, frame.allow_pose_publish);
    out << ",\n";
    out << "      \"has_pose_output\": ";
    write_bool(out, frame.has_pose_output);
    out << ",\n";
    out << "      \"runtime_refinement_candidate\": ";
    write_bool(out, frame.runtime_refinement_candidate);
    out << ",\n";
    out << "      \"relock_candidate\": ";
    write_bool(out, frame.relock_candidate);
    out << ",\n";
    out << "      \"relock_sequence_compatible\": ";
    write_bool(out, frame.relock_sequence_compatible);
    out << ",\n";
    out << "      \"runtime_refinement_streak\": "
        << frame.runtime_refinement_streak << ",\n";
    out << "      \"relock_streak\": "
        << frame.relock_streak << ",\n";
    out << "      \"relock_candidate_index_delta\": "
        << frame.relock_candidate_index_delta << ",\n";
    out << "      \"relock_frame_gap\": "
        << frame.relock_frame_gap << ",\n";
    out << "      \"relock_rejected_reason\": \""
        << jsonEscape(frame.relock_rejected_reason) << "\",\n";
    out << "      \"publish_hold_age_frames\": "
        << frame.publish_hold_age_frames << ",\n";
    out << "      \"gt_safety_state\": \""
        << jsonEscape(frame.gt_safety_state) << "\",\n";
    out << "      \"gt_wrong_pose\": ";
    write_bool(out, frame.gt_wrong_pose);
    out << ",\n";
    out << "      \"gt_recovery_transition\": ";
    write_bool(out, frame.gt_recovery_transition);
    out << ",\n";
    out << "      \"gt_unsafe_transition\": ";
    write_bool(out, frame.gt_unsafe_transition);
    out << ",\n";
    out << "      \"published_translation_error_m\": ";
    write_number(out, frame.published_translation_error_m);
    out << ",\n";
    out << "      \"seed_translation_error_m\": ";
    write_number(out, frame.seed_translation_error_m);
    out << ",\n";
    out << "      \"seed_rotation_error_rad\": ";
    write_number(out, frame.seed_rotation_error_rad);
    out << ",\n";
    out << "      \"refined_translation_error_m\": ";
    write_number(out, frame.refined_translation_error_m);
    out << ",\n";
    out << "      \"refined_rotation_error_rad\": ";
    write_number(out, frame.refined_rotation_error_rad);
    out << ",\n";
    out << "      \"final_translation_error_m\": ";
    write_number(out, frame.final_translation_error_m);
    out << ",\n";
    out << "      \"final_rotation_error_rad\": ";
    write_number(out, frame.final_rotation_error_rad);
    out << ",\n";
    out << "      \"correction_translation_delta_m\": ";
    write_number(out, frame.correction_translation_delta_m);
    out << ",\n";
    out << "      \"correction_rotation_delta_rad\": ";
    write_number(out, frame.correction_rotation_delta_rad);
    out << ",\n";
    out << "      \"final_step_m\": ";
    write_number(out, frame.final_step_m);
    out << ",\n";
    out << "      \"gt_step_m\": ";
    write_number(out, frame.gt_step_m);
    out << ",\n";
    out << "      \"seed_pose\": ";
    write_pose(out, frame.seed_pose);
    out << ",\n";
    out << "      \"refined_pose\": ";
    write_pose(out, frame.refined_pose);
    out << ",\n";
    out << "      \"final_pose\": ";
    write_pose(out, frame.final_pose);
    out << ",\n";
    out << "      \"published_pose\": ";
    if (frame.has_pose_output) {
      write_pose(out, frame.published_pose);
    } else {
      out << "null";
    }
    out << ",\n";
    out << "      \"gt_pose\": ";
    write_pose(out, frame.gt_pose);
    out << "\n";
    out << "    }" << (i + 1 == frames.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

// Constant-velocity prediction in the body frame: T_pred = T_prev * (T_prev_prev^-1 * T_prev).
// Equivalent to applying the previous inter-frame body-frame motion once more.
Eigen::Matrix4d velocityModelPrediction(const Eigen::Matrix4d& T_prev,
                                        const Eigen::Matrix4d& T_prev_prev) {
  const Eigen::Matrix4d delta_body = T_prev_prev.inverse() * T_prev;
  return T_prev * delta_body;
}

MethodResult runLiTAMIN2(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LiTAMIN2DogfoodingOptions& options,
                         bool no_gt_seed = false) {
  using namespace localization_zoo::litamin2;
  MethodResult res;
  res.name = options.use_cov_cost ? "LiTAMIN2" : "LiTAMIN2-ICP";

  LiTAMIN2Params params;
  params.voxel_resolution = options.voxel_resolution;
  params.min_points_per_voxel = options.min_points_per_voxel;
  params.max_iterations = options.max_iterations;
  params.use_cov_cost = options.use_cov_cost;
  params.num_threads = options.num_threads;
  LiTAMIN2Registration reg(params);
  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];  // 初期推定にGTを使用
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.5),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    // scan-to-map: local scan を world map に対して初期値付きで最適化
    const Eigen::Matrix4d T_init_guess =
        no_gt_seed
            ? (i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est)
            : applySeedPerturbation(gt[i], options.seed_perturbation);
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_iterations >= 3) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
    }

    if (i % 10 == 0)
      std::cerr << "\r  [LiTAMIN2] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  if (!no_gt_seed) {
    res.note += seedPerturbationNote(options.seed_perturbation);
  }
  if (!options.use_cov_cost) {
    res.note += " Covariance-shape term disabled.";
  }
  return res;
}

MethodResult runGICP(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const GICPDogfoodingOptions& options,
                     bool no_gt_seed = false) {
  using namespace localization_zoo::gicp;
  MethodResult res;
  res.name = "GICP";

  GICPParams params;
  params.k_neighbors = options.k_neighbors;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_iterations = options.max_iterations;
  GICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess =
        no_gt_seed
            ? (i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est)
            : applySeedPerturbation(gt[i], options.seed_perturbation);
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_correspondences >= 128) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  if (!no_gt_seed) {
    res.note += seedPerturbationNote(options.seed_perturbation);
  }
  return res;
}

MethodResult runSmallGICP(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const SmallGICPDogfoodingOptions& options,
                          bool no_gt_seed = false) {
  using namespace localization_zoo::small_gicp;
  MethodResult res;
  res.name = "Small-GICP";

  SmallGICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.k_neighbors = options.k_neighbors;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_correspondences = options.max_correspondences;
  params.max_iterations = options.max_iterations;
  params.rotation_epsilon = options.rotation_epsilon;
  params.translation_epsilon = options.translation_epsilon;
  params.covariance_regularization = options.covariance_regularization;
  params.fitness_epsilon = options.fitness_epsilon;
  SmallGICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  size_t seed_fallbacks = 0;
  size_t seeded_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess =
        no_gt_seed
            ? (i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est)
            : applySeedPerturbation(gt[i], options.seed_perturbation);
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    const auto result = reg.align(pts_local, T_init_guess);
    ++seeded_frames;
    if ((result.converged || result.num_correspondences >= 96) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
      ++seed_fallbacks;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [Small-GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  if (!no_gt_seed && seeded_frames > 0) {
    res.seed_fallback_rate_pct = 100.0 *
        static_cast<double>(seed_fallbacks) / static_cast<double>(seeded_frames);
    std::cerr << "  [Small-GICP] seed fallbacks=" << seed_fallbacks << "/"
              << seeded_frames << " (" << res.seed_fallback_rate_pct
              << "% rolled back to GT seed)" << std::endl;
  }
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  if (!no_gt_seed) {
    res.note += seedPerturbationNote(options.seed_perturbation);
  }
  return res;
}

MethodResult runVoxelGICP(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const VoxelGICPDogfoodingOptions& options,
                          bool no_gt_seed = false) {
  using namespace localization_zoo::voxel_gicp;
  MethodResult res;
  res.name = "Voxel-GICP";

  VoxelGICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.min_points_per_voxel = options.min_points_per_voxel;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.max_iterations = options.max_iterations;
  params.rotation_epsilon = options.rotation_epsilon;
  params.translation_epsilon = options.translation_epsilon;
  params.covariance_regularization = options.covariance_regularization;
  params.fitness_epsilon = options.fitness_epsilon;
  VoxelGICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    const Eigen::Matrix4d T_init_guess =
        no_gt_seed
            ? (i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est)
            : applySeedPerturbation(gt[i], options.seed_perturbation);
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    const auto result = reg.align(pts_local, T_init_guess);
    if ((result.converged || result.num_correspondences >= 96) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [Voxel-GICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  if (!no_gt_seed) {
    res.note += seedPerturbationNote(options.seed_perturbation);
  }
  return res;
}

MethodResult runALOAM(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const ALOAMDogfoodingOptions& options) {
  using namespace localization_zoo::aloam;
  MethodResult res;
  res.name = "A-LOAM";

  ScanRegistrationParams scan_params;
  scan_params.n_scans = options.n_scans;
  scan_params.minimum_range = options.minimum_range;
  scan_params.scan_period = options.scan_period;
  scan_params.curvature_threshold = options.curvature_threshold;
  scan_params.max_corner_sharp = options.max_corner_sharp;
  scan_params.max_corner_less_sharp = options.max_corner_less_sharp;
  scan_params.max_surf_flat = options.max_surf_flat;
  scan_params.less_flat_leaf_size = options.less_flat_leaf_size;
  ScanRegistration scan_reg(scan_params);

  LaserOdometryParams odom_params;
  odom_params.distance_sq_threshold = options.odom_distance_sq_threshold;
  odom_params.nearby_scan = options.odom_nearby_scan;
  odom_params.num_optimization_iters = options.odom_outer_iters;
  odom_params.ceres_max_iterations = options.odom_ceres_iters;
  odom_params.huber_loss_s = options.odom_huber_loss_s;
  odom_params.enable_distortion = false;
  LaserOdometry laser_odom(odom_params);

  LaserMappingParams map_params;
  map_params.line_resolution = options.map_line_resolution;
  map_params.plane_resolution = options.map_plane_resolution;
  map_params.num_optimization_iters = options.map_outer_iters;
  map_params.ceres_max_iterations = options.map_ceres_iters;
  map_params.huber_loss_s = options.map_huber_loss_s;
  map_params.knn = options.map_knn;
  map_params.knn_max_dist_sq = options.map_knn_max_dist_sq;
  map_params.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  map_params.plane_threshold = options.map_plane_threshold;
  LaserMapping laser_map(map_params);

  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;

    auto cloud = toPclXYZICloud(pts);
    FeatureCloud features = scan_reg.extract(cloud);

    auto odom = laser_odom.process(features);
    if (!odom.valid) {
      if (i % 10 == 0) std::cerr << "\r  [A-LOAM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    auto mapping = laser_map.process(features.corner_less_sharp,
                                     features.surf_less_flat,
                                     features.full_cloud,
                                     odom.q_w_curr,
                                     odom.t_w_curr);
    if (mapping.valid) {
      T_rel.setIdentity();
      T_rel.block<3, 3>(0, 0) = mapping.q_w_curr.toRotationMatrix();
      T_rel.block<3, 1>(0, 3) = mapping.t_w_curr;
    } else {
      // Fallback: use odometry pose when mapping didn't update.
      T_rel.setIdentity();
      T_rel.block<3, 3>(0, 0) = odom.q_w_curr.toRotationMatrix();
      T_rel.block<3, 1>(0, 3) = odom.t_w_curr;
    }
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) std::cerr << "\r  [A-LOAM] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Odometry+mapping pipeline (no GT seed). KITTI assumes n_scans=64.";
  return res;
}

MethodResult runFLOAM(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const FLOAMDogfoodingOptions& options) {
  using namespace localization_zoo::floam;
  MethodResult res;
  res.name = "F-LOAM";

  FLoamParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;

  params.input_point_stride = options.input_point_stride;
  params.mapping_update_interval = options.mapping_update_interval;
  params.enable_mapping = options.enable_mapping;

  FLoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    auto cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) std::cerr << "\r  [F-LOAM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [F-LOAM] " << i << "/" << pcd_dirs.size()
                << " map_updates=" << result.mapping_updates
                << " stride=" << result.num_input_points << "->"
                << result.num_processed_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Fast feature-based odometry+mapping with thinned input and throttled mapping updates (no GT seed).";
  return res;
}

MethodResult runILoam(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const ILoamDogfoodingOptions& options) {
  using namespace localization_zoo::i_loam;
  MethodResult res;
  res.name = "I-LOAM";

  ILoamParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.input_point_stride = options.input_point_stride;
  params.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.nearby_scan = options.odom_nearby_scan;
  params.num_optimization_iters = options.odom_outer_iters;
  params.ceres_max_iterations = options.odom_ceres_iters;
  params.huber_loss_s = options.odom_huber_loss_s;
  params.enable_distortion = false;

  params.use_intensity_weight = options.use_intensity_weight;
  params.use_intensity_correspondence = options.use_intensity_correspondence;
  params.intensity_sigma = options.intensity_sigma;
  params.intensity_corr_weight = options.intensity_corr_weight;

  params.enable_mapping = options.enable_mapping;
  params.mapping_update_interval = options.mapping_update_interval;
  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.huber_loss_s = options.map_huber_loss_s;
  params.mapping.knn = options.map_knn;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.map_plane_threshold;

  ILoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  double weight_acc = 0.0;
  long weight_frames = 0;
  int mapping_updates = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    // 反射強度を保持して読み込む (I-LOAM の本質)。LOAM のスキャン構造を
    // 壊さないよう load 時はダウンサンプリングしない (leaf=0)。
    auto xyzi = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (xyzi.empty()) continue;
    auto cloud = toPclXYZICloud(xyzi);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) std::cerr << "\r  [I-LOAM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    weight_acc += result.mean_intensity_weight;
    ++weight_frames;
    mapping_updates = result.mapping_updates;
    if (i % 10 == 0) {
      std::cerr << "\r  [I-LOAM] " << i << "/" << pcd_dirs.size()
                << " edges=" << result.num_edge_correspondences
                << " planes=" << result.num_plane_correspondences
                << " w=" << result.mean_intensity_weight
                << " map=" << result.mapping_updates;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double mean_w =
      weight_frames ? weight_acc / static_cast<double>(weight_frames) : 1.0;
  res.note =
      "I-LOAM (Park/Jang/Kim, UR 2020): intensity-enhanced LOAM odometry"
      + std::string(options.enable_mapping ? "+mapping" : " (scan-to-scan only)")
      + " on KITTI PointXYZI; no GT seed. mean_intensity_weight=" +
      std::to_string(mean_w) + " mapping_updates=" + std::to_string(mapping_updates);
  return res;
}

MethodResult runPlLoam(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const PlLoamDogfoodingOptions& options) {
  using namespace localization_zoo::pl_loam;
  MethodResult res;
  res.name = options.use_rgb ? "PL-LOAM-RGB" : "PL-LOAM";

  PlLoamParams params;
  params.input_stride = options.input_stride;
  params.max_point_features = options.max_point_features;
  params.max_line_features = options.max_line_features;
  params.patch_radius = options.patch_radius;
  params.use_depth_prior = options.use_depth_prior;
  params.use_line_features = options.use_line_features;
  params.use_scale_correction = options.use_scale_correction;
  params.depth_prior_weight = options.depth_prior_weight;
  params.ceres_max_iterations = options.ceres_max_iterations;
  params.use_rgb_features = options.use_rgb;
  if (options.use_rgb) {
    params.camera = options.rgb_half_res ? CameraModel::kittiColor02HalfRes()
                                         : CameraModel::kittiColor02();
  }

  PlLoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  double scale_acc = 0.0;
  double depth_res_acc = 0.0;
  long valid_frames = 0;
  long rgb_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", 0.0);
    if (xyzi.empty()) continue;

    std::vector<Eigen::Vector3d> points;
    std::vector<float> intensity;
    points.reserve(xyzi.size());
    intensity.reserve(xyzi.size());
    for (const auto& p : xyzi) {
      points.push_back(p.point);
      intensity.push_back(p.intensity);
    }

    PlLoamResult result;
    if (options.use_rgb && !options.rgb_image_root.empty() &&
        i < frame_timestamps.size()) {
      const int global_idx = static_cast<int>(std::lround(frame_timestamps[i]));
      const std::string rgb_path = resolveKittiRawRgbPath(
          options.rgb_image_root, options.rgb_camera_subdir, global_idx);
      GrayscaleImage gray;
      if (gray.loadRgbPng(rgb_path, params.camera.width, params.camera.height)) {
        result = pipeline.processWithGrayscale(points, intensity, gray);
        ++rgb_frames;
      } else {
        result = pipeline.process(points, intensity);
      }
    } else {
      result = pipeline.process(points, intensity);
    }
    if (result.frame_count < 1) continue;

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (result.valid) {
      scale_acc += result.scale_correction;
      depth_res_acc += result.mean_depth_prior_residual;
      ++valid_frames;
    }
    if (i % 10 == 0) {
      std::cerr << "\r  [PL-LOAM] " << i << "/" << pcd_dirs.size()
                << " pts=" << result.num_points << " lines=" << result.num_lines
                << " pm=" << result.num_point_matches
                << " scale=" << result.scale_correction;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double mean_scale =
      valid_frames ? scale_acc / static_cast<double>(valid_frames) : 1.0;
  const double mean_depth_res =
      valid_frames ? depth_res_acc / static_cast<double>(valid_frames) : 0.0;
  res.note =
      std::string("PL-LOAM (Huang et al., ICRA 2020): LiDAR-monocular point+line VO with "
                  "depth priors in PL-BA; no GT seed. eval=") +
      (options.use_rgb ? "KITTI-Raw-RGB" : "pseudo-image") +
      " mean_scale_correction=" + std::to_string(mean_scale) +
      " mean_depth_prior_residual=" + std::to_string(mean_depth_res) +
      " rgb_frames=" + std::to_string(rgb_frames);
  return res;
}

MethodResult runInTenLoam(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const InTenLoamDogfoodingOptions& options) {
  using namespace localization_zoo::inten_loam;
  MethodResult res;
  res.name = "InTEn-LOAM";

  InTenLoamParams params;
  params.input_stride = options.input_stride;
  params.cylindrical.width = options.cyl_width;
  params.cylindrical.height = options.cyl_height;
  params.use_intensity_registration = options.use_intensity_registration;
  params.max_edge_features = options.max_edge_features;
  params.max_surface_features = options.max_surface_features;
  params.max_reflector_features = options.max_reflector_features;
  params.intensity_weight = options.intensity_weight;
  params.enable_tvf = options.enable_tvf;
  params.enable_dor = options.enable_dor;
  params.enable_mapping = options.enable_mapping;
  params.tvf_voxel_size = options.tvf_voxel_size;
  params.tvf_min_observations = options.tvf_min_observations;
  params.dor_range_delta_thresh = options.dor_range_delta_thresh;
  params.map_voxel_size = options.map_voxel_size;
  params.local_map_radius = options.local_map_radius;
  params.mapping_keyframe_interval = options.mapping_keyframe_interval;

  InTenLoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  double int_res_acc = 0.0;
  long valid_frames = 0;
  long mapping_updates = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", 0.0);
    if (xyzi.empty()) continue;
    std::vector<PointI> points;
    points.reserve(xyzi.size());
    for (const auto& p : xyzi) {
      PointI pt;
      pt.p = p.point;
      pt.intensity = p.intensity;
      points.push_back(pt);
    }
    const auto result = pipeline.process(points);
    if (result.frame_count < 1) continue;

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (result.valid) {
      int_res_acc += result.mean_intensity_residual;
      ++valid_frames;
      mapping_updates += result.mapping_updates;
    }
    if (i % 10 == 0) {
      std::cerr << "\r  [InTEn-LOAM] " << i << "/" << pcd_dirs.size()
                << " edge=" << result.num_edge << " refl=" << result.num_reflector;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double mean_int_res =
      valid_frames ? int_res_acc / static_cast<double>(valid_frames) : 0.0;
  res.note =
      "InTEn-LOAM (Li et al., RS 2022/23): cylindrical FEF + intensity registration + "
      "TVF/DOR/mapping; no GT seed. tvf=" +
      std::string(options.enable_tvf ? "on" : "off") + " dor=" +
      (options.enable_dor ? "on" : "off") + " mapping=" +
      (options.enable_mapping ? "on" : "off") +
      " mapping_updates=" + std::to_string(mapping_updates) +
      " mean_intensity_residual=" + std::to_string(mean_int_res);
  return res;
}

MethodResult runMcGicpLo(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const McGicpLoDogfoodingOptions& options) {
  using namespace localization_zoo::mcgicp;
  MethodResult res;
  res.name = "MCGICP-LO";

  McGicpLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.intensity_scale = options.intensity_scale;
  params.normal_epsilon = options.normal_epsilon;
  params.intensity_epsilon = options.intensity_epsilon;
  params.enable_intensity = options.enable_intensity;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  McGicpLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double int_res_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = limitLoadedXYZI(
        loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.source_voxel_size),
        options.max_source_points);
    if (xyzi.empty()) continue;
    std::vector<PointI> points;
    points.reserve(xyzi.size());
    for (const auto& p : xyzi) {
      points.push_back({p.point, static_cast<double>(p.intensity)});
    }
    const auto result = pipeline.registerFrame(points);
    int_res_sum += result.mean_intensity_residual;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [MCGICP-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.3f / scale=%.2f",
                n > 0 ? int_res_sum / static_cast<double>(n) : 0.0,
                options.intensity_scale);
  res.note =
      "MCGICP-LO (Servos/Waslander, RAS 2017): 4D multi-channel GICP with "
      "intensity in point covariance + Mahalanobis correspondences; "
      "constant-velocity prior, no GT seed. mean_intensity_residual/scale=" +
      std::string(buf);
  return res;
}

MethodResult runIcpscLo(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const IcpscLoDogfoodingOptions& options) {
  using namespace localization_zoo::icpsc;
  MethodResult res;
  res.name = "ICPSC-LO";

  IcpscParams params;
  params.cylindrical.width = options.cyl_width;
  params.cylindrical.height = options.cyl_height;
  params.num_rings = options.num_rings;
  params.num_sectors = options.num_sectors;
  params.intensity_sigma = options.intensity_sigma;
  params.adaptive_alpha = options.adaptive_alpha;
  params.enable_intensity = options.enable_intensity;
  params.voxel_size = options.voxel_size;
  params.max_iterations = options.max_iterations;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  IcpscPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double geom_w_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = limitLoadedXYZI(
        loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.source_voxel_size),
        options.max_source_points);
    if (xyzi.empty()) continue;
    std::vector<PointI> points;
    points.reserve(xyzi.size());
    for (const auto& p : xyzi) {
      points.push_back({p.point, p.intensity});
    }
    const auto result = pipeline.registerFrame(points);
    geom_w_sum += result.geom_weight;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [ICPSC-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize()
                << " w_geom=" << result.geom_weight;
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? geom_w_sum / static_cast<double>(n) : 0.0);
  res.note =
      "ICPSC-LO (Zhang et al., JAG 2023): cylindrical intensity shape-context "
      "+ adaptive geom/intensity fusion scan-to-map; loop closure deferred. "
      "no GT seed. mean_geom_weight=" +
      std::string(buf);
  return res;
}

MethodResult runVlom(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const std::vector<double>& frame_timestamps,
                     const VlomDogfoodingOptions& options) {
  using namespace localization_zoo::vlom;
  MethodResult res;
  res.name = options.use_rgb ? "VLOM-RGB" : "VLOM";

  VlomParams params;
  params.input_point_stride = options.input_point_stride;
  params.visual.input_stride = options.visual_input_stride;
  params.visual.max_point_features = options.max_point_features;
  params.visual.max_line_features = options.max_line_features;
  params.visual.use_rgb_features = options.use_rgb;
  if (options.use_rgb) {
    params.visual.camera =
        options.rgb_half_res
            ? localization_zoo::pl_loam::CameraModel::kittiColor02HalfRes()
            : localization_zoo::pl_loam::CameraModel::kittiColor02();
  }
  params.enable_visual_bootstrap = options.enable_visual_bootstrap;
  params.enable_scale_correction = options.enable_scale_correction;
  params.scale_correction_interval = options.scale_correction_interval;
  params.mad_outlier_k = options.mad_outlier_k;
  params.enable_mapping = options.enable_mapping;

  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.nearby_scan = options.odom_nearby_scan;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.huber_loss_s = options.odom_huber_loss_s;

  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.huber_loss_s = options.map_huber_loss_s;
  params.mapping.knn = options.map_knn;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.map_plane_threshold;

  Vlom pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  double scale_acc = 0.0;
  long scale_frames = 0;
  long bootstrap_frames = 0;
  long rgb_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", 0.0);
    if (xyzi.empty()) continue;
    auto cloud = toPclXYZICloud(xyzi);
    std::vector<float> intensity;
    intensity.reserve(xyzi.size());
    for (const auto& p : xyzi) intensity.push_back(p.intensity);

    localization_zoo::pl_loam::GrayscaleImage gray;
    const localization_zoo::pl_loam::GrayscaleImage* gray_ptr = nullptr;
    if (options.use_rgb && !options.rgb_image_root.empty() &&
        i < frame_timestamps.size()) {
      const int global_idx = static_cast<int>(std::lround(frame_timestamps[i]));
      const std::string rgb_path = resolveKittiRawRgbPath(
          options.rgb_image_root, options.rgb_camera_subdir, global_idx);
      if (gray.loadRgbPng(rgb_path, params.visual.camera.width,
                           params.visual.camera.height)) {
        gray_ptr = &gray;
        ++rgb_frames;
      }
    }

    const auto result = pipeline.process(cloud, intensity, gray_ptr);
    if (!result.valid) {
      if (i % 10 == 0) std::cerr << "\r  [VLOM] " << i << "/" << pcd_dirs.size();
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    scale_acc += result.scale_factor;
    ++scale_frames;
    if (result.visual_bootstrap_used) ++bootstrap_frames;
    if (i % 10 == 0) {
      std::cerr << "\r  [VLOM] " << i << "/" << pcd_dirs.size()
                << " bootstrap=" << result.visual_bootstrap_used
                << " scale=" << result.scale_factor;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char note_buf[160];
  std::snprintf(
      note_buf, sizeof(note_buf),
      "VLOM (arXiv:2304.08978): scale correction + visual-bootstrapped A-LOAM; "
      "eval=%s; no GT seed. mean_scale=%.3f bootstrap_frames=%ld rgb_frames=%ld",
      options.use_rgb ? "KITTI-Raw-RGB" : "pseudo-image",
      scale_frames ? scale_acc / static_cast<double>(scale_frames) : 1.0,
      bootstrap_frames, rgb_frames);
  res.note = note_buf;
  return res;
}

struct OdoNetDogfoodingOptions {
  std::string weights_path = "papers/odonet/weights/odonet_kitti.json";
  bool enable_nhc = true;
  bool enable_zupt = true;
  bool enable_cnn_speed = true;
};

MethodResult runOdoNet(const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const std::vector<ImuSampleCsv>& imu_samples,
                       const OdoNetDogfoodingOptions& options) {
  using namespace localization_zoo::odonet;
  MethodResult res;
  res.name = "OdoNet";

  if (imu_samples.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note =
        "imu.csv not found. OdoNet requires synchronized IMU for pseudo-odometer "
        "dead reckoning.";
    return res;
  }
  if (frame_timestamps.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = "OdoNet requires per-frame timestamps aligned with imu.csv.";
    return res;
  }

  std::vector<ImuReading> imu;
  imu.reserve(imu_samples.size());
  for (const auto& sample : imu_samples) {
    ImuReading reading;
    reading.stamp = sample.timestamp;
    reading.gyro = sample.gyro;
    reading.accel = sample.accel;
    imu.push_back(reading);
  }

  OdoNetParams params;
  params.weights_path = options.weights_path;
  params.enable_nhc = options.enable_nhc;
  params.enable_zupt = options.enable_zupt;
  params.enable_cnn_speed = options.enable_cnn_speed;

  long zupt_frames = 0;
  long cnn_frames = 0;
  std::string error;
  auto t0 = Clock::now();
  const auto rel_poses =
      integrateImuTrajectory(imu, frame_timestamps, params, &zupt_frames,
                             &cnn_frames, &error);
  if (rel_poses.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = error.empty() ? "OdoNet integration failed." : error;
    return res;
  }

  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  for (const auto& T : rel_poses) {
    res.poses.push_back(anchorRelativePose(world_anchor, T));
  }
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();

  char note_buf[256];
  std::snprintf(
      note_buf, sizeof(note_buf),
      "OdoNet (arXiv:2109.03091): CNN pseudo-odometer + strapdown INS + "
      "NHC/ZUPT; no GT seed. cnn_frames=%ld zupt_frames=%ld nhc=%d",
      cnn_frames, zupt_frames, options.enable_nhc ? 1 : 0);
  res.note = note_buf;
  return res;
}

struct NhcNetDogfoodingOptions {
  std::string weights_path = "papers/nhc_net/weights/nhc_net_kitti.json";
  bool enable_zupt = true;
  bool adaptive_gain = true;
  double nhc_gain = 6.0;
};

MethodResult runNhcNet(const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const std::vector<ImuSampleCsv>& imu_samples,
                       const NhcNetDogfoodingOptions& options) {
  using namespace localization_zoo::nhc_net;
  MethodResult res;
  res.name = "NHC-Net";

  if (imu_samples.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = "imu.csv not found. NHC-Net requires synchronized IMU samples.";
    return res;
  }

  std::vector<ImuReading> imu;
  imu.reserve(imu_samples.size());
  for (const auto& s : imu_samples) {
    ImuReading r;
    r.stamp = s.timestamp;
    r.gyro = s.gyro;
    r.accel = s.accel;
    imu.push_back(r);
  }

  NhcNetParams params;
  params.weights_path = options.weights_path;
  params.enable_zupt = options.enable_zupt;
  params.adaptive_gain = options.adaptive_gain;
  params.nhc_gain = options.nhc_gain;

  long zupt_frames = 0;
  long cnn_frames = 0;
  std::string error;
  auto t0 = Clock::now();
  const auto rel_poses =
      integrateImuTrajectory(imu, frame_timestamps, params, &zupt_frames,
                             &cnn_frames, &error);
  if (rel_poses.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = error.empty() ? "NHC-Net integration failed." : error;
    return res;
  }

  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  for (const auto& T : rel_poses) {
    res.poses.push_back(anchorRelativePose(world_anchor, T));
  }
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char note_buf[256];
  std::snprintf(
      note_buf, sizeof(note_buf),
      "NHC-Net/VMSC (GPS Solutions 2023): CNN motion-state + adaptive NHC DR; "
      "no GT seed. vmsc_frames=%ld zupt_frames=%ld",
      cnn_frames, zupt_frames);
  res.note = note_buf;
  return res;
}

struct NnZuptDogfoodingOptions {
  std::string weights_path = "papers/nn_zupt/weights/nn_zupt_kitti.json";
  bool enable_zupt = true;
  bool enable_nhc = true;
  bool use_threshold_detector = false;
  double stop_prob_threshold = 0.5;
};

MethodResult runNnZupt(const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const std::vector<ImuSampleCsv>& imu_samples,
                       const NnZuptDogfoodingOptions& options) {
  using namespace localization_zoo::nn_zupt;
  MethodResult res;
  res.name = "NN-ZUPT";

  if (imu_samples.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = "imu.csv not found. NN-ZUPT requires synchronized IMU samples.";
    return res;
  }

  std::vector<ImuReading> imu;
  imu.reserve(imu_samples.size());
  for (const auto& s : imu_samples) {
    ImuReading r;
    r.stamp = s.timestamp;
    r.gyro = s.gyro;
    r.accel = s.accel;
    imu.push_back(r);
  }

  NnZuptParams params;
  params.weights_path = options.weights_path;
  params.enable_zupt = options.enable_zupt;
  params.enable_nhc = options.enable_nhc;
  params.use_threshold_detector = options.use_threshold_detector;
  params.stop_prob_threshold = options.stop_prob_threshold;

  long zupt_frames = 0;
  long nn_frames = 0;
  std::string error;
  auto t0 = Clock::now();
  const auto rel_poses =
      integrateImuTrajectory(imu, frame_timestamps, params, &zupt_frames,
                             &nn_frames, &error);
  if (rel_poses.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note = error.empty() ? "NN-ZUPT integration failed." : error;
    return res;
  }

  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  for (const auto& T : rel_poses) {
    res.poses.push_back(anchorRelativePose(world_anchor, T));
  }
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char note_buf[256];
  std::snprintf(
      note_buf, sizeof(note_buf),
      "NN-ZUPT (Meas. Sci. Technol. 2023): CNN zero-velocity detection + ZUPT/NHC "
      "DR; no GT seed. nn_frames=%ld zupt_frames=%ld threshold=%d",
      nn_frames, zupt_frames, options.use_threshold_detector ? 1 : 0);
  res.note = note_buf;
  return res;
}

MethodResult runLeGOLOAM(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LeGOLOAMDogfoodingOptions& options) {
  using namespace localization_zoo::lego_loam;
  MethodResult res;
  res.name = "LeGO-LOAM";

  LeGOLOAMParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.ground_scan_limit = options.ground_scan_limit;
  params.scan_registration.num_subregions = options.num_subregions;
  params.scan_registration.neighbor_window = options.neighbor_window;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.maximum_range = options.maximum_range;
  params.scan_registration.sensor_height = options.sensor_height;
  params.scan_registration.ground_height_tolerance =
      options.ground_height_tolerance;
  params.scan_registration.sensor_mount_angle_deg =
      options.sensor_mount_angle_deg;
  params.scan_registration.ground_angle_threshold_deg =
      options.ground_angle_threshold_deg;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp =
      options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.nearby_scan = options.odom_nearby_scan;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.huber_loss_s = options.odom_huber_loss_s;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.ceres_max_iterations = options.map_ceres_iters;
  params.mapping.huber_loss_s = options.map_huber_loss_s;
  params.mapping.knn = options.map_knn;
  params.mapping.knn_max_dist_sq = options.map_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.map_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.map_plane_threshold;

  LeGOLOAM pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) {
        std::cerr << "\r  [LeGO-LOAM] " << i << "/" << pcd_dirs.size();
      }
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [LeGO-LOAM] " << i << "/" << pcd_dirs.size()
                << " ground=" << result.num_ground_points
                << " corner=" << result.num_corner_features
                << " surf=" << result.num_surface_features;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Ground-aware feature odometry+mapping (no GT seed). n_scans=64 matches Velodyne HDL-64E layout.";
  return res;
}

MethodResult runMULLS(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const MULLSDogfoodingOptions& options) {
  using namespace localization_zoo::mulls;
  MethodResult res;
  res.name = "MULLS";

  MULLSParams params;
  params.scan_registration.n_scans = options.n_scans;
  params.scan_registration.minimum_range = options.minimum_range;
  params.scan_registration.scan_period = options.scan_period;
  params.scan_registration.curvature_threshold = options.curvature_threshold;
  params.scan_registration.max_corner_sharp = options.max_corner_sharp;
  params.scan_registration.max_corner_less_sharp = options.max_corner_less_sharp;
  params.scan_registration.max_surf_flat = options.max_surf_flat;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;

  params.odometry.distance_sq_threshold = options.odom_distance_sq_threshold;
  params.odometry.nearby_scan = options.odom_nearby_scan;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.odometry.ceres_max_iterations = options.odom_ceres_iters;
  params.odometry.huber_loss_s = options.odom_huber_loss_s;
  params.odometry.enable_distortion = false;

  params.mapping.line_resolution = options.line_resolution;
  params.mapping.plane_resolution = options.plane_resolution;
  params.mapping.point_resolution = options.point_resolution;
  params.mapping.num_optimization_iters = options.mulls_map_iters;
  params.mapping.ceres_max_iterations = options.mulls_ceres_iters;
  params.mapping.huber_loss_s = options.mulls_huber_loss_s;
  params.mapping.knn = options.mulls_knn;
  params.mapping.knn_max_dist_sq = options.mulls_knn_max_dist_sq;
  params.mapping.edge_eigenvalue_ratio = options.mulls_edge_eigenvalue_ratio;
  params.mapping.plane_threshold = options.mulls_plane_threshold;
  params.mapping.point_neighbor_max_dist_sq =
      options.mulls_point_neighbor_max_dist_sq;
  params.mapping.line_weight = options.mulls_line_weight;
  params.mapping.plane_weight = options.mulls_plane_weight;
  params.mapping.point_weight = options.mulls_point_weight;
  params.mapping.full_downsample_rate = options.mulls_full_downsample_rate;
  params.mapping.max_frames_in_map = options.mulls_max_frames_in_map;

  MULLS pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  res.poses.push_back(world_anchor);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", 0.0 /* no downsample */);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    if (!result.valid) {
      if (i % 10 == 0) {
        std::cerr << "\r  [MULLS] " << i << "/" << pcd_dirs.size();
      }
      continue;
    }

    Eigen::Matrix4d T_rel = Eigen::Matrix4d::Identity();
    T_rel.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    T_rel.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, T_rel));

    if (i % 10 == 0) {
      std::cerr << "\r  [MULLS] " << i << "/" << pcd_dirs.size()
                << " L=" << result.num_line_constraints
                << " P=" << result.num_plane_constraints
                << " Pt=" << result.num_point_constraints
                << " map_frames=" << result.num_frames_in_map;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Multi-metric scan-to-map (line/plane/point Ceres) after A-LOAM-style feature odometry (no GT seed).";
  return res;
}

MethodResult runNDT(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt,
                    const NDTDogfoodingOptions& options,
                    bool no_gt_seed = false,
                    const std::vector<Eigen::Matrix4d>* external_seed_poses = nullptr,
                    const std::string& external_seed_note = "") {
  using namespace localization_zoo::ndt;
  MethodResult res;
  res.name = external_seed_poses ? "CT-ICP+NDT" : "NDT";

  NDTParams params;
  params.resolution = options.resolution;
  params.max_iterations = options.max_iterations;
  params.step_size = options.step_size;
  params.convergence_threshold = options.convergence_threshold;
  params.min_points_per_cell = options.min_points_per_cell;
  NDTRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  size_t seed_fallbacks = 0;
  size_t seeded_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    Eigen::Matrix4d T_init_guess = T_est;
    const bool gt_seeded_frame = !external_seed_poses && !no_gt_seed;
    if (external_seed_poses && i < external_seed_poses->size()) {
      T_init_guess = (*external_seed_poses)[i];
    } else if (no_gt_seed) {
      T_init_guess = i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est)
                            : T_est;
    } else {
      T_init_guess = applySeedPerturbation(gt[i], options.seed_perturbation);
    }
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    const auto result = reg.align(pts_local, T_init_guess);
    if (gt_seeded_frame) ++seeded_frames;
    if ((result.converged || result.iterations >= 2) &&
        isReasonableRefinement(result.transformation, T_init_guess,
                               options.max_seed_translation_delta,
                               options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
    } else {
      T_est = T_init_guess;
      if (gt_seeded_frame) ++seed_fallbacks;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      reg.setTarget(map_points);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [NDT] " << i << "/" << pcd_dirs.size()
                << " score=" << std::fixed << std::setprecision(3)
                << result.score;
    }
  }
  std::cerr << std::endl;
  if (seeded_frames > 0) {
    res.seed_fallback_rate_pct = 100.0 *
        static_cast<double>(seed_fallbacks) / static_cast<double>(seeded_frames);
    std::cerr << "  [NDT] seed fallbacks=" << seed_fallbacks << "/"
              << seeded_frames << " (" << res.seed_fallback_rate_pct
              << "% rolled back to GT seed)" << std::endl;
  }
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  if (external_seed_poses) {
    res.note = external_seed_note.empty()
        ? "Uses external pose prior as scan-to-map initial guess."
        : external_seed_note;
  } else {
    res.note = no_gt_seed
        ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
        : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
          "in this dogfooding tool.";
  }
  if (!external_seed_poses && !no_gt_seed) {
    res.note += seedPerturbationNote(options.seed_perturbation);
  }
  return res;
}

MethodResult runFixedMapNDT(const std::vector<std::string>& pcd_dirs,
                            const std::vector<Eigen::Matrix4d>& gt,
                            const NDTDogfoodingOptions& ndt_options,
                            const CTICPDogfoodingOptions& ct_icp_options,
                            const FixedMapNDTOptions& fixed_map_options) {
  using namespace localization_zoo::ndt;

  MethodResult res;
  res.name = "FixedMap-NDT";
  if (fixed_map_options.seed_source == "ct_icp") {
    res.name += "+CTICP";
  } else if (fixed_map_options.seed_source == "scan_context") {
    res.name += "+ScanContext";
  } else if (fixed_map_options.seed_source == "velocity") {
    res.name += "+Vel";
  } else {
    res.name += "+GT";
  }

  NDTParams params;
  params.resolution = ndt_options.resolution;
  params.max_iterations = ndt_options.max_iterations;
  params.step_size = ndt_options.step_size;
  params.convergence_threshold = ndt_options.convergence_threshold;
  params.min_points_per_cell = ndt_options.min_points_per_cell;
  NDTRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  localization_zoo::scan_context::ScanContextParams sc_params;
  sc_params.exclude_recent_frames = 0;
  sc_params.distance_threshold = fixed_map_options.scan_context_distance_threshold;
  sc_params.num_candidates = std::max(1, fixed_map_options.scan_context_top_k);
  localization_zoo::scan_context::ScanContextManager scan_context(sc_params);
  std::vector<Eigen::Matrix4d> scan_context_poses;
  const int map_stride = std::max(1, fixed_map_options.map_stride);
  for (size_t i = 0; i < pcd_dirs.size() && i < gt.size();
       i += static_cast<size_t>(map_stride)) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         ndt_options.source_voxel_size),
                                 ndt_options.max_source_points);
    if (pts_local.empty()) continue;
    addPointsToMap(map_points, pts_local, gt[i], ndt_options.map_max_points,
                   0.0);
    if (fixed_map_options.seed_source == "scan_context") {
      scan_context.addScan(pts_local);
      scan_context_poses.push_back(gt[i]);
    }
  }
  reg.setTarget(map_points);

  MethodResult ct_icp_prior;
  const std::vector<Eigen::Matrix4d>* prior_poses = nullptr;
  double prior_ate = std::numeric_limits<double>::quiet_NaN();
  if (fixed_map_options.seed_source == "ct_icp") {
    ct_icp_prior = runCTICP(pcd_dirs, gt, ct_icp_options, false);
    prior_poses = &ct_icp_prior.poses;
    prior_ate = computeATE(ct_icp_prior.poses, gt);
  }

  Eigen::Matrix4d T_est = gt.empty() ? Eigen::Matrix4d::Identity() : gt[0];
  Eigen::Matrix4d T_prev_prev_est = T_est;
  res.poses.push_back(T_est);
  std::vector<FixedMapNDTTraceFrame> trace_frames;
  FixedMapNDTPublishPolicyState publish_policy_state;
  FixedMapNDTTraceFrame previous_policy_frame;
  bool has_previous_policy_frame = false;
  size_t publish_pose_outputs = 0;
  size_t publish_unknown_outputs = 0;
  size_t publish_hold_outputs = 0;
  size_t gt_wrong_pose_suppressed = 0;
  size_t gt_unsafe_transition_suppressed = 0;
  auto update_publish_counters = [&](const FixedMapNDTTraceFrame& frame) {
    if (frame.publish_action == "publish_pose") {
      ++publish_pose_outputs;
    } else if (frame.publish_action == "hold_last_pose") {
      ++publish_hold_outputs;
    } else if (frame.publish_action == "return_unknown") {
      ++publish_unknown_outputs;
    }
    if (!frame.has_pose_output && frame.gt_wrong_pose) {
      ++gt_wrong_pose_suppressed;
    }
    if (!frame.has_pose_output && frame.gt_unsafe_transition) {
      ++gt_unsafe_transition_suppressed;
    }
  };
  FixedMapNDTTraceFrame frame0;
  frame0.frame_index = 0;
  frame0.decision = "anchor";
  frame0.seed_source = fixed_map_options.seed_source;
  frame0.seed_pose = T_est;
  frame0.refined_pose = T_est;
  frame0.final_pose = T_est;
  finalizeFixedMapNDTTraceFrame(frame0, gt, nullptr);
  annotateFixedMapNDTPublishPolicy(
      frame0, fixed_map_options, publish_policy_state, nullptr);
  update_publish_counters(frame0);
  previous_policy_frame = frame0;
  has_previous_policy_frame = true;
  if (!fixed_map_options.trace_json_path.empty()) {
    trace_frames.push_back(frame0);
  }

  size_t accepted = 0;
  size_t rejected = 0;
  size_t fallback_seed = 0;
  size_t scan_context_hits = 0;
  size_t scan_context_candidates_evaluated = 0;
  double scan_context_distance_sum = 0.0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         ndt_options.source_voxel_size),
                                 ndt_options.max_source_points);
    if (pts_local.empty()) continue;
    if (i == 0) continue;

    const Eigen::Matrix4d T_prev_est_snapshot = T_est;
    FixedMapNDTTraceFrame trace;
    trace.frame_index = i;
    trace.seed_source = fixed_map_options.seed_source;
    trace.seed_pose = T_est;
    trace.refined_pose = T_est;
    trace.final_pose = T_est;

    Eigen::Matrix4d T_seed = T_est;
    if (fixed_map_options.seed_source == "ct_icp") {
      if (prior_poses && i < prior_poses->size() && isFiniteMatrix((*prior_poses)[i])) {
        T_seed = (*prior_poses)[i];
      } else {
        ++fallback_seed;
        trace.seed_fallback = true;
      }
    } else if (fixed_map_options.seed_source == "scan_context") {
      const auto candidates = scan_context.detectLoopCandidates(pts_local);
      trace.scan_context_candidate_count = static_cast<int>(candidates.size());
      if (candidates.empty()) {
        ++fallback_seed;
        trace.seed_fallback = true;
      } else {
        ++scan_context_hits;
        trace.scan_context_hit = true;
        trace.scan_context_front_distance = candidates.front().distance;
        scan_context_distance_sum += candidates.front().distance;

        bool best_valid = false;
        double best_score = std::numeric_limits<double>::infinity();
        Eigen::Matrix4d best_pose = T_est;
        Eigen::Matrix4d best_seed_pose = T_est;
        Eigen::Matrix4d best_refined_pose = T_est;
        int best_iterations = 0;
        bool best_converged = false;
        for (const auto& candidate : candidates) {
          if (!candidate.valid ||
              candidate.index < 0 ||
              candidate.index >= static_cast<int>(scan_context_poses.size())) {
            continue;
          }
          Eigen::Matrix4d candidate_seed =
              scan_context_poses[static_cast<size_t>(candidate.index)];
          const Eigen::Matrix4d yaw_delta =
              (Eigen::Translation3d(0.0, 0.0, 0.0) *
               Eigen::AngleAxisd(candidate.yaw_offset_rad,
                                 Eigen::Vector3d::UnitZ()))
                  .matrix();
          candidate_seed = candidate_seed * yaw_delta;
          const auto result = reg.align(pts_local, candidate_seed);
          ++scan_context_candidates_evaluated;
          ++trace.scan_context_candidates_evaluated;
          if ((result.converged || result.iterations >= 2) &&
              std::isfinite(result.score) &&
              isFiniteMatrix(result.transformation) &&
              isReasonableRefinement(result.transformation, candidate_seed,
                                     ndt_options.max_seed_translation_delta,
                                     ndt_options.max_seed_rotation_delta_rad) &&
              result.score < best_score) {
            best_valid = true;
            best_score = result.score;
            best_pose = result.transformation;
            best_seed_pose = candidate_seed;
            best_refined_pose = result.transformation;
            best_iterations = result.iterations;
            best_converged = result.converged;
            trace.selected_candidate_index = candidate.index;
            trace.selected_candidate_distance = candidate.distance;
            trace.selected_candidate_yaw_offset_rad = candidate.yaw_offset_rad;
          }
        }

        trace.ndt_score = best_score;
        trace.ndt_iterations = best_iterations;
        trace.ndt_converged = best_converged;
        trace.seed_pose = best_seed_pose;
        trace.refined_pose = best_refined_pose;
        if (best_valid) {
          T_est = best_pose;
          ++accepted;
          trace.accepted = true;
          trace.decision = "accepted";
        } else {
          T_est = T_prev_est_snapshot;
          ++rejected;
          trace.accepted = false;
          trace.decision = trace.scan_context_candidates_evaluated > 0
                               ? "rejected"
                               : "no_valid_scan_context_candidate";
        }
        trace.final_pose = T_est;
        finalizeFixedMapNDTTraceFrame(trace, gt, &T_prev_est_snapshot);
        annotateFixedMapNDTPublishPolicy(
            trace, fixed_map_options, publish_policy_state,
            has_previous_policy_frame ? &previous_policy_frame : nullptr);
        update_publish_counters(trace);
        previous_policy_frame = trace;
        has_previous_policy_frame = true;
        T_prev_prev_est = T_prev_est_snapshot;
        res.poses.push_back(T_est);
        if (!fixed_map_options.trace_json_path.empty()) {
          trace_frames.push_back(trace);
        }

        if (i % 10 == 0) {
          std::cerr << "\r  [FixedMap-NDT] " << i << "/" << pcd_dirs.size()
                    << " sc_candidates=" << candidates.size()
                    << " best_score=" << std::fixed << std::setprecision(3)
                    << best_score
                    << " accepted=" << accepted << "/" << (accepted + rejected);
        }
        continue;
      }
    } else if (fixed_map_options.seed_source == "velocity") {
      T_seed = i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est;
    } else {
      T_seed = applySeedPerturbation(gt[i], ndt_options.seed_perturbation);
    }

    trace.seed_pose = T_seed;
    const auto result = reg.align(pts_local, T_seed);
    trace.refined_pose = result.transformation;
    trace.ndt_score = result.score;
    trace.ndt_iterations = result.iterations;
    trace.ndt_converged = result.converged;
    if ((result.converged || result.iterations >= 2) &&
        isFiniteMatrix(result.transformation) &&
        isReasonableRefinement(result.transformation, T_seed,
                               ndt_options.max_seed_translation_delta,
                               ndt_options.max_seed_rotation_delta_rad)) {
      T_est = result.transformation;
      ++accepted;
      trace.accepted = true;
      trace.decision = "accepted";
    } else {
      T_est = T_seed;
      ++rejected;
      trace.accepted = false;
      trace.decision = "rejected_to_seed";
    }
    trace.final_pose = T_est;
    finalizeFixedMapNDTTraceFrame(trace, gt, &T_prev_est_snapshot);
    annotateFixedMapNDTPublishPolicy(
        trace, fixed_map_options, publish_policy_state,
        has_previous_policy_frame ? &previous_policy_frame : nullptr);
    update_publish_counters(trace);
    previous_policy_frame = trace;
    has_previous_policy_frame = true;
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);
    if (!fixed_map_options.trace_json_path.empty()) {
      trace_frames.push_back(trace);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [FixedMap-NDT] " << i << "/" << pcd_dirs.size()
                << " score=" << std::fixed << std::setprecision(3)
                << result.score
                << " accepted=" << accepted << "/" << (accepted + rejected);
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count() +
      ct_icp_prior.time_ms;

  std::ostringstream note;
  note << "Fixed-map NDT: target map built once from GT/reference poses"
       << " (stride=" << map_stride
       << ", map_points=" << map_points.size()
       << "). Seed source=" << fixed_map_options.seed_source
       << ", accepted=" << accepted << "/" << (accepted + rejected)
       << ", rejected=" << rejected
       << ", seed fallbacks=" << fallback_seed
       << ". Runtime publish policy outputs="
       << publish_pose_outputs
       << ", holds=" << publish_hold_outputs
       << ", unknown=" << publish_unknown_outputs
       << ", GT-wrong suppressed=" << gt_wrong_pose_suppressed
       << ", GT-unsafe transitions suppressed="
       << gt_unsafe_transition_suppressed << ".";
  if (fixed_map_options.seed_source == "scan_context") {
    note << " ScanContext hits=" << scan_context_hits << "/" << (pcd_dirs.size() - 1)
         << ", top_k=" << fixed_map_options.scan_context_top_k
         << ", NDT candidates evaluated=" << scan_context_candidates_evaluated
         << ", mean distance=";
    if (scan_context_hits > 0) {
      note << std::fixed << std::setprecision(4)
           << (scan_context_distance_sum / scan_context_hits) << ".";
    } else {
      note << "n/a.";
    }
  }
  if (fixed_map_options.seed_source == "ct_icp") {
    note << " CT-ICP prior ATE=";
    if (std::isfinite(prior_ate)) {
      note << std::fixed << std::setprecision(3) << prior_ate << "m.";
    } else {
      note << "nonfinite.";
    }
    note << " Runtime is CT-ICP+NDT combined.";
  } else if (fixed_map_options.seed_source == "gt") {
    note << seedPerturbationNote(ndt_options.seed_perturbation);
  }
  if (!fixed_map_options.trace_json_path.empty()) {
    writeFixedMapNDTTraceJson(fixed_map_options.trace_json_path,
                              fixed_map_options, pcd_dirs.size(),
                              map_points.size(), trace_frames);
    note << " Per-frame trace=" << fixed_map_options.trace_json_path << ".";
  }
  res.note = note.str();
  return res;
}

MethodResult runKISSICP(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const KISSICPDogfoodingOptions& options) {
  using namespace localization_zoo::kiss_icp;
  MethodResult res;
  res.name = "KISS-ICP";

  KISSICPParams params;
  params.voxel_size = options.voxel_size;
  params.initial_threshold = options.initial_threshold;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  KISSICPPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    const auto result = pipeline.registerFrame(pts_local);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [KISS-ICP] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

MethodResult runGenZICP(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const GenZICPDogfoodingOptions& options) {
  using namespace localization_zoo::genz_icp;
  MethodResult res;
  res.name = "GenZ-ICP";

  GenZICPParams params;
  params.voxel_size = options.voxel_size;
  params.initial_threshold = options.initial_threshold;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  GenZICPPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    const auto result = pipeline.registerFrame(pts_local);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [GenZ-ICP] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Adaptive point-to-plane / point-to-point hybrid odometry (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runAdaptiveICP(const std::vector<std::string>& pcd_dirs,
                            const std::vector<Eigen::Matrix4d>& gt,
                            const AdaptiveICPDogfoodingOptions& options) {
  using namespace localization_zoo::adaptive_icp;
  MethodResult res;
  res.name = "Adaptive-ICP";

  AdaptiveICPParams params;
  params.voxel_size = options.voxel_size;
  params.initial_threshold = options.initial_threshold;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.coarse_max_iterations = options.coarse_max_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.density_percentile = options.density_percentile;
  params.reliable_translation_tau = options.reliable_translation_tau;
  params.motion_decay = options.motion_decay;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  AdaptiveICPPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  int coarse_accepted = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    const auto result = pipeline.registerFrame(pts_local);
    if (result.used_coarse_pose) ++coarse_accepted;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [Adaptive-ICP] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Reliable-initial-pose adaptive ICP: density-filtered coarse pose vs "
      "constant-velocity prediction, robust point-to-plane with sigma_th gating "
      "(no GT seed; anchor matches first GT pose). coarse_accepted=" +
      std::to_string(coarse_accepted);
  return res;
}

MethodResult runRKOLIO(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const std::vector<ImuSampleCsv>& imu_samples,
                       const RKOLIODogfoodingOptions& options) {
  using namespace localization_zoo::rko_lio;
  MethodResult res;
  res.name = "RKO-LIO";

  RKOLIOParams params;
  params.voxel_size = options.voxel_size;
  params.initial_threshold = options.initial_threshold;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  params.gyro_bias_gain = options.gyro_bias_gain;
  RKOLIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    res.poses.push_back(result.pose);

    if (i % 10 == 0) {
      std::cerr << "\r  [RKO-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      imu_frames > 0
          ? "Scan-to-map odometry with IMU rotation prior (GT-seeded init; "
            "IMU gyro preintegration used as ICP rotation guess)."
          : "Scan-to-map odometry, constant-velocity fallback (no imu.csv; "
            "GT-seeded init).";
  return res;
}

MethodResult runFRLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const FRLIODogfoodingOptions& options) {
  using namespace localization_zoo::fr_lio;
  MethodResult res;
  res.name = "FR-LIO";

  if (imu_samples.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note =
        "imu.csv not found. FR-LIO requires synchronized IMU for sub-frame "
        "splitting and inertial priors.";
    return res;
  }

  FRLIOParams params;
  params.source_voxel_size = options.source_voxel_size;
  params.voxel_size = options.voxel_size;
  params.grid_size = options.grid_size;
  params.lidar_range = options.lidar_range;
  params.map_lambda = options.map_lambda;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_subframes = options.max_subframes;
  params.esks_gain = options.esks_gain;
  params.min_valid_matches = options.min_valid_matches;
  FRLIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  int subframe_total = 0;
  int smoothed_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    subframe_total += result.subframes;
    smoothed_total += result.smoothed_subframes;
    res.poses.push_back(result.pose);

    if (i % 10 == 0) {
      std::cerr << "\r  [FR-LIO] " << i << "/" << pcd_dirs.size()
                << " grids=" << pipeline.mapGrids()
                << " sub=" << result.subframes;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  std::ostringstream oss;
  oss << "RC-Vox scan-to-map LIO with adaptive sub-frames + simplified ESKS "
         "(IMU required).";
  if (imu_frames > 0) {
    oss << " IMU prior frames=" << imu_frames;
    if (subframe_total > 0) {
      oss << " avg_subframes="
          << (static_cast<double>(subframe_total) /
              static_cast<double>(pcd_dirs.size()));
    }
    if (smoothed_total > 0) {
      oss << " smoothed_subframes=" << smoothed_total;
    }
  }
  res.note = oss.str();
  return res;
}

MethodResult runPGLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const PGLIODogfoodingOptions& options) {
  using namespace localization_zoo::pg_lio;
  MethodResult res;
  res.name = "PG-LIO";

  if (imu_samples.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note =
        "imu.csv not found. PG-LIO requires synchronized IMU for inertial "
        "priors and degeneracy regularization.";
    return res;
  }

  auto probe = loadPCDXYZI(pcd_dirs.front() + "/cloud.pcd", options.source_voxel_size);
  if (probe.empty()) {
    res.skipped = true;
    res.status = "skipped";
    res.note =
        "cloud.pcd lacks PointXYZI intensity. PG-LIO requires per-point "
        "intensity for photometric NCC factors.";
    return res;
  }

  PGLIOParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.range_image_width = options.range_image_width;
  params.range_image_height = options.range_image_height;
  params.max_patches = options.max_patches;
  params.photometric_weight = options.photometric_weight;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  PGLIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  long photo_patches = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.source_voxel_size);
    if (pts.empty()) continue;
    pts = limitLoadedXYZI(pts, options.max_source_points);
    if (pts.empty()) continue;

    std::vector<IntensityPoint> frame;
    frame.reserve(pts.size());
    for (const auto& p : pts) frame.push_back({p.point, p.intensity});

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.registerFrame(frame, imu_batch);
    if (result.used_imu) ++imu_frames;
    photo_patches += result.photometric_patches;
    res.poses.push_back(result.pose);

    if (i % 10 == 0) {
      std::cerr << "\r  [PG-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize()
                << " patches=" << pipeline.patchCount();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  std::ostringstream oss;
  oss << "Photometric-geometric LIO with NCC range-image patches (IMU required).";
  if (imu_frames > 0) oss << " IMU frames=" << imu_frames;
  if (photo_patches > 0) {
    oss << " photometric_patch_frames=" << photo_patches;
  }
  res.note = oss.str();
  return res;
}

MethodResult runD2LIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const D2LIODogfoodingOptions& options) {
  using namespace localization_zoo::d2lio;
  MethodResult res;
  res.name = "D2-LIO";

  D2LIOParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.base_threshold = options.base_threshold;
  params.max_threshold = options.max_threshold;
  params.cauchy_scale = options.cauchy_scale;
  params.degeneracy_ratio = options.degeneracy_ratio;
  params.imu_prior_weight = options.imu_prior_weight;
  params.gyro_bias_gain = options.gyro_bias_gain;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  D2LIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  long deg_rot_total = 0;
  long deg_trans_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    deg_rot_total += result.degenerate_rot_dirs;
    deg_trans_total += result.degenerate_trans_dirs;
    res.poses.push_back(result.pose);

    if (i % 10 == 0) {
      std::cerr << "\r  [D2-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      (imu_frames > 0
           ? "Directional-degeneracy LiDAR-IMU scan-to-submap (GT-seeded init; "
             "adaptive per-point outlier gate + IMU-prior degeneracy "
             "regularization)."
           : "Scan-to-submap, constant-velocity fallback (no imu.csv; "
             "GT-seeded init).") +
      std::string(" deg_dirs/frame rot=") +
      std::to_string(deg_rot_total) + " trans=" +
      std::to_string(deg_trans_total);
  return res;
}

MethodResult runCTVoxelMap(const std::vector<std::string>& pcd_dirs,
                           const std::vector<Eigen::Matrix4d>& gt,
                           const CTVoxelMapDogfoodingOptions& options) {
  using namespace localization_zoo::ct_voxelmap;
  MethodResult res;
  res.name = "CT-VoxelMap";

  CTVoxelMapParams params;
  params.voxel_size = options.voxel_size;
  params.voxel_min_points = options.voxel_min_points;
  params.planarity_ratio = options.planarity_ratio;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.sigma_depth = options.sigma_depth;
  params.sigma_bearing = options.sigma_bearing;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  CTVoxelMapPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double planar_ratio_sum = 0.0;
  int planar_ratio_n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    const auto result = pipeline.registerFrame(pts_local);
    planar_ratio_sum += result.planar_ratio;
    ++planar_ratio_n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [CT-VoxelMap] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double mean_planar =
      planar_ratio_n > 0 ? planar_ratio_sum / planar_ratio_n : 0.0;
  res.note =
      "Probabilistic adaptive voxel map: uncertainty-weighted hybrid "
      "point-to-plane / point-to-distribution (no GT seed; anchor matches first "
      "GT pose). mean_planar_ratio=" +
      std::to_string(mean_planar);
  return res;
}

MethodResult runRVoxelMap(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const RVoxelMapDogfoodingOptions& options) {
  using namespace localization_zoo::r_voxelmap;
  MethodResult res;
  res.name = "R-VoxelMap";

  RVoxelMapParams params;
  params.voxel_size = options.voxel_size;
  params.voxel_min_points = options.voxel_min_points;
  params.planarity_ratio = options.planarity_ratio;
  params.inlier_dist = options.inlier_dist;
  params.max_depth = options.max_depth;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  RVoxelMapPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double matched_sum = 0.0;
  int matched_n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    matched_sum += result.matched_ratio;
    ++matched_n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [R-VoxelMap] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double mean_matched = matched_n > 0 ? matched_sum / matched_n : 0.0;
  res.note =
      "Recursive plane-fitting voxel map (outlier detect-and-reuse octree; no "
      "GT seed; anchor matches first GT pose). mean_matched_ratio=" +
      std::to_string(mean_matched);
  return res;
}

MethodResult runDammLoam(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const DammLoamDogfoodingOptions& options) {
  using namespace localization_zoo::damm_loam;
  MethodResult res;
  res.name = "DAMM-LOAM";

  DammLoamParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.vertical_cos = options.vertical_cos;
  params.horizontal_cos = options.horizontal_cos;
  params.ground_z = options.ground_z;
  params.edge_weight = options.edge_weight;
  params.degeneracy_ratio = options.degeneracy_ratio;
  params.degeneracy_boost = options.degeneracy_boost;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  DammLoamPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long ground_sum = 0, wall_sum = 0, edge_sum = 0;
  int degenerate_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    ground_sum += result.counts.ground;
    wall_sum += result.counts.wall;
    edge_sum += result.counts.edge;
    if (result.degenerate) ++degenerate_frames;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [DAMM-LOAM] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Degeneracy-aware multi-metric LiDAR odometry: range-image normal-class "
      "weighting (ground/wall/edge) + Hessian-eigen degeneracy boost (no GT "
      "seed; anchor matches first GT pose). ground=" +
      std::to_string(ground_sum) + " wall=" + std::to_string(wall_sum) +
      " edge=" + std::to_string(edge_sum) +
      " degenerate_frames=" + std::to_string(degenerate_frames);
  return res;
}

MethodResult runLodestar(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LodestarDogfoodingOptions& options) {
  using namespace localization_zoo::lodestar;
  MethodResult res;
  res.name = "LODESTAR";

  LodestarParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.t_chi = options.t_chi;
  params.anchor_strength = options.anchor_strength;
  params.active_window = options.active_window;
  params.enable_data_exploitation = options.enable_data_exploitation;
  params.t_loc = options.t_loc;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  LodestarPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  int degenerate_frames = 0;
  long pruned_sum = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    if (result.degenerate) ++degenerate_frames;
    pruned_sum += result.num_pruned;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [LODESTAR] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Degeneracy-aware LiDAR odometry: condition-number Schmidt-Kalman "
      "anchoring of degenerate directions to constant-velocity prediction "
      "(DA-ASKF) + localizability pruning (DA-DE); pure-LiDAR scope, no GT "
      "seed (anchor matches first GT pose). degenerate_frames=" +
      std::to_string(degenerate_frames) +
      " pruned=" + std::to_string(pruned_sum);
  return res;
}

MethodResult runTerrainRbf(const std::vector<std::string>& pcd_dirs,
                           const std::vector<Eigen::Matrix4d>& gt,
                           const TerrainRbfDogfoodingOptions& options) {
  using namespace localization_zoo::terrain_rbf_lio;
  MethodResult res;
  res.name = "Terrain-RBF-LIO";

  TerrainRbfParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.terrain_cell_size = options.terrain_cell_size;
  params.terrain_sigma = options.terrain_sigma;
  params.terrain_ground_band = options.terrain_ground_band;
  params.terrain_weight = options.terrain_weight;
  params.terrain_warmup = options.terrain_warmup;
  params.sensor_height_alpha = options.sensor_height_alpha;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  TerrainRbfPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  int terrain_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    if (result.terrain_active) ++terrain_frames;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Terrain-RBF] " << i << "/" << pcd_dirs.size()
                << " terrain_cells=" << pipeline.terrainSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Terrain-aware RBF-LIO: Gaussian RBF terrain height field (RLS) used as a "
      "vehicle-height-above-terrain soft prior to suppress z-drift (legged-wheel "
      "contact constraint reinterpreted for the car platform); constant-velocity, "
      "no GT seed. terrain_active_frames=" +
      std::to_string(terrain_frames);
  return res;
}

MethodResult runLidarIba(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const LidarIbaDogfoodingOptions& options) {
  using namespace localization_zoo::lidar_iba;
  MethodResult res;
  res.name = "LiDAR-IBA";

  LidarIbaParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.enable_ba = options.enable_ba;
  params.keyframe_interval = options.keyframe_interval;
  params.window_size = options.window_size;
  params.ba_iterations = options.ba_iterations;
  params.plane_voxel = options.plane_voxel;
  params.max_landmarks = options.max_landmarks;
  params.lidar_sigma = options.lidar_sigma;
  params.pose_prior_weight = options.pose_prior_weight;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  LidarIbaPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  int ba_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    if (result.ba_ran) ++ba_frames;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [LiDAR-IBA] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Consistency-improved LiDAR bundle adjustment: stereographic plane-normal "
      "parameterization + sliding-window plane BA with FEJ (oldest keyframe "
      "gauge-fixed) and a front-end odometry prior; constant-velocity, no GT "
      "seed. ba_frames=" +
      std::to_string(ba_frames);
  return res;
}

MethodResult runDaliSlam(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const DaliSlamDogfoodingOptions& options) {
  using namespace localization_zoo::dali_slam;
  MethodResult res;
  res.name = "DALI-SLAM";

  DaliSlamParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.enable_deskew = options.enable_deskew;
  params.spline_quadratic = options.spline_quadratic;
  params.degeneracy_ratio = options.degeneracy_ratio;
  params.enable_degeneracy = options.enable_degeneracy;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  DaliSlamPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  int degenerate_frames = 0;
  int deskew_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    if (result.degenerate) ++degenerate_frames;
    if (result.deskew_applied) ++deskew_frames;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [DALI-SLAM] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "DALI-SLAM: dual-spline motion distortion correction (constant-velocity "
      "spline deskew, azimuth per-point time) + degeneracy-aware solution "
      "remapping; pure-LiDAR, no GT seed. degenerate_frames=" +
      std::to_string(degenerate_frames) +
      " deskew_frames=" + std::to_string(deskew_frames);
  return res;
}

MethodResult runIntensityFlow(const std::vector<std::string>& pcd_dirs,
                              const std::vector<Eigen::Matrix4d>& gt,
                              const IntensityFlowDogfoodingOptions& options) {
  using namespace localization_zoo::intensity_flow;
  MethodResult res;
  res.name = "Intensity-Flow";

  IntensityFlowParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_icp_iterations = options.max_icp_iterations;
  params.initial_threshold = options.initial_threshold;
  params.enable_gradient_flow = options.enable_gradient_flow;
  params.gf_radius = options.gf_radius;
  params.gf_num_bins = options.gf_num_bins;
  params.gf_keep_ratio = options.gf_keep_ratio;
  params.enable_intensity = options.enable_intensity;
  params.intensity_sigma = options.intensity_sigma;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  IntensityFlowPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long sampled_sum = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto xyzi = limitLoadedXYZI(
        loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.source_voxel_size),
        options.max_source_points);
    if (xyzi.empty()) continue;
    std::vector<PointI> pts_local(xyzi.size());
    for (size_t k = 0; k < xyzi.size(); k++) {
      pts_local[k].p = xyzi[k].point;
      pts_local[k].intensity = xyzi[k].intensity;
    }
    const auto result = pipeline.registerFrame(pts_local);
    sampled_sum += result.num_sampled;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Intensity-Flow] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Intensity-gradient-flow odometry: RMS geometric gradient-flow sampling + "
      "intensity-geometry fused point-to-plane matching (intensity-consistency "
      "weight); pure-LiDAR (uses KITTI PointXYZI), no GT seed. sampled_points=" +
      std::to_string(sampled_sum);
  return res;
}

MethodResult runSvnIcp(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const SvnIcpDogfoodingOptions& options) {
  using namespace localization_zoo::svn_icp;
  MethodResult res;
  res.name = "SVN-ICP";

  SvnIcpParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.num_particles = options.num_particles;
  params.svn_iterations = options.svn_iterations;
  params.step_size = options.step_size;
  params.lidar_sigma = options.lidar_sigma;
  params.prior_precision = options.prior_precision;
  params.init_spread_rot = options.init_spread_rot;
  params.init_spread_trans = options.init_spread_trans;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  SvnIcpPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double trans_std_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    trans_std_sum += result.trans_std;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [SVN-ICP] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.4f",
                n > 0 ? trans_std_sum / static_cast<double>(n) : 0.0);
  res.note =
      "SVN-ICP: Stein Variational Newton on SE(3) over M particles for "
      "point-to-plane LiDAR odometry with built-in pose uncertainty (particle "
      "covariance); constant-velocity prior, no GT seed. mean_trans_std_m=" +
      std::string(buf);
  return res;
}

MethodResult runPcrDat(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const PcrDatDogfoodingOptions& options) {
  using namespace localization_zoo::pcr_dat;
  MethodResult res;
  res.name = "PCR-DAT";

  PcrDatParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.distribution_min_points = options.distribution_min_points;
  params.distribution_planarity = options.distribution_planarity;
  params.distance_planarity = options.distance_planarity;
  params.distance_sigma = options.distance_sigma;
  params.distribution_regularization = options.distribution_regularization;
  params.distance_weight = options.distance_weight;
  params.distribution_weight = options.distribution_weight;
  params.robust_scale = options.robust_scale;
  params.prior_precision = options.prior_precision;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  PcrDatPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long n_distrib_sum = 0, n_dist_sum = 0, n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    n_distrib_sum += result.num_distribution;
    n_dist_sum += result.num_distance;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [PCR-DAT] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.0f/%.0f",
                n > 0 ? static_cast<double>(n_distrib_sum) / n : 0.0,
                n > 0 ? static_cast<double>(n_dist_sum) / n : 0.0);
  res.note =
      "PCR-DAT: per-point switch between Gauss-distribution (rich) and "
      "point-to-plane distance (sparse) factors fused in one GN system; "
      "constant-velocity prior, no GT seed. mean_distrib/distance_corr=" +
      std::string(buf);
  return res;
}

MethodResult runSmallMighty(const std::vector<std::string>& pcd_dirs,
                            const std::vector<Eigen::Matrix4d>& gt,
                            const SmallMightyDogfoodingOptions& options) {
  using namespace localization_zoo::small_mighty;
  MethodResult res;
  res.name = "Small-but-Mighty";

  SmallMightyParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.planar_min = options.planar_min;
  params.edge_min = options.edge_min;
  params.contribution_gain = options.contribution_gain;
  params.contribution_cap = options.contribution_cap;
  params.plane_weight = options.plane_weight;
  params.edge_weight = options.edge_weight;
  params.robust_scale = options.robust_scale;
  params.prior_precision = options.prior_precision;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  SmallMightyPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long n_planar_sum = 0, n_edge_sum = 0, n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    n_planar_sum += result.num_planar;
    n_edge_sum += result.num_edge;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Small-but-Mighty] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.0f/%.0f",
                n > 0 ? static_cast<double>(n_planar_sum) / n : 0.0,
                n > 0 ? static_cast<double>(n_edge_sum) / n : 0.0);
  res.note =
      "Small-but-Mighty: stability-aware feature selection (statistical "
      "smoothness distribution) + contribution-weighted point-to-plane/line "
      "optimization; constant-velocity prior, no GT seed. "
      "mean_planar/edge_corr=" +
      std::string(buf);
  return res;
}

MethodResult runMGclo(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const MGcloDogfoodingOptions& options) {
  using namespace localization_zoo::m_gclo;
  MethodResult res;
  res.name = "M-GCLO";

  MGcloParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.ground_normal_threshold = options.ground_normal_threshold;
  params.ground_height_offset = options.ground_height_offset;
  params.ground_weight = options.ground_weight;
  params.nonground_weight = options.nonground_weight;
  params.ground_planarity = options.ground_planarity;
  params.distribution_regularization = options.distribution_regularization;
  params.uncertainty_range_ref = options.uncertainty_range_ref;
  params.robust_scale = options.robust_scale;
  params.prior_precision = options.prior_precision;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  MGcloPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long n_ground_sum = 0, n_nonground_sum = 0, n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    n_ground_sum += result.num_ground;
    n_nonground_sum += result.num_nonground;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [M-GCLO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.0f/%.0f",
                n > 0 ? static_cast<double>(n_ground_sum) / n : 0.0,
                n > 0 ? static_cast<double>(n_nonground_sum) / n : 0.0);
  res.note =
      "M-GCLO: multiple-ground point-to-plane constraints (vertical accuracy) + "
      "non-ground point-to-distribution (NDT) with per-point range-uncertainty "
      "weighting; constant-velocity prior, no GT seed. mean_ground/nonground_corr=" +
      std::string(buf);
  return res;
}

MethodResult runQuadricLo(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const QuadricLoDogfoodingOptions& options) {
  using namespace localization_zoo::quadric_lo;
  MethodResult res;
  res.name = "Quadric-LO";

  QuadricLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.plane_min_neighbors = options.plane_min_neighbors;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.quadric_min_neighbors = options.quadric_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.quadric_weight = options.quadric_weight;
  params.min_grad_norm = options.min_grad_norm;
  params.robust_scale = options.robust_scale;
  params.prior_precision = options.prior_precision;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  QuadricLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long n_quadric_sum = 0, n_plane_sum = 0, n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    n_quadric_sum += result.num_quadric;
    n_plane_sum += result.num_plane;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Quadric-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.0f/%.0f",
                n > 0 ? static_cast<double>(n_quadric_sum) / n : 0.0,
                n > 0 ? static_cast<double>(n_plane_sum) / n : 0.0);
  res.note =
      "Quadric-LO: quadric-surface (implicit q=xAx+bx+c) representation with "
      "point-to-quadric Taubin-distance residuals and plane fallback; "
      "constant-velocity prior, no GT seed. mean_quadric/plane_corr=" +
      std::string(buf);
  return res;
}

MethodResult runDilo(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const DiloDogfoodingOptions& options) {
  using namespace localization_zoo::dilo;
  MethodResult res;
  res.name = "DiLO";

  DiloParams params;
  params.sri_height = options.sri_height;
  params.sri_width = options.sri_width;
  params.fov_up_deg = options.fov_up_deg;
  params.fov_down_deg = options.fov_down_deg;
  params.max_iterations = options.max_iterations;
  params.initial_threshold = options.initial_threshold;
  params.robust_scale = options.robust_scale;
  params.keyframe_translation = options.keyframe_translation;
  params.keyframe_rotation_deg = options.keyframe_rotation_deg;
  params.source_voxel_size = options.gn_voxel_size;  // GN ソースの間引き (SRI は密な frame から構築)
  DiloPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long n_kf = 0, n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    // SRI を密にするため浅い間引きで読み込む。
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    if (result.keyframe_updated) ++n_kf;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [DiLO] " << i << "/" << pcd_dirs.size()
                << " keyframes=" << n_kf;
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%ld", n_kf);
  res.note =
      "DiLO: direct frame-to-keyframe LiDAR odometry via spherical-range-image "
      "projective data association (no NN search) + point-to-plane GN; "
      "constant-velocity prior, no GT seed. keyframes=" +
      std::string(buf);
  return res;
}

MethodResult runNhcLio(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const NhcLioDogfoodingOptions& options) {
  using namespace localization_zoo::nhc_lio;
  MethodResult res;
  res.name = "NHC-LIO";

  NhcLioParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.enable_nhc = options.enable_nhc;
  params.nhc_weight = options.nhc_weight;
  params.nhc_yaw_ref = options.nhc_yaw_ref;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  NhcLioPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double nhc_w_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    nhc_w_sum += result.nhc_weight_used;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [NHC-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? nhc_w_sum / static_cast<double>(n) : 0.0);
  res.note =
      "NHC-LIO: scan-to-map point-to-plane + nonholonomic-constraint factor "
      "(body lateral/vertical velocity ~0) with yaw-rate-adaptive weight; "
      "constant-velocity prior, no GT seed. mean_nhc_weight=" +
      std::string(buf);
  return res;
}

MethodResult runStudentTLo(const std::vector<std::string>& pcd_dirs,
                           const std::vector<Eigen::Matrix4d>& gt,
                           const StudentTLoDogfoodingOptions& options) {
  using namespace localization_zoo::student_t_lo;
  MethodResult res;
  res.name = "Student-T-LO";

  StudentTLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.max_iterations = options.max_iterations;
  params.enable_student_t = options.enable_student_t;
  params.student_t_dof = options.student_t_dof;
  params.scale_init = options.scale_init;
  params.estimate_scale = options.estimate_scale;
  params.scale_floor = options.scale_floor;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  StudentTLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double scale_sum = 0.0;
  double weight_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    scale_sum += result.scale_used;
    weight_sum += result.mean_weight;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Student-T-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.3f m / %.3f",
                n > 0 ? scale_sum / static_cast<double>(n) : 0.0,
                n > 0 ? weight_sum / static_cast<double>(n) : 0.0);
  res.note =
      "Student-T-LO: scan-to-map point-to-plane with Student's-t robust IRLS "
      "weighting w=(nu+1)/(nu+(r/sigma)^2) and EM-updated scale; "
      "constant-velocity prior, no GT seed. mean_scale/mean_weight=" +
      std::string(buf);
  return res;
}

MethodResult runSpectralLo(const std::vector<std::string>& pcd_dirs,
                           const std::vector<Eigen::Matrix4d>& gt,
                           const SpectralLoDogfoodingOptions& options) {
  using namespace localization_zoo::spectral_lo;
  MethodResult res;
  res.name = "Spectral-LO";

  SpectralLoParams params;
  params.bev_size = options.bev_size;
  params.bev_range = options.bev_range;
  params.max_range = options.max_range;
  params.z_min = options.z_min;
  params.z_max = options.z_max;
  params.logpolar_angles = options.logpolar_angles;
  params.logpolar_radii = options.logpolar_radii;
  params.max_yaw_deg = options.max_yaw_deg;
  params.keyframe_translation = options.keyframe_translation;
  SpectralLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double peak_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    peak_sum += result.translation_peak;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Spectral-LO] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? peak_sum / static_cast<double>(n) : 0.0);
  res.note =
      "Spectral-LO: frequency-domain BEV odometry; Fourier-Mellin log-polar "
      "phase correlation for yaw + phase-only correlation for translation "
      "(3-DoF, z held); no ICP, no GT seed. mean_translation_peak=" +
      std::string(buf);
  return res;
}

MethodResult runGmmLo(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const GmmLoDogfoodingOptions& options) {
  using namespace localization_zoo::gmm_lo;
  MethodResult res;
  res.name = "GMM-LO";

  GmmLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_iterations = options.max_iterations;
  params.sigma_init = options.sigma_init;
  params.sigma_final = options.sigma_final;
  params.outlier_weight = options.outlier_weight;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  GmmLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double w_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    w_sum += result.mean_weight;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [GMM-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? w_sum / static_cast<double>(n) : 0.0);
  res.note =
      "GMM-LO: scan-to-map GMM/EM soft-assignment registration; Gaussian "
      "responsibilities with uniform-outlier term, soft-centroid point-to-plane, "
      "deterministic annealing of sigma; CV prior, no GT seed. mean_weight=" +
      std::string(buf);
  return res;
}

MethodResult runGncLo(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const GncLoDogfoodingOptions& options) {
  using namespace localization_zoo::gnc_lo;
  MethodResult res;
  res.name = "GNC-LO";

  GncLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.gnc_truncation = options.gnc_truncation;
  params.gnc_factor = options.gnc_factor;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  GncLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double inlier_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    inlier_sum += result.inlier_ratio;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [GNC-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? inlier_sum / static_cast<double>(n) : 0.0);
  res.note =
      "GNC-LO: scan-to-map point-to-plane with Graduated Non-Convexity (TLS) "
      "robust weighting; mu continuation from convex to non-convex, hard "
      "outlier rejection; CV prior, no GT seed. mean_inlier_ratio=" +
      std::string(buf);
  return res;
}

MethodResult runMccLo(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const MccLoDogfoodingOptions& options) {
  using namespace localization_zoo::mcc_lo;
  MethodResult res;
  res.name = "MCC-LO";

  MccLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.mcc_sigma = options.mcc_sigma;
  params.mcc_adaptive_sigma = options.mcc_adaptive_sigma;
  params.mcc_sigma_floor = options.mcc_sigma_floor;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  MccLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double weight_sum = 0.0;
  double sigma_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    weight_sum += result.mean_weight;
    sigma_sum += result.sigma_used;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [MCC-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.3f sigma=%.3f",
                n > 0 ? weight_sum / static_cast<double>(n) : 0.0,
                n > 0 ? sigma_sum / static_cast<double>(n) : 0.0);
  res.note =
      "MCC-LO: scan-to-map point-to-plane with Maximum Correntropy Criterion "
      "(Welsch/Gaussian kernel) robust weighting and Silverman adaptive kernel "
      "bandwidth; CV prior, no GT seed. mean_weight=" +
      std::string(buf);
  return res;
}

MethodResult runImlsSlam(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const ImlsSlamDogfoodingOptions& options) {
  using namespace localization_zoo::imls_slam;
  MethodResult res;
  res.name = "IMLS-SLAM";

  ImlsSlamParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.normal_radius = options.normal_radius;
  params.imls_h = options.imls_h;
  params.imls_support_radius = options.imls_support_radius;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.samples_per_axis = options.samples_per_axis;
  params.use_observability_sampling = options.use_observability_sampling;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  ImlsSlamPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double residual_sum = 0.0;
  double sample_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    residual_sum += result.mean_abs_residual;
    sample_sum += result.num_samples;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [IMLS-SLAM] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[96];
  std::snprintf(buf, sizeof(buf), "%.3f samples=%.0f",
                n > 0 ? residual_sum / static_cast<double>(n) : 0.0,
                n > 0 ? sample_sum / static_cast<double>(n) : 0.0);
  res.note =
      "IMLS-SLAM: scan-to-model matching on an implicit moving least squares "
      "surface (point-to-implicit-surface) with observability-based sampling; "
      "CV prior, no GT seed. mean|residual|=" +
      std::string(buf);
  return res;
}

MethodResult runMeshLoam(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const MeshLoamDogfoodingOptions& options) {
  using namespace localization_zoo::mesh_loam;
  MethodResult res;
  res.name = "Mesh-LOAM";

  MeshLoamParams params;
  params.voxel_size = options.voxel_size;
  params.imls_h = options.imls_h;
  params.lambda_n = options.lambda_n;
  params.curvature_threshold = options.curvature_threshold;
  params.cos_gate = options.cos_gate;
  params.search_radius = options.search_radius;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.mesh_update_interval = options.mesh_update_interval;
  params.local_map_radius = options.local_map_radius;
  MeshLoamPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double residual_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    residual_sum += result.mean_abs_residual;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [Mesh-LOAM] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize()
                << " facets=" << pipeline.meshSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? residual_sum / static_cast<double>(n) : 0.0);
  res.note =
      "Mesh-LOAM: incremental voxel-hash IMLS-SDF map with block-wise "
      "zero-surface extraction and point-to-mesh GN odometry (normal cosine "
      "gate); CV prior, no GT seed. mean|residual|=" +
      std::string(buf);
  return res;
}

MethodResult runElo(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt,
                    const EloDogfoodingOptions& options) {
  using namespace localization_zoo::elo;
  MethodResult res;
  res.name = "ELO";

  EloParams params;
  params.sri_width = options.sri_width;
  params.sri_height = options.sri_height;
  params.bev_resolution = options.bev_resolution;
  params.sensor_height = options.sensor_height;
  params.max_iterations = options.max_iterations;
  params.max_sri_correspondence_dist = options.max_sri_correspondence_dist;
  params.max_ground_correspondence_dist = options.max_ground_correspondence_dist;
  params.ground_weight = options.ground_weight;
  params.non_ground_weight = options.non_ground_weight;
  params.max_alignment_points = options.max_alignment_points;
  params.model_window_frames = options.model_window_frames;
  EloPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double residual_sum = 0.0;
  double ground_corr_sum = 0.0;
  double nonground_corr_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    residual_sum += result.mean_abs_residual;
    ground_corr_sum += result.num_ground_correspondences;
    nonground_corr_sum += result.num_non_ground_correspondences;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [ELO] " << i << "/" << pcd_dirs.size()
                << " sri=" << pipeline.nonGroundMapSize()
                << " bev=" << pipeline.groundMapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[128];
  std::snprintf(buf, sizeof(buf), "%.3f g=%.0f ng=%.0f",
                n > 0 ? residual_sum / static_cast<double>(n) : 0.0,
                n > 0 ? ground_corr_sum / static_cast<double>(n) : 0.0,
                n > 0 ? nonground_corr_sum / static_cast<double>(n) : 0.0);
  res.note =
      "ELO: frame-to-model projective odometry with non-ground spherical "
      "range-image map, ground BEV map, range-adaptive PCA normals, and "
      "SRI/BEV model fusion; CV prior, no GT seed. mean|residual|=" +
      std::string(buf);
  return res;
}

MethodResult runTcLvgf(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const TcLvgfDogfoodingOptions& options) {
  using namespace localization_zoo::tc_lvgf;
  MethodResult res;
  res.name = "TC-LVGF";

  TcLvgfParams params;
  params.voxel_size = options.voxel_size;
  params.registration_voxel_size = options.registration_voxel_size;
  params.max_registration_points = options.max_registration_points;
  params.image_width = options.image_width;
  params.image_height = options.image_height;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.min_visual_lines = options.min_visual_lines;
  params.max_lidar_lines = options.max_lidar_lines;
  params.max_visual_lines = options.max_visual_lines;
  params.line_weight = options.line_weight;
  params.direction_weight = options.direction_weight;
  params.fusion_pixel_gate = options.fusion_pixel_gate;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  TcLvgfPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  long long visual_lines_total = 0;
  long long fused_lines_total = 0;
  long long plane_corr_total = 0;
  long long line_corr_total = 0;
  long fallback_frames = 0;
  double residual_sum = 0.0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); ++i) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    visual_lines_total += result.num_visual_lines;
    fused_lines_total += result.num_fused_lines;
    plane_corr_total += result.num_plane_correspondences;
    line_corr_total += result.num_line_correspondences;
    residual_sum += result.mean_abs_residual;
    if (result.lidar_fallback) ++fallback_frames;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [TC-LVGF] " << i << "/" << pcd_dirs.size()
                << " map=" << pipeline.mapSize()
                << " lines=" << pipeline.lineMapSize()
                << " fallback=" << fallback_frames;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double denom = std::max<size_t>(1, res.poses.size());
  res.note =
      "TC-LVGF: tightly coupled LiDAR-visual geometric-feature port with "
      "spherical fusion of pseudo-visual range-image lines and LiDAR linear "
      "features, point-to-plane + point-to-line scan-to-map optimization, and "
      "LiDAR fallback on visual-line scarcity. visual_lines/frame=" +
      std::to_string(static_cast<double>(visual_lines_total) / denom) +
      " fused_lines/frame=" +
      std::to_string(static_cast<double>(fused_lines_total) / denom) +
      " plane_corr/frame=" +
      std::to_string(static_cast<double>(plane_corr_total) / denom) +
      " line_corr/frame=" +
      std::to_string(static_cast<double>(line_corr_total) / denom) +
      " fallback_frames=" + std::to_string(fallback_frames) +
      " mean|residual|=" + std::to_string(residual_sum / denom);
  return res;
}

MethodResult runOplLvio(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const OplLvioDogfoodingOptions& options) {
  using namespace localization_zoo::opl_lvio;
  MethodResult res;
  res.name = "OPL-LVIO";

  OplLvioParams params;
  params.voxel_size = options.voxel_size;
  params.registration_voxel_size = options.registration_voxel_size;
  params.max_registration_points = options.max_registration_points;
  params.image_width = options.image_width;
  params.image_height = options.image_height;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.min_visual_points = options.min_visual_points;
  params.max_visual_points = options.max_visual_points;
  params.min_visual_lines = options.min_visual_lines;
  params.max_lidar_lines = options.max_lidar_lines;
  params.max_visual_lines = options.max_visual_lines;
  params.visual_point_weight = options.visual_point_weight;
  params.line_weight = options.line_weight;
  params.direction_weight = options.direction_weight;
  params.helmert_min_scale = options.helmert_min_scale;
  params.helmert_max_scale = options.helmert_max_scale;
  params.fusion_pixel_gate = options.fusion_pixel_gate;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  OplLvioPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  long long visual_points_total = 0;
  long long visual_lines_total = 0;
  long long fused_lines_total = 0;
  long long plane_corr_total = 0;
  long long point_corr_total = 0;
  long long line_corr_total = 0;
  long fallback_frames = 0;
  double point_scale_sum = 0.0;
  double line_scale_sum = 0.0;
  double residual_sum = 0.0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); ++i) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    visual_points_total += result.num_visual_points;
    visual_lines_total += result.num_visual_lines;
    fused_lines_total += result.num_fused_lines;
    plane_corr_total += result.num_plane_correspondences;
    point_corr_total += result.num_point_correspondences;
    line_corr_total += result.num_line_correspondences;
    point_scale_sum += result.point_weight_scale;
    line_scale_sum += result.line_weight_scale;
    residual_sum += result.mean_abs_residual;
    if (result.lidar_fallback) ++fallback_frames;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [OPL-LVIO] " << i << "/" << pcd_dirs.size()
                << " map=" << pipeline.mapSize()
                << " lines=" << pipeline.lineMapSize()
                << " fallback=" << fallback_frames;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double denom = std::max<size_t>(1, res.poses.size());
  res.note =
      "OPL-LVIO: optimized point-line LiDAR-visual-inertial port with "
      "range-image visual point/line proxies, LiDAR depth correlation, "
      "scan-to-map plane + visual point + visual line residuals, and "
      "Helmert-style variance component weighting. visual_points/frame=" +
      std::to_string(static_cast<double>(visual_points_total) / denom) +
      " visual_lines/frame=" +
      std::to_string(static_cast<double>(visual_lines_total) / denom) +
      " fused_lines/frame=" +
      std::to_string(static_cast<double>(fused_lines_total) / denom) +
      " plane_corr/frame=" +
      std::to_string(static_cast<double>(plane_corr_total) / denom) +
      " point_corr/frame=" +
      std::to_string(static_cast<double>(point_corr_total) / denom) +
      " line_corr/frame=" +
      std::to_string(static_cast<double>(line_corr_total) / denom) +
      " point_scale/frame=" + std::to_string(point_scale_sum / denom) +
      " line_scale/frame=" + std::to_string(line_scale_sum / denom) +
      " fallback_frames=" + std::to_string(fallback_frames) +
      " mean|residual|=" + std::to_string(residual_sum / denom);
  return res;
}

void applyVisualLidarAdapterOptions(
    const VisualLidarAdapterDogfoodingOptions& options,
    localization_zoo::opl_lvio::OplLvioParams* params) {
  params->voxel_size = options.voxel_size;
  params->registration_voxel_size = options.registration_voxel_size;
  params->max_registration_points = options.max_registration_points;
  params->image_width = options.image_width;
  params->image_height = options.image_height;
  params->max_icp_iterations = options.max_icp_iterations;
  params->max_correspondence_dist = options.max_correspondence_dist;
  params->min_visual_points = options.min_visual_points;
  params->max_visual_points = options.max_visual_points;
  params->min_visual_lines = options.min_visual_lines;
  params->max_lidar_lines = options.max_lidar_lines;
  params->max_visual_lines = options.max_visual_lines;
  params->local_map_radius = options.local_map_radius;
  params->map_cleanup_interval = options.map_cleanup_interval;
}

template <typename PipelineT, typename ParamsT>
MethodResult runVisualLidarAdapter(
    const std::vector<std::string>& pcd_dirs,
    const std::vector<Eigen::Matrix4d>& gt,
    const VisualLidarAdapterDogfoodingOptions& options,
    const std::string& result_name,
    const std::string& progress_name,
    const std::string& note_prefix) {
  MethodResult res;
  res.name = result_name;

  ParamsT params;
  applyVisualLidarAdapterOptions(options, &params.backend);
  PipelineT pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  long long visual_points_total = 0;
  long long visual_lines_total = 0;
  long long fused_lines_total = 0;
  long long plane_corr_total = 0;
  long long point_corr_total = 0;
  long long line_corr_total = 0;
  long fallback_frames = 0;
  double point_scale_sum = 0.0;
  double line_scale_sum = 0.0;
  double residual_sum = 0.0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); ++i) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    visual_points_total += result.backend.num_visual_points;
    visual_lines_total += result.backend.num_visual_lines;
    fused_lines_total += result.backend.num_fused_lines;
    plane_corr_total += result.backend.num_plane_correspondences;
    point_corr_total += result.backend.num_point_correspondences;
    line_corr_total += result.backend.num_line_correspondences;
    point_scale_sum += result.backend.point_weight_scale;
    line_scale_sum += result.backend.line_weight_scale;
    residual_sum += result.backend.mean_abs_residual;
    if (result.backend.lidar_fallback) ++fallback_frames;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [" << progress_name << "] " << i << "/"
                << pcd_dirs.size() << " map=" << pipeline.mapSize()
                << " lines=" << pipeline.lineMapSize()
                << " fallback=" << fallback_frames;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double denom = std::max<size_t>(1, res.poses.size());
  res.note =
      note_prefix + " visual_points/frame=" +
      std::to_string(static_cast<double>(visual_points_total) / denom) +
      " visual_lines/frame=" +
      std::to_string(static_cast<double>(visual_lines_total) / denom) +
      " fused_lines/frame=" +
      std::to_string(static_cast<double>(fused_lines_total) / denom) +
      " plane_corr/frame=" +
      std::to_string(static_cast<double>(plane_corr_total) / denom) +
      " point_corr/frame=" +
      std::to_string(static_cast<double>(point_corr_total) / denom) +
      " line_corr/frame=" +
      std::to_string(static_cast<double>(line_corr_total) / denom) +
      " point_scale/frame=" + std::to_string(point_scale_sum / denom) +
      " line_scale/frame=" + std::to_string(line_scale_sum / denom) +
      " fallback_frames=" + std::to_string(fallback_frames) +
      " mean|residual|=" + std::to_string(residual_sum / denom);
  return res;
}

MethodResult runTricpLo(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const TricpLoDogfoodingOptions& options) {
  using namespace localization_zoo::tricp_lo;
  MethodResult res;
  res.name = "TrICP-LO";

  TricpLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.normal_min_neighbors = options.normal_min_neighbors;
  params.planarity_threshold = options.planarity_threshold;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.max_iterations = options.max_iterations;
  params.auto_overlap = options.auto_overlap;
  params.overlap_ratio = options.overlap_ratio;
  params.min_overlap = options.min_overlap;
  params.frmsd_lambda = options.frmsd_lambda;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  TricpLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double overlap_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    overlap_sum += result.overlap;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [TrICP-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? overlap_sum / static_cast<double>(n) : 0.0);
  res.note =
      "TrICP-LO: trimmed/fractional ICP (least trimmed squares) point-to-plane "
      "with FRMSD automatic overlap estimation; CV prior, no GT seed. "
      "mean_overlap=" +
      std::string(buf);
  return res;
}

MethodResult runKcLo(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const KcLoDogfoodingOptions& options) {
  using namespace localization_zoo::kc_lo;
  MethodResult res;
  res.name = "KC-LO";

  KcLoParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_iterations = options.max_iterations;
  params.kc_sigma = options.kc_sigma;
  params.kc_sigma_init = options.kc_sigma_init;
  params.kc_anneal = options.kc_anneal;
  params.kc_radius_factor = options.kc_radius_factor;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  KcLoPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  double weight_sum = 0.0;
  long n = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    const auto result = pipeline.registerFrame(pts_local);
    weight_sum += result.mean_weight;
    ++n;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [KC-LO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.3f",
                n > 0 ? weight_sum / static_cast<double>(n) : 0.0);
  const bool annealed = options.kc_sigma_init > options.kc_sigma * 1.0001;
  std::ostringstream note;
  note << "KC-LO: correspondence-free kernel-correlation (Renyi quadratic entropy) "
       << "soft point-to-point with "
       << (annealed ? "coarse-to-fine sigma annealing" : "fixed sigma (no annealing)")
       << "; sigma_init=" << options.kc_sigma_init
       << " sigma_min=" << options.kc_sigma
       << "; CV prior, no GT seed. mean_weight=" << buf;
  res.note =
      note.str();
  return res;
}

MethodResult runDegenSense(const std::vector<std::string>& pcd_dirs,
                           const std::vector<Eigen::Matrix4d>& gt,
                           const std::vector<double>& frame_timestamps,
                           const std::vector<ImuSampleCsv>& imu_samples,
                           const DegenSenseDogfoodingOptions& options) {
  using namespace localization_zoo::degen_sense;
  MethodResult res;
  res.name = "DegenSense";

  DegenSenseParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.warmup_frames = options.warmup_frames;
  params.mad_k = options.mad_k;
  params.gyro_bias_gain = options.gyro_bias_gain;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  DegenSensePipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  long degen_frames = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    if (result.degenerate) ++degen_frames;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [DegenSense] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      (imu_frames > 0
           ? "Degeneracy-factor sensing (condition number) + adaptive "
             "MAD-outlier detection + IMU/LiDAR fusion compensation (GT-seeded "
             "init)."
           : "Degeneracy sensing, constant-velocity fallback (no imu.csv; "
             "GT-seeded init).") +
      std::string(" degenerate_frames=") + std::to_string(degen_frames);
  return res;
}

MethodResult runVibrationLIO(const std::vector<std::string>& pcd_dirs,
                             const std::vector<Eigen::Matrix4d>& gt,
                             const std::vector<double>& frame_timestamps,
                             const std::vector<ImuSampleCsv>& imu_samples,
                             const VibrationLIODogfoodingOptions& options) {
  using namespace localization_zoo::vibration_lio;
  MethodResult res;
  res.name = "Vibration-LIO";

  VibrationLIOParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.sigma_depth = options.sigma_depth;
  params.sigma_bearing = options.sigma_bearing;
  params.gamma = options.gamma;
  params.scan_period = options.scan_period;
  params.gyro_bias_gain = options.gyro_bias_gain;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  VibrationLIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  double vib_sum = 0.0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    vib_sum += result.vibration_omega;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [Vibration-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      (imu_frames > 0
           ? "Post-undistortion uncertainty LIO: IMU-MAD vibration intensity "
             "-> per-point covariance, Mahalanobis point-to-plane (GT-seeded "
             "init)."
           : "Uncertainty-weighted point-to-plane, no vibration estimate (no "
             "imu.csv; GT-seeded init).") +
      std::string(" mean_vib_omega=") +
      std::to_string(imu_frames > 0 ? vib_sum / imu_frames : 0.0);
  return res;
}

MethodResult runIDLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const IDLIODogfoodingOptions& options) {
  using namespace localization_zoo::id_lio;
  MethodResult res;
  res.name = "ID-LIO";

  IDLIOParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.range_image_width = options.range_image_width;
  params.range_image_height = options.range_image_height;
  params.new_object_margin = options.new_object_margin;
  params.disappearance_margin = options.disappearance_margin;
  params.dynamic_weight = options.dynamic_weight;
  params.delayed_removal_frames = options.delayed_removal_frames;
  params.gyro_bias_gain = options.gyro_bias_gain;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  IDLIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  long long dynamic_total = 0;
  long long removed_total = 0;
  long long corr_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); ++i) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.used_imu) ++imu_frames;
    dynamic_total += result.num_dynamic_points;
    removed_total += result.num_removed_points;
    corr_total += result.num_correspondences;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [ID-LIO] " << i << "/" << pcd_dirs.size()
                << " map=" << pipeline.mapSize()
                << " dynamic_map=" << pipeline.dynamicMapPoints();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double denom = std::max<size_t>(1, res.poses.size());
  res.note =
      (imu_frames > 0
           ? "Indexed-point dynamic LIO: pseudo-occupancy dynamic detection, "
             "delayed removal, and IMU gyro prior."
           : "Indexed-point dynamic scan-to-map: pseudo-occupancy dynamic "
             "detection and delayed removal; no imu.csv so constant-velocity "
             "fallback.") +
      std::string(" dynamic/frame=") +
      std::to_string(static_cast<double>(dynamic_total) / denom) +
      " corr/frame=" + std::to_string(static_cast<double>(corr_total) / denom) +
      " removed_sum/frame=" +
      std::to_string(static_cast<double>(removed_total) / denom);
  return res;
}

MethodResult runRFLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const RFLIODogfoodingOptions& options) {
  using namespace localization_zoo::rf_lio;
  MethodResult res;
  res.name = "RF-LIO";

  RfLioParams params;
  params.backend.voxel_size = options.voxel_size;
  params.backend.max_points_per_voxel = options.max_points_per_voxel;
  params.backend.max_icp_iterations = options.max_icp_iterations;
  params.backend.max_correspondence_dist = options.max_correspondence_dist;
  params.backend.range_image_width = options.range_image_width;
  params.backend.range_image_height = options.range_image_height;
  params.backend.new_object_margin = options.new_object_margin;
  params.backend.disappearance_margin = options.disappearance_margin;
  params.backend.dynamic_weight = options.dynamic_weight;
  params.backend.delayed_removal_frames = options.delayed_removal_frames;
  params.backend.gyro_bias_gain = options.gyro_bias_gain;
  params.backend.local_map_radius = options.local_map_radius;
  params.backend.map_cleanup_interval = options.map_cleanup_interval;
  params.range_image_width = options.range_image_width;
  params.range_image_height = options.range_image_height;
  params.foreground_margin = options.foreground_margin;
  params.max_removal_fraction = options.max_removal_fraction;
  params.min_keep_points = options.min_keep_points;
  RfLioPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());

  int imu_frames = 0;
  long long dynamic_total = 0;
  long long backend_removed_total = 0;
  long long corr_total = 0;
  long long removal_first_total = 0;
  long long candidate_total = 0;
  long long kept_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); ++i) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }
    const auto result = pipeline.registerFrame(pts_local, imu_batch);
    if (result.backend.used_imu) ++imu_frames;
    dynamic_total += result.backend.num_dynamic_points;
    backend_removed_total += result.backend.num_removed_points;
    corr_total += result.backend.num_correspondences;
    removal_first_total += result.removal_first_points;
    candidate_total += result.candidate_points;
    kept_total += result.kept_points;
    res.poses.push_back(result.pose);
    if (i % 10 == 0) {
      std::cerr << "\r  [RF-LIO] " << i << "/" << pcd_dirs.size()
                << " map=" << pipeline.mapSize()
                << " dynamic_map=" << pipeline.dynamicMapPoints()
                << " removed_first=" << removal_first_total;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  const double denom = std::max<size_t>(1, res.poses.size());
  res.note =
      (imu_frames > 0
           ? "RF-LIO removal-first LIO: adaptive multi-resolution range-image "
             "foreground removal before indexed dynamic LIO, with IMU gyro prior."
           : "RF-LIO removal-first scan-to-map: adaptive multi-resolution "
             "range-image foreground removal before indexed dynamic scan-to-map; "
             "no imu.csv so constant-velocity fallback.") +
      std::string(" removal_first/frame=") +
      std::to_string(static_cast<double>(removal_first_total) / denom) +
      " candidates/frame=" +
      std::to_string(static_cast<double>(candidate_total) / denom) +
      " kept/frame=" +
      std::to_string(static_cast<double>(kept_total) / denom) +
      " backend_dynamic/frame=" +
      std::to_string(static_cast<double>(dynamic_total) / denom) +
      " corr/frame=" + std::to_string(static_cast<double>(corr_total) / denom) +
      " backend_removed_sum/frame=" +
      std::to_string(static_cast<double>(backend_removed_total) / denom);
  return res;
}

MethodResult runBievrLIO(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const BievrLIODogfoodingOptions& options) {
  using namespace localization_zoo::bievr_lio;
  MethodResult res;
  res.name = "BIEVR-LIO";
  BievrLIOParams params;
  params.voxel_size = options.voxel_size;
  params.voxel_min_points = options.voxel_min_points;
  params.planarity_ratio = options.planarity_ratio;
  params.pixel_res = options.pixel_res;
  params.max_icp_iterations = options.max_icp_iterations;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  BievrLIOPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  double bump_sum = 0.0; int bn = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                   options.source_voxel_size),
                           options.max_source_points);
    if (pts.empty()) continue;
    const auto r = pipeline.registerFrame(pts);
    bump_sum += r.bump_constraint_ratio; ++bn;
    res.poses.push_back(anchorRelativePose(world_anchor, r.pose));
    if (i % 10 == 0)
      std::cerr << "\r  [BIEVR-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Bump-image voxel map: oriented height image gradients add in-plane "
             "constraints (no GT seed; anchor matches first GT pose). "
             "mean_bump_ratio=" + std::to_string(bn > 0 ? bump_sum / bn : 0.0);
  return res;
}

MethodResult runUALIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const UALIODogfoodingOptions& options) {
  using namespace localization_zoo::ua_lio;
  MethodResult res;
  res.name = "UA-LIO";
  UALIOParams params;
  params.voxel_size = options.voxel_size;
  params.voxel_min_points = options.voxel_min_points;
  params.max_icp_iterations = options.max_icp_iterations;
  params.max_correspondence_dist = options.max_correspondence_dist;
  params.ground_constraint = options.ground_constraint;
  params.ground_weight = options.ground_weight;
  params.gyro_bias_gain = options.gyro_bias_gain;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  UALIOPipeline pipeline(params);
  pipeline.setInitialPose(gt.empty() ? Eigen::Matrix4d::Identity() : gt.front());
  int imu_frames = 0; long ground_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                   options.source_voxel_size),
                           options.max_source_points);
    if (pts.empty()) continue;
    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() &&
        frame_timestamps.size() == pcd_dirs.size() && i < frame_timestamps.size())
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    const auto r = pipeline.registerFrame(pts, imu_batch);
    if (r.used_imu) ++imu_frames;
    ground_total += r.ground_cells;
    res.poses.push_back(r.pose);
    if (i % 10 == 0)
      std::cerr << "\r  [UA-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = (imu_frames > 0
                  ? "Uncertainty-aware D2D LIO: per-point covariance + ground "
                    "constraint (GT-seeded init)."
                  : "D2D + ground constraint, constant-velocity (no imu.csv; "
                    "GT-seeded init).") +
             std::string(" ground_cells=") + std::to_string(ground_total);
  return res;
}

MethodResult runCubeLIO(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const CubeLIODogfoodingOptions& options) {
  using namespace localization_zoo::cube_lio;
  MethodResult res;
  res.name = "CUBE-LIO";

  CubeLIOParams params;
  params.voxel_size = options.voxel_size;
  params.max_points_per_voxel = options.max_points_per_voxel;
  params.max_icp_iterations = options.max_icp_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.initial_threshold = options.initial_threshold;
  params.cubemap_size = options.cubemap_size;
  params.gaussian_sigma = options.gaussian_sigma;
  params.semi_dense_quantile = options.semi_dense_quantile;
  params.intensity_sigma = options.intensity_sigma;
  params.local_map_radius = options.local_map_radius;
  params.map_cleanup_interval = options.map_cleanup_interval;
  CubeLIOPipeline pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  long semi_dense_total = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitLoadedXYZI(pts, options.max_input_points);
    if (pts.empty()) continue;

    std::vector<IntensityPoint> frame;
    frame.reserve(pts.size());
    for (const auto& p : pts) frame.push_back({p.point, p.intensity});

    const auto result = pipeline.registerFrame(frame);
    semi_dense_total += result.semi_dense_points;
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [CUBE-LIO] " << i << "/" << pcd_dirs.size()
                << " voxels=" << pipeline.mapSize();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Intensity-cubemap semi-dense odometry: IGM high-gradient feature "
      "selection + intensity-consistency-weighted point-to-plane (no GT seed; "
      "anchor matches first GT pose). semi_dense_total=" +
      std::to_string(semi_dense_total);
  return res;
}

MethodResult runDLO(const std::vector<std::string>& pcd_dirs,
                    const std::vector<Eigen::Matrix4d>& gt,
                    const DLODofeedingOptions& options) {
  using namespace localization_zoo::dlo;
  MethodResult res;
  res.name = "DLO";

  DLOParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_translation_threshold = options.keyframe_translation_threshold;
  params.keyframe_rotation_threshold_rad = options.keyframe_rotation_threshold_rad;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  params.gicp.k_neighbors = options.gicp_k_neighbors;
  params.gicp.max_correspondence_distance = options.gicp_max_correspondence_distance;
  params.gicp.max_iterations = options.gicp_max_iterations;
  params.gicp.rotation_epsilon = options.gicp_rotation_epsilon;
  params.gicp.translation_epsilon = options.gicp_translation_epsilon;
  params.gicp.covariance_regularization = options.gicp_covariance_regularization;
  params.gicp.fitness_epsilon = options.gicp_fitness_epsilon;

  DLO pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [DLO] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " map_pts=" << result.map_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Keyframe scan-to-map odometry with GICP alignment (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runDLIO(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const std::vector<double>& frame_timestamps,
                     const std::vector<ImuSampleCsv>& imu_samples,
                     const DLIODogfoodingOptions& options) {
  using namespace localization_zoo::dlio;
  MethodResult res;
  res.name = "DLIO";

  DLIOParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_translation_threshold = options.keyframe_translation_threshold;
  params.keyframe_rotation_threshold_rad = options.keyframe_rotation_threshold_rad;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  params.gicp.k_neighbors = options.gicp_k_neighbors;
  params.gicp.max_correspondence_distance = options.gicp_max_correspondence_distance;
  params.gicp.max_iterations = options.gicp_max_iterations;
  params.gicp.rotation_epsilon = options.gicp_rotation_epsilon;
  params.gicp.translation_epsilon = options.gicp_translation_epsilon;
  params.gicp.covariance_regularization = options.gicp_covariance_regularization;
  params.gicp.fitness_epsilon = options.gicp_fitness_epsilon;
  params.imu_rotation_fusion_weight = options.imu_rotation_fusion_weight;
  params.imu_translation_fusion_weight = options.imu_translation_fusion_weight;
  params.imu_velocity_fusion_weight = options.imu_velocity_fusion_weight;
  params.lidar_confidence_correspondence_scale =
      options.lidar_confidence_correspondence_scale;

  DLIO pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(cloud, imu_batch);
    res.poses.push_back(anchorRelativePose(world_anchor, result.state.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [DLIO] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " imu=" << (result.imu_used ? "y" : "n")
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Direct LiDAR-inertial odometry: GICP scan-to-map plus IMU preintegration "
      "prior between scans when imu.csv and per-frame timestamps align; otherwise "
      "LiDAR motion prior only (no GT seed).";
  return res;
}

MethodResult runCTICP(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const CTICPDogfoodingOptions& options,
                      bool gt_seed = false) {
  using namespace localization_zoo::ct_icp;
  MethodResult res;
  res.name = "CT-ICP";

  CTICPParams params;
  params.voxel_resolution = options.voxel_resolution;
  params.max_iterations = options.max_iterations;
  params.ceres_max_iterations = options.ceres_max_iterations;
  params.planarity_threshold = options.planarity_threshold;
  params.keypoint_voxel_size = options.keypoint_voxel_size;
  params.max_frames_in_map = options.max_frames_in_map;
  if (options.max_correspondence_dist > 0.0) {
    params.max_correspondence_dist = options.max_correspondence_dist;
  }
  if (options.knn > 0) {
    params.knn = options.knn;
  }
  params.multi_scale_correspondences = options.multi_scale_correspondences;
  params.use_normal_cholesky_solver = options.use_normal_cholesky_solver;
  params.power_planarity = options.power_planarity;
  params.weight_alpha = options.weight_alpha;
  params.weight_neighborhood = options.weight_neighborhood;
  params.min_planarity_floor = options.min_planarity_floor;
  params.flat_regularizer_weight = options.flat_regularizer_weight;
  if (options.cauchy_loss_param > 0.0) {
    params.cauchy_loss_param = options.cauchy_loss_param;
  }
  params.regularizer_n_cap = options.regularizer_n_cap;
  params.use_closest_neighbor_reference = options.use_closest_neighbor_reference;
  params.pca_neighbor_count = options.pca_neighbor_count;
  params.min_distance_between_points = options.min_distance_between_points;
  params.coarse_to_fine = options.coarse_to_fine;
  params.coarse_iterations = options.coarse_iterations;
  params.coarse_search_radius = options.coarse_search_radius;
  params.coarse_planarity_threshold = options.coarse_planarity_threshold;
  params.coarse_cauchy_sigma_mult = options.coarse_cauchy_sigma_mult;
  if (options.location_consistency_weight >= 0.0) {
    params.location_consistency_weight = options.location_consistency_weight;
  }
  if (options.orientation_consistency_weight >= 0.0) {
    params.orientation_consistency_weight = options.orientation_consistency_weight;
  }
  if (options.constant_velocity_weight >= 0.0) {
    params.constant_velocity_weight = options.constant_velocity_weight;
  }
  CTICPRegistration reg(params);

  auto frame_to_matrix = [](const TrajectoryFrame& f) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = f.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = f.end_pose.trans;
    return T;
  };
  auto pose_to_matrix = [](const auto& p) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = p.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = p.trans;
    return T;
  };

  TrajectoryFrame prev;
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  prev.begin_pose.trans = world_anchor.block<3, 1>(0, 3);
  prev.begin_pose.quat = Eigen::Quaterniond(world_anchor.block<3, 3>(0, 0));
  prev.end_pose = prev.begin_pose;
  res.poses.push_back(world_anchor);
  Eigen::Matrix4d T_prev_world = world_anchor;
  Eigen::Matrix4d T_prev_prev_world = world_anchor;

  size_t gated_rollbacks = 0;
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto scan = limitLoadedScan(loadTimedPCD(pcd_dirs[i] + "/cloud.pcd",
                                             options.source_voxel_size),
                                options.max_source_points);
    if (scan.points.empty()) continue;

    std::vector<TimedPoint> timed;
    const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
    for (size_t j = 0; j < scan.points.size(); j++) {
      TimedPoint tp;
      tp.raw_point = scan.points[j];
      tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                             : static_cast<double>(j) / denom;
      timed.push_back(tp);
    }

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      for (auto& tp : timed) world.push_back(prev.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(world);
      continue;
    }

    TrajectoryFrame init;
    Eigen::Matrix4d T_seed_world = T_prev_world;
    if (gt_seed && i < gt.size()) {
      // Seed begin from previous GT end (i-1) and end from current GT (i).
      const Eigen::Matrix4d& T_begin =
          (i - 1 < gt.size()) ? gt[i - 1] : gt[i];
      const Eigen::Matrix4d& T_end = gt[i];
      init.begin_pose.trans = T_begin.block<3, 1>(0, 3);
      init.begin_pose.quat = Eigen::Quaterniond(T_begin.block<3, 3>(0, 0));
      init.end_pose.trans = T_end.block<3, 1>(0, 3);
      init.end_pose.quat = Eigen::Quaterniond(T_end.block<3, 3>(0, 0));
      T_seed_world = T_end;
    } else if (options.refinement_gate && i >= 2) {
      Eigen::Matrix4d T_pred;
      if (options.native_ct_icp_seed) {
        // Extrapolate from CT-ICP's own intra-scan motion: the body delta
        // estimated during the previous scan (begin_pose -> end_pose) is
        // re-applied for one more scan period. This adapts to the actual
        // per-frame motion rather than a constant inter-frame velocity.
        const Eigen::Matrix4d T_prev_begin = pose_to_matrix(prev.begin_pose);
        const Eigen::Matrix4d T_prev_end = pose_to_matrix(prev.end_pose);
        const Eigen::Matrix4d T_motion = T_prev_begin.inverse() * T_prev_end;
        T_pred = T_prev_end * T_motion;
      } else {
        // Velocity-model bootstrap: extend prev inter-frame body-motion once more.
        T_pred = velocityModelPrediction(T_prev_world, T_prev_prev_world);
      }
      init.begin_pose = prev.end_pose;
      init.end_pose.trans = T_pred.block<3, 1>(0, 3);
      init.end_pose.quat = Eigen::Quaterniond(T_pred.block<3, 3>(0, 0));
      T_seed_world = T_pred;
    } else {
      init.begin_pose = init.end_pose = prev.end_pose;
      T_seed_world = T_prev_world;
    }
    auto result = reg.registerFrame(timed, init, &prev);

    Eigen::Matrix4d T_refined = frame_to_matrix(result.frame);
    bool accepted = true;
    if (options.refinement_gate && !gt_seed && i >= 2) {
      // Reject if the refined end pose drifts unreasonably far from the
      // velocity-model seed (e.g. correspondence collapse at sharp turns).
      if (!isReasonableRefinement(T_refined, T_seed_world,
                                  options.max_seed_translation_delta,
                                  options.max_seed_rotation_delta_rad)) {
        accepted = false;
        ++gated_rollbacks;
        // Fall back to the velocity-model seed and rebuild a TrajectoryFrame
        // around it so the next frame's bootstrap stays stable.
        TrajectoryFrame fallback;
        fallback.begin_pose = prev.end_pose;
        fallback.end_pose.trans = T_seed_world.block<3, 1>(0, 3);
        fallback.end_pose.quat =
            Eigen::Quaterniond(T_seed_world.block<3, 3>(0, 0));
        result.frame = fallback;
        T_refined = T_seed_world;
      }
    }

    // Skip map update on rollback: deskewing along an untrusted trajectory
    // pollutes the voxel map and breaks subsequent correspondence search.
    if (accepted) {
      std::vector<Eigen::Vector3d> world;
      for (auto& tp : timed)
        world.push_back(result.frame.transformPoint(tp.raw_point, tp.timestamp));
      reg.addPointsToMap(world);
    }

    res.poses.push_back(T_refined);
    prev = result.frame;
    T_prev_prev_world = T_prev_world;
    T_prev_world = T_refined;

    if (i % 10 == 0)
      std::cerr << "\r  [CT-ICP] " << i << "/" << pcd_dirs.size();
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  if (gt_seed) {
    res.note = "Seeds CT-ICP TrajectoryFrame with GT begin/end pose per scan. "
               "Symmetric of the GT-seeded Policy A path for dogfooding-style "
               "fair-prior comparison.";
  } else if (options.refinement_gate) {
    std::ostringstream oss;
    oss << "Anchor matches first GT pose; subsequent frames use "
        << (options.native_ct_icp_seed ? "CT-ICP native (begin->end) "
                                        : "velocity-model ")
        << "bootstrap with LiTAMIN2-style acceptance gate (trans<="
        << options.max_seed_translation_delta
        << "m, rot<=" << options.max_seed_rotation_delta_rad
        << "rad). " << gated_rollbacks << " rollbacks to seed.";
    res.note = oss.str();
  } else {
    res.note = "Anchor matches first GT pose; subsequent frames rely on CT-ICP's "
               "own continuous-time motion prior (no GT seed).";
  }
  return res;
}

MethodResult runCTICPNDT(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const CTICPDogfoodingOptions& ct_icp_options,
                         const NDTDogfoodingOptions& ndt_options) {
  MethodResult prior = runCTICP(pcd_dirs, gt, ct_icp_options, false);
  const double prior_ate = computeATE(prior.poses, gt);

  std::ostringstream note;
  note << "Hybrid: CT-ICP no-GT odometry prior feeds NDT scan-to-map initial "
          "guess; first frame remains anchored to GT for dogfooding alignment. "
       << "CT-ICP prior ATE=";
  if (std::isfinite(prior_ate)) {
    note << std::fixed << std::setprecision(3) << prior_ate << "m";
  } else {
    note << "nonfinite";
  }
  note << ". Runtime is CT-ICP+NDT combined.";

  MethodResult hybrid =
      runNDT(pcd_dirs, gt, ndt_options, false, &prior.poses, note.str());
  hybrid.name = "CT-ICP+NDT";
  hybrid.time_ms += prior.time_ms;
  return hybrid;
}

MethodResult runCTICPNDTKeyframe(
    const std::vector<std::string>& pcd_dirs,
    const std::vector<Eigen::Matrix4d>& gt,
    const CTICPDogfoodingOptions& ct_icp_options,
    const NDTDogfoodingOptions& ndt_options,
    const CTICPNDTHybridOptions& hybrid_options) {
  using namespace localization_zoo::ndt;

  MethodResult prior = runCTICP(pcd_dirs, gt, ct_icp_options, false);
  const double prior_ate = computeATE(prior.poses, gt);

  MethodResult res;
  res.name = "CT-ICP+NDT-KF";

  NDTParams params;
  params.resolution = ndt_options.resolution;
  params.max_iterations = ndt_options.max_iterations;
  params.step_size = ndt_options.step_size;
  params.convergence_threshold = ndt_options.convergence_threshold;
  params.min_points_per_cell = ndt_options.min_points_per_cell;
  NDTRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt.empty() ? Eigen::Matrix4d::Identity() : gt[0];
  res.poses.push_back(T_est);

  size_t attempted = 0;
  size_t accepted = 0;
  size_t rejected = 0;
  size_t fallback_prior = 0;

  const int interval = std::max(1, hybrid_options.keyframe_interval);
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         ndt_options.source_voxel_size),
                                 ndt_options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, ndt_options.map_max_points,
                     ndt_options.map_radius);
      reg.setTarget(map_points);
      continue;
    }

    Eigen::Matrix4d T_prior = T_est;
    if (i < prior.poses.size() && isFiniteMatrix(prior.poses[i])) {
      T_prior = prior.poses[i];
    } else {
      ++fallback_prior;
    }

    T_est = T_prior;
    const bool should_correct = (i % static_cast<size_t>(interval)) == 0;
    if (should_correct) {
      ++attempted;
      const auto result = reg.align(pts_local, T_prior);
      bool score_ok = true;
      if (hybrid_options.max_ndt_score > 0.0) {
        score_ok = std::isfinite(result.score) &&
                   result.score <= hybrid_options.max_ndt_score;
      }
      const bool accepted_correction =
          (result.converged || result.iterations >= 2) &&
          score_ok &&
          isFiniteMatrix(result.transformation) &&
          isReasonableRefinement(
              result.transformation, T_prior,
              hybrid_options.max_correction_translation_delta,
              hybrid_options.max_correction_rotation_delta_rad);
      if (accepted_correction) {
        T_est = result.transformation;
        ++accepted;
      } else {
        ++rejected;
      }
      if (i % 10 == 0) {
        std::cerr << "\r  [CT-ICP+NDT-KF] " << i << "/" << pcd_dirs.size()
                  << " score=" << std::fixed << std::setprecision(3)
                  << result.score
                  << " accepted=" << accepted << "/" << attempted;
      }
    } else if (i % 10 == 0) {
      std::cerr << "\r  [CT-ICP+NDT-KF] " << i << "/" << pcd_dirs.size()
                << " accepted=" << accepted << "/" << attempted;
    }

    res.poses.push_back(T_est);
    addPointsToMap(map_points, pts_local, T_est, ndt_options.map_max_points,
                   ndt_options.map_radius);
    if (shouldRefreshTargetMap(i, ndt_options.refresh_interval)) {
      reg.setTarget(map_points);
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count() +
      prior.time_ms;

  std::ostringstream note;
  note << "Hybrid keyframe correction: CT-ICP no-GT odometry is the base "
          "trajectory; NDT T1 correction is attempted every "
       << interval << " frames and accepted only when correction delta is <= "
       << hybrid_options.max_correction_translation_delta << "m / "
       << hybrid_options.max_correction_rotation_delta_rad << "rad";
  if (hybrid_options.max_ndt_score > 0.0) {
    note << " and score <= " << hybrid_options.max_ndt_score;
  }
  note << ". accepted=" << accepted << "/" << attempted
       << ", rejected=" << rejected
       << ", finite-prior fallbacks=" << fallback_prior
       << ", CT-ICP prior ATE=";
  if (std::isfinite(prior_ate)) {
    note << std::fixed << std::setprecision(3) << prior_ate << "m";
  } else {
    note << "nonfinite";
  }
  note << ". Runtime is CT-ICP+NDT combined.";
  res.note = note.str();
  return res;
}

MethodResult runCTLIO(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      bool estimate_imu_bias,
                      int fixed_lag_state_window,
                      double fixed_lag_velocity_prior_weight,
                      double fixed_lag_gyro_bias_weight_scale,
                      double fixed_lag_accel_bias_weight_scale,
                      double fixed_lag_history_decay,
                      int fixed_lag_outer_iterations,
                      bool fixed_lag_optimize_history,
                      bool multi_scale_correspondences,
                      bool coarse_to_fine,
                      int coarse_iterations,
                      int coarse_search_radius,
                      double coarse_planarity_threshold,
                      double coarse_cauchy_mult,
                      int max_frames_in_map,
                      int max_iterations,
                      bool use_bspline_trajectory,
                      double bspline_anchor_weight) {
  using namespace localization_zoo::ct_lio;
  MethodResult res;
  res.name = "CT-LIO";
  if (multi_scale_correspondences) {
    res.name += "+ms";
  }
  if (coarse_to_fine) {
    res.name += "+c2f";
  }
  if (use_bspline_trajectory) {
    res.name += "+bspline";
  }
  if (estimate_imu_bias) {
    res.name += "+bias";
  }
  if (fixed_lag_state_window > 1) {
    res.name += "+lag";
  }
  if (fixed_lag_optimize_history) {
    res.name += "+smooth";
  }

  CTLIOParams params;
  params.voxel_resolution = 1.0;
  params.max_iterations = max_iterations;
  params.ceres_max_iterations = 2;
  params.planarity_threshold = 0.1;
  params.keypoint_voxel_size = 1.0;
  params.max_frames_in_map = max_frames_in_map;
  params.multi_scale_correspondences = multi_scale_correspondences;
  params.coarse_to_fine = coarse_to_fine;
  params.coarse_iterations = coarse_iterations;
  params.coarse_search_radius = coarse_search_radius;
  params.coarse_planarity_threshold = coarse_planarity_threshold;
  params.coarse_cauchy_sigma_mult = coarse_cauchy_mult;
  params.use_bspline_trajectory = use_bspline_trajectory;
  params.bspline_anchor_weight = bspline_anchor_weight;
  params.estimate_imu_bias = estimate_imu_bias;
  if (fixed_lag_state_window > 1) {
    params.fixed_lag_state_window = fixed_lag_state_window;
    params.fixed_lag_velocity_prior_weight = fixed_lag_velocity_prior_weight;
    params.fixed_lag_gyro_bias_weight_scale = fixed_lag_gyro_bias_weight_scale;
    params.fixed_lag_accel_bias_weight_scale = fixed_lag_accel_bias_weight_scale;
    params.fixed_lag_history_decay = fixed_lag_history_decay;
    params.fixed_lag_outer_iterations = fixed_lag_outer_iterations;
    params.fixed_lag_optimize_history = fixed_lag_optimize_history;
  }
  CTLIORegistration reg(params);

  CTLIOState state;
  state.frame.begin_pose.trans = gt[0].block<3, 1>(0, 3);
  state.frame.begin_pose.quat = Eigen::Quaterniond(gt[0].block<3, 3>(0, 0));
  state.frame.end_pose = state.frame.begin_pose;
  res.poses.push_back(gt[0]);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto scan = limitLoadedScan(loadTimedPCD(pcd_dirs[i] + "/cloud.pcd", 1.0), 1200);
    if (scan.points.empty()) continue;

    std::vector<localization_zoo::ct_icp::TimedPoint> timed;
    timed.reserve(scan.points.size());
    const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
    for (size_t j = 0; j < scan.points.size(); j++) {
      localization_zoo::ct_icp::TimedPoint tp;
      tp.raw_point = scan.points[j];
      tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                             : static_cast<double>(j) / denom;
      timed.push_back(tp);
    }

    if (i == 0) {
      std::vector<Eigen::Vector3d> world;
      world.reserve(timed.size());
      for (const auto& tp : timed) {
        world.push_back(state.frame.transformPoint(tp.raw_point, tp.timestamp));
      }
      reg.addPointsToMap(world);
      continue;
    }

    double dt = std::max(frame_timestamps[i] - frame_timestamps[i - 1], 1e-3);
    Eigen::Vector3d gt_velocity =
        (gt[i].block<3, 1>(0, 3) - gt[i - 1].block<3, 1>(0, 3)) / dt;
    CTLIOState init = state;
    init.frame.begin_pose = state.frame.end_pose;
    init.frame.end_pose.trans = gt[i].block<3, 1>(0, 3);
    init.frame.end_pose.quat = Eigen::Quaterniond(gt[i].block<3, 3>(0, 0));
    init.begin_velocity = gt_velocity;
    init.end_velocity = gt_velocity;

    auto imu_window =
        selectImuWindow(imu_samples, frame_timestamps[i - 1], frame_timestamps[i]);
    auto result = reg.registerFrame(timed, init, imu_window);

    std::vector<Eigen::Vector3d> world;
    world.reserve(timed.size());
    for (const auto& tp : timed) {
      world.push_back(
          result.state.frame.transformPoint(tp.raw_point, tp.timestamp));
    }
    reg.addPointsToMap(world);

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = result.state.frame.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = result.state.frame.end_pose.trans;
    res.poses.push_back(T);
    state = result.state;

    if (i % 1 == 0) {
      std::cerr << "\r  [CT-LIO] " << i << "/" << pcd_dirs.size()
                << " corr=" << result.num_correspondences;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  return res;
}

MethodResult runXICP(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const XICPDogfoodingOptions& options,
                     bool no_gt_seed = false) {
  using namespace localization_zoo::xicp;
  MethodResult res;
  res.name = "X-ICP";

  XICPParams params;
  params.max_iterations = options.max_iterations;
  params.convergence_threshold = options.convergence_threshold;
  params.kappa_f = options.kappa_f;
  params.kappa_s = options.kappa_s;
  XICPRegistration reg(params);

  std::vector<Eigen::Vector3d> map_points;
  Eigen::Matrix4d T_est = gt[0];
  Eigen::Matrix4d T_prev_prev_est = gt[0];
  res.poses.push_back(T_est);

  // KdTree for NN search in map
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
  pcl::PointCloud<pcl::PointXYZ>::Ptr map_cloud(new pcl::PointCloud<pcl::PointXYZ>);

  auto rebuildKdTree = [&]() {
    map_cloud->clear();
    map_cloud->reserve(map_points.size());
    for (const auto& p : map_points) {
      pcl::PointXYZ pt;
      pt.x = static_cast<float>(p.x());
      pt.y = static_cast<float>(p.y());
      pt.z = static_cast<float>(p.z());
      map_cloud->push_back(pt);
    }
    if (!map_cloud->empty()) {
      kdtree.setInputCloud(map_cloud);
    }
  };

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts_local = limitPoints(loadPCD(pcd_dirs[i] + "/cloud.pcd",
                                         options.source_voxel_size),
                                 options.max_source_points);
    if (pts_local.empty()) continue;

    if (i == 0) {
      addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                     options.map_radius);
      rebuildKdTree();
      continue;
    }

    const Eigen::Matrix4d T_init_guess =
        no_gt_seed
            ? (i >= 2 ? velocityModelPrediction(T_est, T_prev_prev_est) : T_est)
            : gt[i];
    const Eigen::Matrix4d T_prev_est_snapshot = T_est;

    // Transform source points to world frame using initial guess
    Eigen::Matrix3d R_init = T_init_guess.block<3, 3>(0, 0);
    Eigen::Vector3d t_init = T_init_guess.block<3, 1>(0, 3);

    // Build correspondences: for each source point, find nearest map point
    // and compute normal from neighborhood
    std::vector<Correspondence> correspondences;
    if (!map_cloud->empty()) {
      const int K = options.k_neighbors;
      std::vector<int> nn_indices(K);
      std::vector<float> nn_dists(K);
      const double max_dist_sq =
          options.max_correspondence_distance * options.max_correspondence_distance;

      for (const auto& p_local : pts_local) {
        Eigen::Vector3d p_world = R_init * p_local + t_init;
        pcl::PointXYZ query;
        query.x = static_cast<float>(p_world.x());
        query.y = static_cast<float>(p_world.y());
        query.z = static_cast<float>(p_world.z());

        if (kdtree.nearestKSearch(query, K, nn_indices, nn_dists) < 3) continue;
        if (nn_dists[0] > max_dist_sq) continue;

        // Compute normal from neighborhood covariance
        Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
        int valid_nn = std::min(K, static_cast<int>(nn_indices.size()));
        for (int k = 0; k < valid_nn; k++) {
          centroid += map_points[nn_indices[k]];
        }
        centroid /= valid_nn;

        Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();
        for (int k = 0; k < valid_nn; k++) {
          Eigen::Vector3d d = map_points[nn_indices[k]] - centroid;
          cov += d * d.transpose();
        }
        cov /= valid_nn;

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(cov);
        Eigen::Vector3d normal = solver.eigenvectors().col(0);  // smallest eigenvalue

        // Ensure consistent normal orientation (pointing toward sensor)
        if (normal.dot(p_world - map_points[nn_indices[0]]) > 0) {
          normal = -normal;
        }

        Correspondence corr;
        corr.source = p_local;
        corr.target = map_points[nn_indices[0]];
        corr.normal = normal;
        correspondences.push_back(corr);
      }
    }

    bool accepted = false;
    if (correspondences.size() >= 64) {
      const auto result = reg.align(correspondences, T_init_guess);
      if ((result.converged || correspondences.size() >= 128) &&
          isReasonableRefinement(result.transformation, T_init_guess,
                                 options.max_seed_translation_delta,
                                 options.max_seed_rotation_delta_rad)) {
        T_est = result.transformation;
        accepted = true;
      }
    }
    if (!accepted) {
      T_est = T_init_guess;
    }
    T_prev_prev_est = T_prev_est_snapshot;
    res.poses.push_back(T_est);

    addPointsToMap(map_points, pts_local, T_est, options.map_max_points,
                   options.map_radius);
    if (shouldRefreshTargetMap(i, options.refresh_interval)) {
      rebuildKdTree();
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [X-ICP] " << i << "/" << pcd_dirs.size()
                << " corr=" << correspondences.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = no_gt_seed
      ? "Uses velocity-model prediction as scan-to-map initial guess (no GT seed)."
      : "Uses GT-seeded scan-to-map initialization with weak-update fallback "
        "in this dogfooding tool.";
  return res;
}

MethodResult runFastLio2(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const std::vector<double>& frame_timestamps,
                         const std::vector<ImuSampleCsv>& imu_samples,
                         const FastLio2DogfoodingOptions& options) {
  using namespace localization_zoo::fast_lio2;
  MethodResult res;
  res.name = "FAST-LIO2";

  FastLio2Params params;
  params.max_iterations = options.max_iterations;
  params.ceres_max_iterations = options.ceres_max_iterations;
  params.voxel_resolution = options.voxel_resolution;
  params.keypoint_voxel_size = options.keypoint_voxel_size;
  params.knn = options.knn;
  params.max_correspondence_distance = options.max_correspondence_distance;
  params.planarity_threshold = options.planarity_threshold;
  params.max_frames_in_map = options.max_frames_in_map;

  FastLio2 pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    // Filter by range
    std::vector<Eigen::Vector3d> filtered;
    filtered.reserve(pts.size());
    for (const auto& p : pts) {
      double r = p.norm();
      if (r >= options.min_range && r <= options.max_range) {
        filtered.push_back(p);
      }
    }

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(filtered, imu_batch);

    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.state.quat.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.state.trans;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [FAST-LIO2] " << i << "/" << pcd_dirs.size()
                << " imu=" << (result.imu_used ? "y" : "n")
                << " corr=" << result.num_correspondences
                << " map=" << result.map_size;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "LiDAR-inertial odometry with iterated Kalman filter and voxel map "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runHdlGraphSlam(const std::vector<std::string>& pcd_dirs,
                             const std::vector<Eigen::Matrix4d>& gt,
                             const HdlGraphSlamDogfoodingOptions& options) {
  using namespace localization_zoo::hdl_graph_slam;
  MethodResult res;
  res.name = "HDL-Graph-SLAM";

  HdlGraphSlamParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_stride = options.keyframe_stride;
  params.max_keyframes_in_submap = options.max_keyframes_in_submap;
  params.loop_stride = options.loop_stride;
  params.enable_loop_closure = options.enable_loop_closure;
  params.enable_floor_constraint = options.enable_floor_constraint;

  HdlGraphSlam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [HDL-Graph-SLAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " submap=" << result.submap_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "NDT odometry with pose-graph optimization and ScanContext loop closure "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runVgicpSlam(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const VgicpSlamDogfoodingOptions& options) {
  using namespace localization_zoo::vgicp_slam;
  MethodResult res;
  res.name = "VGICP-SLAM";

  VgicpSlamParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.keyframe_stride = options.keyframe_stride;
  params.max_keyframes_in_submap = options.max_keyframes_in_submap;
  params.loop_stride = options.loop_stride;
  params.enable_loop_closure = options.enable_loop_closure;

  VgicpSlam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud =
        toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    res.poses.push_back(anchorRelativePose(world_anchor, result.pose));

    if (i % 10 == 0) {
      std::cerr << "\r  [VGICP-SLAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " submap=" << result.submap_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms =
      std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Voxel-GICP odometry with pose-graph optimization and ScanContext loop closure "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runSuMa(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const SuMaDogfoodingOptions& options) {
  using namespace localization_zoo::suma;
  MethodResult res;
  res.name = "SuMa";
  SuMaParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.surfel_resolution = options.surfel_resolution;
  params.surfel_stride = options.surfel_stride;
  params.max_iterations = options.max_iterations;
  params.max_frames_in_map = options.max_frames_in_map;
  SuMa pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCDXYZI(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitLoadedXYZI(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [SuMa] " << i << "/" << pcd_dirs.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Surfel-based mapping (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runBalm2(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const Balm2DogfoodingOptions& options) {
  using namespace localization_zoo::balm2;
  MethodResult res;
  res.name = "BALM2";
  Balm2Params params;
  params.corner_resolution = options.corner_resolution;
  params.surface_resolution = options.surface_resolution;
  params.corner_stride = options.corner_stride;
  params.surface_stride = options.surface_stride;
  params.window_size = options.window_size;
  params.num_outer_iterations = options.num_outer_iterations;
  params.ceres_max_iterations = options.ceres_max_iterations;
  BALM2 pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [BALM2] " << i << "/" << pcd_dirs.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "Bundle adjustment for lidar mapping (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runIscLoam(const std::vector<std::string>& pcd_dirs,
                        const std::vector<Eigen::Matrix4d>& gt,
                        const IscLoamDogfoodingOptions& options) {
  using namespace localization_zoo::isc_loam;
  MethodResult res;
  res.name = "ISC-LOAM";
  IscLoamParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.keyframe_stride = options.keyframe_stride;
  params.loop_stride = options.loop_stride;
  params.enable_loop_closure = options.enable_loop_closure;
  IscLoam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [ISC-LOAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges;
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Intensity scan context + LOAM with loop closure using preserved PCD "
      "intensity fields (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runLoamLivox(const std::vector<std::string>& pcd_dirs,
                          const std::vector<Eigen::Matrix4d>& gt,
                          const LoamLivoxDogfoodingOptions& options) {
  using namespace localization_zoo::loam_livox;
  MethodResult res;
  res.name = "LOAM-Livox";
  LivoxLOAMParams params;
  params.scan_registration.less_flat_leaf_size = options.less_flat_leaf_size;
  params.odometry.num_optimization_iters = options.odom_outer_iters;
  params.mapping.num_optimization_iters = options.map_outer_iters;
  params.mapping.line_resolution = options.map_line_resolution;
  params.mapping.plane_resolution = options.map_plane_resolution;
  LivoxLOAM pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);
    const auto result = pipeline.process(cloud);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [LOAM-Livox] " << i << "/" << pcd_dirs.size();
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note = "LOAM adapted for Livox LiDAR (no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runLioSam(const std::vector<std::string>& pcd_dirs,
                       const std::vector<Eigen::Matrix4d>& gt,
                       const std::vector<double>& frame_timestamps,
                       const std::vector<ImuSampleCsv>& imu_samples,
                       const LioSamDogfoodingOptions& options) {
  using namespace localization_zoo::lio_sam;
  MethodResult res;
  res.name = "LIO-SAM";
  LioSamParams params;
  params.keyframe_stride = options.keyframe_stride;
  params.enable_loop_closure = options.enable_loop_closure;
  params.enable_imu_rotation_prior = options.enable_imu_rotation_prior;
  LioSam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(cloud, imu_batch);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [LIO-SAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " imu=" << (result.imu_used ? "y" : "n");
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Tightly-coupled LiDAR-inertial odometry with factor graph and loop closure "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runLINS(const std::vector<std::string>& pcd_dirs,
                     const std::vector<Eigen::Matrix4d>& gt,
                     const std::vector<double>& frame_timestamps,
                     const std::vector<ImuSampleCsv>& imu_samples,
                     const LINSDogfoodingOptions& options) {
  using namespace localization_zoo::lins;
  MethodResult res;
  res.name = "LINS";
  LINSParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.max_correspondences = options.max_correspondences;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  LINS pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;
    const pcl::PointCloud<pcl::PointXYZI>::ConstPtr cloud = toPclXYZICloud(pts);

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(cloud, imu_batch);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.state.rotation;
    pose.block<3, 1>(0, 3) = result.state.position;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [LINS] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " corr=" << result.num_correspondences
                << " map=" << result.map_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "LiDAR-inertial navigation with error-state iterated Kalman filter "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runFastLioSlam(const std::vector<std::string>& pcd_dirs,
                            const std::vector<Eigen::Matrix4d>& gt,
                            const std::vector<double>& frame_timestamps,
                            const std::vector<ImuSampleCsv>& imu_samples,
                            const FastLioSlamDogfoodingOptions& options) {
  using namespace localization_zoo::fast_lio_slam;
  MethodResult res;
  res.name = "FAST-LIO-SLAM";
  FastLioSlamParams params;
  params.front_end.max_iterations = options.max_iterations;
  params.front_end.ceres_max_iterations = options.ceres_max_iterations;
  params.front_end.voxel_resolution = options.voxel_resolution;
  params.front_end.keypoint_voxel_size = options.keypoint_voxel_size;
  params.front_end.max_frames_in_map = options.max_frames_in_map;
  params.keyframe_stride = options.keyframe_stride;
  params.loop_stride = options.loop_stride;
  params.enable_loop_closure = options.enable_loop_closure;
  FastLioSlam pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    std::vector<Eigen::Vector3d> filtered;
    filtered.reserve(pts.size());
    for (const auto& p : pts) {
      double r = p.norm();
      if (r >= options.min_range && r <= options.max_range) {
        filtered.push_back(p);
      }
    }

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(filtered, imu_batch);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.q_w_curr.toRotationMatrix();
    pose.block<3, 1>(0, 3) = result.t_w_curr;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [FAST-LIO-SLAM] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " loops=" << result.num_loop_edges
                << " imu=" << (result.imu_used ? "y" : "n");
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "FAST-LIO2 front-end with ScanContext loop closure and pose-graph optimization "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runPointLio(const std::vector<std::string>& pcd_dirs,
                         const std::vector<Eigen::Matrix4d>& gt,
                         const std::vector<double>& frame_timestamps,
                         const std::vector<ImuSampleCsv>& imu_samples,
                         const PointLioDogfoodingOptions& options) {
  using namespace localization_zoo::point_lio;
  MethodResult res;
  res.name = "Point-LIO";
  PointLioParams params;
  params.min_range = options.min_range;
  params.max_range = options.max_range;
  params.registration_voxel_size = options.registration_voxel_size;
  params.map_voxel_size = options.map_voxel_size;
  params.max_correspondences = options.max_correspondences;
  params.max_keyframes_in_map = options.max_keyframes_in_map;
  PointLio pipeline(params);
  const Eigen::Matrix4d world_anchor =
      gt.empty() ? Eigen::Matrix4d::Identity() : gt.front();
  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto pts = loadPCD(pcd_dirs[i] + "/cloud.pcd", options.input_voxel_size);
    if (pts.empty()) continue;
    pts = limitPoints(pts, options.max_input_points);
    if (pts.empty()) continue;

    std::vector<Eigen::Vector3d> filtered;
    filtered.reserve(pts.size());
    for (const auto& p : pts) {
      double r = p.norm();
      if (r >= options.min_range && r <= options.max_range) {
        filtered.push_back(p);
      }
    }

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(filtered, imu_batch);
    Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
    pose.block<3, 3>(0, 0) = result.state.rotation;
    pose.block<3, 1>(0, 3) = result.state.position;
    res.poses.push_back(anchorRelativePose(world_anchor, pose));
    if (i % 10 == 0) {
      std::cerr << "\r  [Point-LIO] " << i << "/" << pcd_dirs.size()
                << " kf=" << result.num_keyframes
                << " corr=" << result.num_correspondences
                << " imu=" << (result.imu_used ? "y" : "n")
                << " map=" << result.map_points;
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Point-level LiDAR-inertial odometry with error-state iterated Kalman filter "
      "(no GT seed; anchor matches first GT pose).";
  return res;
}

MethodResult runCLINS(const std::vector<std::string>& pcd_dirs,
                      const std::vector<Eigen::Matrix4d>& gt,
                      const std::vector<double>& frame_timestamps,
                      const std::vector<ImuSampleCsv>& imu_samples,
                      const CLINSDogfoodingOptions& options) {
  using namespace localization_zoo::clins;
  using namespace localization_zoo::ct_lio;
  MethodResult res;
  res.name = "CLINS";

  CLINSParams params;
  CLINS pipeline(params);

  CTLIOState state;
  state.frame.begin_pose.trans = gt[0].block<3, 1>(0, 3);
  state.frame.begin_pose.quat = Eigen::Quaterniond(gt[0].block<3, 3>(0, 0));
  state.frame.end_pose = state.frame.begin_pose;
  res.poses.push_back(gt[0]);

  auto t0 = Clock::now();
  for (size_t i = 0; i < pcd_dirs.size(); i++) {
    auto scan = limitLoadedScan(
        loadTimedPCD(pcd_dirs[i] + "/cloud.pcd", options.source_voxel_size),
        options.max_source_points);
    if (scan.points.empty()) continue;

    std::vector<localization_zoo::ct_icp::TimedPoint> timed;
    timed.reserve(scan.points.size());
    const size_t denom = scan.points.size() > 1 ? scan.points.size() - 1 : 1;
    for (size_t j = 0; j < scan.points.size(); j++) {
      localization_zoo::ct_icp::TimedPoint tp;
      tp.raw_point = scan.points[j];
      tp.timestamp = scan.has_per_point_time ? scan.relative_times[j]
                                             : static_cast<double>(j) / denom;
      timed.push_back(tp);
    }

    std::vector<localization_zoo::imu_preintegration::ImuSample> imu_batch;
    if (i > 0 && !imu_samples.empty() && frame_timestamps.size() == pcd_dirs.size() &&
        i < frame_timestamps.size()) {
      imu_batch = selectImuWindow(imu_samples, frame_timestamps[i - 1],
                                  frame_timestamps[i]);
    }

    const auto result = pipeline.process(timed, imu_batch);

    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3, 3>(0, 0) = result.state.frame.end_pose.quat.toRotationMatrix();
    T.block<3, 1>(0, 3) = result.state.frame.end_pose.trans;
    if (i > 0) {
      res.poses.push_back(T);
    }

    if (i % 10 == 0) {
      std::cerr << "\r  [CLINS] " << i << "/" << pcd_dirs.size()
                << " map=" << result.map_size;
    }
  }
  std::cerr << std::endl;
  res.time_ms = std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
  res.note =
      "Continuous-time LiDAR-inertial SLAM with CT-LIO registration "
      "(GT-seeded initialization).";
  return res;
}

void savePosesKITTI(const std::vector<Eigen::Matrix4d>& poses, const std::string& path) {
  std::ofstream f(path);
  for (auto& T : poses) {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 4; j++)
        f << std::setprecision(9) << T(i, j) << (i == 2 && j == 3 ? "\n" : " ");
  }
}

void writeJsonNumberOrNull(std::ostream& out, double value) {
  if (std::isfinite(value)) {
    out << std::fixed << std::setprecision(6) << value;
  } else {
    out << "null";
  }
}

void writeSummaryJson(const std::string& path,
                      const std::string& pcd_dir,
                      const std::string& gt_csv,
                      size_t num_frames,
                      double trajectory_length_m,
                      FrameTimestampSource frame_timestamp_source,
                      const std::vector<MethodResult>& results) {
  std::ofstream out(path);
  out << "{\n";
  out << "  \"pcd_dir\": \"" << jsonEscape(pcd_dir) << "\",\n";
  out << "  \"gt_csv\": \"" << jsonEscape(gt_csv) << "\",\n";
  out << "  \"num_frames\": " << num_frames << ",\n";
  out << "  \"trajectory_length_m\": " << std::fixed << std::setprecision(6)
      << trajectory_length_m << ",\n";
  out << "  \"timestamp_source\": \""
      << jsonEscape(frameTimestampSourceName(frame_timestamp_source)) << "\",\n";
  out << "  \"methods\": [\n";
  for (size_t i = 0; i < results.size(); i++) {
    const auto& r = results[i];
    out << "    {\n";
    out << "      \"name\": \"" << jsonEscape(r.name) << "\",\n";
    out << "      \"status\": \"" << jsonEscape(r.status) << "\",\n";
    if (r.skipped) {
      out << "      \"ate_m\": null,\n";
      out << "      \"rpe_trans_pct\": null,\n";
      out << "      \"rpe_rot_deg_per_m\": null,\n";
      out << "      \"frames\": 0,\n";
      out << "      \"time_ms\": null,\n";
      out << "      \"fps\": null,\n";
      out << "      \"seed_fallback_rate_pct\": null,\n";
    } else {
      const double fps =
          r.time_ms > 0.0 ? r.poses.size() / (r.time_ms / 1000.0) : 0.0;
      out << "      \"ate_m\": ";
      writeJsonNumberOrNull(out, r.ate);
      out << ",\n";
      if (r.has_rpe) {
        out << "      \"rpe_trans_pct\": ";
        writeJsonNumberOrNull(out, r.rpe_trans_pct);
        out << ",\n";
        out << "      \"rpe_rot_deg_per_m\": ";
        writeJsonNumberOrNull(out, r.rpe_rot_deg_per_m);
        out << ",\n";
      } else {
        out << "      \"rpe_trans_pct\": null,\n";
        out << "      \"rpe_rot_deg_per_m\": null,\n";
      }
      out << "      \"frames\": " << r.poses.size() << ",\n";
      out << "      \"time_ms\": ";
      writeJsonNumberOrNull(out, r.time_ms);
      out << ",\n";
      out << "      \"fps\": ";
      writeJsonNumberOrNull(out, fps);
      out << ",\n";
      out << "      \"seed_fallback_rate_pct\": ";
      if (r.seed_fallback_rate_pct >= 0.0) {
        writeJsonNumberOrNull(out, r.seed_fallback_rate_pct);
      } else {
        out << "null";
      }
      out << ",\n";
    }
    out << "      \"note\": \"" << jsonEscape(r.note) << "\"\n";
    out << "    }" << (i + 1 == results.size() ? "\n" : ",\n");
  }
  out << "  ]\n";
  out << "}\n";
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <pcd_dir> <gt_csv> [max_frames] [--force-ct-lio]"
              << " [--methods litamin2,gicp,small_gicp,voxel_gicp,ndt,kiss_icp,genz_icp,adaptive_icp,d2lio,ct_voxelmap,cube_lio,r_voxelmap,degen_sense,vibration_lio,id_lio,rf_lio,bievr_lio,ua_lio,damm_loam,lodestar,terrain_rbf_lio,lidar_iba,dali_slam,intensity_flow,svn_icp,pcr_dat,small_mighty,m_gclo,quadric_lo,dilo,nhc_lio,student_t_lo,spectral_lo,gmm_lo,gnc_lo,mcc_lo,imls_slam,mesh_loam,elo,tc_lvgf,opl_lvio,v_loam15,tc_vlo,ad_vlo,tc_mvlo,tricp_lo,kc_lo,i_loam,pl_loam,inten_loam,mcgicp,icpsc,vlom,odonet,nhc_net,nn_zupt,dlo,dlio,aloam,floam,"
              << "lego_loam,mulls,ct_lio,ct_icp,ct_icp_ndt,ct_icp_ndt_keyframe,fixed_map_ndt,suma,balm2,isc_loam,loam_livox,lio_sam,lins,"
              << "fast_lio_slam,point_lio,clins]"
              << " [--summary-json path]"
              << " [--litamin2-paper-profile]"
              << " [--litamin2-icp-only]"
              << " [--litamin2-voxel-resolution X]"
              << " [--litamin2-max-iterations N]"
              << " [--litamin2-max-source-points N]"
              << " [--litamin2-num-threads N]"
              << " [--gicp-fast-profile]"
              << " [--gicp-dense-profile]"
              << " [--gicp-k-neighbors N]"
              << " [--gicp-max-iterations N]"
              << " [--aloam-kitti-profile]"
              << " [--aloam-fast-profile]"
              << " [--aloam-dense-profile]"
              << " [--floam-kitti-profile]"
              << " [--floam-fast-profile]"
              << " [--floam-dense-profile]"
              << " [--lego-loam-kitti-profile]"
              << " [--lego-loam-fast-profile]"
              << " [--lego-loam-dense-profile]"
              << " [--mulls-kitti-profile]"
              << " [--mulls-fast-profile]"
              << " [--mulls-dense-profile]"
              << " [--dlo-kitti-profile]"
              << " [--dlo-fast-profile]"
              << " [--dlo-dense-profile]"
              << " [--dlio-kitti-profile]"
              << " [--dlio-fast-profile]"
              << " [--dlio-dense-profile]"
              << " [--small-gicp-fast-profile]"
              << " [--small-gicp-dense-profile]"
              << " [--voxel-gicp-fast-profile]"
              << " [--voxel-gicp-dense-profile]"
              << " [--voxel-gicp-voxel-resolution X]"
              << " [--voxel-gicp-min-points-per-voxel N]"
              << " [--voxel-gicp-max-correspondence-distance X]"
              << " [--voxel-gicp-max-iterations N]"
              << " [--small-gicp-voxel-resolution X]"
              << " [--small-gicp-k-neighbors N]"
              << " [--small-gicp-max-correspondence-distance X]"
              << " [--small-gicp-max-correspondences N]"
              << " [--small-gicp-max-iterations N]"
              << " [--small-gicp-max-seed-translation-delta M]"
              << " [--small-gicp-max-seed-rotation-delta-rad R]"
              << " [--ndt-fast-profile]"
              << " [--ndt-dense-profile]"
              << " [--ndt-resolution X]"
              << " [--ndt-max-iterations N]"
              << " [--fixed-map-ndt-seed-source gt|velocity|ct_icp|scan_context]"
              << " [--fixed-map-ndt-map-stride N]"
              << " [--fixed-map-ndt-scan-context-threshold X]"
              << " [--fixed-map-ndt-scan-context-top-k N]"
              << " [--fixed-map-ndt-trace-json path]"
              << " [--fixed-map-ndt-publish-min-stable-frames N]"
              << " [--fixed-map-ndt-publish-max-hold-frames N]"
              << " [--fixed-map-ndt-scan-context-relock-min-confirmations N]"
              << " [--fixed-map-ndt-scan-context-relock-max-distance X]"
              << " [--fixed-map-ndt-scan-context-relock-max-ndt-score X]"
              << " [--fixed-map-ndt-scan-context-relock-max-score-delta X]"
              << " [--kiss-fast-profile]"
              << " [--kiss-dense-profile]"
              << " [--kiss-voxel-size X]"
              << " [--kiss-max-iterations N]"
              << " [--ct-icp-fast-profile]"
              << " [--ct-icp-dense-profile]"
              << " [--ct-icp-voxel-resolution X]"
              << " [--ct-icp-max-iterations N]"
              << " [--ct-icp-ndt-keyframe-interval N]"
              << " [--ct-icp-ndt-max-correction-translation M]"
              << " [--ct-icp-ndt-max-correction-rotation-rad R]"
              << " [--ct-icp-ndt-max-score S]"
              << " [--xicp-fast-profile]"
              << " [--xicp-dense-profile]"
              << " [--fast-lio2-fast-profile]"
              << " [--fast-lio2-dense-profile]"
              << " [--hdl-graph-slam-fast-profile]"
              << " [--hdl-graph-slam-dense-profile]"
              << " [--vgicp-slam-fast-profile]"
              << " [--vgicp-slam-dense-profile]"
              << " [--suma-fast-profile]"
              << " [--suma-dense-profile]"
              << " [--balm2-fast-profile]"
              << " [--balm2-dense-profile]"
              << " [--isc-loam-fast-profile]"
              << " [--isc-loam-dense-profile]"
              << " [--loam-livox-fast-profile]"
              << " [--loam-livox-dense-profile]"
              << " [--lio-sam-fast-profile]"
              << " [--lio-sam-dense-profile]"
              << " [--lins-fast-profile]"
              << " [--lins-dense-profile]"
               << " [--fast-lio-slam-fast-profile]"
               << " [--fast-lio-slam-dense-profile]"
               << " [--point-lio-fast-profile]"
               << " [--point-lio-dense-profile]"
               << " [--clins-fast-profile]"
               << " [--clins-dense-profile]"
               << " [--ct-lio-estimate-bias]"
               << " [--ct-lio-fixed-lag-window N]"
              << " [--ct-lio-fixed-lag-velocity-weight W]"
              << " [--ct-lio-fixed-lag-gyro-bias-scale W]"
              << " [--ct-lio-fixed-lag-accel-bias-scale W]"
              << " [--ct-lio-fixed-lag-history-decay W]"
              << " [--ct-lio-fixed-lag-outer-iterations N]"
              << " [--ct-lio-fixed-lag-smoother]"
              << " [--seed-perturb-x M] [--seed-perturb-y M]"
              << " [--seed-perturb-z M] [--seed-perturb-yaw-deg DEG]"
              << " [--no-gt-seed] [--ct-icp-gt-seed]" << std::endl;
    return 1;
  }

  std::string pcd_dir = argv[1];
  std::string gt_csv = argv[2];
  int max_frames = -1;
  bool force_ct_lio = false;
  bool no_gt_seed = false;
  bool ct_icp_gt_seed = false;
  SeedPerturbation seed_perturbation;
  bool ct_lio_estimate_bias = false;
  bool ct_lio_multi_scale = false;
  bool ct_lio_coarse_to_fine = false;
  int ct_lio_coarse_iterations = 2;
  int ct_lio_coarse_search_radius = 2;
  double ct_lio_coarse_planarity_threshold = 0.02;
  double ct_lio_coarse_cauchy_mult = 2.0;
  bool ct_lio_bspline = false;
  double ct_lio_bspline_anchor_weight = 500.0;
  int ct_lio_max_frames_in_map = 10;
  int ct_lio_max_iterations = 6;
  std::string summary_json_path;
  int ct_lio_fixed_lag_window = 1;
  double ct_lio_fixed_lag_velocity_weight = 0.0;
  double ct_lio_fixed_lag_gyro_bias_scale = 0.25;
  double ct_lio_fixed_lag_accel_bias_scale = 0.25;
  double ct_lio_fixed_lag_history_decay = 1.0;
  int ct_lio_fixed_lag_outer_iterations = 3;
  bool ct_lio_fixed_lag_smoother = false;
  LiTAMIN2DogfoodingOptions litamin2_options;
  GICPDogfoodingOptions gicp_options;
  SmallGICPDogfoodingOptions small_gicp_options;
  VoxelGICPDogfoodingOptions voxel_gicp_options;
  ALOAMDogfoodingOptions aloam_options;
  FLOAMDogfoodingOptions floam_options;
  ILoamDogfoodingOptions i_loam_options;
  PlLoamDogfoodingOptions pl_loam_options;
  InTenLoamDogfoodingOptions inten_loam_options;
  McGicpLoDogfoodingOptions mcgicp_options;
  IcpscLoDogfoodingOptions icpsc_options;
  VlomDogfoodingOptions vlom_options;
  OdoNetDogfoodingOptions odonet_options;
  NhcNetDogfoodingOptions nhc_net_options;
  NnZuptDogfoodingOptions nn_zupt_options;
  LeGOLOAMDogfoodingOptions lego_loam_options;
  MULLSDogfoodingOptions mulls_options;
  NDTDogfoodingOptions ndt_options;
  FixedMapNDTOptions fixed_map_ndt_options;
  KISSICPDogfoodingOptions kiss_icp_options;
  GenZICPDogfoodingOptions genz_icp_options;
  AdaptiveICPDogfoodingOptions adaptive_icp_options;
  D2LIODogfoodingOptions d2lio_options;
  CTVoxelMapDogfoodingOptions ct_voxelmap_options;
  RVoxelMapDogfoodingOptions r_voxelmap_options;
  DammLoamDogfoodingOptions damm_loam_options;
  LodestarDogfoodingOptions lodestar_options;
  TerrainRbfDogfoodingOptions terrain_rbf_options;
  LidarIbaDogfoodingOptions lidar_iba_options;
  DaliSlamDogfoodingOptions dali_slam_options;
  IntensityFlowDogfoodingOptions intensity_flow_options;
  SvnIcpDogfoodingOptions svn_icp_options;
  PcrDatDogfoodingOptions pcr_dat_options;
  SmallMightyDogfoodingOptions small_mighty_options;
  MGcloDogfoodingOptions m_gclo_options;
  QuadricLoDogfoodingOptions quadric_lo_options;
  DiloDogfoodingOptions dilo_options;
  NhcLioDogfoodingOptions nhc_lio_options;
  StudentTLoDogfoodingOptions student_t_lo_options;
  SpectralLoDogfoodingOptions spectral_lo_options;
  GmmLoDogfoodingOptions gmm_lo_options;
  GncLoDogfoodingOptions gnc_lo_options;
  MccLoDogfoodingOptions mcc_lo_options;
  ImlsSlamDogfoodingOptions imls_slam_options;
  MeshLoamDogfoodingOptions mesh_loam_options;
  EloDogfoodingOptions elo_options;
  TcLvgfDogfoodingOptions tc_lvgf_options;
  OplLvioDogfoodingOptions opl_lvio_options;
  VisualLidarAdapterDogfoodingOptions v_loam15_options;
  VisualLidarAdapterDogfoodingOptions tc_vlo_options;
  VisualLidarAdapterDogfoodingOptions ad_vlo_options;
  VisualLidarAdapterDogfoodingOptions tc_mvlo_options;
  TricpLoDogfoodingOptions tricp_lo_options;
  KcLoDogfoodingOptions kc_lo_options;
  DegenSenseDogfoodingOptions degen_sense_options;
  VibrationLIODogfoodingOptions vibration_lio_options;
  IDLIODogfoodingOptions id_lio_options;
  RFLIODogfoodingOptions rf_lio_options;
  BievrLIODogfoodingOptions bievr_lio_options;
  UALIODogfoodingOptions ua_lio_options;
  CubeLIODogfoodingOptions cube_lio_options;
  RKOLIODogfoodingOptions rko_lio_options;
  FRLIODogfoodingOptions fr_lio_options;
  PGLIODogfoodingOptions pg_lio_options;
  CTICPDogfoodingOptions ct_icp_options;
  CTICPNDTHybridOptions ct_icp_ndt_options;
  DLODofeedingOptions dlo_options;
  DLIODogfoodingOptions dlio_options;
  XICPDogfoodingOptions xicp_options;
  FastLio2DogfoodingOptions fast_lio2_options;
  HdlGraphSlamDogfoodingOptions hdl_graph_slam_options;
  VgicpSlamDogfoodingOptions vgicp_slam_options;
  SuMaDogfoodingOptions suma_options;
  Balm2DogfoodingOptions balm2_options;
  IscLoamDogfoodingOptions isc_loam_options;
  LoamLivoxDogfoodingOptions loam_livox_options;
  LioSamDogfoodingOptions lio_sam_options;
  LINSDogfoodingOptions lins_options;
  FastLioSlamDogfoodingOptions fast_lio_slam_options;
  PointLioDogfoodingOptions point_lio_options;
  CLINSDogfoodingOptions clins_options;
  std::vector<std::string> selected_methods = {
      "litamin2", "gicp", "small_gicp", "aloam", "floam", "ndt", "kiss_icp",
      "ct_lio"};
  for (int i = 3; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--force-ct-lio") {
      force_ct_lio = true;
      continue;
    }
    if (arg == "--no-gt-seed") {
      no_gt_seed = true;
      continue;
    }
    if (arg == "--ct-icp-gt-seed") {
      ct_icp_gt_seed = true;
      continue;
    }
    if (arg == "--seed-perturb-x") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      seed_perturbation.x = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--seed-perturb-x=", 0) == 0) {
      seed_perturbation.x =
          std::stod(arg.substr(std::string("--seed-perturb-x=").size()));
      continue;
    }
    if (arg == "--seed-perturb-y") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      seed_perturbation.y = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--seed-perturb-y=", 0) == 0) {
      seed_perturbation.y =
          std::stod(arg.substr(std::string("--seed-perturb-y=").size()));
      continue;
    }
    if (arg == "--seed-perturb-z") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      seed_perturbation.z = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--seed-perturb-z=", 0) == 0) {
      seed_perturbation.z =
          std::stod(arg.substr(std::string("--seed-perturb-z=").size()));
      continue;
    }
    if (arg == "--seed-perturb-yaw-deg") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      seed_perturbation.yaw_rad = std::stod(argv[++i]) * M_PI / 180.0;
      continue;
    }
    if (arg.rfind("--seed-perturb-yaw-deg=", 0) == 0) {
      seed_perturbation.yaw_rad = std::stod(arg.substr(
          std::string("--seed-perturb-yaw-deg=").size())) * M_PI / 180.0;
      continue;
    }
    if (arg == "--ct-lio-multi-scale") {
      ct_lio_multi_scale = true;
      continue;
    }
    if (arg == "--ct-lio-coarse-to-fine") {
      ct_lio_coarse_to_fine = true;
      continue;
    }
    if (arg == "--ct-lio-coarse-iterations") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_coarse_iterations = std::max(0, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-lio-coarse-search-radius") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_coarse_search_radius = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-lio-coarse-planarity-threshold") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_coarse_planarity_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-lio-coarse-cauchy-mult") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_coarse_cauchy_mult = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-lio-bspline") {
      ct_lio_bspline = true;
      continue;
    }
    if (arg == "--ct-lio-bspline-anchor-weight") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_bspline_anchor_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-lio-max-frames-in-map") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_max_frames_in_map = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-lio-max-iterations") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_lio_max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-lio-estimate-bias") {
      ct_lio_estimate_bias = true;
      continue;
    }
    if (arg == "--summary-json") {
      if (i + 1 >= argc) {
        std::cerr << "--summary-json requires a path" << std::endl;
        return 1;
      }
      summary_json_path = argv[++i];
      continue;
    }
    if (arg.rfind("--summary-json=", 0) == 0) {
      summary_json_path = arg.substr(std::string("--summary-json=").size());
      continue;
    }
    if (arg == "--litamin2-paper-profile") {
      litamin2_options.voxel_resolution = 3.0;
      litamin2_options.max_iterations = 8;
      litamin2_options.use_cov_cost = true;
      litamin2_options.map_max_points = 25000;
      litamin2_options.refresh_interval = 2;
      litamin2_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--litamin2-icp-only") {
      litamin2_options.use_cov_cost = false;
      continue;
    }
    if (arg == "--litamin2-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      litamin2_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--litamin2-voxel-resolution=", 0) == 0) {
      litamin2_options.voxel_resolution = std::stod(
          arg.substr(std::string("--litamin2-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--litamin2-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--litamin2-max-iterations=", 0) == 0) {
      litamin2_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--litamin2-max-iterations=").size())));
      continue;
    }
    if (arg == "--litamin2-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--litamin2-max-source-points=", 0) == 0) {
      litamin2_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--litamin2-max-source-points=").size()))));
      continue;
    }
    if (arg == "--litamin2-num-threads") {
      if (i + 1 >= argc) {
        std::cerr << "--litamin2-num-threads requires an integer value"
                  << std::endl;
        return 1;
      }
      litamin2_options.num_threads = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--litamin2-num-threads=", 0) == 0) {
      litamin2_options.num_threads = std::max(
          1, std::stoi(arg.substr(std::string("--litamin2-num-threads=").size())));
      continue;
    }
    if (arg == "--gicp-fast-profile") {
      gicp_options.source_voxel_size = 1.25;
      gicp_options.max_source_points = 1800;
      gicp_options.k_neighbors = 6;
      gicp_options.max_correspondence_distance = 2.0;
      gicp_options.max_iterations = 6;
      gicp_options.map_max_points = 30000;
      gicp_options.refresh_interval = 6;
      gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--gicp-dense-profile") {
      gicp_options.source_voxel_size = 0.75;
      gicp_options.max_source_points = 3200;
      gicp_options.k_neighbors = 12;
      gicp_options.max_correspondence_distance = 3.0;
      gicp_options.max_iterations = 10;
      gicp_options.map_max_points = 55000;
      gicp_options.refresh_interval = 3;
      gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--gicp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      gicp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-source-voxel-size=", 0) == 0) {
      gicp_options.source_voxel_size =
          std::stod(arg.substr(std::string("--gicp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--gicp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-max-source-points=", 0) == 0) {
      gicp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--gicp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--gicp-k-neighbors") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-k-neighbors requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.k_neighbors = std::max(3, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--gicp-k-neighbors=", 0) == 0) {
      gicp_options.k_neighbors =
          std::max(3, std::stoi(arg.substr(std::string("--gicp-k-neighbors=").size())));
      continue;
    }
    if (arg == "--gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-correspondence-distance requires a numeric value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-max-correspondence-distance=", 0) == 0) {
      gicp_options.max_correspondence_distance = std::stod(
          arg.substr(std::string("--gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--gicp-max-iterations=", 0) == 0) {
      gicp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--gicp-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-map-max-points=", 0) == 0) {
      gicp_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--gicp-map-max-points=").size()))));
      continue;
    }
    if (arg == "--gicp-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      gicp_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--gicp-refresh-interval=", 0) == 0) {
      gicp_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--gicp-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--gicp-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--gicp-map-radius requires a numeric value" << std::endl;
        return 1;
      }
      gicp_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--gicp-map-radius=", 0) == 0) {
      gicp_options.map_radius =
          std::stod(arg.substr(std::string("--gicp-map-radius=").size()));
      continue;
    }
    if (arg == "--aloam-kitti-profile") {
      aloam_options.n_scans = 64;
      aloam_options.scan_period = 0.1f;
      aloam_options.minimum_range = 1.0f;
      aloam_options.curvature_threshold = 0.1f;
      aloam_options.less_flat_leaf_size = 0.2f;
      aloam_options.odom_outer_iters = 2;
      aloam_options.odom_ceres_iters = 4;
      aloam_options.map_outer_iters = 2;
      aloam_options.map_ceres_iters = 4;
      aloam_options.map_line_resolution = 0.4;
      aloam_options.map_plane_resolution = 0.8;
      aloam_options.map_knn = 5;
      aloam_options.map_knn_max_dist_sq = 1.0;
      aloam_options.map_edge_eigenvalue_ratio = 3.0;
      aloam_options.map_plane_threshold = 0.2;
      continue;
    }
    if (arg == "--aloam-fast-profile") {
      aloam_options.curvature_threshold = 0.15f;
      aloam_options.less_flat_leaf_size = 0.35f;
      aloam_options.odom_outer_iters = 1;
      aloam_options.odom_ceres_iters = 3;
      aloam_options.map_outer_iters = 1;
      aloam_options.map_ceres_iters = 3;
      aloam_options.map_line_resolution = 0.6;
      aloam_options.map_plane_resolution = 1.2;
      aloam_options.map_knn = 5;
      aloam_options.map_knn_max_dist_sq = 1.5;
      aloam_options.map_plane_threshold = 0.25;
      continue;
    }
    if (arg == "--aloam-dense-profile") {
      aloam_options.curvature_threshold = 0.08f;
      aloam_options.less_flat_leaf_size = 0.15f;
      aloam_options.odom_outer_iters = 2;
      aloam_options.odom_ceres_iters = 6;
      aloam_options.map_outer_iters = 2;
      aloam_options.map_ceres_iters = 6;
      aloam_options.map_line_resolution = 0.3;
      aloam_options.map_plane_resolution = 0.6;
      aloam_options.map_knn = 7;
      aloam_options.map_knn_max_dist_sq = 1.0;
      aloam_options.map_plane_threshold = 0.18;
      continue;
    }
    if (arg == "--floam-kitti-profile") {
      floam_options.n_scans = 64;
      floam_options.scan_period = 0.1f;
      floam_options.minimum_range = 1.0f;
      floam_options.input_point_stride = 2;
      floam_options.mapping_update_interval = 2;
      floam_options.enable_mapping = true;
      continue;
    }
    if (arg == "--i-loam-no-intensity") {
      // 強度経路を無効化し、純幾何 LOAM ベースラインとして走らせる(ablation)。
      i_loam_options.use_intensity_weight = false;
      i_loam_options.use_intensity_correspondence = false;
      continue;
    }
    if (arg == "--i-loam-no-mapping") {
      i_loam_options.enable_mapping = false;
      continue;
    }
    if (arg == "--i-loam-dense-profile") {
      i_loam_options.curvature_threshold = 0.08f;
      i_loam_options.less_flat_leaf_size = 0.15f;
      i_loam_options.odom_outer_iters = 2;
      i_loam_options.odom_ceres_iters = 6;
      i_loam_options.map_outer_iters = 2;
      i_loam_options.map_ceres_iters = 6;
      i_loam_options.map_line_resolution = 0.3;
      i_loam_options.map_plane_resolution = 0.6;
      i_loam_options.map_knn_max_dist_sq = 0.8;
      i_loam_options.map_plane_threshold = 0.15;
      continue;
    }
    if (arg == "--i-loam-intensity-sigma" && i + 1 < argc) {
      i_loam_options.intensity_sigma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--i-loam-corr-weight" && i + 1 < argc) {
      i_loam_options.intensity_corr_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--i-loam-stride" && i + 1 < argc) {
      i_loam_options.input_point_stride =
          static_cast<size_t>(std::stoul(argv[++i]));
      continue;
    }
    if (arg == "--pl-loam-fast-profile") {
      pl_loam_options.input_stride = 3;
      pl_loam_options.max_point_features = 200;
      pl_loam_options.max_line_features = 48;
      pl_loam_options.ceres_max_iterations = 8;
      continue;
    }
    if (arg == "--pl-loam-dense-profile") {
      pl_loam_options.input_stride = 1;
      pl_loam_options.max_point_features = 320;
      pl_loam_options.max_line_features = 80;
      pl_loam_options.patch_radius = 5;
      pl_loam_options.ceres_max_iterations = 15;
      continue;
    }
    if (arg == "--pl-loam-no-depth-prior") {
      pl_loam_options.use_depth_prior = false;
      continue;
    }
    if (arg == "--pl-loam-no-lines") {
      pl_loam_options.use_line_features = false;
      continue;
    }
    if (arg == "--pl-loam-no-scale") {
      pl_loam_options.use_scale_correction = false;
      continue;
    }
    if (arg == "--pl-loam-stride" && i + 1 < argc) {
      pl_loam_options.input_stride = static_cast<size_t>(std::stoul(argv[++i]));
      continue;
    }
    if (arg == "--pl-loam-rgb-root" && i + 1 < argc) {
      pl_loam_options.rgb_image_root = argv[++i];
      pl_loam_options.use_rgb = true;
      continue;
    }
    if (arg == "--pl-loam-rgb-camera" && i + 1 < argc) {
      pl_loam_options.rgb_camera_subdir = argv[++i];
      continue;
    }
    if (arg == "--pl-loam-rgb-full-res") {
      pl_loam_options.rgb_half_res = false;
      continue;
    }
    if (arg == "--inten-loam-fast-profile") {
      inten_loam_options.input_stride = 3;
      inten_loam_options.cyl_width = 720;
      inten_loam_options.cyl_height = 48;
      inten_loam_options.max_edge_features = 150;
      inten_loam_options.max_surface_features = 300;
      continue;
    }
    if (arg == "--inten-loam-dense-profile") {
      inten_loam_options.input_stride = 1;
      inten_loam_options.cyl_width = 1024;
      inten_loam_options.cyl_height = 64;
      inten_loam_options.max_edge_features = 220;
      inten_loam_options.max_surface_features = 500;
      inten_loam_options.max_reflector_features = 150;
      continue;
    }
    if (arg == "--inten-loam-no-intensity") {
      inten_loam_options.use_intensity_registration = false;
      continue;
    }
    if (arg == "--inten-loam-no-tvf") {
      inten_loam_options.enable_tvf = false;
      continue;
    }
    if (arg == "--inten-loam-no-dor") {
      inten_loam_options.enable_dor = false;
      continue;
    }
    if (arg == "--inten-loam-no-mapping") {
      inten_loam_options.enable_mapping = false;
      continue;
    }
    if (arg == "--inten-loam-stride" && i + 1 < argc) {
      inten_loam_options.input_stride = static_cast<size_t>(std::stoul(argv[++i]));
      continue;
    }
    // --- mcgicp ---
    if (arg == "--mcgicp-fast-profile") {
      mcgicp_options.source_voxel_size = 0.5;
      mcgicp_options.max_source_points = 4000;
      mcgicp_options.voxel_size = 1.0;
      mcgicp_options.max_iterations = 15;
      mcgicp_options.local_map_radius = 45.0;
      mcgicp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--mcgicp-dense-profile") {
      mcgicp_options.source_voxel_size = 0.35;
      mcgicp_options.max_source_points = 6000;
      mcgicp_options.voxel_size = 0.8;
      mcgicp_options.max_iterations = 20;
      mcgicp_options.local_map_radius = 80.0;
      mcgicp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--mcgicp-no-intensity") {
      mcgicp_options.enable_intensity = false;
      continue;
    }
    if (arg == "--mcgicp-intensity-scale" && i + 1 < argc) {
      mcgicp_options.intensity_scale = std::stod(argv[++i]);
      continue;
    }
    // --- icpsc ---
    if (arg == "--icpsc-fast-profile") {
      icpsc_options.source_voxel_size = 0.5;
      icpsc_options.max_source_points = 4000;
      icpsc_options.cyl_width = 720;
      icpsc_options.cyl_height = 48;
      icpsc_options.voxel_size = 1.0;
      icpsc_options.max_iterations = 12;
      icpsc_options.local_map_radius = 45.0;
      icpsc_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--icpsc-dense-profile") {
      icpsc_options.source_voxel_size = 0.35;
      icpsc_options.max_source_points = 6000;
      icpsc_options.cyl_width = 1024;
      icpsc_options.cyl_height = 64;
      icpsc_options.voxel_size = 0.8;
      icpsc_options.max_iterations = 15;
      icpsc_options.local_map_radius = 80.0;
      icpsc_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--icpsc-no-intensity") {
      icpsc_options.enable_intensity = false;
      continue;
    }
    // --- vlom ---
    if (arg == "--vlom-fast-profile") {
      vlom_options.input_point_stride = 3;
      vlom_options.visual_input_stride = 3;
      vlom_options.max_point_features = 200;
      vlom_options.max_line_features = 48;
      vlom_options.scale_correction_interval = 8;
      continue;
    }
    if (arg == "--vlom-dense-profile") {
      vlom_options.input_point_stride = 1;
      vlom_options.visual_input_stride = 1;
      vlom_options.max_point_features = 320;
      vlom_options.max_line_features = 80;
      vlom_options.scale_correction_interval = 5;
      continue;
    }
    if (arg == "--vlom-no-bootstrap") {
      vlom_options.enable_visual_bootstrap = false;
      continue;
    }
    if (arg == "--vlom-no-scale") {
      vlom_options.enable_scale_correction = false;
      continue;
    }
    if (arg == "--vlom-rgb-root" && i + 1 < argc) {
      vlom_options.rgb_image_root = argv[++i];
      vlom_options.use_rgb = true;
      continue;
    }
    if (arg == "--vlom-rgb-camera" && i + 1 < argc) {
      vlom_options.rgb_camera_subdir = argv[++i];
      continue;
    }
    if (arg == "--vlom-rgb-full-res") {
      vlom_options.rgb_half_res = false;
      continue;
    }
    // --- odonet ---
    if (arg == "--odonet-weights" && i + 1 < argc) {
      odonet_options.weights_path = argv[++i];
      continue;
    }
    if (arg == "--odonet-no-nhc") {
      odonet_options.enable_nhc = false;
      continue;
    }
    if (arg == "--odonet-no-zupt") {
      odonet_options.enable_zupt = false;
      continue;
    }
    if (arg == "--odonet-nhc-only") {
      odonet_options.enable_cnn_speed = false;
      continue;
    }
    // --- nhc_net ---
    if (arg == "--nhc-net-weights" && i + 1 < argc) {
      nhc_net_options.weights_path = argv[++i];
      continue;
    }
    if (arg == "--nhc-net-no-zupt") {
      nhc_net_options.enable_zupt = false;
      continue;
    }
    if (arg == "--nhc-net-fixed-gain") {
      nhc_net_options.adaptive_gain = false;
      continue;
    }
    if (arg == "--nhc-net-gain" && i + 1 < argc) {
      nhc_net_options.nhc_gain = std::stod(argv[++i]);
      continue;
    }
    // --- nn_zupt ---
    if (arg == "--nn-zupt-weights" && i + 1 < argc) {
      nn_zupt_options.weights_path = argv[++i];
      continue;
    }
    if (arg == "--nn-zupt-no-zupt") {
      nn_zupt_options.enable_zupt = false;
      continue;
    }
    if (arg == "--nn-zupt-no-nhc") {
      nn_zupt_options.enable_nhc = false;
      continue;
    }
    if (arg == "--nn-zupt-threshold-only") {
      nn_zupt_options.use_threshold_detector = true;
      continue;
    }
    if (arg == "--nn-zupt-prob" && i + 1 < argc) {
      nn_zupt_options.stop_prob_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--floam-fast-profile") {
      floam_options.input_point_stride = 4;
      floam_options.mapping_update_interval = 3;
      floam_options.curvature_threshold = 0.12f;
      floam_options.less_flat_leaf_size = 0.4f;
      floam_options.odom_outer_iters = 1;
      floam_options.odom_ceres_iters = 2;
      floam_options.map_outer_iters = 1;
      floam_options.map_ceres_iters = 2;
      floam_options.map_line_resolution = 0.8;
      floam_options.map_plane_resolution = 1.4;
      floam_options.map_knn_max_dist_sq = 5.0;
      continue;
    }
    if (arg == "--floam-dense-profile") {
      floam_options.input_point_stride = 1;
      floam_options.mapping_update_interval = 1;
      floam_options.curvature_threshold = 0.06f;
      floam_options.less_flat_leaf_size = 0.2f;
      floam_options.odom_outer_iters = 2;
      floam_options.odom_ceres_iters = 4;
      floam_options.map_outer_iters = 2;
      floam_options.map_ceres_iters = 4;
      floam_options.map_line_resolution = 0.5;
      floam_options.map_plane_resolution = 0.9;
      floam_options.map_knn_max_dist_sq = 3.0;
      continue;
    }
    if (arg == "--lego-loam-kitti-profile") {
      lego_loam_options.n_scans = 64;
      lego_loam_options.scan_period = 0.1f;
      lego_loam_options.minimum_range = 1.0f;
      lego_loam_options.maximum_range = 100.0f;
      lego_loam_options.ground_scan_limit = 6;
      lego_loam_options.sensor_height = 1.73f;
      lego_loam_options.ground_height_tolerance = 0.4f;
      lego_loam_options.curvature_threshold = 0.1f;
      lego_loam_options.less_flat_leaf_size = 0.2f;
      lego_loam_options.odom_distance_sq_threshold = 25.0;
      lego_loam_options.odom_outer_iters = 2;
      lego_loam_options.odom_ceres_iters = 4;
      lego_loam_options.map_outer_iters = 2;
      lego_loam_options.map_ceres_iters = 4;
      lego_loam_options.map_line_resolution = 0.4;
      lego_loam_options.map_plane_resolution = 0.8;
      lego_loam_options.map_knn = 5;
      lego_loam_options.map_knn_max_dist_sq = 1.0;
      lego_loam_options.map_edge_eigenvalue_ratio = 3.0;
      lego_loam_options.map_plane_threshold = 0.2;
      continue;
    }
    if (arg == "--lego-loam-fast-profile") {
      lego_loam_options.curvature_threshold = 0.15f;
      lego_loam_options.less_flat_leaf_size = 0.35f;
      lego_loam_options.odom_outer_iters = 1;
      lego_loam_options.odom_ceres_iters = 3;
      lego_loam_options.map_outer_iters = 1;
      lego_loam_options.map_ceres_iters = 3;
      lego_loam_options.map_line_resolution = 0.6;
      lego_loam_options.map_plane_resolution = 1.2;
      lego_loam_options.map_knn_max_dist_sq = 1.5;
      lego_loam_options.map_plane_threshold = 0.25;
      continue;
    }
    if (arg == "--lego-loam-dense-profile") {
      lego_loam_options.curvature_threshold = 0.08f;
      lego_loam_options.less_flat_leaf_size = 0.15f;
      lego_loam_options.odom_outer_iters = 2;
      lego_loam_options.odom_ceres_iters = 6;
      lego_loam_options.map_outer_iters = 2;
      lego_loam_options.map_ceres_iters = 6;
      lego_loam_options.map_line_resolution = 0.3;
      lego_loam_options.map_plane_resolution = 0.6;
      lego_loam_options.map_knn = 7;
      lego_loam_options.map_knn_max_dist_sq = 1.0;
      lego_loam_options.map_plane_threshold = 0.18;
      continue;
    }
    if (arg == "--mulls-kitti-profile") {
      mulls_options.n_scans = 64;
      mulls_options.scan_period = 0.1f;
      mulls_options.minimum_range = 1.0f;
      mulls_options.curvature_threshold = 0.1f;
      mulls_options.less_flat_leaf_size = 0.2f;
      mulls_options.odom_distance_sq_threshold = 25.0;
      mulls_options.odom_outer_iters = 2;
      mulls_options.odom_ceres_iters = 4;
      mulls_options.line_resolution = 0.4;
      mulls_options.plane_resolution = 0.8;
      mulls_options.point_resolution = 1.0;
      mulls_options.mulls_map_iters = 2;
      mulls_options.mulls_ceres_iters = 4;
      mulls_options.mulls_knn = 5;
      mulls_options.mulls_knn_max_dist_sq = 1.0;
      mulls_options.mulls_point_neighbor_max_dist_sq = 4.0;
      mulls_options.mulls_point_weight = 0.3;
      mulls_options.mulls_full_downsample_rate = 5;
      mulls_options.mulls_max_frames_in_map = 30;
      continue;
    }
    if (arg == "--mulls-fast-profile") {
      mulls_options.curvature_threshold = 0.15f;
      mulls_options.less_flat_leaf_size = 0.35f;
      mulls_options.odom_outer_iters = 1;
      mulls_options.odom_ceres_iters = 3;
      mulls_options.line_resolution = 0.55;
      mulls_options.plane_resolution = 1.0;
      mulls_options.point_resolution = 1.2;
      mulls_options.mulls_map_iters = 1;
      mulls_options.mulls_ceres_iters = 3;
      mulls_options.mulls_knn_max_dist_sq = 1.5;
      mulls_options.mulls_point_neighbor_max_dist_sq = 5.0;
      mulls_options.mulls_point_weight = 0.2;
      mulls_options.mulls_full_downsample_rate = 8;
      mulls_options.mulls_max_frames_in_map = 20;
      continue;
    }
    if (arg == "--mulls-dense-profile") {
      mulls_options.curvature_threshold = 0.08f;
      mulls_options.less_flat_leaf_size = 0.15f;
      mulls_options.odom_outer_iters = 2;
      mulls_options.odom_ceres_iters = 6;
      mulls_options.line_resolution = 0.3;
      mulls_options.plane_resolution = 0.6;
      mulls_options.point_resolution = 0.85;
      mulls_options.mulls_map_iters = 2;
      mulls_options.mulls_ceres_iters = 6;
      mulls_options.mulls_knn = 7;
      mulls_options.mulls_knn_max_dist_sq = 1.0;
      mulls_options.mulls_plane_threshold = 0.18;
      mulls_options.mulls_max_frames_in_map = 40;
      continue;
    }
    if (arg == "--dlo-kitti-profile") {
      dlo_options.input_voxel_size = 0.5;
      dlo_options.max_input_points = 6000;
      dlo_options.min_range = 1.0;
      dlo_options.max_range = 100.0;
      dlo_options.registration_voxel_size = 0.5;
      dlo_options.map_voxel_size = 0.8;
      dlo_options.keyframe_translation_threshold = 0.6;
      dlo_options.keyframe_rotation_threshold_rad =
          8.0 * 3.14159265358979323846 / 180.0;
      dlo_options.max_keyframes_in_map = 30;
      dlo_options.gicp_k_neighbors = 15;
      dlo_options.gicp_max_correspondence_distance = 4.0;
      dlo_options.gicp_max_iterations = 25;
      continue;
    }
    if (arg == "--dlo-fast-profile") {
      dlo_options.input_voxel_size = 0.65;
      dlo_options.max_input_points = 4000;
      dlo_options.registration_voxel_size = 0.85;
      dlo_options.map_voxel_size = 1.1;
      dlo_options.keyframe_translation_threshold = 0.75;
      dlo_options.max_keyframes_in_map = 20;
      dlo_options.gicp_k_neighbors = 12;
      dlo_options.gicp_max_correspondence_distance = 5.5;
      dlo_options.gicp_max_iterations = 14;
      continue;
    }
    if (arg == "--dlo-dense-profile") {
      dlo_options.input_voxel_size = 0.35;
      dlo_options.max_input_points = 9000;
      dlo_options.registration_voxel_size = 0.35;
      dlo_options.map_voxel_size = 0.55;
      dlo_options.keyframe_translation_threshold = 0.45;
      dlo_options.max_keyframes_in_map = 40;
      dlo_options.gicp_k_neighbors = 18;
      dlo_options.gicp_max_correspondence_distance = 3.5;
      dlo_options.gicp_max_iterations = 35;
      continue;
    }
    if (arg == "--dlio-kitti-profile") {
      dlio_options.input_voxel_size = 0.5;
      dlio_options.max_input_points = 6000;
      dlio_options.min_range = 1.0;
      dlio_options.max_range = 100.0;
      dlio_options.registration_voxel_size = 0.5;
      dlio_options.map_voxel_size = 0.8;
      dlio_options.keyframe_translation_threshold = 0.6;
      dlio_options.keyframe_rotation_threshold_rad =
          8.0 * 3.14159265358979323846 / 180.0;
      dlio_options.max_keyframes_in_map = 30;
      dlio_options.gicp_k_neighbors = 15;
      dlio_options.gicp_max_correspondence_distance = 4.0;
      dlio_options.gicp_max_iterations = 25;
      dlio_options.imu_rotation_fusion_weight = 0.15;
      dlio_options.imu_translation_fusion_weight = 0.2;
      dlio_options.imu_velocity_fusion_weight = 0.2;
      dlio_options.lidar_confidence_correspondence_scale = 100.0;
      continue;
    }
    if (arg == "--dlio-fast-profile") {
      dlio_options.input_voxel_size = 0.65;
      dlio_options.max_input_points = 4000;
      dlio_options.registration_voxel_size = 0.85;
      dlio_options.map_voxel_size = 1.1;
      dlio_options.keyframe_translation_threshold = 0.75;
      dlio_options.max_keyframes_in_map = 20;
      dlio_options.gicp_k_neighbors = 12;
      dlio_options.gicp_max_correspondence_distance = 5.5;
      dlio_options.gicp_max_iterations = 14;
      dlio_options.imu_rotation_fusion_weight = 0.08;
      dlio_options.imu_translation_fusion_weight = 0.12;
      dlio_options.imu_velocity_fusion_weight = 0.12;
      dlio_options.lidar_confidence_correspondence_scale = 80.0;
      continue;
    }
    if (arg == "--dlio-dense-profile") {
      dlio_options.input_voxel_size = 0.35;
      dlio_options.max_input_points = 9000;
      dlio_options.registration_voxel_size = 0.35;
      dlio_options.map_voxel_size = 0.55;
      dlio_options.keyframe_translation_threshold = 0.45;
      dlio_options.max_keyframes_in_map = 40;
      dlio_options.gicp_k_neighbors = 18;
      dlio_options.gicp_max_correspondence_distance = 3.5;
      dlio_options.gicp_max_iterations = 35;
      dlio_options.imu_rotation_fusion_weight = 0.2;
      dlio_options.imu_translation_fusion_weight = 0.25;
      dlio_options.imu_velocity_fusion_weight = 0.25;
      dlio_options.lidar_confidence_correspondence_scale = 120.0;
      continue;
    }
    if (arg == "--small-gicp-fast-profile") {
      small_gicp_options.source_voxel_size = 1.5;
      small_gicp_options.max_source_points = 1600;
      small_gicp_options.voxel_resolution = 1.5;
      small_gicp_options.k_neighbors = 10;
      small_gicp_options.max_correspondence_distance = 3.0;
      small_gicp_options.max_correspondences = 160;
      small_gicp_options.max_iterations = 10;
      small_gicp_options.map_max_points = 25000;
      small_gicp_options.refresh_interval = 6;
      small_gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--small-gicp-dense-profile") {
      small_gicp_options.source_voxel_size = 0.75;
      small_gicp_options.max_source_points = 3200;
      small_gicp_options.voxel_resolution = 0.9;
      small_gicp_options.k_neighbors = 14;
      small_gicp_options.max_correspondence_distance = 4.5;
      small_gicp_options.max_correspondences = 320;
      small_gicp_options.max_iterations = 24;
      small_gicp_options.map_max_points = 50000;
      small_gicp_options.refresh_interval = 3;
      small_gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--small-gicp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-source-voxel-size=", 0) == 0) {
      small_gicp_options.source_voxel_size = std::stod(
          arg.substr(std::string("--small-gicp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--small-gicp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-max-source-points=", 0) == 0) {
      small_gicp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--small-gicp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-voxel-resolution=", 0) == 0) {
      small_gicp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--small-gicp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--small-gicp-k-neighbors") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-k-neighbors requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.k_neighbors = std::max(3, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-k-neighbors=", 0) == 0) {
      small_gicp_options.k_neighbors = std::max(
          3, std::stoi(arg.substr(std::string("--small-gicp-k-neighbors=").size())));
      continue;
    }
    if (arg == "--small-gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr
            << "--small-gicp-max-correspondence-distance requires a numeric value"
            << std::endl;
        return 1;
      }
      small_gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-max-correspondence-distance=", 0) == 0) {
      small_gicp_options.max_correspondence_distance = std::stod(
          arg.substr(
              std::string("--small-gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--small-gicp-max-correspondences") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-correspondences requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_correspondences = std::max(16, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-max-correspondences=", 0) == 0) {
      small_gicp_options.max_correspondences = std::max(
          16, std::stoi(arg.substr(
                  std::string("--small-gicp-max-correspondences=").size())));
      continue;
    }
    if (arg == "--small-gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--small-gicp-max-iterations=", 0) == 0) {
      small_gicp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--small-gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--small-gicp-max-seed-translation-delta") {
      if (i + 1 >= argc) {
        std::cerr
            << "--small-gicp-max-seed-translation-delta requires a numeric value"
            << std::endl;
        return 1;
      }
      small_gicp_options.max_seed_translation_delta = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-max-seed-translation-delta=", 0) == 0) {
      small_gicp_options.max_seed_translation_delta = std::stod(arg.substr(
          std::string("--small-gicp-max-seed-translation-delta=").size()));
      continue;
    }
    if (arg == "--small-gicp-max-seed-rotation-delta-rad") {
      if (i + 1 >= argc) {
        std::cerr
            << "--small-gicp-max-seed-rotation-delta-rad requires a numeric value"
            << std::endl;
        return 1;
      }
      small_gicp_options.max_seed_rotation_delta_rad = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-max-seed-rotation-delta-rad=", 0) == 0) {
      small_gicp_options.max_seed_rotation_delta_rad = std::stod(arg.substr(
          std::string("--small-gicp-max-seed-rotation-delta-rad=").size()));
      continue;
    }
    if (arg == "--small-gicp-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-map-max-points=", 0) == 0) {
      small_gicp_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-map-max-points=").size()))));
      continue;
    }
    if (arg == "--small-gicp-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--small-gicp-refresh-interval=", 0) == 0) {
      small_gicp_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--small-gicp-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--small-gicp-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--small-gicp-map-radius requires a numeric value"
                  << std::endl;
        return 1;
      }
      small_gicp_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--small-gicp-map-radius=", 0) == 0) {
      small_gicp_options.map_radius = std::stod(
          arg.substr(std::string("--small-gicp-map-radius=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-fast-profile") {
      voxel_gicp_options.source_voxel_size = 1.5;
      voxel_gicp_options.max_source_points = 1600;
      voxel_gicp_options.voxel_resolution = 1.5;
      voxel_gicp_options.min_points_per_voxel = 1;
      voxel_gicp_options.max_correspondence_distance = 3.0;
      voxel_gicp_options.max_iterations = 10;
      voxel_gicp_options.map_max_points = 25000;
      voxel_gicp_options.refresh_interval = 6;
      voxel_gicp_options.map_radius = 35.0;
      continue;
    }
    if (arg == "--voxel-gicp-dense-profile") {
      voxel_gicp_options.source_voxel_size = 0.75;
      voxel_gicp_options.max_source_points = 3200;
      voxel_gicp_options.voxel_resolution = 0.9;
      voxel_gicp_options.min_points_per_voxel = 3;
      voxel_gicp_options.max_correspondence_distance = 4.5;
      voxel_gicp_options.max_iterations = 24;
      voxel_gicp_options.map_max_points = 50000;
      voxel_gicp_options.refresh_interval = 3;
      voxel_gicp_options.map_radius = 55.0;
      continue;
    }
    if (arg == "--voxel-gicp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--voxel-gicp-voxel-resolution=", 0) == 0) {
      voxel_gicp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--voxel-gicp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-min-points-per-voxel") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-min-points-per-voxel requires an integer value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.min_points_per_voxel =
          std::max(2, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--voxel-gicp-min-points-per-voxel=", 0) == 0) {
      voxel_gicp_options.min_points_per_voxel = std::max(
          2, std::stoi(
                 arg.substr(std::string("--voxel-gicp-min-points-per-voxel=").size())));
      continue;
    }
    if (arg == "--voxel-gicp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-max-correspondence-distance requires a value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.max_correspondence_distance = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--voxel-gicp-max-correspondence-distance=", 0) == 0) {
      voxel_gicp_options.max_correspondence_distance = std::stod(
          arg.substr(
              std::string("--voxel-gicp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--voxel-gicp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--voxel-gicp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      voxel_gicp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--voxel-gicp-max-iterations=", 0) == 0) {
      voxel_gicp_options.max_iterations = std::max(
          1, std::stoi(
                 arg.substr(std::string("--voxel-gicp-max-iterations=").size())));
      continue;
    }
    if (arg == "--ndt-fast-profile") {
      ndt_options.source_voxel_size = 1.0;
      ndt_options.max_source_points = 1600;
      ndt_options.resolution = 2.0;
      ndt_options.max_iterations = 4;
      ndt_options.step_size = 0.25;
      ndt_options.map_max_points = 18000;
      ndt_options.refresh_interval = 10;
      ndt_options.map_radius = 30.0;
      continue;
    }
    if (arg == "--ndt-dense-profile") {
      ndt_options.source_voxel_size = 0.6;
      ndt_options.max_source_points = 2600;
      ndt_options.resolution = 1.2;
      ndt_options.max_iterations = 7;
      ndt_options.step_size = 0.15;
      ndt_options.map_max_points = 30000;
      ndt_options.refresh_interval = 6;
      ndt_options.map_radius = 40.0;
      continue;
    }
    if (arg == "--ndt-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ndt_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-source-voxel-size=", 0) == 0) {
      ndt_options.source_voxel_size =
          std::stod(arg.substr(std::string("--ndt-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--ndt-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-max-source-points=", 0) == 0) {
      ndt_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--ndt-max-source-points=").size()))));
      continue;
    }
    if (arg == "--ndt-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-resolution requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-resolution=", 0) == 0) {
      ndt_options.resolution =
          std::stod(arg.substr(std::string("--ndt-resolution=").size()));
      continue;
    }
    if (arg == "--ndt-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ndt-max-iterations=", 0) == 0) {
      ndt_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--ndt-max-iterations=").size())));
      continue;
    }
    if (arg == "--ndt-step-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-step-size requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.step_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-step-size=", 0) == 0) {
      ndt_options.step_size =
          std::stod(arg.substr(std::string("--ndt-step-size=").size()));
      continue;
    }
    if (arg == "--ndt-map-max-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-map-max-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.map_max_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-map-max-points=", 0) == 0) {
      ndt_options.map_max_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--ndt-map-max-points=").size()))));
      continue;
    }
    if (arg == "--ndt-refresh-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-refresh-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      ndt_options.refresh_interval =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ndt-refresh-interval=", 0) == 0) {
      ndt_options.refresh_interval = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(std::string("--ndt-refresh-interval=").size()))));
      continue;
    }
    if (arg == "--ndt-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--ndt-map-radius requires a numeric value" << std::endl;
        return 1;
      }
      ndt_options.map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ndt-map-radius=", 0) == 0) {
      ndt_options.map_radius =
          std::stod(arg.substr(std::string("--ndt-map-radius=").size()));
      continue;
    }
    if (arg == "--fixed-map-ndt-seed-source") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-seed-source requires gt, velocity, ct_icp, or scan_context"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.seed_source = normalizeMethodId(argv[++i]);
      if (fixed_map_ndt_options.seed_source != "gt" &&
          fixed_map_ndt_options.seed_source != "velocity" &&
          fixed_map_ndt_options.seed_source != "ct_icp" &&
          fixed_map_ndt_options.seed_source != "scan_context") {
        std::cerr << "--fixed-map-ndt-seed-source must be gt, velocity, ct_icp, or scan_context"
                  << std::endl;
        return 1;
      }
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-seed-source=", 0) == 0) {
      fixed_map_ndt_options.seed_source = normalizeMethodId(
          arg.substr(std::string("--fixed-map-ndt-seed-source=").size()));
      if (fixed_map_ndt_options.seed_source != "gt" &&
          fixed_map_ndt_options.seed_source != "velocity" &&
          fixed_map_ndt_options.seed_source != "ct_icp" &&
          fixed_map_ndt_options.seed_source != "scan_context") {
        std::cerr << "--fixed-map-ndt-seed-source must be gt, velocity, ct_icp, or scan_context"
                  << std::endl;
        return 1;
      }
      continue;
    }
    if (arg == "--fixed-map-ndt-map-stride") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-map-stride requires an integer value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.map_stride = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-map-stride=", 0) == 0) {
      fixed_map_ndt_options.map_stride = std::max(
          1, std::stoi(arg.substr(
                 std::string("--fixed-map-ndt-map-stride=").size())));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_distance_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-threshold=", 0) == 0) {
      fixed_map_ndt_options.scan_context_distance_threshold = std::stod(
          arg.substr(std::string(
                         "--fixed-map-ndt-scan-context-threshold=").size()));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-top-k") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-top-k requires an integer value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_top_k = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-top-k=", 0) == 0) {
      fixed_map_ndt_options.scan_context_top_k = std::max(
          1, std::stoi(arg.substr(
                 std::string("--fixed-map-ndt-scan-context-top-k=").size())));
      continue;
    }
    if (arg == "--fixed-map-ndt-trace-json") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-trace-json requires a path"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.trace_json_path = argv[++i];
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-trace-json=", 0) == 0) {
      fixed_map_ndt_options.trace_json_path =
          arg.substr(std::string("--fixed-map-ndt-trace-json=").size());
      continue;
    }
    if (arg == "--fixed-map-ndt-publish-min-stable-frames") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-publish-min-stable-frames requires an integer value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.publish_min_stable_frames =
          std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-publish-min-stable-frames=", 0) == 0) {
      fixed_map_ndt_options.publish_min_stable_frames =
          std::max(1, std::stoi(arg.substr(
                          std::string("--fixed-map-ndt-publish-min-stable-frames=").size())));
      continue;
    }
    if (arg == "--fixed-map-ndt-publish-max-hold-frames") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-publish-max-hold-frames requires an integer value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.publish_max_hold_frames =
          std::max(0, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-publish-max-hold-frames=", 0) == 0) {
      fixed_map_ndt_options.publish_max_hold_frames =
          std::max(0, std::stoi(arg.substr(
                          std::string("--fixed-map-ndt-publish-max-hold-frames=").size())));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-relock-min-confirmations") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-relock-min-confirmations requires an integer value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_relock_min_confirmations =
          std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-relock-min-confirmations=", 0) == 0) {
      fixed_map_ndt_options.scan_context_relock_min_confirmations =
          std::max(1, std::stoi(arg.substr(
                          std::string("--fixed-map-ndt-scan-context-relock-min-confirmations=").size())));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-relock-max-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-relock-max-distance requires a numeric value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_relock_max_distance =
          std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-relock-max-distance=", 0) == 0) {
      fixed_map_ndt_options.scan_context_relock_max_distance =
          std::stod(arg.substr(
              std::string("--fixed-map-ndt-scan-context-relock-max-distance=").size()));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-relock-max-ndt-score") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-relock-max-ndt-score requires a numeric value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_relock_max_ndt_score =
          std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-relock-max-ndt-score=", 0) == 0) {
      fixed_map_ndt_options.scan_context_relock_max_ndt_score =
          std::stod(arg.substr(
              std::string("--fixed-map-ndt-scan-context-relock-max-ndt-score=").size()));
      continue;
    }
    if (arg == "--fixed-map-ndt-scan-context-relock-max-score-delta") {
      if (i + 1 >= argc) {
        std::cerr << "--fixed-map-ndt-scan-context-relock-max-score-delta requires a numeric value"
                  << std::endl;
        return 1;
      }
      fixed_map_ndt_options.scan_context_relock_max_score_delta =
          std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--fixed-map-ndt-scan-context-relock-max-score-delta=", 0) == 0) {
      fixed_map_ndt_options.scan_context_relock_max_score_delta =
          std::stod(arg.substr(
              std::string("--fixed-map-ndt-scan-context-relock-max-score-delta=").size()));
      continue;
    }
    if (arg == "--kiss-fast-profile") {
      kiss_icp_options.source_voxel_size = 0.75;
      kiss_icp_options.max_source_points = 2500;
      kiss_icp_options.voxel_size = 1.25;
      kiss_icp_options.initial_threshold = 1.75;
      kiss_icp_options.max_points_per_voxel = 10;
      kiss_icp_options.max_icp_iterations = 20;
      kiss_icp_options.local_map_radius = 45.0;
      kiss_icp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--kiss-dense-profile") {
      kiss_icp_options.source_voxel_size = 0.35;
      kiss_icp_options.max_source_points = 6000;
      kiss_icp_options.voxel_size = 0.8;
      kiss_icp_options.initial_threshold = 1.25;
      kiss_icp_options.max_points_per_voxel = 16;
      kiss_icp_options.max_icp_iterations = 40;
      kiss_icp_options.local_map_radius = 80.0;
      kiss_icp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--genz-fast-profile") {
      genz_icp_options.source_voxel_size = 0.75;
      genz_icp_options.max_source_points = 2500;
      genz_icp_options.voxel_size = 1.25;
      genz_icp_options.initial_threshold = 1.75;
      genz_icp_options.max_points_per_voxel = 10;
      genz_icp_options.max_icp_iterations = 20;
      genz_icp_options.planarity_threshold = 0.55;
      genz_icp_options.local_map_radius = 45.0;
      genz_icp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--genz-dense-profile") {
      genz_icp_options.source_voxel_size = 0.35;
      genz_icp_options.max_source_points = 6000;
      genz_icp_options.voxel_size = 0.8;
      genz_icp_options.initial_threshold = 1.25;
      genz_icp_options.max_points_per_voxel = 16;
      genz_icp_options.max_icp_iterations = 40;
      genz_icp_options.planarity_threshold = 0.5;
      genz_icp_options.local_map_radius = 80.0;
      genz_icp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--genz-planarity-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--genz-planarity-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      genz_icp_options.planarity_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--genz-planarity-threshold=", 0) == 0) {
      genz_icp_options.planarity_threshold =
          std::stod(arg.substr(std::string("--genz-planarity-threshold=").size()));
      continue;
    }
    if (arg == "--adaptive-icp-fast-profile") {
      adaptive_icp_options.source_voxel_size = 0.75;
      adaptive_icp_options.max_source_points = 2500;
      adaptive_icp_options.voxel_size = 1.25;
      adaptive_icp_options.initial_threshold = 1.75;
      adaptive_icp_options.max_points_per_voxel = 10;
      adaptive_icp_options.max_icp_iterations = 20;
      adaptive_icp_options.coarse_max_iterations = 6;
      adaptive_icp_options.planarity_threshold = 0.55;
      adaptive_icp_options.local_map_radius = 45.0;
      adaptive_icp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--adaptive-icp-dense-profile") {
      adaptive_icp_options.source_voxel_size = 0.35;
      adaptive_icp_options.max_source_points = 6000;
      adaptive_icp_options.voxel_size = 0.8;
      adaptive_icp_options.initial_threshold = 1.25;
      adaptive_icp_options.max_points_per_voxel = 16;
      adaptive_icp_options.max_icp_iterations = 40;
      adaptive_icp_options.coarse_max_iterations = 10;
      adaptive_icp_options.planarity_threshold = 0.5;
      adaptive_icp_options.local_map_radius = 80.0;
      adaptive_icp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--adaptive-icp-density-percentile") {
      if (i + 1 >= argc) {
        std::cerr << "--adaptive-icp-density-percentile requires a numeric value"
                  << std::endl;
        return 1;
      }
      adaptive_icp_options.density_percentile = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--adaptive-icp-reliable-tau") {
      if (i + 1 >= argc) {
        std::cerr << "--adaptive-icp-reliable-tau requires a numeric value"
                  << std::endl;
        return 1;
      }
      adaptive_icp_options.reliable_translation_tau = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--d2lio-fast-profile") {
      d2lio_options.source_voxel_size = 0.75;
      d2lio_options.max_source_points = 2500;
      d2lio_options.voxel_size = 1.25;
      d2lio_options.max_points_per_voxel = 10;
      d2lio_options.max_icp_iterations = 20;
      d2lio_options.local_map_radius = 45.0;
      d2lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--d2lio-dense-profile") {
      d2lio_options.source_voxel_size = 0.35;
      d2lio_options.max_source_points = 6000;
      d2lio_options.voxel_size = 0.8;
      d2lio_options.max_points_per_voxel = 16;
      d2lio_options.max_icp_iterations = 40;
      d2lio_options.local_map_radius = 80.0;
      d2lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--d2lio-degeneracy-ratio") {
      if (i + 1 >= argc) {
        std::cerr << "--d2lio-degeneracy-ratio requires a value"
                  << std::endl;
        return 1;
      }
      d2lio_options.degeneracy_ratio = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--d2lio-imu-prior-weight") {
      if (i + 1 >= argc) {
        std::cerr << "--d2lio-imu-prior-weight requires a value" << std::endl;
        return 1;
      }
      d2lio_options.imu_prior_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-voxelmap-fast-profile") {
      ct_voxelmap_options.source_voxel_size = 0.75;
      ct_voxelmap_options.max_source_points = 2500;
      ct_voxelmap_options.voxel_size = 1.25;
      ct_voxelmap_options.voxel_min_points = 5;
      ct_voxelmap_options.max_icp_iterations = 20;
      ct_voxelmap_options.local_map_radius = 45.0;
      ct_voxelmap_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--ct-voxelmap-dense-profile") {
      ct_voxelmap_options.source_voxel_size = 0.35;
      ct_voxelmap_options.max_source_points = 6000;
      ct_voxelmap_options.voxel_size = 0.8;
      ct_voxelmap_options.voxel_min_points = 8;
      ct_voxelmap_options.max_icp_iterations = 40;
      ct_voxelmap_options.local_map_radius = 80.0;
      ct_voxelmap_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--ct-voxelmap-planarity-ratio") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-voxelmap-planarity-ratio requires a value"
                  << std::endl;
        return 1;
      }
      ct_voxelmap_options.planarity_ratio = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--r-voxelmap-fast-profile") {
      // R-VoxelMap は平面ベースなので密な入力が必要 (疎だと平面が形成されない)。
      r_voxelmap_options.source_voxel_size = 0.45;
      r_voxelmap_options.max_source_points = 5000;
      r_voxelmap_options.voxel_size = 1.0;
      r_voxelmap_options.max_icp_iterations = 20;
      r_voxelmap_options.max_depth = 2;
      r_voxelmap_options.local_map_radius = 45.0;
      r_voxelmap_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--r-voxelmap-dense-profile") {
      r_voxelmap_options.source_voxel_size = 0.35;
      r_voxelmap_options.max_source_points = 6000;
      r_voxelmap_options.voxel_size = 0.8;
      r_voxelmap_options.max_icp_iterations = 40;
      r_voxelmap_options.max_depth = 3;
      r_voxelmap_options.local_map_radius = 80.0;
      r_voxelmap_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--r-voxelmap-max-depth") {
      if (i + 1 >= argc) { std::cerr << "--r-voxelmap-max-depth requires a value" << std::endl; return 1; }
      r_voxelmap_options.max_depth = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--damm-loam-fast-profile") {
      damm_loam_options.source_voxel_size = 0.5;
      damm_loam_options.max_source_points = 4000;
      damm_loam_options.voxel_size = 1.0;
      damm_loam_options.max_icp_iterations = 20;
      damm_loam_options.local_map_radius = 45.0;
      damm_loam_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--damm-loam-dense-profile") {
      damm_loam_options.source_voxel_size = 0.35;
      damm_loam_options.max_source_points = 6000;
      damm_loam_options.voxel_size = 0.8;
      damm_loam_options.max_icp_iterations = 40;
      damm_loam_options.local_map_radius = 80.0;
      damm_loam_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--damm-loam-degeneracy-ratio") {
      if (i + 1 >= argc) { std::cerr << "--damm-loam-degeneracy-ratio requires a value" << std::endl; return 1; }
      damm_loam_options.degeneracy_ratio = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--damm-loam-edge-weight") {
      if (i + 1 >= argc) { std::cerr << "--damm-loam-edge-weight requires a value" << std::endl; return 1; }
      damm_loam_options.edge_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--lodestar-fast-profile") {
      lodestar_options.source_voxel_size = 0.5;
      lodestar_options.max_source_points = 4000;
      lodestar_options.voxel_size = 1.0;
      lodestar_options.max_icp_iterations = 20;
      lodestar_options.local_map_radius = 45.0;
      lodestar_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--lodestar-dense-profile") {
      lodestar_options.source_voxel_size = 0.35;
      lodestar_options.max_source_points = 6000;
      lodestar_options.voxel_size = 0.8;
      lodestar_options.max_icp_iterations = 40;
      lodestar_options.local_map_radius = 80.0;
      lodestar_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--lodestar-t-chi") {
      if (i + 1 >= argc) { std::cerr << "--lodestar-t-chi requires a value" << std::endl; return 1; }
      lodestar_options.t_chi = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--lodestar-anchor-strength") {
      if (i + 1 >= argc) { std::cerr << "--lodestar-anchor-strength requires a value" << std::endl; return 1; }
      lodestar_options.anchor_strength = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--lodestar-no-data-exploitation") {
      lodestar_options.enable_data_exploitation = false;
      continue;
    }
    // --- terrain_rbf_lio ---
    if (arg == "--terrain-rbf-fast-profile") {
      terrain_rbf_options.source_voxel_size = 0.5;
      terrain_rbf_options.max_source_points = 4000;
      terrain_rbf_options.voxel_size = 1.0;
      terrain_rbf_options.max_icp_iterations = 20;
      terrain_rbf_options.local_map_radius = 45.0;
      terrain_rbf_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--terrain-rbf-dense-profile") {
      terrain_rbf_options.source_voxel_size = 0.35;
      terrain_rbf_options.max_source_points = 6000;
      terrain_rbf_options.voxel_size = 0.8;
      terrain_rbf_options.max_icp_iterations = 40;
      terrain_rbf_options.local_map_radius = 80.0;
      terrain_rbf_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--terrain-rbf-weight") {
      if (i + 1 >= argc) { std::cerr << "--terrain-rbf-weight requires a value" << std::endl; return 1; }
      terrain_rbf_options.terrain_weight = std::stod(argv[++i]);
      continue;
    }
    // --- lidar_iba ---
    if (arg == "--lidar-iba-fast-profile") {
      lidar_iba_options.source_voxel_size = 0.5;
      lidar_iba_options.max_source_points = 4000;
      lidar_iba_options.voxel_size = 1.0;
      lidar_iba_options.max_icp_iterations = 20;
      lidar_iba_options.local_map_radius = 45.0;
      lidar_iba_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--lidar-iba-dense-profile") {
      lidar_iba_options.source_voxel_size = 0.35;
      lidar_iba_options.max_source_points = 6000;
      lidar_iba_options.voxel_size = 0.8;
      lidar_iba_options.max_icp_iterations = 40;
      lidar_iba_options.local_map_radius = 80.0;
      lidar_iba_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--lidar-iba-no-ba") {
      lidar_iba_options.enable_ba = false;
      continue;
    }
    if (arg == "--lidar-iba-pose-prior-weight") {
      if (i + 1 >= argc) { std::cerr << "--lidar-iba-pose-prior-weight requires a value" << std::endl; return 1; }
      lidar_iba_options.pose_prior_weight = std::stod(argv[++i]);
      continue;
    }
    // --- dali_slam ---
    if (arg == "--dali-slam-fast-profile") {
      dali_slam_options.source_voxel_size = 0.5;
      dali_slam_options.max_source_points = 4000;
      dali_slam_options.voxel_size = 1.0;
      dali_slam_options.max_icp_iterations = 20;
      dali_slam_options.local_map_radius = 45.0;
      dali_slam_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--dali-slam-dense-profile") {
      dali_slam_options.source_voxel_size = 0.35;
      dali_slam_options.max_source_points = 6000;
      dali_slam_options.voxel_size = 0.8;
      dali_slam_options.max_icp_iterations = 40;
      dali_slam_options.local_map_radius = 80.0;
      dali_slam_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--dali-slam-no-deskew") {
      dali_slam_options.enable_deskew = false;
      continue;
    }
    if (arg == "--dali-slam-deskew") {
      dali_slam_options.enable_deskew = true;
      continue;
    }
    if (arg == "--dali-slam-degeneracy-ratio") {
      if (i + 1 >= argc) { std::cerr << "--dali-slam-degeneracy-ratio requires a value" << std::endl; return 1; }
      dali_slam_options.degeneracy_ratio = std::stod(argv[++i]);
      continue;
    }
    // --- intensity_flow ---
    if (arg == "--intensity-flow-fast-profile") {
      intensity_flow_options.source_voxel_size = 0.5;
      intensity_flow_options.max_source_points = 4000;
      intensity_flow_options.voxel_size = 1.0;
      intensity_flow_options.max_icp_iterations = 20;
      intensity_flow_options.local_map_radius = 45.0;
      intensity_flow_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--intensity-flow-dense-profile") {
      intensity_flow_options.source_voxel_size = 0.35;
      intensity_flow_options.max_source_points = 6000;
      intensity_flow_options.voxel_size = 0.8;
      intensity_flow_options.max_icp_iterations = 40;
      intensity_flow_options.local_map_radius = 80.0;
      intensity_flow_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--intensity-flow-keep-ratio") {
      if (i + 1 >= argc) { std::cerr << "--intensity-flow-keep-ratio requires a value" << std::endl; return 1; }
      intensity_flow_options.gf_keep_ratio = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--intensity-flow-no-intensity") {
      intensity_flow_options.enable_intensity = false;
      continue;
    }
    // --- svn_icp ---
    if (arg == "--svn-icp-fast-profile") {
      svn_icp_options.source_voxel_size = 0.5;
      svn_icp_options.max_source_points = 4000;
      svn_icp_options.voxel_size = 1.0;
      svn_icp_options.num_particles = 8;
      svn_icp_options.svn_iterations = 3;
      svn_icp_options.local_map_radius = 45.0;
      svn_icp_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--svn-icp-dense-profile") {
      svn_icp_options.source_voxel_size = 0.35;
      svn_icp_options.max_source_points = 6000;
      svn_icp_options.voxel_size = 0.8;
      svn_icp_options.num_particles = 12;
      svn_icp_options.svn_iterations = 4;
      svn_icp_options.local_map_radius = 80.0;
      svn_icp_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--svn-icp-num-particles") {
      if (i + 1 >= argc) { std::cerr << "--svn-icp-num-particles requires a value" << std::endl; return 1; }
      svn_icp_options.num_particles = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--svn-icp-iterations") {
      if (i + 1 >= argc) { std::cerr << "--svn-icp-iterations requires a value" << std::endl; return 1; }
      svn_icp_options.svn_iterations = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--svn-icp-lidar-sigma") {
      if (i + 1 >= argc) { std::cerr << "--svn-icp-lidar-sigma requires a value" << std::endl; return 1; }
      svn_icp_options.lidar_sigma = std::stod(argv[++i]);
      continue;
    }
    // --- pcr_dat ---
    if (arg == "--pcr-dat-fast-profile") {
      pcr_dat_options.source_voxel_size = 0.5;
      pcr_dat_options.max_source_points = 4000;
      pcr_dat_options.voxel_size = 1.0;
      pcr_dat_options.max_iterations = 15;
      pcr_dat_options.local_map_radius = 45.0;
      pcr_dat_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--pcr-dat-dense-profile") {
      pcr_dat_options.source_voxel_size = 0.35;
      pcr_dat_options.max_source_points = 6000;
      pcr_dat_options.voxel_size = 0.8;
      pcr_dat_options.max_iterations = 20;
      pcr_dat_options.local_map_radius = 80.0;
      pcr_dat_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--pcr-dat-distribution-min-points") {
      if (i + 1 >= argc) { std::cerr << "--pcr-dat-distribution-min-points requires a value" << std::endl; return 1; }
      pcr_dat_options.distribution_min_points = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--pcr-dat-distance-sigma") {
      if (i + 1 >= argc) { std::cerr << "--pcr-dat-distance-sigma requires a value" << std::endl; return 1; }
      pcr_dat_options.distance_sigma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--pcr-dat-robust-scale") {
      if (i + 1 >= argc) { std::cerr << "--pcr-dat-robust-scale requires a value" << std::endl; return 1; }
      pcr_dat_options.robust_scale = std::stod(argv[++i]);
      continue;
    }
    // --- small_mighty ---
    if (arg == "--small-mighty-fast-profile") {
      small_mighty_options.source_voxel_size = 0.5;
      small_mighty_options.max_source_points = 4000;
      small_mighty_options.voxel_size = 1.0;
      small_mighty_options.max_iterations = 15;
      small_mighty_options.local_map_radius = 45.0;
      small_mighty_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--small-mighty-dense-profile") {
      small_mighty_options.source_voxel_size = 0.35;
      small_mighty_options.max_source_points = 6000;
      small_mighty_options.voxel_size = 0.8;
      small_mighty_options.max_iterations = 20;
      small_mighty_options.local_map_radius = 80.0;
      small_mighty_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--small-mighty-contribution-gain") {
      if (i + 1 >= argc) { std::cerr << "--small-mighty-contribution-gain requires a value" << std::endl; return 1; }
      small_mighty_options.contribution_gain = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--small-mighty-edge-weight") {
      if (i + 1 >= argc) { std::cerr << "--small-mighty-edge-weight requires a value" << std::endl; return 1; }
      small_mighty_options.edge_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--small-mighty-planar-min") {
      if (i + 1 >= argc) { std::cerr << "--small-mighty-planar-min requires a value" << std::endl; return 1; }
      small_mighty_options.planar_min = std::stod(argv[++i]);
      continue;
    }
    // --- m_gclo ---
    if (arg == "--m-gclo-fast-profile") {
      m_gclo_options.source_voxel_size = 0.5;
      m_gclo_options.max_source_points = 4000;
      m_gclo_options.voxel_size = 1.0;
      m_gclo_options.max_iterations = 15;
      m_gclo_options.local_map_radius = 45.0;
      m_gclo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--m-gclo-dense-profile") {
      m_gclo_options.source_voxel_size = 0.35;
      m_gclo_options.max_source_points = 6000;
      m_gclo_options.voxel_size = 0.8;
      m_gclo_options.max_iterations = 20;
      m_gclo_options.local_map_radius = 80.0;
      m_gclo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--m-gclo-ground-weight") {
      if (i + 1 >= argc) { std::cerr << "--m-gclo-ground-weight requires a value" << std::endl; return 1; }
      m_gclo_options.ground_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--m-gclo-ground-normal-threshold") {
      if (i + 1 >= argc) { std::cerr << "--m-gclo-ground-normal-threshold requires a value" << std::endl; return 1; }
      m_gclo_options.ground_normal_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--m-gclo-uncertainty-range-ref") {
      if (i + 1 >= argc) { std::cerr << "--m-gclo-uncertainty-range-ref requires a value" << std::endl; return 1; }
      m_gclo_options.uncertainty_range_ref = std::stod(argv[++i]);
      continue;
    }
    // --- quadric_lo ---
    if (arg == "--quadric-lo-fast-profile") {
      quadric_lo_options.source_voxel_size = 0.5;
      quadric_lo_options.max_source_points = 4000;
      quadric_lo_options.voxel_size = 1.0;
      quadric_lo_options.max_iterations = 15;
      quadric_lo_options.local_map_radius = 45.0;
      quadric_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--quadric-lo-dense-profile") {
      quadric_lo_options.source_voxel_size = 0.35;
      quadric_lo_options.max_source_points = 6000;
      quadric_lo_options.voxel_size = 0.8;
      quadric_lo_options.max_iterations = 20;
      quadric_lo_options.local_map_radius = 80.0;
      quadric_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--quadric-lo-quadric-min-neighbors") {
      if (i + 1 >= argc) { std::cerr << "--quadric-lo-quadric-min-neighbors requires a value" << std::endl; return 1; }
      quadric_lo_options.quadric_min_neighbors = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--quadric-lo-quadric-weight") {
      if (i + 1 >= argc) { std::cerr << "--quadric-lo-quadric-weight requires a value" << std::endl; return 1; }
      quadric_lo_options.quadric_weight = std::stod(argv[++i]);
      continue;
    }
    // --- dilo ---
    if (arg == "--dilo-fast-profile") {
      dilo_options.source_voxel_size = 0.0;  // SRI は全点群から
      dilo_options.max_source_points = 200000;
      dilo_options.gn_voxel_size = 0.6;
      dilo_options.sri_width = 900;
      dilo_options.max_iterations = 20;
      continue;
    }
    if (arg == "--dilo-dense-profile") {
      dilo_options.source_voxel_size = 0.0;  // SRI は全点群から
      dilo_options.max_source_points = 200000;
      dilo_options.gn_voxel_size = 0.4;
      dilo_options.sri_width = 1024;
      dilo_options.max_iterations = 30;
      continue;
    }
    if (arg == "--dilo-keyframe-translation") {
      if (i + 1 >= argc) { std::cerr << "--dilo-keyframe-translation requires a value" << std::endl; return 1; }
      dilo_options.keyframe_translation = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--dilo-keyframe-rotation-deg") {
      if (i + 1 >= argc) { std::cerr << "--dilo-keyframe-rotation-deg requires a value" << std::endl; return 1; }
      dilo_options.keyframe_rotation_deg = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--dilo-sri-width") {
      if (i + 1 >= argc) { std::cerr << "--dilo-sri-width requires a value" << std::endl; return 1; }
      dilo_options.sri_width = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--dilo-initial-threshold") {
      if (i + 1 >= argc) { std::cerr << "--dilo-initial-threshold requires a value" << std::endl; return 1; }
      dilo_options.initial_threshold = std::stod(argv[++i]);
      continue;
    }
    // --- nhc_lio ---
    if (arg == "--nhc-lio-fast-profile") {
      nhc_lio_options.source_voxel_size = 0.5;
      nhc_lio_options.max_source_points = 4000;
      nhc_lio_options.voxel_size = 1.0;
      nhc_lio_options.max_iterations = 15;
      nhc_lio_options.local_map_radius = 45.0;
      nhc_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--nhc-lio-dense-profile") {
      nhc_lio_options.source_voxel_size = 0.35;
      nhc_lio_options.max_source_points = 6000;
      nhc_lio_options.voxel_size = 0.8;
      nhc_lio_options.max_iterations = 20;
      nhc_lio_options.local_map_radius = 80.0;
      nhc_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--nhc-lio-weight") {
      if (i + 1 >= argc) { std::cerr << "--nhc-lio-weight requires a value" << std::endl; return 1; }
      nhc_lio_options.nhc_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--nhc-lio-yaw-ref") {
      if (i + 1 >= argc) { std::cerr << "--nhc-lio-yaw-ref requires a value" << std::endl; return 1; }
      nhc_lio_options.nhc_yaw_ref = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--nhc-lio-disable") {
      nhc_lio_options.enable_nhc = false;
      continue;
    }
    // --- student_t_lo ---
    if (arg == "--student-t-lo-fast-profile") {
      student_t_lo_options.source_voxel_size = 0.5;
      student_t_lo_options.max_source_points = 4000;
      student_t_lo_options.voxel_size = 1.0;
      student_t_lo_options.max_iterations = 15;
      student_t_lo_options.local_map_radius = 45.0;
      student_t_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--student-t-lo-dense-profile") {
      student_t_lo_options.source_voxel_size = 0.35;
      student_t_lo_options.max_source_points = 6000;
      student_t_lo_options.voxel_size = 0.8;
      student_t_lo_options.max_iterations = 20;
      student_t_lo_options.local_map_radius = 80.0;
      student_t_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--student-t-lo-dof") {
      if (i + 1 >= argc) { std::cerr << "--student-t-lo-dof requires a value" << std::endl; return 1; }
      student_t_lo_options.student_t_dof = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--student-t-lo-scale") {
      if (i + 1 >= argc) { std::cerr << "--student-t-lo-scale requires a value" << std::endl; return 1; }
      student_t_lo_options.scale_init = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--student-t-lo-fixed-scale") {
      student_t_lo_options.estimate_scale = false;
      continue;
    }
    if (arg == "--student-t-lo-gaussian") {
      student_t_lo_options.enable_student_t = false;
      continue;
    }
    // --- spectral_lo ---
    if (arg == "--spectral-lo-bev-size") {
      if (i + 1 >= argc) { std::cerr << "--spectral-lo-bev-size requires a value" << std::endl; return 1; }
      spectral_lo_options.bev_size = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--spectral-lo-bev-range") {
      if (i + 1 >= argc) { std::cerr << "--spectral-lo-bev-range requires a value" << std::endl; return 1; }
      spectral_lo_options.bev_range = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--spectral-lo-max-yaw-deg") {
      if (i + 1 >= argc) { std::cerr << "--spectral-lo-max-yaw-deg requires a value" << std::endl; return 1; }
      spectral_lo_options.max_yaw_deg = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--spectral-lo-keyframe-translation") {
      if (i + 1 >= argc) { std::cerr << "--spectral-lo-keyframe-translation requires a value" << std::endl; return 1; }
      spectral_lo_options.keyframe_translation = std::stod(argv[++i]);
      continue;
    }
    // --- gmm_lo ---
    if (arg == "--gmm-lo-fast-profile") {
      gmm_lo_options.source_voxel_size = 0.5;
      gmm_lo_options.max_source_points = 4000;
      gmm_lo_options.voxel_size = 1.0;
      gmm_lo_options.max_iterations = 15;
      gmm_lo_options.local_map_radius = 45.0;
      gmm_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--gmm-lo-dense-profile") {
      gmm_lo_options.source_voxel_size = 0.35;
      gmm_lo_options.max_source_points = 6000;
      gmm_lo_options.voxel_size = 0.8;
      gmm_lo_options.max_iterations = 20;
      gmm_lo_options.local_map_radius = 80.0;
      gmm_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--gmm-lo-sigma-init") {
      if (i + 1 >= argc) { std::cerr << "--gmm-lo-sigma-init requires a value" << std::endl; return 1; }
      gmm_lo_options.sigma_init = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--gmm-lo-sigma-final") {
      if (i + 1 >= argc) { std::cerr << "--gmm-lo-sigma-final requires a value" << std::endl; return 1; }
      gmm_lo_options.sigma_final = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--gmm-lo-outlier-weight") {
      if (i + 1 >= argc) { std::cerr << "--gmm-lo-outlier-weight requires a value" << std::endl; return 1; }
      gmm_lo_options.outlier_weight = std::stod(argv[++i]);
      continue;
    }
    // --- gnc_lo ---
    if (arg == "--gnc-lo-fast-profile") {
      gnc_lo_options.source_voxel_size = 0.5;
      gnc_lo_options.max_source_points = 4000;
      gnc_lo_options.voxel_size = 1.0;
      gnc_lo_options.max_iterations = 15;
      gnc_lo_options.local_map_radius = 45.0;
      gnc_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--gnc-lo-dense-profile") {
      gnc_lo_options.source_voxel_size = 0.35;
      gnc_lo_options.max_source_points = 6000;
      gnc_lo_options.voxel_size = 0.8;
      gnc_lo_options.max_iterations = 20;
      gnc_lo_options.local_map_radius = 80.0;
      gnc_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--gnc-lo-truncation") {
      if (i + 1 >= argc) { std::cerr << "--gnc-lo-truncation requires a value" << std::endl; return 1; }
      gnc_lo_options.gnc_truncation = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--gnc-lo-factor") {
      if (i + 1 >= argc) { std::cerr << "--gnc-lo-factor requires a value" << std::endl; return 1; }
      gnc_lo_options.gnc_factor = std::stod(argv[++i]);
      continue;
    }
    // --- mcc_lo ---
    if (arg == "--mcc-lo-fast-profile") {
      mcc_lo_options.source_voxel_size = 0.5;
      mcc_lo_options.max_source_points = 4000;
      mcc_lo_options.voxel_size = 1.0;
      mcc_lo_options.max_iterations = 15;
      mcc_lo_options.local_map_radius = 45.0;
      mcc_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--mcc-lo-dense-profile") {
      mcc_lo_options.source_voxel_size = 0.35;
      mcc_lo_options.max_source_points = 6000;
      mcc_lo_options.voxel_size = 0.8;
      mcc_lo_options.max_iterations = 20;
      mcc_lo_options.local_map_radius = 80.0;
      mcc_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--mcc-lo-sigma") {
      if (i + 1 >= argc) { std::cerr << "--mcc-lo-sigma requires a value" << std::endl; return 1; }
      mcc_lo_options.mcc_sigma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--mcc-lo-fixed-sigma") {
      mcc_lo_options.mcc_adaptive_sigma = false;
      continue;
    }
    // --- mesh_loam ---
    if (arg == "--mesh-loam-fast-profile") {
      mesh_loam_options.source_voxel_size = 0.5;
      mesh_loam_options.max_source_points = 5000;
      mesh_loam_options.voxel_size = 0.2;
      mesh_loam_options.mesh_update_interval = 8;
      mesh_loam_options.local_map_radius = 60.0;
      continue;
    }
    if (arg == "--mesh-loam-dense-profile") {
      mesh_loam_options.source_voxel_size = 0.25;
      mesh_loam_options.max_source_points = 8000;
      mesh_loam_options.voxel_size = 0.1;
      mesh_loam_options.mesh_update_interval = 5;
      mesh_loam_options.local_map_radius = 80.0;
      continue;
    }
    if (arg == "--mesh-loam-voxel-size") {
      if (i + 1 >= argc) { std::cerr << "--mesh-loam-voxel-size requires a value" << std::endl; return 1; }
      mesh_loam_options.voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--mesh-loam-cos-gate") {
      if (i + 1 >= argc) { std::cerr << "--mesh-loam-cos-gate requires a value" << std::endl; return 1; }
      mesh_loam_options.cos_gate = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--mesh-loam-mesh-interval") {
      if (i + 1 >= argc) { std::cerr << "--mesh-loam-mesh-interval requires a value" << std::endl; return 1; }
      mesh_loam_options.mesh_update_interval = std::stoi(argv[++i]);
      continue;
    }
    // --- elo ---
    if (arg == "--elo-fast-profile") {
      elo_options.source_voxel_size = 0.25;
      elo_options.max_source_points = 35000;
      elo_options.sri_width = 1024;
      elo_options.bev_resolution = 0.3;
      elo_options.max_alignment_points = 8000;
      elo_options.max_iterations = 14;
      elo_options.ground_weight = 5.0;
      continue;
    }
    if (arg == "--elo-dense-profile") {
      elo_options.source_voxel_size = 0.15;
      elo_options.max_source_points = 80000;
      elo_options.sri_width = 2048;
      elo_options.bev_resolution = 0.2;
      elo_options.max_alignment_points = 18000;
      elo_options.max_iterations = 20;
      elo_options.ground_weight = 5.0;
      continue;
    }
    if (arg == "--elo-sri-width") {
      if (i + 1 >= argc) { std::cerr << "--elo-sri-width requires a value" << std::endl; return 1; }
      elo_options.sri_width = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--elo-bev-resolution") {
      if (i + 1 >= argc) { std::cerr << "--elo-bev-resolution requires a value" << std::endl; return 1; }
      elo_options.bev_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--elo-ground-weight") {
      if (i + 1 >= argc) { std::cerr << "--elo-ground-weight requires a value" << std::endl; return 1; }
      elo_options.ground_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--elo-max-alignment-points") {
      if (i + 1 >= argc) { std::cerr << "--elo-max-alignment-points requires a value" << std::endl; return 1; }
      elo_options.max_alignment_points = std::stoi(argv[++i]);
      continue;
    }
    // --- tc_lvgf ---
    if (arg == "--tc-lvgf-fast-profile") {
      tc_lvgf_options.source_voxel_size = 0.55;
      tc_lvgf_options.max_source_points = 3500;
      tc_lvgf_options.voxel_size = 1.1;
      tc_lvgf_options.registration_voxel_size = 1.0;
      tc_lvgf_options.max_registration_points = 2800;
      tc_lvgf_options.image_width = 768;
      tc_lvgf_options.image_height = 64;
      tc_lvgf_options.max_icp_iterations = 18;
      tc_lvgf_options.max_lidar_lines = 120;
      tc_lvgf_options.max_visual_lines = 80;
      tc_lvgf_options.line_weight = 0.2;
      tc_lvgf_options.direction_weight = 0.05;
      tc_lvgf_options.local_map_radius = 55.0;
      tc_lvgf_options.map_cleanup_interval = 3;
      continue;
    }
    if (arg == "--tc-lvgf-dense-profile") {
      tc_lvgf_options.source_voxel_size = 0.35;
      tc_lvgf_options.max_source_points = 7000;
      tc_lvgf_options.voxel_size = 0.8;
      tc_lvgf_options.registration_voxel_size = 0.7;
      tc_lvgf_options.max_registration_points = 5500;
      tc_lvgf_options.image_width = 1024;
      tc_lvgf_options.image_height = 80;
      tc_lvgf_options.max_icp_iterations = 32;
      tc_lvgf_options.max_lidar_lines = 220;
      tc_lvgf_options.max_visual_lines = 160;
      tc_lvgf_options.line_weight = 0.2;
      tc_lvgf_options.direction_weight = 0.05;
      tc_lvgf_options.local_map_radius = 80.0;
      tc_lvgf_options.map_cleanup_interval = 5;
      continue;
    }
    if (arg == "--tc-lvgf-line-weight") {
      if (i + 1 >= argc) { std::cerr << "--tc-lvgf-line-weight requires a value" << std::endl; return 1; }
      tc_lvgf_options.line_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--tc-lvgf-direction-weight") {
      if (i + 1 >= argc) { std::cerr << "--tc-lvgf-direction-weight requires a value" << std::endl; return 1; }
      tc_lvgf_options.direction_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--tc-lvgf-min-visual-lines") {
      if (i + 1 >= argc) { std::cerr << "--tc-lvgf-min-visual-lines requires a value" << std::endl; return 1; }
      tc_lvgf_options.min_visual_lines = std::stoi(argv[++i]);
      continue;
    }
    // --- opl_lvio ---
    if (arg == "--opl-lvio-fast-profile") {
      opl_lvio_options.source_voxel_size = 0.55;
      opl_lvio_options.max_source_points = 3500;
      opl_lvio_options.voxel_size = 1.1;
      opl_lvio_options.registration_voxel_size = 1.0;
      opl_lvio_options.max_registration_points = 2800;
      opl_lvio_options.image_width = 768;
      opl_lvio_options.image_height = 64;
      opl_lvio_options.max_icp_iterations = 18;
      opl_lvio_options.max_visual_points = 220;
      opl_lvio_options.max_lidar_lines = 120;
      opl_lvio_options.max_visual_lines = 80;
      opl_lvio_options.visual_point_weight = 0.06;
      opl_lvio_options.line_weight = 0.0;
      opl_lvio_options.direction_weight = 0.0;
      opl_lvio_options.local_map_radius = 55.0;
      opl_lvio_options.map_cleanup_interval = 3;
      continue;
    }
    if (arg == "--opl-lvio-dense-profile") {
      opl_lvio_options.source_voxel_size = 0.35;
      opl_lvio_options.max_source_points = 7000;
      opl_lvio_options.voxel_size = 0.8;
      opl_lvio_options.registration_voxel_size = 0.7;
      opl_lvio_options.max_registration_points = 5500;
      opl_lvio_options.image_width = 1024;
      opl_lvio_options.image_height = 80;
      opl_lvio_options.max_icp_iterations = 32;
      opl_lvio_options.max_visual_points = 420;
      opl_lvio_options.max_lidar_lines = 220;
      opl_lvio_options.max_visual_lines = 160;
      opl_lvio_options.visual_point_weight = 0.08;
      opl_lvio_options.line_weight = 0.0;
      opl_lvio_options.direction_weight = 0.0;
      opl_lvio_options.local_map_radius = 80.0;
      opl_lvio_options.map_cleanup_interval = 5;
      continue;
    }
    if (arg == "--opl-lvio-visual-point-weight") {
      if (i + 1 >= argc) { std::cerr << "--opl-lvio-visual-point-weight requires a value" << std::endl; return 1; }
      opl_lvio_options.visual_point_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--opl-lvio-line-weight") {
      if (i + 1 >= argc) { std::cerr << "--opl-lvio-line-weight requires a value" << std::endl; return 1; }
      opl_lvio_options.line_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--opl-lvio-direction-weight") {
      if (i + 1 >= argc) { std::cerr << "--opl-lvio-direction-weight requires a value" << std::endl; return 1; }
      opl_lvio_options.direction_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--opl-lvio-min-visual-points") {
      if (i + 1 >= argc) { std::cerr << "--opl-lvio-min-visual-points requires a value" << std::endl; return 1; }
      opl_lvio_options.min_visual_points = std::stoi(argv[++i]);
      continue;
    }
    if (applyVisualLidarAdapterProfileArg(arg, "v-loam15", &v_loam15_options) ||
        applyVisualLidarAdapterProfileArg(arg, "tc-vlo", &tc_vlo_options) ||
        applyVisualLidarAdapterProfileArg(arg, "ad-vlo", &ad_vlo_options) ||
        applyVisualLidarAdapterProfileArg(arg, "tc-mvlo", &tc_mvlo_options)) {
      continue;
    }
    // --- imls_slam ---
    if (arg == "--imls-slam-fast-profile") {
      imls_slam_options.source_voxel_size = 0.5;
      imls_slam_options.max_source_points = 4000;
      imls_slam_options.voxel_size = 1.0;
      imls_slam_options.imls_h = 0.5;
      imls_slam_options.max_iterations = 15;
      imls_slam_options.samples_per_axis = 100;
      imls_slam_options.local_map_radius = 45.0;
      imls_slam_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--imls-slam-dense-profile") {
      imls_slam_options.source_voxel_size = 0.35;
      imls_slam_options.max_source_points = 6000;
      imls_slam_options.voxel_size = 0.8;
      imls_slam_options.imls_h = 0.4;
      imls_slam_options.max_iterations = 20;
      imls_slam_options.samples_per_axis = 150;
      imls_slam_options.local_map_radius = 80.0;
      imls_slam_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--imls-slam-h") {
      if (i + 1 >= argc) { std::cerr << "--imls-slam-h requires a value" << std::endl; return 1; }
      imls_slam_options.imls_h = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--imls-slam-no-sampling") {
      imls_slam_options.use_observability_sampling = false;
      continue;
    }
    // --- tricp_lo ---
    if (arg == "--tricp-lo-fast-profile") {
      tricp_lo_options.source_voxel_size = 0.5;
      tricp_lo_options.max_source_points = 4000;
      tricp_lo_options.voxel_size = 1.0;
      tricp_lo_options.max_iterations = 15;
      tricp_lo_options.local_map_radius = 45.0;
      tricp_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--tricp-lo-dense-profile") {
      tricp_lo_options.source_voxel_size = 0.35;
      tricp_lo_options.max_source_points = 6000;
      tricp_lo_options.voxel_size = 0.8;
      tricp_lo_options.max_iterations = 20;
      tricp_lo_options.local_map_radius = 80.0;
      tricp_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--tricp-lo-overlap") {
      if (i + 1 >= argc) { std::cerr << "--tricp-lo-overlap requires a value" << std::endl; return 1; }
      tricp_lo_options.overlap_ratio = std::stod(argv[++i]);
      tricp_lo_options.auto_overlap = false;
      continue;
    }
    if (arg == "--tricp-lo-fixed-overlap") {
      tricp_lo_options.auto_overlap = false;
      continue;
    }
    if (arg == "--tricp-lo-lambda") {
      if (i + 1 >= argc) { std::cerr << "--tricp-lo-lambda requires a value" << std::endl; return 1; }
      tricp_lo_options.frmsd_lambda = std::stod(argv[++i]);
      continue;
    }
    // --- kc_lo ---
    if (arg == "--kc-lo-fast-profile") {
      kc_lo_options.source_voxel_size = 0.5;
      kc_lo_options.max_source_points = 4000;
      kc_lo_options.voxel_size = 1.0;
      kc_lo_options.max_iterations = 15;
      kc_lo_options.local_map_radius = 45.0;
      kc_lo_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--kc-lo-dense-profile") {
      kc_lo_options.source_voxel_size = 0.35;
      kc_lo_options.max_source_points = 6000;
      kc_lo_options.voxel_size = 0.8;
      kc_lo_options.max_iterations = 20;
      kc_lo_options.kc_sigma = 0.4;
      kc_lo_options.kc_sigma_init = 1.5;
      kc_lo_options.local_map_radius = 80.0;
      kc_lo_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--kc-lo-sigma") {
      if (i + 1 >= argc) { std::cerr << "--kc-lo-sigma requires a value" << std::endl; return 1; }
      kc_lo_options.kc_sigma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--kc-lo-sigma-init") {
      if (i + 1 >= argc) { std::cerr << "--kc-lo-sigma-init requires a value" << std::endl; return 1; }
      kc_lo_options.kc_sigma_init = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--kc-lo-no-anneal") {
      kc_lo_options.kc_sigma_init = kc_lo_options.kc_sigma;
      continue;
    }
    if (arg == "--degen-sense-fast-profile") {
      degen_sense_options.source_voxel_size = 0.5;
      degen_sense_options.max_source_points = 4000;
      degen_sense_options.voxel_size = 1.0;
      degen_sense_options.max_icp_iterations = 20;
      degen_sense_options.local_map_radius = 45.0;
      degen_sense_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--degen-sense-dense-profile") {
      degen_sense_options.source_voxel_size = 0.35;
      degen_sense_options.max_source_points = 6000;
      degen_sense_options.voxel_size = 0.8;
      degen_sense_options.max_icp_iterations = 40;
      degen_sense_options.local_map_radius = 80.0;
      degen_sense_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--degen-sense-mad-k") {
      if (i + 1 >= argc) { std::cerr << "--degen-sense-mad-k requires a value" << std::endl; return 1; }
      degen_sense_options.mad_k = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--vibration-lio-fast-profile") {
      vibration_lio_options.source_voxel_size = 0.5;
      vibration_lio_options.max_source_points = 4000;
      vibration_lio_options.voxel_size = 1.0;
      vibration_lio_options.max_icp_iterations = 20;
      vibration_lio_options.local_map_radius = 45.0;
      vibration_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--vibration-lio-dense-profile") {
      vibration_lio_options.source_voxel_size = 0.35;
      vibration_lio_options.max_source_points = 6000;
      vibration_lio_options.voxel_size = 0.8;
      vibration_lio_options.max_icp_iterations = 40;
      vibration_lio_options.local_map_radius = 80.0;
      vibration_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--vibration-lio-gamma") {
      if (i + 1 >= argc) { std::cerr << "--vibration-lio-gamma requires a value" << std::endl; return 1; }
      vibration_lio_options.gamma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--id-lio-fast-profile") {
      id_lio_options.source_voxel_size = 0.5;
      id_lio_options.max_source_points = 4000;
      id_lio_options.voxel_size = 1.0;
      id_lio_options.max_icp_iterations = 20;
      id_lio_options.range_image_width = 768;
      id_lio_options.range_image_height = 64;
      id_lio_options.local_map_radius = 45.0;
      id_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--id-lio-dense-profile") {
      id_lio_options.source_voxel_size = 0.35;
      id_lio_options.max_source_points = 7000;
      id_lio_options.voxel_size = 0.8;
      id_lio_options.max_icp_iterations = 40;
      id_lio_options.range_image_width = 1024;
      id_lio_options.range_image_height = 80;
      id_lio_options.local_map_radius = 80.0;
      id_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--id-lio-dynamic-weight") {
      if (i + 1 >= argc) { std::cerr << "--id-lio-dynamic-weight requires a value" << std::endl; return 1; }
      id_lio_options.dynamic_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--id-lio-delayed-removal-frames") {
      if (i + 1 >= argc) { std::cerr << "--id-lio-delayed-removal-frames requires a value" << std::endl; return 1; }
      id_lio_options.delayed_removal_frames = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--id-lio-new-object-margin") {
      if (i + 1 >= argc) { std::cerr << "--id-lio-new-object-margin requires a value" << std::endl; return 1; }
      id_lio_options.new_object_margin = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--id-lio-disappearance-margin") {
      if (i + 1 >= argc) { std::cerr << "--id-lio-disappearance-margin requires a value" << std::endl; return 1; }
      id_lio_options.disappearance_margin = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--rf-lio-fast-profile") {
      rf_lio_options.source_voxel_size = 0.5;
      rf_lio_options.max_source_points = 4000;
      rf_lio_options.voxel_size = 1.0;
      rf_lio_options.max_icp_iterations = 20;
      rf_lio_options.range_image_width = 768;
      rf_lio_options.range_image_height = 64;
      rf_lio_options.foreground_margin = 1.0;
      rf_lio_options.max_removal_fraction = 0.25;
      rf_lio_options.min_keep_points = 1400;
      rf_lio_options.local_map_radius = 45.0;
      rf_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--rf-lio-dense-profile") {
      rf_lio_options.source_voxel_size = 0.35;
      rf_lio_options.max_source_points = 7000;
      rf_lio_options.voxel_size = 0.8;
      rf_lio_options.max_icp_iterations = 40;
      rf_lio_options.range_image_width = 1024;
      rf_lio_options.range_image_height = 80;
      rf_lio_options.foreground_margin = 1.0;
      rf_lio_options.max_removal_fraction = 0.25;
      rf_lio_options.min_keep_points = 2200;
      rf_lio_options.local_map_radius = 80.0;
      rf_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--rf-lio-foreground-margin") {
      if (i + 1 >= argc) { std::cerr << "--rf-lio-foreground-margin requires a value" << std::endl; return 1; }
      rf_lio_options.foreground_margin = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--rf-lio-max-removal-fraction") {
      if (i + 1 >= argc) { std::cerr << "--rf-lio-max-removal-fraction requires a value" << std::endl; return 1; }
      rf_lio_options.max_removal_fraction = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--bievr-lio-fast-profile") {
      // bump-image は平面ベースなので密な入力が必要 (疎だと平面が形成されない)。
      bievr_lio_options.source_voxel_size = 0.3;
      bievr_lio_options.max_source_points = 8000;
      bievr_lio_options.voxel_size = 1.0;
      bievr_lio_options.pixel_res = 0.25;
      bievr_lio_options.max_icp_iterations = 20;
      bievr_lio_options.local_map_radius = 45.0;
      bievr_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--bievr-lio-dense-profile") {
      bievr_lio_options.source_voxel_size = 0.3;
      bievr_lio_options.max_source_points = 9000;
      bievr_lio_options.voxel_size = 0.8;
      bievr_lio_options.pixel_res = 0.2;
      bievr_lio_options.max_icp_iterations = 40;
      bievr_lio_options.local_map_radius = 80.0;
      bievr_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--ua-lio-fast-profile") {
      ua_lio_options.source_voxel_size = 0.5;
      ua_lio_options.max_source_points = 4000;
      ua_lio_options.voxel_size = 1.0;
      ua_lio_options.max_icp_iterations = 20;
      ua_lio_options.local_map_radius = 45.0;
      ua_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--ua-lio-dense-profile") {
      ua_lio_options.source_voxel_size = 0.35;
      ua_lio_options.max_source_points = 6000;
      ua_lio_options.voxel_size = 0.8;
      ua_lio_options.max_icp_iterations = 40;
      ua_lio_options.local_map_radius = 80.0;
      ua_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--ua-lio-no-ground") {
      ua_lio_options.ground_constraint = false;
      continue;
    }
    if (arg == "--cube-lio-fast-profile") {
      cube_lio_options.input_voxel_size = 0.5;
      cube_lio_options.max_input_points = 40000;
      cube_lio_options.voxel_size = 1.25;
      cube_lio_options.max_points_per_voxel = 10;
      cube_lio_options.max_icp_iterations = 20;
      cube_lio_options.cubemap_size = 192;
      cube_lio_options.local_map_radius = 45.0;
      cube_lio_options.map_cleanup_interval = 2;
      continue;
    }
    if (arg == "--cube-lio-dense-profile") {
      cube_lio_options.input_voxel_size = 0.3;
      cube_lio_options.max_input_points = 80000;
      cube_lio_options.voxel_size = 0.8;
      cube_lio_options.max_points_per_voxel = 16;
      cube_lio_options.max_icp_iterations = 40;
      cube_lio_options.cubemap_size = 320;
      cube_lio_options.local_map_radius = 80.0;
      cube_lio_options.map_cleanup_interval = 6;
      continue;
    }
    if (arg == "--cube-lio-semi-dense-quantile") {
      if (i + 1 >= argc) {
        std::cerr << "--cube-lio-semi-dense-quantile requires a value"
                  << std::endl;
        return 1;
      }
      cube_lio_options.semi_dense_quantile = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--cube-lio-intensity-sigma") {
      if (i + 1 >= argc) {
        std::cerr << "--cube-lio-intensity-sigma requires a value" << std::endl;
        return 1;
      }
      cube_lio_options.intensity_sigma = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--rko-lio-gyro-bias-gain") {
      if (i + 1 >= argc) {
        std::cerr << "--rko-lio-gyro-bias-gain requires a numeric value"
                  << std::endl;
        return 1;
      }
      rko_lio_options.gyro_bias_gain = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--rko-lio-gyro-bias-gain=", 0) == 0) {
      rko_lio_options.gyro_bias_gain =
          std::stod(arg.substr(std::string("--rko-lio-gyro-bias-gain=").size()));
      continue;
    }
    if (arg == "--fr-lio-fast-profile") {
      fr_lio_options.source_voxel_size = 0.5;
      fr_lio_options.max_source_points = 4500;
      fr_lio_options.voxel_size = 0.5;
      fr_lio_options.grid_size = 2.0;
      fr_lio_options.max_icp_iterations = 20;
      fr_lio_options.max_subframes = 3;
      fr_lio_options.min_valid_matches = 25;
      continue;
    }
    if (arg == "--fr-lio-dense-profile") {
      fr_lio_options.source_voxel_size = 0.3;
      fr_lio_options.max_source_points = 8000;
      fr_lio_options.voxel_size = 0.35;
      fr_lio_options.grid_size = 1.5;
      fr_lio_options.max_icp_iterations = 35;
      fr_lio_options.max_subframes = 4;
      fr_lio_options.min_valid_matches = 40;
      continue;
    }
    if (arg == "--pg-lio-fast-profile") {
      pg_lio_options.source_voxel_size = 0.5;
      pg_lio_options.max_source_points = 4500;
      pg_lio_options.voxel_size = 1.0;
      pg_lio_options.max_icp_iterations = 25;
      pg_lio_options.range_image_width = 512;
      pg_lio_options.range_image_height = 64;
      pg_lio_options.max_patches = 32;
      continue;
    }
    if (arg == "--pg-lio-dense-profile") {
      pg_lio_options.source_voxel_size = 0.3;
      pg_lio_options.max_source_points = 8000;
      pg_lio_options.voxel_size = 0.8;
      pg_lio_options.max_icp_iterations = 40;
      pg_lio_options.range_image_width = 768;
      pg_lio_options.range_image_height = 96;
      pg_lio_options.max_patches = 64;
      pg_lio_options.photometric_weight = 1.5;
      continue;
    }
    if (arg == "--kiss-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-source-voxel-size=", 0) == 0) {
      kiss_icp_options.source_voxel_size =
          std::stod(arg.substr(std::string("--kiss-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--kiss-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--kiss-max-source-points=", 0) == 0) {
      kiss_icp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--kiss-max-source-points=").size()))));
      continue;
    }
    if (arg == "--kiss-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-voxel-size requires a numeric value" << std::endl;
        return 1;
      }
      kiss_icp_options.voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-voxel-size=", 0) == 0) {
      kiss_icp_options.voxel_size =
          std::stod(arg.substr(std::string("--kiss-voxel-size=").size()));
      continue;
    }
    if (arg == "--kiss-initial-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-initial-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.initial_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-initial-threshold=", 0) == 0) {
      kiss_icp_options.initial_threshold = std::stod(
          arg.substr(std::string("--kiss-initial-threshold=").size()));
      continue;
    }
    if (arg == "--kiss-max-points-per-voxel") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-points-per-voxel requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_points_per_voxel = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-max-points-per-voxel=", 0) == 0) {
      kiss_icp_options.max_points_per_voxel = std::max(
          1, std::stoi(arg.substr(std::string("--kiss-max-points-per-voxel=").size())));
      continue;
    }
    if (arg == "--kiss-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.max_icp_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-max-iterations=", 0) == 0) {
      kiss_icp_options.max_icp_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--kiss-max-iterations=").size())));
      continue;
    }
    if (arg == "--kiss-local-map-radius") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-local-map-radius requires a numeric value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.local_map_radius = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--kiss-local-map-radius=", 0) == 0) {
      kiss_icp_options.local_map_radius = std::stod(
          arg.substr(std::string("--kiss-local-map-radius=").size()));
      continue;
    }
    if (arg == "--kiss-map-cleanup-interval") {
      if (i + 1 >= argc) {
        std::cerr << "--kiss-map-cleanup-interval requires an integer value"
                  << std::endl;
        return 1;
      }
      kiss_icp_options.map_cleanup_interval = std::max(0, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--kiss-map-cleanup-interval=", 0) == 0) {
      kiss_icp_options.map_cleanup_interval = std::max(
          0, std::stoi(arg.substr(std::string("--kiss-map-cleanup-interval=").size())));
      continue;
    }
    if (arg == "--ct-icp-fast-profile") {
      ct_icp_options.source_voxel_size = 1.0;
      ct_icp_options.max_source_points = 350;
      ct_icp_options.voxel_resolution = 1.8;
      ct_icp_options.max_iterations = 6;
      ct_icp_options.ceres_max_iterations = 1;
      ct_icp_options.planarity_threshold = 0.10;
      ct_icp_options.keypoint_voxel_size = 1.5;
      ct_icp_options.max_frames_in_map = 6;
      continue;
    }
    if (arg == "--ct-icp-dense-profile") {
      ct_icp_options.source_voxel_size = 0.6;
      ct_icp_options.max_source_points = 700;
      ct_icp_options.voxel_resolution = 1.2;
      ct_icp_options.max_iterations = 10;
      ct_icp_options.ceres_max_iterations = 2;
      ct_icp_options.planarity_threshold = 0.06;
      ct_icp_options.keypoint_voxel_size = 1.0;
      ct_icp_options.max_frames_in_map = 10;
      continue;
    }
    if (arg == "--ct-icp-source-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-source-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.source_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-source-voxel-size=", 0) == 0) {
      ct_icp_options.source_voxel_size = std::stod(
          arg.substr(std::string("--ct-icp-source-voxel-size=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-source-points") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-source-points requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_source_points =
          static_cast<size_t>(std::max(1, std::stoi(argv[++i])));
      continue;
    }
    if (arg.rfind("--ct-icp-max-source-points=", 0) == 0) {
      ct_icp_options.max_source_points = static_cast<size_t>(std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-icp-max-source-points=").size()))));
      continue;
    }
    if (arg == "--ct-icp-voxel-resolution") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-voxel-resolution requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.voxel_resolution = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-voxel-resolution=", 0) == 0) {
      ct_icp_options.voxel_resolution = std::stod(
          arg.substr(std::string("--ct-icp-voxel-resolution=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-max-iterations=", 0) == 0) {
      ct_icp_options.max_iterations = std::max(
          1, std::stoi(arg.substr(std::string("--ct-icp-max-iterations=").size())));
      continue;
    }
    if (arg == "--ct-icp-ceres-max-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-ceres-max-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.ceres_max_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-ceres-max-iterations=", 0) == 0) {
      ct_icp_options.ceres_max_iterations = std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-icp-ceres-max-iterations=").size())));
      continue;
    }
    if (arg == "--ct-icp-max-correspondence-distance") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-correspondence-distance requires a numeric value (squared meters)"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_correspondence_dist = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-max-correspondence-distance=", 0) == 0) {
      ct_icp_options.max_correspondence_dist = std::stod(
          arg.substr(std::string("--ct-icp-max-correspondence-distance=").size()));
      continue;
    }
    if (arg == "--ct-icp-knn") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-knn requires an integer value" << std::endl;
        return 1;
      }
      ct_icp_options.knn = std::stoi(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-knn=", 0) == 0) {
      ct_icp_options.knn = std::stoi(
          arg.substr(std::string("--ct-icp-knn=").size()));
      continue;
    }
    if (arg == "--ct-icp-multi-scale") {
      ct_icp_options.multi_scale_correspondences = true;
      continue;
    }
    if (arg == "--ct-icp-normal-cholesky") {
      ct_icp_options.use_normal_cholesky_solver = true;
      continue;
    }
    if (arg == "--ct-icp-refinement-gate") {
      ct_icp_options.refinement_gate = true;
      continue;
    }
    if (arg == "--ct-icp-native-seed") {
      ct_icp_options.native_ct_icp_seed = true;
      continue;
    }
    if (arg == "--ct-icp-ndt-keyframe-interval") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_ndt_options.keyframe_interval = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-ndt-keyframe-interval=", 0) == 0) {
      ct_icp_ndt_options.keyframe_interval = std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-icp-ndt-keyframe-interval=").size())));
      continue;
    }
    if (arg == "--ct-icp-ndt-max-correction-translation") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_ndt_options.max_correction_translation_delta = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-ndt-max-correction-translation=", 0) == 0) {
      ct_icp_ndt_options.max_correction_translation_delta = std::stod(
          arg.substr(std::string(
                         "--ct-icp-ndt-max-correction-translation=").size()));
      continue;
    }
    if (arg == "--ct-icp-ndt-max-correction-rotation-rad") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_ndt_options.max_correction_rotation_delta_rad = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-ndt-max-correction-rotation-rad=", 0) == 0) {
      ct_icp_ndt_options.max_correction_rotation_delta_rad = std::stod(
          arg.substr(std::string(
                         "--ct-icp-ndt-max-correction-rotation-rad=").size()));
      continue;
    }
    if (arg == "--ct-icp-ndt-max-score") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_ndt_options.max_ndt_score = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-ndt-max-score=", 0) == 0) {
      ct_icp_ndt_options.max_ndt_score =
          std::stod(arg.substr(std::string("--ct-icp-ndt-max-score=").size()));
      continue;
    }
    // Paper weight scheme bundle (Gap B+D+E+F):
    //   power_planarity=2, weight_alpha=0.9, weight_neighborhood=0.1,
    //   min_planarity_floor=0.01, planarity_threshold=0.01 (緩く),
    //   cauchy_loss_param=0.1, flat_regularizer_weight=true
    if (arg == "--ct-icp-paper-weights") {
      ct_icp_options.power_planarity = 2.0;
      ct_icp_options.weight_alpha = 0.9;
      ct_icp_options.weight_neighborhood = 0.1;
      ct_icp_options.min_planarity_floor = 0.01;
      ct_icp_options.planarity_threshold = 0.01;
      ct_icp_options.cauchy_loss_param = 0.1;
      ct_icp_options.flat_regularizer_weight = true;
      continue;
    }
    if (arg == "--ct-icp-power-planarity") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.power_planarity = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-weight-alpha") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.weight_alpha = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-weight-neighborhood") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.weight_neighborhood = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-min-planarity-floor") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.min_planarity_floor = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-flat-regularizer") {
      ct_icp_options.flat_regularizer_weight = true;
      continue;
    }
    if (arg == "--ct-icp-regularizer-n-cap") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.regularizer_n_cap = std::stoi(argv[++i]);
      continue;
    }
    // Pick 2 (Gap A+C): paper-aligned mapping & correspondence anchor.
    if (arg == "--ct-icp-paper-mapping") {
      // Bundle: closest-neighbor reference + PCA over 20 + min_distance 0.1.
      ct_icp_options.use_closest_neighbor_reference = true;
      ct_icp_options.pca_neighbor_count = 20;
      ct_icp_options.min_distance_between_points = 0.1;
      // paper の voxel insertion 前提では knn も増やす方が一貫
      if (ct_icp_options.knn <= 0) ct_icp_options.knn = 20;
      continue;
    }
    if (arg == "--ct-icp-closest-neighbor-reference") {
      ct_icp_options.use_closest_neighbor_reference = true;
      continue;
    }
    if (arg == "--ct-icp-pca-neighbor-count") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.pca_neighbor_count = std::stoi(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-min-distance-between-points") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.min_distance_between_points = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-coarse-to-fine") {
      ct_icp_options.coarse_to_fine = true;
      continue;
    }
    if (arg == "--ct-icp-coarse-iterations") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.coarse_iterations = std::max(0, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-icp-coarse-search-radius") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.coarse_search_radius = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg == "--ct-icp-coarse-planarity-threshold") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.coarse_planarity_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-coarse-cauchy-mult") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.coarse_cauchy_sigma_mult = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-cauchy-sigma") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.cauchy_loss_param = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-location-consistency-weight") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.location_consistency_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-orientation-consistency-weight") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.orientation_consistency_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-constant-velocity-weight") {
      if (i + 1 >= argc) { std::cerr << arg << " requires value\n"; return 1; }
      ct_icp_options.constant_velocity_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg == "--ct-icp-max-seed-translation-delta") {
      if (i + 1 >= argc) {
        std::cerr
            << "--ct-icp-max-seed-translation-delta requires a numeric value"
            << std::endl;
        return 1;
      }
      ct_icp_options.max_seed_translation_delta = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-max-seed-translation-delta=", 0) == 0) {
      ct_icp_options.max_seed_translation_delta = std::stod(arg.substr(
          std::string("--ct-icp-max-seed-translation-delta=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-seed-rotation-delta-rad") {
      if (i + 1 >= argc) {
        std::cerr
            << "--ct-icp-max-seed-rotation-delta-rad requires a numeric value"
            << std::endl;
        return 1;
      }
      ct_icp_options.max_seed_rotation_delta_rad = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-max-seed-rotation-delta-rad=", 0) == 0) {
      ct_icp_options.max_seed_rotation_delta_rad = std::stod(arg.substr(
          std::string("--ct-icp-max-seed-rotation-delta-rad=").size()));
      continue;
    }
    if (arg == "--ct-icp-paper-arch") {
      // Bundle: multi-scale + normal-cholesky + refinement-gate (LiTAMIN2-style)
      ct_icp_options.multi_scale_correspondences = true;
      ct_icp_options.use_normal_cholesky_solver = true;
      ct_icp_options.refinement_gate = true;
      continue;
    }
    if (arg == "--ct-icp-planarity-threshold") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-planarity-threshold requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.planarity_threshold = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-planarity-threshold=", 0) == 0) {
      ct_icp_options.planarity_threshold = std::stod(
          arg.substr(std::string("--ct-icp-planarity-threshold=").size()));
      continue;
    }
    if (arg == "--ct-icp-keypoint-voxel-size") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-keypoint-voxel-size requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.keypoint_voxel_size = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-icp-keypoint-voxel-size=", 0) == 0) {
      ct_icp_options.keypoint_voxel_size = std::stod(
          arg.substr(std::string("--ct-icp-keypoint-voxel-size=").size()));
      continue;
    }
    if (arg == "--ct-icp-max-frames-in-map") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-icp-max-frames-in-map requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_icp_options.max_frames_in_map = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-icp-max-frames-in-map=", 0) == 0) {
      ct_icp_options.max_frames_in_map = std::max(
          1, std::stoi(arg.substr(std::string("--ct-icp-max-frames-in-map=").size())));
      continue;
    }
    if (arg == "--xicp-fast-profile") {
      xicp_options.source_voxel_size = 1.4;
      xicp_options.max_source_points = 1800;
      xicp_options.max_iterations = 20;
      xicp_options.k_neighbors = 8;
      xicp_options.max_correspondence_distance = 2.2;
      xicp_options.map_max_points = 30000;
      xicp_options.refresh_interval = 6;
      xicp_options.map_radius = 35.0;
      xicp_options.max_seed_translation_delta = 1.5;
      xicp_options.max_seed_rotation_delta_rad = 0.20;
      continue;
    }
    if (arg == "--xicp-dense-profile") {
      xicp_options.source_voxel_size = 0.7;
      xicp_options.max_source_points = 3200;
      xicp_options.max_iterations = 40;
      xicp_options.k_neighbors = 12;
      xicp_options.max_correspondence_distance = 3.5;
      xicp_options.map_max_points = 50000;
      xicp_options.refresh_interval = 3;
      xicp_options.map_radius = 55.0;
      xicp_options.max_seed_translation_delta = 2.5;
      xicp_options.max_seed_rotation_delta_rad = 0.35;
      continue;
    }
    if (arg == "--fast-lio2-fast-profile") {
      fast_lio2_options.input_voxel_size = 0.7;
      fast_lio2_options.max_input_points = 3500;
      fast_lio2_options.max_iterations = 5;
      fast_lio2_options.ceres_max_iterations = 4;
      fast_lio2_options.voxel_resolution = 1.2;
      fast_lio2_options.keypoint_voxel_size = 0.7;
      fast_lio2_options.knn = 4;
      fast_lio2_options.max_correspondence_distance = 2.5;
      fast_lio2_options.planarity_threshold = 0.15;
      fast_lio2_options.max_frames_in_map = 20;
      continue;
    }
    if (arg == "--fast-lio2-dense-profile") {
      fast_lio2_options.input_voxel_size = 0.35;
      fast_lio2_options.max_input_points = 9000;
      fast_lio2_options.max_iterations = 10;
      fast_lio2_options.ceres_max_iterations = 8;
      fast_lio2_options.voxel_resolution = 0.8;
      fast_lio2_options.keypoint_voxel_size = 0.35;
      fast_lio2_options.knn = 6;
      fast_lio2_options.max_correspondence_distance = 3.5;
      fast_lio2_options.planarity_threshold = 0.08;
      fast_lio2_options.max_frames_in_map = 40;
      continue;
    }
    if (arg == "--hdl-graph-slam-fast-profile") {
      hdl_graph_slam_options.input_voxel_size = 0.7;
      hdl_graph_slam_options.max_input_points = 4000;
      hdl_graph_slam_options.registration_voxel_size = 0.9;
      hdl_graph_slam_options.map_voxel_size = 1.4;
      hdl_graph_slam_options.keyframe_stride = 2;
      hdl_graph_slam_options.max_keyframes_in_submap = 20;
      hdl_graph_slam_options.loop_stride = 5;
      hdl_graph_slam_options.enable_loop_closure = false;
      hdl_graph_slam_options.enable_floor_constraint = false;
      continue;
    }
    if (arg == "--hdl-graph-slam-dense-profile") {
      hdl_graph_slam_options.input_voxel_size = 0.35;
      hdl_graph_slam_options.max_input_points = 9000;
      hdl_graph_slam_options.registration_voxel_size = 0.5;
      hdl_graph_slam_options.map_voxel_size = 0.7;
      hdl_graph_slam_options.keyframe_stride = 1;
      hdl_graph_slam_options.max_keyframes_in_submap = 40;
      hdl_graph_slam_options.loop_stride = 2;
      hdl_graph_slam_options.enable_loop_closure = true;
      hdl_graph_slam_options.enable_floor_constraint = true;
      continue;
    }
    if (arg == "--vgicp-slam-fast-profile") {
      vgicp_slam_options.input_voxel_size = 0.7;
      vgicp_slam_options.max_input_points = 4000;
      vgicp_slam_options.registration_voxel_size = 0.55;
      vgicp_slam_options.map_voxel_size = 0.85;
      vgicp_slam_options.keyframe_stride = 2;
      vgicp_slam_options.max_keyframes_in_submap = 20;
      vgicp_slam_options.loop_stride = 5;
      vgicp_slam_options.enable_loop_closure = false;
      continue;
    }
    if (arg == "--vgicp-slam-dense-profile") {
      vgicp_slam_options.input_voxel_size = 0.35;
      vgicp_slam_options.max_input_points = 9000;
      vgicp_slam_options.registration_voxel_size = 0.3;
      vgicp_slam_options.map_voxel_size = 0.45;
      vgicp_slam_options.keyframe_stride = 1;
      vgicp_slam_options.max_keyframes_in_submap = 40;
      vgicp_slam_options.loop_stride = 2;
      vgicp_slam_options.enable_loop_closure = true;
      continue;
    }
    if (arg == "--suma-fast-profile") {
      suma_options.input_voxel_size = 0.7;
      suma_options.max_input_points = 4000;
      suma_options.surfel_resolution = 0.6;
      suma_options.surfel_stride = 2;
      suma_options.max_iterations = 3;
      suma_options.max_frames_in_map = 12;
      continue;
    }
    if (arg == "--suma-dense-profile") {
      suma_options.input_voxel_size = 0.35;
      suma_options.max_input_points = 9000;
      suma_options.surfel_resolution = 0.3;
      suma_options.surfel_stride = 1;
      suma_options.max_iterations = 6;
      suma_options.max_frames_in_map = 30;
      continue;
    }
    if (arg == "--balm2-fast-profile") {
      balm2_options.input_voxel_size = 0.7;
      balm2_options.max_input_points = 4000;
      balm2_options.corner_resolution = 0.4;
      balm2_options.surface_resolution = 0.8;
      balm2_options.corner_stride = 3;
      balm2_options.surface_stride = 4;
      balm2_options.window_size = 4;
      balm2_options.num_outer_iterations = 1;
      balm2_options.ceres_max_iterations = 4;
      continue;
    }
    if (arg == "--balm2-dense-profile") {
      balm2_options.input_voxel_size = 0.45;
      balm2_options.max_input_points = 7000;
      balm2_options.corner_resolution = 0.25;
      balm2_options.surface_resolution = 0.5;
      balm2_options.corner_stride = 1;
      balm2_options.surface_stride = 2;
      balm2_options.window_size = 6;
      balm2_options.num_outer_iterations = 2;
      balm2_options.ceres_max_iterations = 6;
      continue;
    }
    if (arg == "--isc-loam-fast-profile") {
      isc_loam_options.input_voxel_size = 0.7;
      isc_loam_options.max_input_points = 4000;
      isc_loam_options.keyframe_stride = 2;
      isc_loam_options.loop_stride = 5;
      isc_loam_options.enable_loop_closure = false;
      continue;
    }
    if (arg == "--isc-loam-dense-profile") {
      isc_loam_options.input_voxel_size = 0.35;
      isc_loam_options.max_input_points = 9000;
      isc_loam_options.keyframe_stride = 1;
      isc_loam_options.loop_stride = 2;
      isc_loam_options.enable_loop_closure = true;
      continue;
    }
    if (arg == "--loam-livox-fast-profile") {
      loam_livox_options.input_voxel_size = 0.7;
      loam_livox_options.max_input_points = 4000;
      loam_livox_options.less_flat_leaf_size = 0.35f;
      loam_livox_options.odom_outer_iters = 1;
      loam_livox_options.map_outer_iters = 1;
      loam_livox_options.map_line_resolution = 0.6;
      loam_livox_options.map_plane_resolution = 1.2;
      continue;
    }
    if (arg == "--loam-livox-dense-profile") {
      loam_livox_options.input_voxel_size = 0.35;
      loam_livox_options.max_input_points = 9000;
      loam_livox_options.less_flat_leaf_size = 0.15f;
      loam_livox_options.odom_outer_iters = 2;
      loam_livox_options.map_outer_iters = 2;
      loam_livox_options.map_line_resolution = 0.3;
      loam_livox_options.map_plane_resolution = 0.6;
      continue;
    }
    if (arg == "--lio-sam-fast-profile") {
      lio_sam_options.input_voxel_size = 0.7;
      lio_sam_options.max_input_points = 4000;
      lio_sam_options.keyframe_stride = 2;
      lio_sam_options.enable_loop_closure = false;
      lio_sam_options.enable_imu_rotation_prior = true;
      continue;
    }
    if (arg == "--lio-sam-dense-profile") {
      lio_sam_options.input_voxel_size = 0.45;
      lio_sam_options.max_input_points = 7000;
      lio_sam_options.keyframe_stride = 1;
      lio_sam_options.enable_loop_closure = true;
      lio_sam_options.enable_imu_rotation_prior = true;
      continue;
    }
    if (arg == "--lins-fast-profile") {
      lins_options.input_voxel_size = 0.7;
      lins_options.max_input_points = 4000;
      lins_options.registration_voxel_size = 0.7;
      lins_options.map_voxel_size = 1.0;
      lins_options.max_correspondences = 140;
      lins_options.max_keyframes_in_map = 20;
      continue;
    }
    if (arg == "--lins-dense-profile") {
      lins_options.input_voxel_size = 0.35;
      lins_options.max_input_points = 9000;
      lins_options.registration_voxel_size = 0.35;
      lins_options.map_voxel_size = 0.55;
      lins_options.max_correspondences = 280;
      lins_options.max_keyframes_in_map = 40;
      continue;
    }
    if (arg == "--fast-lio-slam-fast-profile") {
      fast_lio_slam_options.input_voxel_size = 0.7;
      fast_lio_slam_options.max_input_points = 3500;
      fast_lio_slam_options.max_iterations = 5;
      fast_lio_slam_options.ceres_max_iterations = 4;
      fast_lio_slam_options.voxel_resolution = 1.2;
      fast_lio_slam_options.keypoint_voxel_size = 0.7;
      fast_lio_slam_options.max_frames_in_map = 20;
      fast_lio_slam_options.keyframe_stride = 2;
      fast_lio_slam_options.loop_stride = 5;
      fast_lio_slam_options.enable_loop_closure = false;
      continue;
    }
    if (arg == "--fast-lio-slam-dense-profile") {
      fast_lio_slam_options.input_voxel_size = 0.35;
      fast_lio_slam_options.max_input_points = 9000;
      fast_lio_slam_options.max_iterations = 10;
      fast_lio_slam_options.ceres_max_iterations = 8;
      fast_lio_slam_options.voxel_resolution = 0.8;
      fast_lio_slam_options.keypoint_voxel_size = 0.35;
      fast_lio_slam_options.max_frames_in_map = 40;
      fast_lio_slam_options.keyframe_stride = 1;
      fast_lio_slam_options.loop_stride = 2;
      fast_lio_slam_options.enable_loop_closure = true;
      continue;
    }
    if (arg == "--point-lio-fast-profile") {
      point_lio_options.input_voxel_size = 0.7;
      point_lio_options.max_input_points = 4000;
      point_lio_options.registration_voxel_size = 0.6;
      point_lio_options.map_voxel_size = 0.9;
      point_lio_options.max_correspondences = 160;
      point_lio_options.max_keyframes_in_map = 20;
      continue;
    }
    if (arg == "--point-lio-dense-profile") {
      point_lio_options.input_voxel_size = 0.35;
      point_lio_options.max_input_points = 9000;
      point_lio_options.registration_voxel_size = 0.3;
      point_lio_options.map_voxel_size = 0.5;
      point_lio_options.max_correspondences = 320;
      point_lio_options.max_keyframes_in_map = 40;
      continue;
    }
    if (arg == "--clins-fast-profile") {
      clins_options.source_voxel_size = 1.0;
      clins_options.max_source_points = 300;
      continue;
    }
    if (arg == "--clins-dense-profile") {
      clins_options.source_voxel_size = 0.45;
      clins_options.max_source_points = 900;
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-window") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-window requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_window = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-window=", 0) == 0) {
      ct_lio_fixed_lag_window = std::max(
          1, std::stoi(arg.substr(std::string("--ct-lio-fixed-lag-window=").size())));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-velocity-weight") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-velocity-weight requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_velocity_weight = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-velocity-weight=", 0) == 0) {
      ct_lio_fixed_lag_velocity_weight = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-velocity-weight=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-gyro-bias-scale") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-gyro-bias-scale requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_gyro_bias_scale = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-gyro-bias-scale=", 0) == 0) {
      ct_lio_fixed_lag_gyro_bias_scale = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-gyro-bias-scale=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-accel-bias-scale") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-accel-bias-scale requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_accel_bias_scale = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-accel-bias-scale=", 0) == 0) {
      ct_lio_fixed_lag_accel_bias_scale = std::stod(
          arg.substr(std::string("--ct-lio-fixed-lag-accel-bias-scale=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-history-decay") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-history-decay requires a numeric value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_history_decay = std::stod(argv[++i]);
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-history-decay=", 0) == 0) {
      ct_lio_fixed_lag_history_decay =
          std::stod(arg.substr(std::string("--ct-lio-fixed-lag-history-decay=").size()));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-outer-iterations") {
      if (i + 1 >= argc) {
        std::cerr << "--ct-lio-fixed-lag-outer-iterations requires an integer value"
                  << std::endl;
        return 1;
      }
      ct_lio_fixed_lag_outer_iterations = std::max(1, std::stoi(argv[++i]));
      continue;
    }
    if (arg.rfind("--ct-lio-fixed-lag-outer-iterations=", 0) == 0) {
      ct_lio_fixed_lag_outer_iterations = std::max(
          1, std::stoi(arg.substr(
                 std::string("--ct-lio-fixed-lag-outer-iterations=").size())));
      continue;
    }
    if (arg == "--ct-lio-fixed-lag-smoother") {
      ct_lio_fixed_lag_smoother = true;
      continue;
    }
    if (arg == "--methods") {
      if (i + 1 >= argc) {
        std::cerr << "--methods requires a comma-separated value" << std::endl;
        return 1;
      }
      selected_methods = splitMethodList(argv[++i]);
      continue;
    }
    if (arg.rfind("--methods=", 0) == 0) {
      selected_methods = splitMethodList(arg.substr(std::string("--methods=").size()));
      continue;
    }
    if (!arg.empty() && arg[0] == '-') {
      std::cerr << "Unknown option: " << arg << std::endl;
      return 1;
    }
    max_frames = std::stoi(arg);
  }

  if (selected_methods.empty()) {
    std::cerr
        << "No methods selected. Supported methods: litamin2, gicp, small_gicp, "
        << "voxel_gicp, ndt, kiss_icp, dlo, dlio, aloam, floam, lego_loam, mulls, "
        << "ct_lio, ct_icp, ct_icp_ndt, ct_icp_ndt_keyframe, fixed_map_ndt, xicp, fast_lio2, hdl_graph_slam, vgicp_slam, "
        << "suma, balm2, isc_loam, loam_livox, lio_sam, lins, fast_lio_slam, "
        << "point_lio, clins"
        << std::endl;
    return 1;
  }
  for (const auto& method : selected_methods) {
    if (!isSupportedMethod(method)) {
      std::cerr << "Unsupported method: " << method
                << " (supported: litamin2, gicp, small_gicp, voxel_gicp, ndt, "
                   "kiss_icp, dlo, dlio, aloam, floam, lego_loam, mulls, ct_lio, "
                   "ct_icp, ct_icp_ndt, ct_icp_ndt_keyframe, fixed_map_ndt, xicp, fast_lio2, hdl_graph_slam, vgicp_slam, "
                   "suma, balm2, isc_loam, loam_livox, lio_sam, lins, "
                   "fast_lio_slam, point_lio, clins)"
                << std::endl;
      return 1;
    }
  }
  litamin2_options.seed_perturbation = seed_perturbation;
  gicp_options.seed_perturbation = seed_perturbation;
  small_gicp_options.seed_perturbation = seed_perturbation;
  voxel_gicp_options.seed_perturbation = seed_perturbation;
  ndt_options.seed_perturbation = seed_perturbation;

  auto pcd_dirs = listPCDDirs(pcd_dir);
  size_t total_pcd_frames = pcd_dirs.size();
  if (max_frames > 0 && max_frames < (int)pcd_dirs.size())
    pcd_dirs.resize(max_frames);

  auto gt_poses_raw = loadGTPoses(gt_csv);
  auto frame_timestamps = loadFrameTimestampsFromCsv(pcd_dir, pcd_dirs.size());
  FrameTimestampSource frame_timestamp_source =
      FrameTimestampSource::kFrameTimestampCsv;
  if (frame_timestamps.empty()) {
    frame_timestamps = loadFrameTimestampsFromGraph(pcd_dirs);
    frame_timestamp_source = FrameTimestampSource::kGraphTrajectory;
  }
  if (frame_timestamps.empty()) {
    frame_timestamps =
        sampleFrameTimestamps(gt_poses_raw, pcd_dirs.size(), total_pcd_frames);
    frame_timestamp_source = FrameTimestampSource::kSampledGT;
  }
  auto frame_gap_stats = computeFrameGapStats(frame_timestamps);
  if (frame_timestamp_source == FrameTimestampSource::kFrameTimestampCsv &&
      frame_gap_stats.valid &&
      frame_gap_stats.median_gap > 0.5) {
    // Some preprocessed sequences provide sparse/keyframe timestamps that do not
    // line up with the dense scan sequence expected by most methods. When that
    // happens, fall back to index-based GT sampling to keep evaluation aligned.
    frame_timestamps =
        sampleFrameTimestamps(gt_poses_raw, pcd_dirs.size(), total_pcd_frames);
    frame_timestamp_source = FrameTimestampSource::kSampledGT;
    frame_gap_stats = computeFrameGapStats(frame_timestamps);
  }
  auto gt = sampleGTPoseMatrices(gt_poses_raw, frame_timestamps);

  std::cout << "========================================" << std::endl;
  std::cout << "  PCD Dogfooding" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "PCD frames: " << pcd_dirs.size() << std::endl;
  std::cout << "GT poses: " << gt.size() << std::endl;

  fs::path imu_csv_path = fs::path(pcd_dir) / "imu.csv";
  if (!fs::exists(imu_csv_path)) {
    imu_csv_path = fs::path(pcd_dir).parent_path() / "imu.csv";
  }
  std::vector<ImuSampleCsv> imu_samples;
  if (fs::exists(imu_csv_path)) {
    imu_samples = loadImuCsv(imu_csv_path.string());
    std::cout << "IMU samples: " << imu_samples.size() << std::endl;
  } else {
    std::cout << "IMU samples: 0 (imu.csv not found)" << std::endl;
  }

  std::cout << "Timestamp source: " << frameTimestampSourceName(frame_timestamp_source)
            << std::endl;
  if (frame_gap_stats.valid) {
    std::cout << "Frame gap [s]: min=" << std::fixed << std::setprecision(3)
              << frame_gap_stats.min_gap
              << " median=" << frame_gap_stats.median_gap
              << " mean=" << frame_gap_stats.mean_gap
              << " max=" << frame_gap_stats.max_gap << std::endl;
  } else {
    std::cout << "Frame gap [s]: unavailable" << std::endl;
  }

  // 走行距離
  double dist = 0;
  for (size_t i = 1; i < gt.size(); i++)
    dist += (gt[i].block<3, 1>(0, 3) - gt[i-1].block<3, 1>(0, 3)).norm();
  std::cout << "Trajectory length: " << std::fixed << std::setprecision(1) << dist << " m" << std::endl;

  std::vector<MethodResult> results;

  if (no_gt_seed) {
    std::cout << "[no-gt-seed] Scan-to-map methods will use velocity-model prediction instead of GT initialization." << std::endl;
  }
  if (ct_icp_gt_seed) {
    std::cout << "[ct-icp-gt-seed] CT-ICP TrajectoryFrame init will be seeded from GT begin/end poses." << std::endl;
  }
  if (seed_perturbation.enabled()) {
    std::cout << "[seed-perturb] local-frame dx=" << std::fixed
              << std::setprecision(3) << seed_perturbation.x
              << "m dy=" << seed_perturbation.y
              << "m dz=" << seed_perturbation.z
              << "m yaw="
              << seed_perturbation.yaw_rad * 180.0 / M_PI
              << "deg" << std::endl;
  }

  if (isMethodEnabled(selected_methods, "litamin2")) {
    std::cout << "\nRunning LiTAMIN2..." << std::endl;
    std::cout << "  voxel_resolution=" << litamin2_options.voxel_resolution
              << " max_iterations=" << litamin2_options.max_iterations
              << " max_source_points=" << litamin2_options.max_source_points
              << " use_cov_cost=" << (litamin2_options.use_cov_cost ? "on" : "off")
              << " num_threads=" << litamin2_options.num_threads << std::endl;
    results.push_back(runLiTAMIN2(pcd_dirs, gt, litamin2_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "gicp")) {
    std::cout << "Running GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << gicp_options.source_voxel_size
              << " max_source_points=" << gicp_options.max_source_points
              << " k_neighbors=" << gicp_options.k_neighbors
              << " max_iterations=" << gicp_options.max_iterations
              << " map_max_points=" << gicp_options.map_max_points << std::endl;
    results.push_back(runGICP(pcd_dirs, gt, gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "small_gicp")) {
    std::cout << "Running Small-GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << small_gicp_options.source_voxel_size
              << " max_source_points=" << small_gicp_options.max_source_points
              << " voxel_resolution=" << small_gicp_options.voxel_resolution
              << " k_neighbors=" << small_gicp_options.k_neighbors
              << " max_iterations=" << small_gicp_options.max_iterations
              << " max_correspondences=" << small_gicp_options.max_correspondences
              << " map_max_points=" << small_gicp_options.map_max_points
              << std::endl;
    results.push_back(
        runSmallGICP(pcd_dirs, gt, small_gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "voxel_gicp")) {
    std::cout << "Running Voxel-GICP..." << std::endl;
    std::cout << "  source_voxel_size=" << voxel_gicp_options.source_voxel_size
              << " max_source_points=" << voxel_gicp_options.max_source_points
              << " voxel_resolution=" << voxel_gicp_options.voxel_resolution
              << " min_points_per_voxel="
              << voxel_gicp_options.min_points_per_voxel
              << " max_iterations=" << voxel_gicp_options.max_iterations
              << " map_max_points=" << voxel_gicp_options.map_max_points
              << std::endl;
    results.push_back(
        runVoxelGICP(pcd_dirs, gt, voxel_gicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "aloam")) {
    std::cout << "Running A-LOAM..." << std::endl;
    std::cout << "  n_scans=" << aloam_options.n_scans
              << " scan_period=" << aloam_options.scan_period
              << " curvature_threshold=" << aloam_options.curvature_threshold
              << " less_flat_leaf_size=" << aloam_options.less_flat_leaf_size
              << " odom_iters=" << aloam_options.odom_outer_iters
              << " map_iters=" << aloam_options.map_outer_iters
              << std::endl;
    results.push_back(runALOAM(pcd_dirs, gt, aloam_options));
  }

  if (isMethodEnabled(selected_methods, "floam")) {
    std::cout << "Running F-LOAM..." << std::endl;
    std::cout << "  n_scans=" << floam_options.n_scans
              << " stride=" << floam_options.input_point_stride
              << " map_interval=" << floam_options.mapping_update_interval
              << " curvature_threshold=" << floam_options.curvature_threshold
              << " odom_iters=" << floam_options.odom_outer_iters
              << " map_iters=" << floam_options.map_outer_iters
              << std::endl;
    results.push_back(runFLOAM(pcd_dirs, gt, floam_options));
  }

  if (isMethodEnabled(selected_methods, "lego_loam")) {
    std::cout << "Running LeGO-LOAM..." << std::endl;
    std::cout << "  n_scans=" << lego_loam_options.n_scans
              << " sensor_height=" << lego_loam_options.sensor_height
              << " ground_scan_limit=" << lego_loam_options.ground_scan_limit
              << " curvature_threshold=" << lego_loam_options.curvature_threshold
              << " odom_iters=" << lego_loam_options.odom_outer_iters
              << " map_iters=" << lego_loam_options.map_outer_iters
              << std::endl;
    results.push_back(runLeGOLOAM(pcd_dirs, gt, lego_loam_options));
  }

  if (isMethodEnabled(selected_methods, "mulls")) {
    std::cout << "Running MULLS..." << std::endl;
    std::cout << "  n_scans=" << mulls_options.n_scans
              << " mulls_map_iters=" << mulls_options.mulls_map_iters
              << " mulls_ceres_iters=" << mulls_options.mulls_ceres_iters
              << " max_frames_in_map=" << mulls_options.mulls_max_frames_in_map
              << " point_w=" << mulls_options.mulls_point_weight << std::endl;
    results.push_back(runMULLS(pcd_dirs, gt, mulls_options));
  }

  if (isMethodEnabled(selected_methods, "ndt")) {
    std::cout << "Running NDT..." << std::endl;
    std::cout << "  source_voxel_size=" << ndt_options.source_voxel_size
              << " max_source_points=" << ndt_options.max_source_points
              << " resolution=" << ndt_options.resolution
              << " max_iterations=" << ndt_options.max_iterations
              << " map_max_points=" << ndt_options.map_max_points << std::endl;
    results.push_back(runNDT(pcd_dirs, gt, ndt_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "fixed_map_ndt")) {
    std::cout << "Running FixedMap-NDT..." << std::endl;
    std::cout << "  source_voxel_size=" << ndt_options.source_voxel_size
              << " max_source_points=" << ndt_options.max_source_points
              << " resolution=" << ndt_options.resolution
              << " max_iterations=" << ndt_options.max_iterations
              << " map_max_points=" << ndt_options.map_max_points
              << " map_stride=" << fixed_map_ndt_options.map_stride
              << " seed_source=" << fixed_map_ndt_options.seed_source
              << " publish_min_stable_frames="
              << fixed_map_ndt_options.publish_min_stable_frames
              << " publish_max_hold_frames="
              << fixed_map_ndt_options.publish_max_hold_frames
              << " sc_relock_min_confirmations="
              << fixed_map_ndt_options.scan_context_relock_min_confirmations
              << " sc_relock_max_distance=" << std::fixed << std::setprecision(3)
              << fixed_map_ndt_options.scan_context_relock_max_distance
              << " sc_relock_max_ndt_score="
              << fixed_map_ndt_options.scan_context_relock_max_ndt_score
              << " trace_json="
              << (fixed_map_ndt_options.trace_json_path.empty()
                      ? "off"
                      : fixed_map_ndt_options.trace_json_path)
              << std::endl;
    results.push_back(runFixedMapNDT(pcd_dirs, gt, ndt_options,
                                     ct_icp_options, fixed_map_ndt_options));
  }

  if (isMethodEnabled(selected_methods, "kiss_icp")) {
    std::cout << "Running KISS-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << kiss_icp_options.source_voxel_size
              << " max_source_points=" << kiss_icp_options.max_source_points
              << " voxel_size=" << kiss_icp_options.voxel_size
              << " max_iterations=" << kiss_icp_options.max_icp_iterations
              << " local_map_radius=" << kiss_icp_options.local_map_radius
              << std::endl;
    results.push_back(runKISSICP(pcd_dirs, gt, kiss_icp_options));
  }

  if (isMethodEnabled(selected_methods, "genz_icp")) {
    std::cout << "Running GenZ-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << genz_icp_options.source_voxel_size
              << " max_source_points=" << genz_icp_options.max_source_points
              << " voxel_size=" << genz_icp_options.voxel_size
              << " planarity_threshold=" << genz_icp_options.planarity_threshold
              << " max_iterations=" << genz_icp_options.max_icp_iterations
              << std::endl;
    results.push_back(runGenZICP(pcd_dirs, gt, genz_icp_options));
  }

  if (isMethodEnabled(selected_methods, "adaptive_icp")) {
    std::cout << "Running Adaptive-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << adaptive_icp_options.source_voxel_size
              << " voxel_size=" << adaptive_icp_options.voxel_size
              << " density_percentile=" << adaptive_icp_options.density_percentile
              << " reliable_tau=" << adaptive_icp_options.reliable_translation_tau
              << " max_iterations=" << adaptive_icp_options.max_icp_iterations
              << std::endl;
    results.push_back(runAdaptiveICP(pcd_dirs, gt, adaptive_icp_options));
  }

  if (isMethodEnabled(selected_methods, "rko_lio")) {
    std::cout << "Running RKO-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << rko_lio_options.source_voxel_size
              << " max_source_points=" << rko_lio_options.max_source_points
              << " voxel_size=" << rko_lio_options.voxel_size
              << " max_iterations=" << rko_lio_options.max_icp_iterations
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runRKOLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                                rko_lio_options));
  }

  if (isMethodEnabled(selected_methods, "fr_lio")) {
    std::cout << "Running FR-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << fr_lio_options.source_voxel_size
              << " voxel_size=" << fr_lio_options.voxel_size
              << " grid_size=" << fr_lio_options.grid_size
              << " max_subframes=" << fr_lio_options.max_subframes
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runFRLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               fr_lio_options));
  }

  if (isMethodEnabled(selected_methods, "pg_lio")) {
    std::cout << "Running PG-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << pg_lio_options.source_voxel_size
              << " voxel_size=" << pg_lio_options.voxel_size
              << " range_image=" << pg_lio_options.range_image_width << "x"
              << pg_lio_options.range_image_height
              << " max_patches=" << pg_lio_options.max_patches
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runPGLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               pg_lio_options));
  }

  if (isMethodEnabled(selected_methods, "d2lio")) {
    std::cout << "Running D2-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << d2lio_options.source_voxel_size
              << " voxel_size=" << d2lio_options.voxel_size
              << " degeneracy_ratio="
              << d2lio_options.degeneracy_ratio
              << " imu_prior_weight=" << d2lio_options.imu_prior_weight
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runD2LIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               d2lio_options));
  }

  if (isMethodEnabled(selected_methods, "ct_voxelmap")) {
    std::cout << "Running CT-VoxelMap..." << std::endl;
    std::cout << "  source_voxel_size=" << ct_voxelmap_options.source_voxel_size
              << " voxel_size=" << ct_voxelmap_options.voxel_size
              << " voxel_min_points=" << ct_voxelmap_options.voxel_min_points
              << " planarity_ratio=" << ct_voxelmap_options.planarity_ratio
              << " max_iterations=" << ct_voxelmap_options.max_icp_iterations
              << std::endl;
    results.push_back(runCTVoxelMap(pcd_dirs, gt, ct_voxelmap_options));
  }

  if (isMethodEnabled(selected_methods, "r_voxelmap")) {
    std::cout << "Running R-VoxelMap..." << std::endl;
    std::cout << "  source_voxel_size=" << r_voxelmap_options.source_voxel_size
              << " voxel_size=" << r_voxelmap_options.voxel_size
              << " max_depth=" << r_voxelmap_options.max_depth
              << " inlier_dist=" << r_voxelmap_options.inlier_dist
              << " max_iterations=" << r_voxelmap_options.max_icp_iterations
              << std::endl;
    results.push_back(runRVoxelMap(pcd_dirs, gt, r_voxelmap_options));
  }

  if (isMethodEnabled(selected_methods, "damm_loam")) {
    std::cout << "Running DAMM-LOAM..." << std::endl;
    std::cout << "  source_voxel_size=" << damm_loam_options.source_voxel_size
              << " voxel_size=" << damm_loam_options.voxel_size
              << " degeneracy_ratio=" << damm_loam_options.degeneracy_ratio
              << " edge_weight=" << damm_loam_options.edge_weight
              << " max_iterations=" << damm_loam_options.max_icp_iterations
              << std::endl;
    results.push_back(runDammLoam(pcd_dirs, gt, damm_loam_options));
  }

  if (isMethodEnabled(selected_methods, "lodestar")) {
    std::cout << "Running LODESTAR..." << std::endl;
    std::cout << "  source_voxel_size=" << lodestar_options.source_voxel_size
              << " voxel_size=" << lodestar_options.voxel_size
              << " t_chi=" << lodestar_options.t_chi
              << " anchor_strength=" << lodestar_options.anchor_strength
              << " data_exploitation=" << lodestar_options.enable_data_exploitation
              << " max_iterations=" << lodestar_options.max_icp_iterations
              << std::endl;
    results.push_back(runLodestar(pcd_dirs, gt, lodestar_options));
  }

  if (isMethodEnabled(selected_methods, "terrain_rbf_lio")) {
    std::cout << "Running Terrain-RBF-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << terrain_rbf_options.source_voxel_size
              << " voxel_size=" << terrain_rbf_options.voxel_size
              << " terrain_cell=" << terrain_rbf_options.terrain_cell_size
              << " terrain_weight=" << terrain_rbf_options.terrain_weight
              << " max_iterations=" << terrain_rbf_options.max_icp_iterations
              << std::endl;
    results.push_back(runTerrainRbf(pcd_dirs, gt, terrain_rbf_options));
  }

  if (isMethodEnabled(selected_methods, "lidar_iba")) {
    std::cout << "Running LiDAR-IBA..." << std::endl;
    std::cout << "  source_voxel_size=" << lidar_iba_options.source_voxel_size
              << " voxel_size=" << lidar_iba_options.voxel_size
              << " enable_ba=" << lidar_iba_options.enable_ba
              << " window=" << lidar_iba_options.window_size
              << " pose_prior=" << lidar_iba_options.pose_prior_weight
              << " max_iterations=" << lidar_iba_options.max_icp_iterations
              << std::endl;
    results.push_back(runLidarIba(pcd_dirs, gt, lidar_iba_options));
  }

  if (isMethodEnabled(selected_methods, "dali_slam")) {
    std::cout << "Running DALI-SLAM..." << std::endl;
    std::cout << "  source_voxel_size=" << dali_slam_options.source_voxel_size
              << " voxel_size=" << dali_slam_options.voxel_size
              << " deskew=" << dali_slam_options.enable_deskew
              << " degeneracy_ratio=" << dali_slam_options.degeneracy_ratio
              << " max_iterations=" << dali_slam_options.max_icp_iterations
              << std::endl;
    results.push_back(runDaliSlam(pcd_dirs, gt, dali_slam_options));
  }

  if (isMethodEnabled(selected_methods, "intensity_flow")) {
    std::cout << "Running Intensity-Flow..." << std::endl;
    std::cout << "  source_voxel_size=" << intensity_flow_options.source_voxel_size
              << " voxel_size=" << intensity_flow_options.voxel_size
              << " keep_ratio=" << intensity_flow_options.gf_keep_ratio
              << " intensity=" << intensity_flow_options.enable_intensity
              << " max_iterations=" << intensity_flow_options.max_icp_iterations
              << std::endl;
    results.push_back(runIntensityFlow(pcd_dirs, gt, intensity_flow_options));
  }

  if (isMethodEnabled(selected_methods, "svn_icp")) {
    std::cout << "Running SVN-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << svn_icp_options.source_voxel_size
              << " voxel_size=" << svn_icp_options.voxel_size
              << " num_particles=" << svn_icp_options.num_particles
              << " svn_iterations=" << svn_icp_options.svn_iterations
              << " lidar_sigma=" << svn_icp_options.lidar_sigma
              << std::endl;
    results.push_back(runSvnIcp(pcd_dirs, gt, svn_icp_options));
  }

  if (isMethodEnabled(selected_methods, "pcr_dat")) {
    std::cout << "Running PCR-DAT..." << std::endl;
    std::cout << "  source_voxel_size=" << pcr_dat_options.source_voxel_size
              << " voxel_size=" << pcr_dat_options.voxel_size
              << " distribution_min_points="
              << pcr_dat_options.distribution_min_points
              << " distance_sigma=" << pcr_dat_options.distance_sigma
              << std::endl;
    results.push_back(runPcrDat(pcd_dirs, gt, pcr_dat_options));
  }

  if (isMethodEnabled(selected_methods, "small_mighty")) {
    std::cout << "Running Small-but-Mighty..." << std::endl;
    std::cout << "  source_voxel_size=" << small_mighty_options.source_voxel_size
              << " voxel_size=" << small_mighty_options.voxel_size
              << " contribution_gain=" << small_mighty_options.contribution_gain
              << " edge_weight=" << small_mighty_options.edge_weight
              << std::endl;
    results.push_back(runSmallMighty(pcd_dirs, gt, small_mighty_options));
  }

  if (isMethodEnabled(selected_methods, "m_gclo")) {
    std::cout << "Running M-GCLO..." << std::endl;
    std::cout << "  source_voxel_size=" << m_gclo_options.source_voxel_size
              << " voxel_size=" << m_gclo_options.voxel_size
              << " ground_weight=" << m_gclo_options.ground_weight
              << " ground_normal_threshold="
              << m_gclo_options.ground_normal_threshold << std::endl;
    results.push_back(runMGclo(pcd_dirs, gt, m_gclo_options));
  }

  if (isMethodEnabled(selected_methods, "quadric_lo")) {
    std::cout << "Running Quadric-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << quadric_lo_options.source_voxel_size
              << " voxel_size=" << quadric_lo_options.voxel_size
              << " quadric_min_neighbors="
              << quadric_lo_options.quadric_min_neighbors
              << " quadric_weight=" << quadric_lo_options.quadric_weight
              << std::endl;
    results.push_back(runQuadricLo(pcd_dirs, gt, quadric_lo_options));
  }

  if (isMethodEnabled(selected_methods, "dilo")) {
    std::cout << "Running DiLO..." << std::endl;
    std::cout << "  source_voxel_size=" << dilo_options.source_voxel_size
              << " sri=" << dilo_options.sri_height << "x"
              << dilo_options.sri_width
              << " keyframe_translation=" << dilo_options.keyframe_translation
              << std::endl;
    results.push_back(runDilo(pcd_dirs, gt, dilo_options));
  }

  if (isMethodEnabled(selected_methods, "nhc_lio")) {
    std::cout << "Running NHC-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << nhc_lio_options.source_voxel_size
              << " voxel_size=" << nhc_lio_options.voxel_size
              << " nhc_weight=" << nhc_lio_options.nhc_weight
              << " enable_nhc=" << nhc_lio_options.enable_nhc << std::endl;
    results.push_back(runNhcLio(pcd_dirs, gt, nhc_lio_options));
  }

  if (isMethodEnabled(selected_methods, "student_t_lo")) {
    std::cout << "Running Student-T-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << student_t_lo_options.source_voxel_size
              << " voxel_size=" << student_t_lo_options.voxel_size
              << " dof=" << student_t_lo_options.student_t_dof
              << " enable_student_t=" << student_t_lo_options.enable_student_t
              << std::endl;
    results.push_back(runStudentTLo(pcd_dirs, gt, student_t_lo_options));
  }

  if (isMethodEnabled(selected_methods, "spectral_lo")) {
    std::cout << "Running Spectral-LO..." << std::endl;
    std::cout << "  bev_size=" << spectral_lo_options.bev_size
              << " bev_range=" << spectral_lo_options.bev_range
              << " max_yaw_deg=" << spectral_lo_options.max_yaw_deg << std::endl;
    results.push_back(runSpectralLo(pcd_dirs, gt, spectral_lo_options));
  }

  if (isMethodEnabled(selected_methods, "gmm_lo")) {
    std::cout << "Running GMM-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << gmm_lo_options.source_voxel_size
              << " voxel_size=" << gmm_lo_options.voxel_size
              << " sigma_init=" << gmm_lo_options.sigma_init
              << " sigma_final=" << gmm_lo_options.sigma_final << std::endl;
    results.push_back(runGmmLo(pcd_dirs, gt, gmm_lo_options));
  }

  if (isMethodEnabled(selected_methods, "gnc_lo")) {
    std::cout << "Running GNC-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << gnc_lo_options.source_voxel_size
              << " voxel_size=" << gnc_lo_options.voxel_size
              << " gnc_truncation=" << gnc_lo_options.gnc_truncation
              << " gnc_factor=" << gnc_lo_options.gnc_factor << std::endl;
    results.push_back(runGncLo(pcd_dirs, gt, gnc_lo_options));
  }

  if (isMethodEnabled(selected_methods, "mcc_lo")) {
    std::cout << "Running MCC-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << mcc_lo_options.source_voxel_size
              << " voxel_size=" << mcc_lo_options.voxel_size
              << " mcc_sigma=" << mcc_lo_options.mcc_sigma
              << " adaptive_sigma=" << mcc_lo_options.mcc_adaptive_sigma << std::endl;
    results.push_back(runMccLo(pcd_dirs, gt, mcc_lo_options));
  }

  if (isMethodEnabled(selected_methods, "imls_slam")) {
    std::cout << "Running IMLS-SLAM..." << std::endl;
    std::cout << "  source_voxel_size=" << imls_slam_options.source_voxel_size
              << " voxel_size=" << imls_slam_options.voxel_size
              << " imls_h=" << imls_slam_options.imls_h
              << " sampling=" << imls_slam_options.use_observability_sampling
              << std::endl;
    results.push_back(runImlsSlam(pcd_dirs, gt, imls_slam_options));
  }

  if (isMethodEnabled(selected_methods, "mesh_loam")) {
    std::cout << "Running Mesh-LOAM..." << std::endl;
    std::cout << "  source_voxel_size=" << mesh_loam_options.source_voxel_size
              << " voxel_size=" << mesh_loam_options.voxel_size
              << " mesh_interval=" << mesh_loam_options.mesh_update_interval
              << " cos_gate=" << mesh_loam_options.cos_gate << std::endl;
    results.push_back(runMeshLoam(pcd_dirs, gt, mesh_loam_options));
  }

  if (isMethodEnabled(selected_methods, "elo")) {
    std::cout << "Running ELO..." << std::endl;
    std::cout << "  source_voxel_size=" << elo_options.source_voxel_size
              << " max_source_points=" << elo_options.max_source_points
              << " sri=" << elo_options.sri_width << "x" << elo_options.sri_height
              << " bev_res=" << elo_options.bev_resolution
              << " ground_weight=" << elo_options.ground_weight << std::endl;
    results.push_back(runElo(pcd_dirs, gt, elo_options));
  }

  if (isMethodEnabled(selected_methods, "tc_lvgf")) {
    std::cout << "Running TC-LVGF..." << std::endl;
    std::cout << "  source_voxel_size=" << tc_lvgf_options.source_voxel_size
              << " voxel_size=" << tc_lvgf_options.voxel_size
              << " image=" << tc_lvgf_options.image_width << "x"
              << tc_lvgf_options.image_height
              << " line_weight=" << tc_lvgf_options.line_weight
              << " direction_weight=" << tc_lvgf_options.direction_weight
              << " min_visual_lines=" << tc_lvgf_options.min_visual_lines
              << std::endl;
    results.push_back(runTcLvgf(pcd_dirs, gt, tc_lvgf_options));
  }

  if (isMethodEnabled(selected_methods, "opl_lvio")) {
    std::cout << "Running OPL-LVIO..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << opl_lvio_options.source_voxel_size
              << " voxel_size=" << opl_lvio_options.voxel_size
              << " image=" << opl_lvio_options.image_width << "x"
              << opl_lvio_options.image_height
              << " visual_point_weight=" << opl_lvio_options.visual_point_weight
              << " line_weight=" << opl_lvio_options.line_weight
              << " direction_weight=" << opl_lvio_options.direction_weight
              << " min_visual_points=" << opl_lvio_options.min_visual_points
              << std::endl;
    results.push_back(runOplLvio(pcd_dirs, gt, opl_lvio_options));
  }

  if (isMethodEnabled(selected_methods, "v_loam15")) {
    std::cout << "Running V-LOAM2015..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << v_loam15_options.source_voxel_size
              << " voxel_size=" << v_loam15_options.voxel_size
              << " image=" << v_loam15_options.image_width << "x"
              << v_loam15_options.image_height << std::endl;
    results.push_back(runVisualLidarAdapter<
                      localization_zoo::v_loam15::VLoam15Pipeline,
                      localization_zoo::v_loam15::VLoam15Params>(
        pcd_dirs, gt, v_loam15_options, "V-LOAM2015", "V-LOAM2015",
        "V-LOAM2015: visual-lidar odometry port with range-image "
        "pseudo-visual bootstrapping and LiDAR scan-to-map refinement; "
        "global mapping/loop closure are out of scope on KITTI PCD."));
  }

  if (isMethodEnabled(selected_methods, "tc_vlo")) {
    std::cout << "Running TC-VLO..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << tc_vlo_options.source_voxel_size
              << " voxel_size=" << tc_vlo_options.voxel_size
              << " image=" << tc_vlo_options.image_width << "x"
              << tc_vlo_options.image_height << std::endl;
    results.push_back(runVisualLidarAdapter<
                      localization_zoo::tc_vlo::TcVloPipeline,
                      localization_zoo::tc_vlo::TcVloParams>(
        pcd_dirs, gt, tc_vlo_options, "TC-VLO", "TC-VLO",
        "TC-VLO: tightly coupled vision-lidar odometry port with separate "
        "visual point and LiDAR voxel maps jointly optimized through "
        "plane, point, and line residuals."));
  }

  if (isMethodEnabled(selected_methods, "ad_vlo")) {
    std::cout << "Running AD-VLO..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << ad_vlo_options.source_voxel_size
              << " voxel_size=" << ad_vlo_options.voxel_size
              << " image=" << ad_vlo_options.image_width << "x"
              << ad_vlo_options.image_height << std::endl;
    results.push_back(runVisualLidarAdapter<
                      localization_zoo::ad_vlo::AdVloPipeline,
                      localization_zoo::ad_vlo::AdVloParams>(
        pcd_dirs, gt, ad_vlo_options, "AD-VLO", "AD-VLO",
        "AD-VLO: direct visual-laser odometry port with occlusion-aware "
        "range discontinuity gating, planar patch emphasis, and LiDAR "
        "scan-to-map refinement."));
  }

  if (isMethodEnabled(selected_methods, "tc_mvlo")) {
    std::cout << "Running TC-MVLO..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << tc_mvlo_options.source_voxel_size
              << " voxel_size=" << tc_mvlo_options.voxel_size
              << " image=" << tc_mvlo_options.image_width << "x"
              << tc_mvlo_options.image_height << std::endl;
    results.push_back(runVisualLidarAdapter<
                      localization_zoo::tc_mvlo::TcMvloPipeline,
                      localization_zoo::tc_mvlo::TcMvloParams>(
        pcd_dirs, gt, tc_mvlo_options, "TC-MVLO", "TC-MVLO",
        "TC-MVLO: tightly coupled monocular visual-lidar odometry port "
        "using range-image visual points, fused visual/LiDAR line support, "
        "and LiDAR scan-to-map constraints; loop closure is out of scope."));
  }

  if (isMethodEnabled(selected_methods, "tricp_lo")) {
    std::cout << "Running TrICP-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << tricp_lo_options.source_voxel_size
              << " voxel_size=" << tricp_lo_options.voxel_size
              << " auto_overlap=" << tricp_lo_options.auto_overlap
              << " lambda=" << tricp_lo_options.frmsd_lambda << std::endl;
    results.push_back(runTricpLo(pcd_dirs, gt, tricp_lo_options));
  }

  if (isMethodEnabled(selected_methods, "kc_lo")) {
    std::cout << "Running KC-LO..." << std::endl;
    std::cout << "  source_voxel_size=" << kc_lo_options.source_voxel_size
              << " voxel_size=" << kc_lo_options.voxel_size
              << " kc_sigma=" << kc_lo_options.kc_sigma
              << " sigma_init=" << kc_lo_options.kc_sigma_init << std::endl;
    results.push_back(runKcLo(pcd_dirs, gt, kc_lo_options));
  }

  if (isMethodEnabled(selected_methods, "i_loam")) {
    std::cout << "Running I-LOAM..." << std::endl;
    std::cout << "  n_scans=" << i_loam_options.n_scans
              << " stride=" << i_loam_options.input_point_stride
              << " mapping=" << i_loam_options.enable_mapping
              << " intensity_weight=" << i_loam_options.use_intensity_weight
              << " intensity_corr=" << i_loam_options.use_intensity_correspondence
              << " intensity_sigma=" << i_loam_options.intensity_sigma
              << " corr_weight=" << i_loam_options.intensity_corr_weight
              << std::endl;
    results.push_back(runILoam(pcd_dirs, gt, i_loam_options));
  }
  if (isMethodEnabled(selected_methods, "pl_loam")) {
    std::cout << "\n=== PL-LOAM ===" << std::endl;
    std::cout << "  stride=" << pl_loam_options.input_stride
              << " points=" << pl_loam_options.max_point_features
              << " lines=" << pl_loam_options.max_line_features
              << " depth_prior=" << pl_loam_options.use_depth_prior
              << " line_features=" << pl_loam_options.use_line_features
              << " scale_corr=" << pl_loam_options.use_scale_correction
              << " rgb=" << pl_loam_options.use_rgb
              << std::endl;
    results.push_back(runPlLoam(pcd_dirs, gt, frame_timestamps, pl_loam_options));
  }
  if (isMethodEnabled(selected_methods, "inten_loam")) {
    std::cout << "\n=== InTEn-LOAM ===" << std::endl;
    std::cout << "  stride=" << inten_loam_options.input_stride
              << " cyl=" << inten_loam_options.cyl_width << "x"
              << inten_loam_options.cyl_height
              << " intensity_reg=" << inten_loam_options.use_intensity_registration
              << " tvf=" << inten_loam_options.enable_tvf
              << " dor=" << inten_loam_options.enable_dor
              << " mapping=" << inten_loam_options.enable_mapping
              << std::endl;
    results.push_back(runInTenLoam(pcd_dirs, gt, inten_loam_options));
  }
  if (isMethodEnabled(selected_methods, "mcgicp")) {
    std::cout << "\n=== MCGICP-LO ===" << std::endl;
    std::cout << "  source_voxel_size=" << mcgicp_options.source_voxel_size
              << " voxel_size=" << mcgicp_options.voxel_size
              << " intensity_scale=" << mcgicp_options.intensity_scale
              << " enable_intensity=" << mcgicp_options.enable_intensity
              << std::endl;
    results.push_back(runMcGicpLo(pcd_dirs, gt, mcgicp_options));
  }
  if (isMethodEnabled(selected_methods, "icpsc")) {
    std::cout << "\n=== ICPSC-LO ===" << std::endl;
    std::cout << "  source_voxel_size=" << icpsc_options.source_voxel_size
              << " cyl=" << icpsc_options.cyl_width << "x"
              << icpsc_options.cyl_height
              << " enable_intensity=" << icpsc_options.enable_intensity
              << std::endl;
    results.push_back(runIcpscLo(pcd_dirs, gt, icpsc_options));
  }
  if (isMethodEnabled(selected_methods, "vlom")) {
    std::cout << "\n=== VLOM ===" << std::endl;
    std::cout << "  stride=" << vlom_options.input_point_stride
              << " bootstrap=" << vlom_options.enable_visual_bootstrap
              << " scale_corr=" << vlom_options.enable_scale_correction
              << " rgb=" << vlom_options.use_rgb
              << std::endl;
    results.push_back(runVlom(pcd_dirs, gt, frame_timestamps, vlom_options));
  }
  if (isMethodEnabled(selected_methods, "odonet")) {
    std::cout << "\n=== OdoNet ===" << std::endl;
    std::cout << "  weights=" << odonet_options.weights_path
              << " nhc=" << odonet_options.enable_nhc
              << " zupt=" << odonet_options.enable_zupt
              << " cnn=" << odonet_options.enable_cnn_speed << std::endl;
    results.push_back(
        runOdoNet(gt, frame_timestamps, imu_samples, odonet_options));
  }
  if (isMethodEnabled(selected_methods, "nhc_net")) {
    std::cout << "\n=== NHC-Net ===" << std::endl;
    std::cout << "  weights=" << nhc_net_options.weights_path
              << " adaptive=" << nhc_net_options.adaptive_gain
              << " gain=" << nhc_net_options.nhc_gain << std::endl;
    results.push_back(
        runNhcNet(gt, frame_timestamps, imu_samples, nhc_net_options));
  }
  if (isMethodEnabled(selected_methods, "nn_zupt")) {
    std::cout << "\n=== NN-ZUPT ===" << std::endl;
    std::cout << "  weights=" << nn_zupt_options.weights_path
              << " prob=" << nn_zupt_options.stop_prob_threshold
              << " threshold_only=" << nn_zupt_options.use_threshold_detector
              << std::endl;
    results.push_back(
        runNnZupt(gt, frame_timestamps, imu_samples, nn_zupt_options));
  }

  if (isMethodEnabled(selected_methods, "degen_sense")) {
    std::cout << "Running DegenSense..." << std::endl;
    std::cout << "  source_voxel_size=" << degen_sense_options.source_voxel_size
              << " voxel_size=" << degen_sense_options.voxel_size
              << " warmup_frames=" << degen_sense_options.warmup_frames
              << " mad_k=" << degen_sense_options.mad_k
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runDegenSense(pcd_dirs, gt, frame_timestamps, imu_samples,
                                    degen_sense_options));
  }

  if (isMethodEnabled(selected_methods, "vibration_lio")) {
    std::cout << "Running Vibration-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << vibration_lio_options.source_voxel_size
              << " voxel_size=" << vibration_lio_options.voxel_size
              << " gamma=" << vibration_lio_options.gamma
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runVibrationLIO(pcd_dirs, gt, frame_timestamps,
                                      imu_samples, vibration_lio_options));
  }

  if (isMethodEnabled(selected_methods, "id_lio")) {
    std::cout << "Running ID-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << id_lio_options.source_voxel_size
              << " voxel_size=" << id_lio_options.voxel_size
              << " range_image=" << id_lio_options.range_image_width << "x"
              << id_lio_options.range_image_height
              << " dynamic_weight=" << id_lio_options.dynamic_weight
              << " delayed_removal_frames="
              << id_lio_options.delayed_removal_frames
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runIDLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               id_lio_options));
  }

  if (isMethodEnabled(selected_methods, "rf_lio")) {
    std::cout << "Running RF-LIO..." << std::endl;
    std::cout << std::setprecision(3)
              << "  source_voxel_size=" << rf_lio_options.source_voxel_size
              << " voxel_size=" << rf_lio_options.voxel_size
              << " range_image=" << rf_lio_options.range_image_width << "x"
              << rf_lio_options.range_image_height
              << " foreground_margin=" << rf_lio_options.foreground_margin
              << " max_removal_fraction="
              << rf_lio_options.max_removal_fraction
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runRFLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               rf_lio_options));
  }

  if (isMethodEnabled(selected_methods, "bievr_lio")) {
    std::cout << "Running BIEVR-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << bievr_lio_options.source_voxel_size
              << " voxel_size=" << bievr_lio_options.voxel_size
              << " pixel_res=" << bievr_lio_options.pixel_res << std::endl;
    results.push_back(runBievrLIO(pcd_dirs, gt, bievr_lio_options));
  }

  if (isMethodEnabled(selected_methods, "ua_lio")) {
    std::cout << "Running UA-LIO..." << std::endl;
    std::cout << "  source_voxel_size=" << ua_lio_options.source_voxel_size
              << " voxel_size=" << ua_lio_options.voxel_size
              << " ground_constraint=" << ua_lio_options.ground_constraint
              << " imu=" << (imu_samples.empty() ? "absent" : "present")
              << std::endl;
    results.push_back(runUALIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                               ua_lio_options));
  }

  if (isMethodEnabled(selected_methods, "cube_lio")) {
    std::cout << "Running CUBE-LIO..." << std::endl;
    std::cout << "  input_voxel_size=" << cube_lio_options.input_voxel_size
              << " voxel_size=" << cube_lio_options.voxel_size
              << " cubemap_size=" << cube_lio_options.cubemap_size
              << " semi_dense_quantile=" << cube_lio_options.semi_dense_quantile
              << " intensity_sigma=" << cube_lio_options.intensity_sigma
              << std::endl;
    results.push_back(runCubeLIO(pcd_dirs, gt, cube_lio_options));
  }

  if (isMethodEnabled(selected_methods, "dlo")) {
    std::cout << "Running DLO..." << std::endl;
    std::cout << "  input_voxel_size=" << dlo_options.input_voxel_size
              << " max_input_points=" << dlo_options.max_input_points
              << " reg_voxel=" << dlo_options.registration_voxel_size
              << " map_voxel=" << dlo_options.map_voxel_size
              << " max_keyframes=" << dlo_options.max_keyframes_in_map
              << " gicp_iters=" << dlo_options.gicp_max_iterations << std::endl;
    results.push_back(runDLO(pcd_dirs, gt, dlo_options));
  }

  if (isMethodEnabled(selected_methods, "dlio")) {
    std::cout << "Running DLIO..." << std::endl;
    std::cout << "  input_voxel_size=" << dlio_options.input_voxel_size
              << " max_input_points=" << dlio_options.max_input_points
              << " reg_voxel=" << dlio_options.registration_voxel_size
              << " map_voxel=" << dlio_options.map_voxel_size
              << " imu_rot_w=" << dlio_options.imu_rotation_fusion_weight
              << " imu_trans_w=" << dlio_options.imu_translation_fusion_weight
              << std::endl;
    results.push_back(runDLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                              dlio_options));
  }

  constexpr double kCTLIORecommendedMedianGapSec = 0.5;
  if (isMethodEnabled(selected_methods, "ct_lio")) {
    if (imu_samples.empty()) {
      results.push_back(makeSkippedResult(
          "CT-LIO", "imu.csv not found. CT-LIO requires synchronized IMU samples."));
    } else if (!force_ct_lio &&
               frame_timestamp_source != FrameTimestampSource::kSampledGT &&
               frame_gap_stats.valid &&
               frame_gap_stats.median_gap > kCTLIORecommendedMedianGapSec) {
      std::ostringstream oss;
      oss << "median frame gap " << std::fixed << std::setprecision(3)
          << frame_gap_stats.median_gap << " s exceeds CT-LIO threshold "
          << kCTLIORecommendedMedianGapSec
          << " s. Input looks like sparse graph keyframes/submaps, not raw scans. "
          << "Use a dense scan sequence or pass --force-ct-lio to override.";
      results.push_back(makeSkippedResult("CT-LIO", oss.str()));
      std::cout << "Skipping CT-LIO: " << oss.str() << std::endl;
    } else {
      std::cout << "Running CT-LIO..." << std::endl;
      if (ct_lio_fixed_lag_window > 1) {
        std::cout << "  fixed-lag window=" << ct_lio_fixed_lag_window
                  << " velocity_weight=" << ct_lio_fixed_lag_velocity_weight
                  << " gyro_bias_scale=" << ct_lio_fixed_lag_gyro_bias_scale
                  << " accel_bias_scale=" << ct_lio_fixed_lag_accel_bias_scale
                  << " history_decay=" << ct_lio_fixed_lag_history_decay
                  << " outer_iterations=" << ct_lio_fixed_lag_outer_iterations
                  << " smoother="
                  << (ct_lio_fixed_lag_smoother ? "on" : "off")
                  << std::endl;
      }
      results.push_back(
          runCTLIO(pcd_dirs, gt, frame_timestamps, imu_samples,
                   ct_lio_estimate_bias, ct_lio_fixed_lag_window,
                   ct_lio_fixed_lag_velocity_weight,
                   ct_lio_fixed_lag_gyro_bias_scale,
                   ct_lio_fixed_lag_accel_bias_scale,
                   ct_lio_fixed_lag_history_decay,
                   ct_lio_fixed_lag_outer_iterations,
                   ct_lio_fixed_lag_smoother,
                   ct_lio_multi_scale,
                   ct_lio_coarse_to_fine,
                   ct_lio_coarse_iterations,
                   ct_lio_coarse_search_radius,
                   ct_lio_coarse_planarity_threshold,
                   ct_lio_coarse_cauchy_mult,
                   ct_lio_max_frames_in_map,
                   ct_lio_max_iterations,
                   ct_lio_bspline,
                   ct_lio_bspline_anchor_weight));
    }
  }

  if (isMethodEnabled(selected_methods, "ct_icp")) {
    std::cout << "Running CT-ICP..." << std::endl;
    std::cout << "  source_voxel_size=" << ct_icp_options.source_voxel_size
              << " max_source_points=" << ct_icp_options.max_source_points
              << " voxel_resolution=" << ct_icp_options.voxel_resolution
              << " max_iterations=" << ct_icp_options.max_iterations
              << " max_frames_in_map=" << ct_icp_options.max_frames_in_map
              << std::endl;
    results.push_back(runCTICP(pcd_dirs, gt, ct_icp_options, ct_icp_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "ct_icp_ndt")) {
    std::cout << "Running CT-ICP+NDT..." << std::endl;
    std::cout << "  ct_icp_voxel_resolution=" << ct_icp_options.voxel_resolution
              << " ct_icp_max_iterations=" << ct_icp_options.max_iterations
              << " ndt_resolution=" << ndt_options.resolution
              << " ndt_max_iterations=" << ndt_options.max_iterations
              << std::endl;
    results.push_back(runCTICPNDT(pcd_dirs, gt, ct_icp_options, ndt_options));
  }

  if (isMethodEnabled(selected_methods, "ct_icp_ndt_keyframe")) {
    std::cout << "Running CT-ICP+NDT keyframe..." << std::endl;
    std::cout << "  ct_icp_voxel_resolution=" << ct_icp_options.voxel_resolution
              << " ct_icp_max_iterations=" << ct_icp_options.max_iterations
              << " ndt_resolution=" << ndt_options.resolution
              << " ndt_max_iterations=" << ndt_options.max_iterations
              << " keyframe_interval="
              << ct_icp_ndt_options.keyframe_interval
              << " max_correction_translation="
              << ct_icp_ndt_options.max_correction_translation_delta
              << " max_correction_rotation_rad="
              << ct_icp_ndt_options.max_correction_rotation_delta_rad
              << std::endl;
    results.push_back(runCTICPNDTKeyframe(
        pcd_dirs, gt, ct_icp_options, ndt_options, ct_icp_ndt_options));
  }

  if (isMethodEnabled(selected_methods, "xicp")) {
    std::cout << "Running X-ICP..." << std::endl;
    results.push_back(runXICP(pcd_dirs, gt, xicp_options, no_gt_seed));
  }

  if (isMethodEnabled(selected_methods, "fast_lio2")) {
    std::cout << "Running FAST-LIO2..." << std::endl;
    results.push_back(runFastLio2(pcd_dirs, gt, frame_timestamps, imu_samples, fast_lio2_options));
  }

  if (isMethodEnabled(selected_methods, "hdl_graph_slam")) {
    std::cout << "Running HDL-Graph-SLAM..." << std::endl;
    results.push_back(runHdlGraphSlam(pcd_dirs, gt, hdl_graph_slam_options));
  }

  if (isMethodEnabled(selected_methods, "vgicp_slam")) {
    std::cout << "Running VGICP-SLAM..." << std::endl;
    results.push_back(runVgicpSlam(pcd_dirs, gt, vgicp_slam_options));
  }

  if (isMethodEnabled(selected_methods, "suma")) {
    std::cout << "Running SuMa..." << std::endl;
    results.push_back(runSuMa(pcd_dirs, gt, suma_options));
  }

  if (isMethodEnabled(selected_methods, "balm2")) {
    std::cout << "Running BALM2..." << std::endl;
    results.push_back(runBalm2(pcd_dirs, gt, balm2_options));
  }

  if (isMethodEnabled(selected_methods, "isc_loam")) {
    std::cout << "Running ISC-LOAM..." << std::endl;
    results.push_back(runIscLoam(pcd_dirs, gt, isc_loam_options));
  }

  if (isMethodEnabled(selected_methods, "loam_livox")) {
    std::cout << "Running LOAM-Livox..." << std::endl;
    results.push_back(runLoamLivox(pcd_dirs, gt, loam_livox_options));
  }

  if (isMethodEnabled(selected_methods, "lio_sam")) {
    std::cout << "Running LIO-SAM..." << std::endl;
    results.push_back(runLioSam(pcd_dirs, gt, frame_timestamps, imu_samples,
                                lio_sam_options));
  }

  if (isMethodEnabled(selected_methods, "lins")) {
    std::cout << "Running LINS..." << std::endl;
    results.push_back(runLINS(pcd_dirs, gt, frame_timestamps, imu_samples,
                              lins_options));
  }

  if (isMethodEnabled(selected_methods, "fast_lio_slam")) {
    std::cout << "Running FAST-LIO-SLAM..." << std::endl;
    results.push_back(runFastLioSlam(pcd_dirs, gt, frame_timestamps, imu_samples,
                                     fast_lio_slam_options));
  }

  if (isMethodEnabled(selected_methods, "point_lio")) {
    std::cout << "Running Point-LIO..." << std::endl;
    results.push_back(runPointLio(pcd_dirs, gt, frame_timestamps, imu_samples,
                                  point_lio_options));
  }

  if (isMethodEnabled(selected_methods, "clins")) {
    if (imu_samples.empty()) {
      results.push_back(makeSkippedResult(
          "CLINS", "imu.csv not found. CLINS requires synchronized IMU samples."));
    } else {
      std::cout << "Running CLINS..." << std::endl;
      results.push_back(runCLINS(pcd_dirs, gt, frame_timestamps, imu_samples,
                                 clins_options));
    }
  }

  // 結果表示
  std::cout << "\n========================================" << std::endl;
  std::cout << "  RESULTS (Real Data)" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << std::left
            << std::setw(24) << "Method"
            << std::setw(12) << "Status"
            << std::setw(12) << "ATE [m]"
            << std::setw(15) << "Drift[m/100m]"
            << std::setw(12) << "Frames"
            << std::setw(15) << "Time [ms]"
            << std::setw(12) << "FPS"
            << std::endl;
  std::cout << std::string(102, '-') << std::endl;

  fs::create_directories("dogfooding_results");
  savePosesKITTI(gt, "dogfooding_results/gt.txt");

  for (auto& r : results) {
    if (r.skipped) {
      std::cout << std::setw(24) << r.name
                << std::setw(12) << r.status
                << std::setw(12) << "-"
                << std::setw(15) << "-"
                << std::setw(12) << "-"
                << std::setw(15) << "-"
                << std::setw(12) << "-"
                << std::endl;
      continue;
    }

    r.ate = computeATE(r.poses, gt);
    const RPEMetrics rpe = computeRPE(r.poses, gt, rpeSegmentLengthM(dist));
    r.has_rpe = rpe.available;
    if (rpe.available) {
      r.rpe_trans_pct = rpe.trans_pct;
      r.rpe_rot_deg_per_m = rpe.rot_deg_per_m;
    }
    if (!std::isfinite(r.ate) ||
        (r.has_rpe && (!std::isfinite(r.rpe_trans_pct) ||
                       !std::isfinite(r.rpe_rot_deg_per_m)))) {
      r.status = "invalid_metric";
    }
    double fps = r.time_ms > 0.0 ? r.poses.size() / (r.time_ms / 1000.0) : 0.0;
    std::ostringstream drift_ss;
    if (r.has_rpe && std::isfinite(r.rpe_trans_pct)) {
      drift_ss << std::fixed << std::setprecision(3) << r.rpe_trans_pct;
    } else {
      drift_ss << "-";
    }
    std::cout << std::setw(24) << r.name
              << std::setw(12) << r.status
              << std::setw(12) << std::setprecision(3) << r.ate
              << std::setw(15) << drift_ss.str()
              << std::setw(12) << r.poses.size()
              << std::setw(15) << std::setprecision(1) << r.time_ms
              << std::setw(12) << std::setprecision(1) << fps
              << std::endl;

    std::string fname = r.name;
    std::replace(fname.begin(), fname.end(), '-', '_');
    savePosesKITTI(r.poses, "dogfooding_results/" + fname + ".txt");
  }

  bool printed_notes = false;
  for (const auto& r : results) {
    if (!r.note.empty()) {
      if (!printed_notes) {
        std::cout << "\nNotes:" << std::endl;
        printed_notes = true;
      }
      std::cout << "- " << r.name << ": " << r.note << std::endl;
    }
  }

  if (!summary_json_path.empty()) {
    writeSummaryJson(summary_json_path, pcd_dir, gt_csv, pcd_dirs.size(), dist,
                     frame_timestamp_source, results);
  }

  std::cout << "\nResults saved to dogfooding_results/" << std::endl;
  return 0;
}
