from pathlib import Path
import importlib.util
_path=Path(__file__).resolve().parent.parent/'inten_reciprocal_support/transform.py'
_spec=importlib.util.spec_from_file_location('prior_control_parent_transform',_path)
_parent=importlib.util.module_from_spec(_spec);_spec.loader.exec_module(_parent)

def transformed(source):
    source=_parent.transformed(source)
    old='prior_information = detail::translationPriorInformation(sys.H, t, params_.s2s_translation_prior_ratio);'
    new='prior_information = prior_control::select(detail::translationPriorInformation(sys.H, t, params_.s2s_translation_prior_ratio), replay::common_prior, translation_only, replay::reference_joint, replay::reference_refinement);'
    assert source.count(old)==1
    source=source.replace(old,new)
    old='const double cost = detail::addTranslationPrior(prior_information, prediction, t, &sys.H, &sys.g);'
    assert source.count(old)==1
    source=source.replace(old,'if (replay::common_prior) prior_control::observe(it, translation_only, prior_information, prediction, t, sys.H, sys.g);\n      '+old)
    return '#include "prior_control.h"\n'+source
