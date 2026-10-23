#include "mutual.h"
#include <gtest/gtest.h>
using namespace mutual;
std::vector<Feature> features(std::initializer_list<Vec> x){std::vector<Feature> f;for(auto p:x){Feature a;a.p=p;f.push_back(a);}return f;}
TEST(Reciprocal, KnownRigidCorrespondencesReturnToOriginalSources) {
  auto f=features({Vec(0,0,0),Vec(2,1,0),Vec(-1,3,2)});Index index(f);
  Eigen::Matrix3d R=Eigen::AngleAxisd(.4,Vec::UnitZ()).toRotationMatrix();Vec t(.3,-.2,.1);
  for(int i=0;i<3;++i){Vec anchor=R*f[i].p+t;EXPECT_TRUE(index.accepts(i,R.transpose()*(anchor-t)));}
}
TEST(Reciprocal, ManyToOneAssociationRejectsTheNonNearestSource) {
  auto f=features({Vec(0,0,0),Vec(1,0,0)});Index index(f);
  EXPECT_TRUE(index.accepts(0,Vec(.1,0,0)));EXPECT_FALSE(index.accepts(1,Vec(.1,0,0)));
}
TEST(Reciprocal, CoincidentSourcesUseDistanceTiesWithoutOrderBias) {
  auto f=features({Vec(0,0,0),Vec(0,0,0),Vec(1,0,0)});Index index(f);
  EXPECT_TRUE(index.accepts(0,Vec::Zero()));EXPECT_TRUE(index.accepts(1,Vec::Zero()));EXPECT_FALSE(index.accepts(2,Vec::Zero()));
}
TEST(Reciprocal, EmptyAndInvalidQueriesCannotInventSupport) {
  std::vector<Feature> empty;Index a(empty);EXPECT_FALSE(a.accepts(0,Vec::Zero()));
  auto f=features({Vec::Zero()});Index b(f);EXPECT_FALSE(b.accepts(-1,Vec::Zero()));
  EXPECT_FALSE(b.accepts(0,Vec(std::numeric_limits<double>::infinity(),0,0)));
}
