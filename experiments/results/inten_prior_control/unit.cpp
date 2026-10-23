#include "mutual.h"
#include "prior_control.h"
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
TEST(PriorControl, DisabledInterventionPreservesDerivedInformationExactly) {
  Eigen::Matrix3d derived=Eigen::Matrix3d::Random(),other=7*Eigen::Matrix3d::Identity();
  EXPECT_TRUE(prior_control::select(derived,false,false,&other,&other)==derived);
  EXPECT_TRUE(prior_control::select(derived,false,true,nullptr,nullptr)==derived);
}
TEST(PriorControl, SharedInformationDoesNotDependOnFilteredGeometry) {
  Eigen::Matrix3d fixed=3*Eigen::Matrix3d::Identity(),a=Eigen::Matrix3d::Zero(),b=17*Eigen::Matrix3d::Identity();
  EXPECT_TRUE(prior_control::select(a,true,false,&fixed,nullptr)==fixed);
  EXPECT_TRUE(prior_control::select(b,true,false,&fixed,nullptr)==fixed);
}
TEST(PriorControl, JointAndTranslationRefinementUseTheirOwnFixedReferences) {
  Eigen::Matrix3d joint=3*Eigen::Matrix3d::Identity(),refinement=9*Eigen::Matrix3d::Identity();
  EXPECT_TRUE(prior_control::select(Eigen::Matrix3d::Zero(),true,false,&joint,&refinement)==joint);
  EXPECT_TRUE(prior_control::select(Eigen::Matrix3d::Zero(),true,true,&joint,&refinement)==refinement);
}
TEST(PriorControl, MissingOrNonfiniteReferenceFailsWithoutFallback) {
  Eigen::Matrix3d derived=Eigen::Matrix3d::Identity(),invalid=derived;
  invalid(0,0)=std::numeric_limits<double>::quiet_NaN();
  EXPECT_THROW(prior_control::select(derived,true,false,nullptr,nullptr),std::runtime_error);
  EXPECT_THROW(prior_control::select(derived,true,true,&derived,&invalid),std::runtime_error);
}
