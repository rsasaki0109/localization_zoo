def transformed(source):
    s = '#include "diagnostic_hooks.h"\n' + source
    needle = 'double huber, System* also = nullptr) {'
    assert s.count(needle) == 1
    s = s.replace(needle, 'double huber, System* also = nullptr, bool cauchy = false) {')
    needle = '    const double w = std::abs(r) <= huber ? 1.0 : huber / std::abs(r);'
    assert s.count(needle) == 1
    s = s.replace(needle, '    const double w = cauchy ? 1.0 / (1.0 + (r / huber) * (r / huber))\n                            : (std::abs(r) <= huber ? 1.0 : huber / std::abs(r));')
    needle = '\n                        std::vector<int>* indices, std::vector<float>* distances) {'
    assert s.count(needle) == 1
    s = s.replace(needle, '\n                        std::vector<int>* indices, std::vector<float>* distances, bool cauchy = false) {')
    assert s.count('system->add(x, gradient, residual, huber, also);') == 1
    s = s.replace('system->add(x, gradient, residual, huber, also);', 'system->add(x, gradient, residual, huber, also, cauchy);')
    assert s.count('system->add(value.x, value.gradient, value.value, huber, also);') == 1
    s = s.replace('system->add(value.x, value.gradient, value.value, huber, also);', 'system->add(value.x, value.gradient, value.value, huber, also, cauchy);')
    needle = 'accumulateFeatures(src, R, t, plane, huber, params_.accelerated, &sys, cs(1 + cls), &idx, &d2);'
    a = s.index('int InTenLoamPaper::solveS2S(')
    b = s.index('int InTenLoamPaper::solveS2M(', a)
    assert s[a:b].count(needle) == 1
    s = s[:a] + s[a:b].replace(needle, 'accumulateFeatures(src, R, t, plane, huber, params_.accelerated, &sys, cs(1 + cls), &idx, &d2,\n                         cls == 1 && diagnostic::mode == 1);') + s[b:]
    needle = '    solveS2S(frame, &rel, &cs);'
    assert s.count(needle) == 1
    s = s.replace(needle, '\n'.join([
        '    auto* observation = diagnostic::terminal_trace;',
        '    if (observation) { observation->called = true; observation->input = rel; }',
        '    solveS2S(frame, &rel, &cs, observation);',
        '    if (observation) {',
        '      observation->output = rel;',
        '      observation->returned_pairs = cs.cls[0].n + cs.cls[1].n + cs.cls[2].n + cs.cls[3].n;',
        '    }']))
    return s
