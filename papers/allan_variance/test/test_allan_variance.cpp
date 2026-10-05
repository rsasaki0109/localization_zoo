#include "allan_variance/allan_variance.h"

#include <gtest/gtest.h>

#include <cmath>
#include <random>

using namespace localization_zoo::allan_variance;

namespace {

// Direct evaluation of TUM VI eq. (11) with explicit cluster averages.
double bruteForceAdev(const std::vector<double>& x, long m) {
  const long M = static_cast<long>(x.size());
  std::vector<double> g(M - m + 1);
  for (long i = 0; i + m <= M; ++i) {
    double s = 0;
    for (long j = 0; j < m; ++j) s += x[i + j];
    g[i] = s / m;
  }
  double acc = 0;
  const long terms = M - 2 * m + 1;
  for (long i = 0; i < terms; ++i) acc += (g[i + m] - g[i]) * (g[i + m] - g[i]);
  return std::sqrt(acc / (2.0 * terms));
}

}  // namespace

TEST(AllanVariance, ClusterSizesAreLogSpacedAndBounded) {
  const auto sizes = logClusterSizes(10001, 10);
  ASSERT_FALSE(sizes.empty());
  EXPECT_EQ(sizes.front(), 1);
  EXPECT_EQ(sizes.back(), 5000);
  for (size_t i = 1; i < sizes.size(); ++i) EXPECT_GT(sizes[i], sizes[i - 1]);
  EXPECT_TRUE(logClusterSizes(2, 10).empty());
}

TEST(AllanVariance, CumulativeSumMatchesDefinition) {
  std::mt19937 rng(1);
  std::normal_distribution<double> n(5.0, 2.0);  // offset must not matter
  std::vector<double> x(997);
  for (double& v : x) v = n(rng);
  const std::vector<long> sizes = {1, 2, 7, 50, 333, 498};
  const auto curve = overlappingAllanDeviation(x, 10.0, sizes);
  ASSERT_EQ(curve.size(), sizes.size());
  for (size_t i = 0; i < sizes.size(); ++i) {
    EXPECT_DOUBLE_EQ(curve[i].tau, sizes[i] / 10.0);
    EXPECT_NEAR(curve[i].adev, bruteForceAdev(x, sizes[i]), 1e-10);
    EXPECT_EQ(curve[i].clusters, 997 - 2 * sizes[i] + 1);
  }
}

TEST(AllanVariance, RecoversWhiteNoiseDensity) {
  // Discrete std sigma_d at rate f has continuous density sigma_d / sqrt(f).
  const double rate = 100.0, density = 0.01;
  std::mt19937 rng(2);
  std::normal_distribution<double> n(0.0, density * std::sqrt(rate));
  std::vector<double> x(400000);
  for (double& v : x) v = n(rng);
  const auto curve = overlappingAllanDeviation(x, rate, logClusterSizes(x.size(), 20));
  EXPECT_NEAR(*fixedSlopeFit(curve, -0.5, 0.02, 1.0, 1.0), density, 0.01 * density);
  EXPECT_NEAR(extractNoiseParameters(curve).white_noise_density, density, 0.05 * density);
}

TEST(AllanVariance, RecoversBiasRandomWalk) {
  // b_k = b_{k-1} + N(0, sigma_b^2 / f): Allan deviation sigma_b sqrt(tau / 3).
  const double rate = 50.0, sigma_b = 1e-3;
  std::mt19937 rng(3);
  std::normal_distribution<double> n(0.0, sigma_b / std::sqrt(rate));
  std::vector<double> x(1000000);
  double b = 0;
  for (double& v : x) v = (b += n(rng));
  const auto curve = overlappingAllanDeviation(x, rate, logClusterSizes(x.size(), 20));
  EXPECT_NEAR(*fixedSlopeFit(curve, 0.5, 0.1, 100.0, 3.0), sigma_b, 0.05 * sigma_b);
  EXPECT_NEAR(extractNoiseParameters(curve).bias_random_walk, sigma_b, 0.1 * sigma_b);
}

TEST(AllanVariance, FitHelpersHandleEmptyRangesAndMeanCurves) {
  const std::vector<AllanPoint> a = {{0.1, 2.0, 1}, {1.0, 4.0, 1}};
  const std::vector<AllanPoint> b = {{0.1, 4.0, 1}, {1.0, 8.0, 1}};
  EXPECT_FALSE(fixedSlopeFit(a, -0.5, 10.0, 20.0, 1.0).has_value());
  const auto mean = meanCurve({a, b});
  EXPECT_DOUBLE_EQ(mean[0].adev, 3.0);
  EXPECT_DOUBLE_EQ(mean[1].adev, 6.0);
  // A curve with a flat floor: bias instability = floor / 0.664.
  const std::vector<AllanPoint> floor = {{1, 3.0, 1}, {10, 1.0, 1}, {100, 2.0, 1}};
  const NoiseParameters p = extractNoiseParameters(floor);
  EXPECT_NEAR(p.bias_instability, 1.0 / std::sqrt(2 * std::log(2.0) / M_PI), 1e-12);
  EXPECT_DOUBLE_EQ(p.bias_instability_tau, 10.0);
}
