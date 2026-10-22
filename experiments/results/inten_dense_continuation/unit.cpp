#include "scorer.h"
#include <Eigen/Geometry>
#include <gtest/gtest.h>
using namespace dense_continuation;
using localization_zoo::inten_loam::PaperFeature;
TEST(DenseContinuation,VoxelsPreserveNegativeCellsAndFixedPopulation) {
  std::vector<PaperFeature> pts(3);pts[0].p=Vec(-.19,0,0);pts[1].p=Vec(-.11,0,0);pts[2].p=Vec(.01,0,0);
  const auto cells=voxelize(pts,.2);ASSERT_EQ(cells.size(),2u);EXPECT_EQ(cells[0].key[0],-1);
  EXPECT_EQ(cells[0].count,2);EXPECT_NEAR(cells[0].point.x(),-.15,1e-14);EXPECT_EQ(cells[1].count,1);
}
TEST(DenseContinuation,UnmatchedPointsKeepTheirCappedPenalty) {
  std::vector<Cell> source={{{0,0,0},1,Vec(0,0,0)},{{1,0,0},1,Vec(.1,0,0)},{{50,0,0},1,Vec(10,0,0)}};
  std::vector<Cell> target={{{0,0,0},1,Vec(0,0,0)}};Index index(target);
  auto result=score(source,index,Eigen::Matrix4d::Identity(),1.);
  ASSERT_TRUE(result.valid);EXPECT_EQ(result.population,3u);EXPECT_EQ(result.within_gate,2u);EXPECT_NEAR(result.mean,1.01/3,1e-14);
  std::vector<Cell> empty;Index empty_index(empty);EXPECT_FALSE(score(source,empty_index,Eigen::Matrix4d::Identity(),1.).valid);
}
TEST(DenseContinuation,SelectionNeedsBothDirectionsAndRejectsRoundoffTies) {
  Direction base{true,.1,10,10},better{true,.05,10,10},worse{true,.2,10,10},tie{true,.1-1e-13,10,10},invalid;
  EXPECT_TRUE(chooseExtended(base,base,better,better));EXPECT_FALSE(chooseExtended(base,base,better,worse));
  EXPECT_FALSE(chooseExtended(base,base,tie,better));EXPECT_FALSE(chooseExtended(base,base,better,invalid));
}
TEST(DenseContinuation,CorrectRigidPoseImprovesBothIndependentPopulations) {
  std::vector<Cell> previous,current;Eigen::Matrix4d T=Eigen::Matrix4d::Identity();
  T.block<3,3>(0,0)=Eigen::AngleAxisd(.2,Vec::UnitZ()).toRotationMatrix();T.block<3,1>(0,3)=Vec(.4,.3,-.1);
  for(const Vec& p:{Vec(0,0,0),Vec(1,0,0),Vec(0,2,0),Vec(0,0,3)}) {
    previous.push_back({{},1,p});current.push_back({{},1,T.block<3,3>(0,0).transpose()*(p-T.block<3,1>(0,3))});
  }
  Index a(previous),b(current);const auto I=Eigen::Matrix4d::Identity().eval();
  EXPECT_TRUE(chooseExtended(score(current,a,I,1.),score(previous,b,I,1.),score(current,a,T,1.),score(previous,b,T.inverse(),1.)));
}
