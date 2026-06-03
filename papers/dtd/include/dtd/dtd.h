#pragma once

#include <Eigen/Core>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace localization_zoo {
namespace dtd {

/// 三角形ディスクリプタ (DTD)。3 keypoint の辺長 (回転/並進不変) と
/// 各頂点近傍のエントロピー (局所記述) からなる。
struct TriangleDescriptor {
  Eigen::Vector3d p[3];     ///< 頂点 3D 座標 (昇順の辺対応に並べ替え済み)
  double side[3] = {0, 0, 0};  ///< 昇順の辺長 l_min, l_mid, l_max
  double entropy[3] = {0, 0, 0};  ///< 対応頂点近傍のエントロピー
  int frame = -1;
};

struct DTDParams {
  double projection_range = 50.0;  ///< BEV 投影半径 L [m]
  int grid_size = 300;             ///< 密度画像 M×M
  double density_threshold = 0.05; ///< σ_k (地面/ノイズ抑制)
  int max_keypoints = 150;         ///< GFTT 近似で選ぶ keypoint 数
  double dedup_radius = 2.0;        ///< keypoint 非極大抑制半径 [m] (重複柱の統合)
  double min_side = 2.0;           ///< 三角形辺長の下限 [m]
  double max_side = 30.0;          ///< 三角形辺長の上限 [m]
  int knn_triangle = 4;            ///< 各 keypoint から三角形を作る近傍数
  double side_hash_res = 1.0;      ///< 辺長ハッシュの量子化 [m]
  double entropy_sim_threshold = 0.90;  ///< σ_s (エントロピー類似度)
  double verify_dist = 1.0;        ///< σ_d 再投影誤差しきい [m]
  int min_inliers = 4;             ///< σ_n ループ確定の最小インライア
};

/// keypoint: 3D 座標 + 近傍エントロピー。
struct Keypoint {
  Eigen::Vector3d position = Eigen::Vector3d::Zero();
  double entropy = 0.0;
};

struct LoopResult {
  bool detected = false;
  int matched_frame = -1;
  int inliers = 0;
  Eigen::Matrix3d rotation = Eigen::Matrix3d::Identity();
  Eigen::Vector3d translation = Eigen::Vector3d::Zero();
};

/// DTD: Density Triangle Descriptor によるループ閉じ検出器
/// (arXiv/PMC 2025, 著者コード未公開の from-paper 実装)。
class DTDDetector {
public:
  explicit DTDDetector(const DTDParams& params = DTDParams());

  /// 1 スキャン (センサ系点群) からディスクリプタを構築する。
  std::vector<TriangleDescriptor> describe(
      const std::vector<Eigen::Vector3d>& cloud, int frame) const;

  /// ディスクリプタをデータベースへ追加 (ハッシュ登録)。
  void addToDatabase(const std::vector<TriangleDescriptor>& descs);

  /// 現フレームのディスクリプタでループ候補を検索・幾何検証する。
  /// @param min_frame_gap これより新しい (frame 差が小さい) 候補は無視。
  LoopResult detectLoop(const std::vector<TriangleDescriptor>& query,
                        int current_frame, int min_frame_gap) const;

  /// keypoint 抽出 (BEV 密度画像 + 局所分散の大きいセル)。テスト/可視化用に公開。
  std::vector<Keypoint> extractKeypoints(
      const std::vector<Eigen::Vector3d>& cloud) const;

  size_t databaseSize() const { return database_.size(); }

private:
  std::uint64_t hashKey(const TriangleDescriptor& d) const;

  DTDParams params_;
  std::vector<TriangleDescriptor> database_;
  std::unordered_map<std::uint64_t, std::vector<int>> hash_table_;  ///< key→db index
};

}  // namespace dtd
}  // namespace localization_zoo
