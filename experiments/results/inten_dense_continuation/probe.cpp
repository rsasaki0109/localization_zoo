#include "scorer.h"
#include <Eigen/LU>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <set>
#include <stdexcept>
using namespace dense_continuation;
using localization_zoo::inten_loam::PaperFrame;
std::map<int,PaperFrame> load(const char* name) {
  std::ifstream in(name);if(!in)throw std::runtime_error("Missing features");
  std::map<int,PaperFrame> frames;int frame;
  while(in>>frame) {
    if(frames.count(frame))throw std::runtime_error("Duplicate frame");auto& f=frames[frame];
    for(auto* p:{&f.edge,&f.ground,&f.facade,&f.reflector,&f.all}) {
      size_t n;if(!(in>>n))throw std::runtime_error("Missing count");
      for(size_t i=0;i<n;++i) {
        localization_zoo::inten_loam::PaperFeature x;
        if(!(in>>x.p.x()>>x.p.y()>>x.p.z()>>x.row>>x.cluster>>x.intensity))throw std::runtime_error("Bad feature");p->push_back(x);
      }
    }
  }
  if(!in.eof())throw std::runtime_error("Malformed features");return frames;
}
void cloud(int frame,const std::vector<Cell>& cells,size_t population) {
  std::cout<<"{\"type\":\"cloud\",\"frame\":"<<frame<<",\"source_points\":"<<population<<",\"voxel_m\":0.2,\"cells\":[";
  bool comma=false;for(const auto& c:cells) {if(comma)std::cout<<',';comma=true;std::cout<<'['<<c.key[0]<<','<<c.key[1]<<','<<c.key[2]<<','<<c.count<<','<<c.point.x()<<','<<c.point.y()<<','<<c.point.z()<<']';}
  std::cout<<"]}\n";
}
void direction(const Direction& d) {
  std::cout<<"{\"valid\":"<<(d.valid?"true":"false")<<",\"population\":"<<d.population<<",\"within_gate\":"<<d.within_gate<<",\"mean\":";
  if(d.valid)std::cout<<d.mean;else std::cout<<"null";std::cout<<'}';
}
int main(int argc,char** argv) {
  try {
    if(argc!=2)throw std::runtime_error("Expected cached features");std::cout<<std::setprecision(17);
    const auto frames=load(argv[1]);std::set<int> printed;int frame,loaded=-1;
    std::vector<Cell> previous,current;std::unique_ptr<Index> prev_tree,curr_tree;
    while(std::cin>>frame) {
      if(frame!=loaded) {
        previous=voxelize(frames.at(frame-1).all,.2);current=voxelize(frames.at(frame).all,.2);
        prev_tree=std::make_unique<Index>(previous);curr_tree=std::make_unique<Index>(current);loaded=frame;
        if(printed.insert(frame-1).second)cloud(frame-1,previous,frames.at(frame-1).all.size());
        if(printed.insert(frame).second)cloud(frame,current,frames.at(frame).all.size());
      }
      Direction forward[12],reverse[12];
      for(int mode=0;mode<12;++mode) {
        int actual; if(!(std::cin>>actual)||actual!=mode)throw std::runtime_error("Arm order changed");
        Eigen::Matrix4d T;for(int i=0;i<16;++i)if(!(std::cin>>T(i/4,i%4)))throw std::runtime_error("Missing estimated pose");
        forward[mode]=score(current,*prev_tree,T,1.);reverse[mode]=score(previous,*curr_tree,T.inverse(),1.);
        std::cout<<"{\"type\":\"score\",\"frame\":"<<frame<<",\"mode\":"<<mode<<",\"forward\":";direction(forward[mode]);
        std::cout<<",\"reverse\":";direction(reverse[mode]);std::cout<<"}\n";
      }
      std::cout<<"{\"type\":\"decision\",\"frame\":"<<frame<<",\"selected_arms\":[";
      for(int mode=0;mode<6;++mode) {if(mode)std::cout<<',';std::cout<<(chooseExtended(forward[mode],reverse[mode],forward[mode+6],reverse[mode+6])?mode+6:mode);}
      std::cout<<"]}\n"<<std::flush;
    }
    if(!std::cin.eof()||!std::cout)throw std::runtime_error("Malformed input or failed output");return 0;
  }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
