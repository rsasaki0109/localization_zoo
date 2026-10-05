#include "allan_variance/allan_variance.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace localization_zoo {
namespace allan_variance {

std::vector<long> logClusterSizes(long num_samples, int points_per_decade) {
  std::vector<long> sizes;
  const long max_m = (num_samples - 1) / 2;
  if (max_m < 1 || points_per_decade < 1) return sizes;
  const double decades = std::log10(static_cast<double>(max_m));
  const int steps = static_cast<int>(std::ceil(decades * points_per_decade));
  for (int i = 0; i <= steps; ++i) {
    const long m = std::min<long>(
        max_m, std::lround(std::pow(10.0, static_cast<double>(i) / points_per_decade)));
    if (sizes.empty() || m > sizes.back()) sizes.push_back(m);
  }
  return sizes;
}

std::vector<AllanPoint> overlappingAllanDeviation(const std::vector<double>& samples,
                                                  double sampling_rate,
                                                  const std::vector<long>& cluster_sizes) {
  if (sampling_rate <= 0) throw std::invalid_argument("sampling_rate must be positive");
  const long M = static_cast<long>(samples.size());
  // Cumulative sum with the mean removed first, so long recordings keep
  // precision (the Allan variance is invariant to a constant offset).
  double mean = 0.0;
  for (double x : samples) mean += x;
  mean = M ? mean / M : 0.0;
  std::vector<double> cum(M + 1, 0.0);
  for (long i = 0; i < M; ++i) cum[i + 1] = cum[i] + (samples[i] - mean);

  std::vector<AllanPoint> curve;
  for (long m : cluster_sizes) {
    const long terms = M - 2 * m + 1;
    if (m < 1 || terms < 1) continue;
    // g_{i+m} - g_i = (S[i+2m] - 2 S[i+m] + S[i]) / m with S the cumulative sum.
    double acc = 0.0;
    for (long i = 0; i < terms; ++i) {
      const double d = cum[i + 2 * m] - 2 * cum[i + m] + cum[i];
      acc += d * d;
    }
    const double avar = acc / (static_cast<double>(m) * m) / (2.0 * terms);
    curve.push_back({m / sampling_rate, std::sqrt(avar), terms});
  }
  return curve;
}

std::optional<double> fixedSlopeFit(const std::vector<AllanPoint>& curve, double slope,
                                    double tau_min, double tau_max, double tau_eval) {
  double offset = 0.0;
  int n = 0;
  for (const AllanPoint& p : curve) {
    if (p.tau < tau_min || p.tau > tau_max || p.adev <= 0) continue;
    offset += std::log10(p.adev) - slope * std::log10(p.tau);
    ++n;
  }
  if (n == 0) return std::nullopt;
  return std::pow(10.0, offset / n + slope * std::log10(tau_eval));
}

std::optional<double> tangentFit(const std::vector<AllanPoint>& curve, double slope,
                                 double tau_eval) {
  double best = std::numeric_limits<double>::infinity();
  std::optional<double> result;
  for (size_t i = 0; i + 1 < curve.size(); ++i) {
    const AllanPoint& a = curve[i];
    const AllanPoint& b = curve[i + 1];
    if (a.adev <= 0 || b.adev <= 0) continue;
    const double local = (std::log10(b.adev) - std::log10(a.adev)) /
                         (std::log10(b.tau) - std::log10(a.tau));
    if (std::abs(local - slope) < best) {
      best = std::abs(local - slope);
      const double offset = std::log10(a.adev) - slope * std::log10(a.tau);
      result = std::pow(10.0, offset + slope * std::log10(tau_eval));
    }
  }
  return result;
}

NoiseParameters extractNoiseParameters(const std::vector<AllanPoint>& curve) {
  NoiseParameters out;
  out.white_noise_density = tangentFit(curve, -0.5, 1.0).value_or(0.0);
  out.bias_random_walk = tangentFit(curve, 0.5, 3.0).value_or(0.0);
  const auto min_it = std::min_element(curve.begin(), curve.end(),
                                       [](const AllanPoint& a, const AllanPoint& b) {
                                         return a.adev < b.adev;
                                       });
  if (min_it != curve.end()) {
    out.bias_instability = min_it->adev / std::sqrt(2 * std::log(2.0) / M_PI);
    out.bias_instability_tau = min_it->tau;
  }
  return out;
}

std::vector<AllanPoint> meanCurve(const std::vector<std::vector<AllanPoint>>& curves) {
  if (curves.empty()) return {};
  std::vector<AllanPoint> out = curves.front();
  for (size_t c = 1; c < curves.size(); ++c) {
    if (curves[c].size() != out.size()) throw std::invalid_argument("curve sizes differ");
    for (size_t i = 0; i < out.size(); ++i) out[i].adev += curves[c][i].adev;
  }
  for (AllanPoint& p : out) p.adev /= static_cast<double>(curves.size());
  return out;
}

}  // namespace allan_variance
}  // namespace localization_zoo
