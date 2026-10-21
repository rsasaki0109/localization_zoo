"""Isolated cached-feature replay; production files and public trace guard stay intact."""


def transformed(source):
    result = '#include "hooks.h"\n' + source
    needle = '  const PaperFrame frame = extractFeatures(deskewed.empty() ? points : deskewed, intensity);'
    assert result.count(needle) == 1
    result = result.replace(needle, '  const PaperFrame frame = diagnostic::frame ? *diagnostic::frame :\n'
                                   '      extractFeatures(deskewed.empty() ? points : deskewed, intensity);')
    needle = '        if (cls == 1 && facade_dynamic && source.cluster >= 0 &&'
    assert result.count(needle) == 1
    result = result.replace(needle, '        if (cls == 1 && diagnostic::excluded.count(source.cluster)) return false;\n' + needle)
    needle = '    solveS2S(frame, &rel, &cs);'
    assert result.count(needle) == 1
    result = result.replace(needle, '\n'.join([
        '    auto* observation = diagnostic::terminal_trace;',
        '    if (observation) { observation->called = true; observation->input = rel; }',
        '    solveS2S(frame, &rel, &cs, observation);',
        '    if (observation) {',
        '      observation->output = rel;',
        '      observation->returned_pairs = cs.cls[0].n + cs.cls[1].n + cs.cls[2].n + cs.cls[3].n;',
        '    }']))
    return result
