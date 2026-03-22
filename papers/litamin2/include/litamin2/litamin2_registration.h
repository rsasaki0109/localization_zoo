#pragma once

#include "litamin2/gaussian_voxel_map.h"

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vector>
#include <memory>

namespace localization_zoo {
namespace litamin2 {

/// LiTAMIN2 レジストレーション結果
struct RegistrationResult {
  Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
  int num_iterations = 0;
  double final_error = 0.0;
  bool converged = false;
};

/// LiTAMIN2 レジストレーション パラメータ
struct LiTAMIN2Params {
  double voxel_resolution = 3.0;        // ボクセルサイズ [m]
  int min_points_per_voxel = 3;         // ボクセルあたり最小点数
  int max_iterations = 64;              // 最大反復回数
  double rotation_epsilon = 2e-3;       // 回転の収束閾値
  double translation_epsilon = 5e-4;    // 並進の収束閾値
  double lambda = 1e-6;                 // Frobenius正規化の正則化パラメータ
  double sigma_icp = 0.5;              // ICPロバスト重みの閾値
  double sigma_cov = 3.0;              // 共分散ロバスト重みの閾値
  bool use_cov_cost = true;            // 共分散コストを使用するか
  int num_threads = 1;                 // OpenMPスレッド数
};

/// LiTAMIN2 点群レジストレーション
///
/// 対称KLダイバージェンスに基づくICPコスト関数で
/// 正規分布ボクセル同士のアライメントを行う
class LiTAMIN2Registration {
public:
  explicit LiTAMIN2Registration(const LiTAMIN2Params& params = LiTAMIN2Params())
      : params_(params) {}

  /// ターゲット点群を設定 (ボクセルマップを構築)
  void setTarget(const std::vector<Eigen::Vector3d>& target_points);

  /// ソース点群をターゲットにアライメント
  /// @param source_points ソース点群
  /// @param initial_guess 初期変換行列 (4x4)
  /// @return レジストレーション結果
  RegistrationResult align(const std::vector<Eigen::Vector3d>& source_points,
                           const Eigen::Matrix4d& initial_guess =
                               Eigen::Matrix4d::Identity());

  const LiTAMIN2Params& params() const { return params_; }
  LiTAMIN2Params& params() { return params_; }

private:
  /// ソース点群のボクセルマップを構築
  void buildSourceVoxelMap(const std::vector<Eigen::Vector3d>& source_points);

  /// 対応関係を更新
  /// @return 対応するsource/targetボクセルのペア
  std::vector<std::pair<std::shared_ptr<GaussianVoxel>,
                        std::shared_ptr<GaussianVoxel>>>
  updateCorrespondences(const Eigen::Matrix3d& R,
                        const Eigen::Vector3d& t) const;

  /// コスト関数を線形化し、ガウスニュートンの H, b を計算
  /// @return 合計エラー
  double linearize(
      const Eigen::Matrix3d& R, const Eigen::Vector3d& t,
      const std::vector<std::pair<std::shared_ptr<GaussianVoxel>,
                                  std::shared_ptr<GaussianVoxel>>>& corrs,
      Eigen::Matrix<double, 6, 6>& H, Eigen::Matrix<double, 6, 1>& b) const;

  /// 収束判定
  bool isConverged(const Eigen::Matrix<double, 6, 1>& delta) const;

  LiTAMIN2Params params_;
  std::unique_ptr<GaussianVoxelMap> target_voxel_map_;
  std::unique_ptr<GaussianVoxelMap> source_voxel_map_;
};

}  // namespace litamin2
}  // namespace localization_zoo
