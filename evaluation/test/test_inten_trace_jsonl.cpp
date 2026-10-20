#include "../src/inten_trace_jsonl.h"
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>

namespace trace = localization_zoo::evaluation::inten_trace;
using namespace localization_zoo::inten_loam;

TEST(InTenTraceJsonl, PreservesFailedStepValuesAndMatrixOrder) {
  auto out = trace::record();
  Eigen::Matrix<double, 2, 2> matrix;
  matrix << 1, 2, 3, 4;
  trace::matrix(out, matrix);
  EXPECT_EQ(out.str(), "[1,2,3,4]");
  InTenLoamPaperTrace t;
  t.s2s.called = true;
  t.s2s.stop = PaperSolverStop::NonFiniteStep;
  t.s2s.iterations.emplace_back();
  auto& step = t.s2s.iterations.back();
  step.step_attempted = true;
  step.delta << std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(),
      0.12345678901234568, 0, 0;
  const auto line = trace::frame(7, "scan\"\\\n\t\r\x01/cloud.pcd", t, {}, Eigen::Matrix4d::Identity());
  EXPECT_NE(line.find("\"delta\":[\"NaN\",\"+Infinity\",\"-Infinity\",0.12345678901234568,0,0]"), std::string::npos);
  EXPECT_NE(line.find("\"stop\":\"nonfinite_step\""), std::string::npos);
  EXPECT_NE(line.find("\"step_attempted\":true,\"step_finite\":false,\"update_applied\":false"), std::string::npos);
  EXPECT_NE(line.find("scan\\\"\\\\\\n\\t\\r\\u0001/cloud.pcd"), std::string::npos);
  EXPECT_EQ(line.find('\n'), std::string::npos);
}

TEST(InTenTraceJsonl, RecordsEffectiveSettingsAndSkippedInput) {
  InTenLoamPaperParams p;
  p.s2m_plane_max_dist = 0.05;
  p.use_mapping = false;
  p.accelerated = true;
  p.s2s_row_support = true;
  const auto line = trace::header(p, {"runner", "space and \"quote\""}, 3, 5, 80, 0.2,
                                  Eigen::Matrix4d::Identity(), Eigen::Matrix4d::Identity());
  EXPECT_NE(line.find("\"use_mapping\":false"), std::string::npos);
  EXPECT_NE(line.find("\"accelerated\":true"), std::string::npos);
  EXPECT_NE(line.find("\"s2s_row_support\":true"), std::string::npos);
  const auto defaults = trace::header(InTenLoamPaperParams{}, {}, 0, 0, 0, 0,
                                      Eigen::Matrix4d::Identity(), Eigen::Matrix4d::Identity());
  EXPECT_NE(defaults.find("\"accelerated\":false"), std::string::npos);
  EXPECT_EQ(defaults.find("s2s_row_support"), std::string::npos);
  EXPECT_NE(line.find("\"min_range_m\":5"), std::string::npos);
  EXPECT_NE(line.find("space and \\\"quote\\\""), std::string::npos);
  EXPECT_NE(trace::skipped(2, "empty/cloud.pcd").find("\"reason\":\"empty_cloud\""), std::string::npos);
  EXPECT_STREQ(trace::stopName(PaperSolverStop::NotRun), "not_run");
  EXPECT_STREQ(trace::stopName(PaperSolverStop::NoIterations), "no_iterations");
  EXPECT_STREQ(trace::stopName(PaperSolverStop::InsufficientPairs), "insufficient_pairs");
  EXPECT_STREQ(trace::stopName(PaperSolverStop::Converged), "converged");
  EXPECT_STREQ(trace::stopName(PaperSolverStop::IterationLimit), "iteration_limit");
}

TEST(InTenTraceJsonl, NeverOverwritesAndMarksTraversalCompletion) {
  namespace fs = std::filesystem;
  const auto path = fs::path(testing::TempDir()) /
      ("inten-trace-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  ASSERT_TRUE(fs::create_directory(path));
  struct Cleanup { fs::path p; ~Cleanup() { fs::remove_all(p); } } cleanup{path};
  const auto file = (path / "trace.jsonl").string();
  {
    trace::Writer writer(file);
    writer.write("{\"type\":\"header\"}");
    EXPECT_THROW(trace::Writer duplicate(file), std::runtime_error);
    writer.finish(2, 1);
    EXPECT_THROW(writer.write("{}"), std::runtime_error);
  }
  std::ifstream stream(file);
  std::string first, last;
  ASSERT_TRUE(static_cast<bool>(std::getline(stream, first)));
  ASSERT_TRUE(static_cast<bool>(std::getline(stream, last)));
  EXPECT_EQ(first, "{\"type\":\"header\"}");
  EXPECT_EQ(last, "{\"type\":\"end\",\"processed_frames\":2,\"skipped_frames\":1}");
  EXPECT_THROW(trace::Writer missing((path / "absent" / "trace.jsonl").string()), std::runtime_error);
}
