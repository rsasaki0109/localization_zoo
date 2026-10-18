#include "common/initial_registration.h"

#include <gtest/gtest.h>

#include <memory>
#include <vector>

using localization_zoo::registration::initialCoarseThenFine;

TEST(InitialRegistration, CoarseStateFeedsFineAndFineSuppliesResult) {
  std::vector<int> order;
  double pose = 2.0;
  bool ran = false;
  const int samples = initialCoarseThenFine(1, 1, 10,
      [&](int budget) { order.push_back(budget); pose += 3.0; },
      [&]() { order.push_back(20); EXPECT_EQ(pose, 5.0); pose *= 2; return 42; }, &ran);
  EXPECT_TRUE(ran);
  EXPECT_EQ(order, (std::vector<int>{10, 20}));
  EXPECT_EQ(pose, 10.0);
  EXPECT_EQ(samples, 42);
}

TEST(InitialRegistration, IncludesFirstAndLastRequestedRegistrationOnly) {
  for (int index : {-1, 0, 1, 2, 3, 4}) {
    int calls = 0;
    bool ran = true;
    initialCoarseThenFine(index, 3, 7,
        [&](int budget) { EXPECT_EQ(budget, 7); ++calls; }, []() {}, &ran);
    EXPECT_EQ(calls, index >= 1 && index <= 3 ? 1 : 0);
    EXPECT_EQ(ran, calls == 1);
  }
}

TEST(InitialRegistration, DisabledBudgetsLeaveOnlyOrdinaryRegistration) {
  for (int frames : {-2, 0, 1}) {
    for (int iterations : {-2, 0, 10}) {
      int coarse_calls = 0, fine_calls = 0;
      bool ran = true;
      initialCoarseThenFine(1, frames, iterations,
          [&](int) { ++coarse_calls; }, [&]() { ++fine_calls; }, &ran);
      EXPECT_EQ(coarse_calls, frames > 0 && iterations > 0 ? 1 : 0);
      EXPECT_EQ(fine_calls, 1);
      EXPECT_EQ(ran, coarse_calls == 1);
    }
  }
}

TEST(InitialRegistration, SupportsMoveOnlyCallbacksAndReferenceResults) {
  int pose = 0;
  auto coarse = [budget = std::make_unique<int>(5), &pose](int iterations) {
    pose = *budget + iterations;
  };
  int& result = initialCoarseThenFine(1, 1, 2, std::move(coarse), [&]() -> int& { return pose; });
  EXPECT_EQ(result, 7);
  result = 9;
  EXPECT_EQ(pose, 9);
}
