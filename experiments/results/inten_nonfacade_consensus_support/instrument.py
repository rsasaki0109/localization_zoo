'Exact replay hooks plus prospective native common-support observation/veto.'
def transformed(source):
    result='#include "replay_hooks.h"\n#include "support_hooks.h"\n'+source
    changes=[
      ('  const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);',
       '  const PaperFrame frame = replay::frame ? *replay::frame : extractFeatures(deskewed.empty() ? points : deskewed, intensity);'),
      ('  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;',
       '  Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;\n  if (replay::seed) rel = *replay::seed;'),
      ('  if (!accelerated) {','  if (!accelerated || support::observing) {')]
    for old,new in changes:
        assert result.count(old)==1,old
        result=result.replace(old,new)
    start=result.index('int InTenLoamPaper::solveS2S(');end=result.index('int InTenLoamPaper::solveS2M(',start)
    body=result[start:end]
    changes=[
      ('      *residual = r;\n      return true;',
       '      *residual = r;\n      if (support::observing) support::line(x, *a, r, huber, qj.p, v);\n      return true;'),
      ('        *residual = nrm.dot(x - qj.p);\n        return true;',
       '        *residual = nrm.dot(x - qj.p);\n        if (support::observing) support::plane(1 + cls, x, *a, *residual, huber, qj.p);\n        return true;'),
      ('        *residual = w * (mu - f.intensity);\n        return true;',
       '''        *residual = w * (mu - f.intensity);
        if (support::observing) {
          double other_mu = 0.; Eigen::Vector3d other_gradient;
          const bool complete = prev.intensity.eval(x + support::report.delta, &other_mu, &other_gradient);
          support::field(x, *a, *residual, huber, mu, other_mu, f.intensity, w, complete);
        }
        return true;'''),
      ('    counts[3] = sys.n - before_intensity;\n',
       '''    counts[3] = sys.n - before_intensity;
    if (support::observing) {
      observeSystem(trace, sys, *T, counts);
      if (trace) trace->stop = PaperSolverStop::NoIterations;
      return pairs;  // Passive query only: no prior, step, or pose update.
    }
''')]
    for old,new in changes:
        assert body.count(old)==1,old
        body=body.replace(old,new)
    result=result[:start]+body+result[end:]
    old='        if (count >= 30 && refined.allFinite()) {'
    new='''        bool support_allowed = true;
        if (support::enabled && count >= 30 && refined.allFinite()) {
          support::begin(rel, refined);
          Eigen::Matrix4d query = rel;
          const int queried = solveS2S(frame, &query, nullptr, &support::report.query,
                                      facade_dynamic.empty() ? nullptr : &facade_dynamic);
          support::report.query.output = query;
          support::report.query.returned_pairs = queried;
          if (!(query.array() == rel.array()).all())
            throw std::runtime_error("Passive support query changed original estimate");
          support_allowed = support::finish();
        }
        if (count >= 30 && refined.allFinite() && support_allowed) {'''
    assert result.count(old)==1
    result=result.replace(old,new)
    old='detail::selectTranslationConsensus(detail::clusterTranslationProfiles(\n          prev_->all.pts, frame, consensus_seed, params_.accelerated));'
    assert result.count(old)==1
    return result.replace(old,'detail::selectTranslationConsensus(*replay::profiles);')
