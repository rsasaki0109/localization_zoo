def transformed(source):
    def once(text,a,b):
        assert text.count(a)==1,a
        return text.replace(a,b)
    source=once(source,'const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);',
        'const PaperFrame frame = replay::frame ? *replay::frame : extractFeatures(deskewed.empty() ? points : deskewed, intensity);')
    source=once(source,'Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;',
        'Eigen::Matrix4d rel = params_.s2s_mapped_prior && params_.use_mapping ? last_mapped_rel_ : last_rel_;\n  if (replay::seed) rel = *replay::seed;')
    a=source.index('int InTenLoamPaper::solveS2S(');b=source.index('int InTenLoamPaper::solveS2M(')
    body=source[a:b]
    body=once(body,'const Index& prev = *prev_;','const Index& prev = *prev_;\n  mutual::Backward backward(curr,replay::reciprocal);')
    body=once(body,'for (int it = 0; it < params_.s2s_iterations; ++it) {',
        'for (int it = 0; it < params_.s2s_iterations; ++it) {\n    mutual::Observation reciprocal_observation(backward,*T,it,translation_only);')
    body=once(body,'auto edge = [&](const PaperFeature&, const Eigen::Vector3d& x, Eigen::Vector3d* a,',
        'auto edge = [&](const PaperFeature& source, const Eigen::Vector3d& x, Eigen::Vector3d* a,')
    body=once(body,'if (n < 2 || ds[0] > gate) return false;',
        'if (replay::reciprocal) reciprocal_observation.note(0,static_cast<int>(&source-curr.edge.data()),n>=2 && ds[0]<=gate ? ids[0] : -1);\n      if (n < 2 || ds[0] > gate) return false;')
    body=once(body,'const PaperFeature& qj = prev.edge.pts[static_cast<size_t>(ids[0])];',
        'const PaperFeature& qj = prev.edge.pts[static_cast<size_t>(ids[0])];\n      if (replay::reciprocal && !reciprocal_observation.accepts(0,static_cast<int>(&source-curr.edge.data()),qj.p)) return false;')
    body=once(body,'if (n < 3 || ds[0] > gate) return false;',
        'if (replay::reciprocal) reciprocal_observation.note(1+cls,static_cast<int>(&source-src.data()),n>=3 && ds[0]<=gate ? ids[0] : -1);\n        if (n < 3 || ds[0] > gate) return false;')
    body=once(body,'const PaperFeature& qj = tree.pts[static_cast<size_t>(ids[0])];',
        'const PaperFeature& qj = tree.pts[static_cast<size_t>(ids[0])];\n        if (replay::reciprocal && !reciprocal_observation.accepts(1+cls,static_cast<int>(&source-src.data()),qj.p)) return false;')
    return '#include "replay_hooks.h"\n#include "mutual.h"\n'+source[:a]+body+source[b:]
