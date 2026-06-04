#include "dtd/dtd.h"

#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <array>
#include <cmath>

namespace localization_zoo {
namespace dtd {

namespace {

// セル統計: 点数 + 1次/2次モーメント (エントロピー計算用)。
struct CellStat {
  int count = 0;
  Eigen::Vector3d sum = Eigen::Vector3d::Zero();
  Eigen::Matrix3d sum_outer = Eigen::Matrix3d::Zero();
};

}  // namespace

DTDDetector::DTDDetector(const DTDParams& params) : params_(params) {}

std::vector<Keypoint> DTDDetector::extractKeypoints(
    const std::vector<Eigen::Vector3d>& cloud) const {
  const int M = params_.grid_size;
  const double L = params_.projection_range;
  const double cell = 2.0 * L / M;
  std::unordered_map<int, CellStat> cells;  // key = v*M+u

  int cmax = 0;
  for (const auto& p : cloud) {
    if (std::abs(p.x()) >= L || std::abs(p.y()) >= L) continue;
    const int u = static_cast<int>((p.x() + L) / cell);
    const int v = static_cast<int>((p.y() + L) / cell);
    if (u < 0 || u >= M || v < 0 || v >= M) continue;
    auto& c = cells[v * M + u];
    c.count += 1;
    c.sum += p;
    c.sum_outer += p * p.transpose();
    cmax = std::max(cmax, c.count);
  }
  if (cmax == 0) return {};

  // 密度しきい (σ_k) 以上のセルを候補にし、近傍密度差 (局所構造変化) で
  // GFTT を近似したスコアで上位を keypoint とする。
  const int thr = static_cast<int>(params_.density_threshold * cmax);
  auto countAt = [&](int u, int v) {
    auto it = cells.find(v * M + u);
    return it == cells.end() ? 0 : it->second.count;
  };

  std::vector<std::pair<double, int>> scored;  // (score, key)
  for (const auto& kv : cells) {
    if (kv.second.count < thr || kv.second.count < 3) continue;
    const int key = kv.first;
    const int u = key % M, v = key / M;
    // 局所コントラスト = 中央 - 周囲平均 (Good-Features-To-Track 近似)。
    double around = 0.0;
    int na = 0;
    for (int du = -1; du <= 1; du++)
      for (int dv = -1; dv <= 1; dv++) {
        if (du == 0 && dv == 0) continue;
        around += countAt(u + du, v + dv);
        ++na;
      }
    const double score = kv.second.count - (na ? around / na : 0.0);
    scored.emplace_back(score, key);
  }
  std::sort(scored.begin(), scored.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

  // スコア順に走査し、非極大抑制 (dedup_radius 内の重複を統合) しながら採用。
  std::vector<Keypoint> kps;
  const double dedup_sq = params_.dedup_radius * params_.dedup_radius;
  for (const auto& sc_key : scored) {
    if (static_cast<int>(kps.size()) >= params_.max_keypoints) break;
    const auto& c = cells[sc_key.second];
    Keypoint kp;
    kp.position = c.sum / c.count;  // セル内 3D 重心
    bool dup = false;
    for (const auto& k : kps)
      if ((k.position - kp.position).squaredNorm() < dedup_sq) { dup = true; break; }
    if (dup) continue;
    // エントロピー H = ½ log((2πe)^3 |Σ|)。
    Eigen::Matrix3d cov =
        c.sum_outer / c.count - kp.position * kp.position.transpose();
    cov = 0.5 * (cov + cov.transpose()) + 1e-6 * Eigen::Matrix3d::Identity();
    const double det = std::max(cov.determinant(), 1e-12);
    kp.entropy = 0.5 * std::log(std::pow(2.0 * M_PI * M_E, 3) * det);
    kps.push_back(kp);
  }
  return kps;
}

std::vector<TriangleDescriptor> DTDDetector::describe(
    const std::vector<Eigen::Vector3d>& cloud, int frame) const {
  const auto kps = extractKeypoints(cloud);
  std::vector<TriangleDescriptor> descs;
  if (kps.size() < 3) return descs;

  // 各 keypoint に対し最近傍 knn を求め、三角形を構成する。
  const int n = static_cast<int>(kps.size());
  for (int i = 0; i < n; i++) {
    // 近傍 knn を距離で選ぶ (簡易 KD-tree 代替)。
    std::vector<std::pair<double, int>> nbr;
    for (int j = 0; j < n; j++) {
      if (j == i) continue;
      const double d = (kps[i].position - kps[j].position).norm();
      if (d >= params_.min_side && d <= params_.max_side) nbr.emplace_back(d, j);
    }
    std::sort(nbr.begin(), nbr.end());
    const int kk = std::min<int>(params_.knn_triangle, nbr.size());
    for (int a = 0; a < kk; a++)
      for (int b = a + 1; b < kk; b++) {
        const int j = nbr[a].second, k = nbr[b].second;
        // i<j<k のときだけ作り重複三角形を排除。
        if (!(i < j && i < k)) continue;
        const double ljk = (kps[j].position - kps[k].position).norm();
        if (ljk < params_.min_side || ljk > params_.max_side) continue;

        // 辺長を昇順に並べ、対応頂点も並べ替える (回転/並進不変な正準形)。
        // 辺 l_ij(頂点k対辺), l_ik(頂点j対辺), l_jk(頂点i対辺)。
        struct VE { double opp_side; int vid; };
        // 各頂点の "対辺" を計算: 頂点 i の対辺 = jk, j の対辺 = ik, k の対辺 = ij。
        const double l_ij = nbr[a].first;  // = dist(i,j)
        const double l_ik = nbr[b].first;  // = dist(i,k)
        std::array<VE, 3> ve = {VE{ljk, i}, VE{l_ik, j}, VE{l_ij, k}};
        std::sort(ve.begin(), ve.end(),
                  [](const VE& x, const VE& y) { return x.opp_side < y.opp_side; });

        TriangleDescriptor d;
        for (int t = 0; t < 3; t++) {
          d.p[t] = kps[ve[t].vid].position;
          d.entropy[t] = kps[ve[t].vid].entropy;
          d.side[t] = ve[t].opp_side;
        }
        d.frame = frame;
        descs.push_back(d);
      }
  }
  return descs;
}

std::uint64_t DTDDetector::hashKey(const TriangleDescriptor& d) const {
  const double r = params_.side_hash_res;
  const std::uint64_t a = static_cast<std::uint64_t>(d.side[0] / r);
  const std::uint64_t b = static_cast<std::uint64_t>(d.side[1] / r);
  const std::uint64_t c = static_cast<std::uint64_t>(d.side[2] / r);
  return (a * 73856093ULL) ^ (b * 19349663ULL) ^ (c * 83492791ULL);
}

void DTDDetector::addToDatabase(const std::vector<TriangleDescriptor>& descs) {
  for (const auto& d : descs) {
    const int idx = static_cast<int>(database_.size());
    database_.push_back(d);
    hash_table_[hashKey(d)].push_back(idx);
  }
}

LoopResult DTDDetector::detectLoop(const std::vector<TriangleDescriptor>& query,
                                   int current_frame, int min_frame_gap) const {
  LoopResult result;
  // フレーム別の投票 + マッチペア収集。
  std::unordered_map<int, int> votes;
  std::unordered_map<int, std::vector<std::pair<int, int>>> matches;  // frame→(qi,dbidx)

  for (int qi = 0; qi < static_cast<int>(query.size()); qi++) {
    const auto& q = query[qi];
    auto it = hash_table_.find(hashKey(q));
    if (it == hash_table_.end()) continue;
    for (int dbidx : it->second) {
      const auto& cand = database_[dbidx];
      if (current_frame - cand.frame < min_frame_gap) continue;
      // 辺長一致 (ハッシュ近傍内の最終確認)。
      bool side_ok = true;
      for (int t = 0; t < 3; t++)
        if (std::abs(q.side[t] - cand.side[t]) > params_.side_hash_res)
          side_ok = false;
      if (!side_ok) continue;
      // エントロピーベクトルのコサイン類似度。
      Eigen::Vector3d he(q.entropy[0], q.entropy[1], q.entropy[2]);
      Eigen::Vector3d hc(cand.entropy[0], cand.entropy[1], cand.entropy[2]);
      const double denom = he.norm() * hc.norm();
      const double sim = denom > 1e-9 ? he.dot(hc) / denom : 0.0;
      if (sim < params_.entropy_sim_threshold) continue;
      votes[cand.frame] += 1;
      matches[cand.frame].push_back({qi, dbidx});
    }
  }
  if (votes.empty()) return result;

  // 最多得票フレームを候補に。
  int best_frame = -1, best_votes = 0;
  for (const auto& kv : votes)
    if (kv.second > best_votes) { best_votes = kv.second; best_frame = kv.first; }
  if (best_frame < 0) return result;

  // 幾何検証: 対応点集合から SVD で R,t を推定し再投影誤差でインライアを数える。
  const auto& mp = matches[best_frame];
  std::vector<Eigen::Vector3d> src, dst;
  for (const auto& m : mp) {
    const auto& q = query[m.first];
    const auto& c = database_[m.second];
    for (int t = 0; t < 3; t++) { src.push_back(q.p[t]); dst.push_back(c.p[t]); }
  }
  if (src.size() < 3) return result;

  Eigen::Vector3d sc = Eigen::Vector3d::Zero(), dc = Eigen::Vector3d::Zero();
  for (size_t i = 0; i < src.size(); i++) { sc += src[i]; dc += dst[i]; }
  sc /= src.size();
  dc /= dst.size();
  Eigen::Matrix3d W = Eigen::Matrix3d::Zero();
  for (size_t i = 0; i < src.size(); i++)
    W += (src[i] - sc) * (dst[i] - dc).transpose();
  Eigen::JacobiSVD<Eigen::Matrix3d> svd(W, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::Matrix3d R = svd.matrixV() * svd.matrixU().transpose();
  if (R.determinant() < 0) {
    Eigen::Matrix3d V = svd.matrixV();
    V.col(2) *= -1;
    R = V * svd.matrixU().transpose();
  }
  const Eigen::Vector3d t = dc - R * sc;

  int inliers = 0;
  for (size_t i = 0; i < src.size(); i++)
    if ((dst[i] - (R * src[i] + t)).norm() < params_.verify_dist) ++inliers;

  if (inliers >= params_.min_inliers) {
    result.detected = true;
    result.matched_frame = best_frame;
    result.inliers = inliers;
    result.rotation = R;
    result.translation = t;
  }
  return result;
}

}  // namespace dtd
}  // namespace localization_zoo
