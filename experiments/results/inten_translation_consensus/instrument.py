"""Extend the pinned cached-feature diagnostic with first-query observation only."""
import importlib.util
from pathlib import Path


def transformed(source):
    previous = Path(__file__).resolve().parent.parent/'inten_cluster_motion/instrument.py'
    spec = importlib.util.spec_from_file_location('prior_cluster_instrument', previous)
    parent = importlib.util.module_from_spec(spec); spec.loader.exec_module(parent)
    result = parent.transformed(source)
    old = '      if (query(f, x, &gradient, &residual, indices, distances))\n        system->add(x, gradient, residual, huber, also, cauchy);'
    assert result.count(old) == 1
    result = result.replace(old, '      if (query(f, x, &gradient, &residual, indices, distances)) {\n'
                            '        diagnostic::residual(x, gradient, residual, huber);\n'
                            '        system->add(x, gradient, residual, huber, also, cauchy);\n      }')
    old = '    if (value.valid) system->add(value.x, value.gradient, value.value, huber, also, cauchy);'
    assert result.count(old) == 1
    result = result.replace(old, '    if (value.valid) {\n'
                            '      diagnostic::residual(value.x, value.gradient, value.value, huber);\n'
                            '      system->add(value.x, value.gradient, value.value, huber, also, cauchy);\n    }')
    a = result.index('int InTenLoamPaper::solveS2S('); b = result.index('int InTenLoamPaper::solveS2M(', a)
    body = result[a:b]
    changes = {
        '    System sys;': '    diagnostic::iteration = it;\n    System sys;',
        '    accumulateFeatures(curr.edge,': '    diagnostic::cls = 0;\n    accumulateFeatures(curr.edge,',
        '      accumulateFeatures(src,': '      diagnostic::cls = 1 + cls;\n      accumulateFeatures(src,',
        '      accumulateFeatures(curr.reflector,': '      diagnostic::cls = 3;\n      accumulateFeatures(curr.reflector,'}
    for old, new in changes.items():
        assert body.count(old) == 1
        body = body.replace(old, new)
    return result[:a]+body+result[b:]
