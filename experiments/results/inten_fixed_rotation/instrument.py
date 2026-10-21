"""Declared cached replay plus an opt-in conditional translation solve."""
import importlib.util
from pathlib import Path

def transformed(source):
    path=Path(__file__).resolve().parent.parent/'inten_translation_consensus/instrument.py'
    spec=importlib.util.spec_from_file_location('rotation_parent_instrument',path)
    previous=importlib.util.module_from_spec(spec);spec.loader.exec_module(previous)
    result=previous.transformed(source)
    start=result.index('int InTenLoamPaper::solveS2S(')
    end=result.index('int InTenLoamPaper::solveS2M(',start)
    body=result[start:end]
    old='    const Vec6 delta = (sys.H + 1e-6 * Mat6::Identity()).ldlt().solve(-sys.g);'
    assert body.count(old)==1
    body=body.replace(old,'''    Vec6 delta;
    if (diagnostic::translation_only) {
      delta.setZero();
      delta.head<3>() = (sys.H.topLeftCorner<3, 3>() +
          1e-6 * Eigen::Matrix3d::Identity()).ldlt().solve(-sys.g.head<3>());
    } else {
      delta = (sys.H + 1e-6 * Mat6::Identity()).ldlt().solve(-sys.g);
    }''')
    old='    *T = expSe3(delta) * *T;'
    assert body.count(old)==1
    body=body.replace(old,'''    if (diagnostic::translation_only)
      T->block<3, 1>(0, 3) += delta.head<3>();
    else
      *T = expSe3(delta) * *T;''')
    return result[:start]+body+result[end:]
