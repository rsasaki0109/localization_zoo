#include "mutual.h"
#include "prior_control.h"
#include "support_information.h"
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
TEST(SupportInformation, PassiveFlagRestoresOnException) {
  replay::reciprocal=true;
  EXPECT_THROW({support_information::UnfilteredQuery passive;EXPECT_FALSE(replay::reciprocal);throw std::runtime_error("query");},std::runtime_error);
  EXPECT_TRUE(replay::reciprocal);
  replay::reciprocal=false;
}
struct InvalidOnlySystem {
  Eigen::Matrix<double,6,6> H=Eigen::Matrix<double,6,6>::Zero();
  Eigen::Matrix<double,6,1> g=Eigen::Matrix<double,6,1>::Zero();
  int n=0;bool track_residual=false;double weighted_squared_residual=0.;
  void add(const Vec&,const Vec&,double,double,InvalidOnlySystem*,bool){throw std::runtime_error("An invalid query was accumulated");}
};
TEST(SupportInformation, InvalidFieldQueryCannotInventValidLostSupport) {
  replay::mode=6;Frame frame;
  support_information::Frame prepared(frame);
  support_information::Context context{prepared,Eigen::Matrix4d::Identity(),0,false};
  const auto source=features({Vec::Zero()});
  auto query=[](const Feature&,const Vec&,Vec*,double*,std::vector<int>*,std::vector<float>*){return false;};
  testing::internal::CaptureStdout();
  EXPECT_NO_THROW(support_information::inspect<InvalidOnlySystem>(context,3,source,nullptr,query,.1));
  const auto text=testing::internal::GetCapturedStdout();
  EXPECT_NE(text.find("[false,true,[]]"),std::string::npos);
  EXPECT_NE(text.find("\"count\":0"),std::string::npos);
  replay::mode=-1;
}
