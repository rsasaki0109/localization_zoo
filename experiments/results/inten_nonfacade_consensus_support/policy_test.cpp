#include "support_hooks.h"
void check(bool value){if(!value)throw std::runtime_error("Common-support policy regression");}
int main(){
 check(support::rho(.05,.1)==.05*.05);
 check(std::abs(support::rho(-.3,.1)-.05)<1e-15);
 support::report=support::Report();support::report.counts[0]=1;
 support::report.original[0]=2.;support::report.candidate[0]=1.;check(support::finish());
 support::report.candidate[2]=1e100;check(support::finish());
 support::report.complete=false;check(!support::finish());
 support::report.complete=true;support::report.candidate[0]=2.;check(!support::finish());
 support::report=support::Report();check(!support::finish());
 return 0;
}
