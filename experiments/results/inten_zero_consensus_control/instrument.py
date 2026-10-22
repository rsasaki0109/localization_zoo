"""Diagnostic-only pair/seed replay and declared seed-by-mask interventions."""
def transformed(source):
    result = '#include "replay_hooks.h"\n' + source
    replacements = [
      ('  const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);',
       '  const PaperFrame frame = replay::frame ? *replay::frame : extractFeatures(deskewed.empty() ? points : deskewed, intensity);'),
      ('  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;',
       '  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;\n  if (replay::seed) rel = *replay::seed;'),
      ('detail::selectTranslationConsensus(detail::clusterTranslationProfiles(\n          prev_->all.pts, frame, consensus_seed, params_.accelerated));',
       'detail::selectTranslationConsensus(*replay::profiles);\n      replay::select(&consensus);'),
      ('((consensus.best_index - 60) * .05)', '((replay::seedIndex(consensus.best_index) - 60) * .05)')]
    for old, new in replacements:
        assert result.count(old) == 1, old
        result = result.replace(old, new)
    return result
