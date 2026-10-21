"""Replay committed extracted frames and estimated input seed; estimator otherwise exact."""
def transformed(source):
    result='#include "replay_hooks.h"\n'+source
    old='  const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);'
    assert result.count(old)==1
    result=result.replace(old,'  const PaperFrame frame = replay::frame ? *replay::frame : extractFeatures(deskewed.empty() ? points : deskewed, intensity);')
    old='  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;'
    assert result.count(old)==1
    return result.replace(old,old+'\n  if (replay::seed) rel = *replay::seed;')
