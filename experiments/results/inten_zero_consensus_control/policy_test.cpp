#include "replay_hooks.h"
#include <stdexcept>
using namespace localization_zoo::inten_loam;
void check(bool ok) { if (!ok) throw std::runtime_error("Diagnostic policy regression"); }
int main() {
  PaperConsensusTrace original;
  original.valid_clusters=3; original.best_index=1;original.usable=true;
  original.votes[60]=2;original.votes[1]=3;
  for(int k=0;k<3;++k) {PaperClusterAdmission a;a.cluster=k+2;a.bits[0]=2;
    if(k<2)a.bits[0]|=1ULL<<60;original.admissions.push_back(a);}
  replay::mode=1;auto c=original;replay::select(&c);check(c.best_index==1);
  replay::mode=2;c=original;replay::select(&c);
  check(c.best_index==60 && c.usable && c.excluded_clusters==std::vector<int>{4});
  original.votes[60]=1;c=original;replay::select(&c);check(c.best_index==1);
  original.usable=false;original.best_index=0;c=original;replay::select(&c);check(!c.usable && c.best_index==0);
  replay::mode=5;c=original;replay::select(&c);check(c.excluded_clusters.empty());
  check(replay::seedIndex(20)==20);replay::mode=6;check(replay::seedIndex(20)==60);
  return 0;
}
