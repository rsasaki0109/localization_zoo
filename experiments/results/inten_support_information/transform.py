from pathlib import Path
import importlib.util
_path=Path(__file__).resolve().parent.parent/'inten_prior_control/transform.py'
_spec=importlib.util.spec_from_file_location('support_parent_transform',_path)
_parent=importlib.util.module_from_spec(_spec);_spec.loader.exec_module(_parent)
def transformed(source):
    source=_parent.transformed(source)
    a=source.index('int InTenLoamPaper::solveS2S(');b=source.index('int InTenLoamPaper::solveS2M(')
    body=source[a:b]
    def once(old,new):
        nonlocal body
        assert body.count(old)==1,old
        body=body.replace(old,new)
    once('const Index& prev = *prev_;','const Index& prev = *prev_;\n  support_information::Frame support_frame(curr);')
    once('    System sys;','    support_information::Context support_context{support_frame,*T,it,translation_only};\n    System sys;')
    edge='    accumulateFeatures(curr.edge, R, t, edge, huber, params_.accelerated, &sys, cs(0), &idx, &d2);'
    once(edge,edge+'\n    support_information::inspect<System>(support_context,0,curr.edge,&prev.edge.pts,edge,huber);')
    plane='                         cls == 1 && params_.s2s_facade_cauchy);'
    once(plane,plane+'\n      support_information::inspect<System>(support_context,1+cls,src,&tree.pts,plane,huber,cls==1 && params_.s2s_facade_cauchy);')
    intensity='      accumulateFeatures(curr.reflector, R, t, intensity, huber, params_.accelerated, &sys, cs(3), &idx, &d2);'
    once(intensity,intensity+'\n      support_information::inspect<System>(support_context,3,curr.reflector,nullptr,intensity,huber);')
    return '#include "support_information.h"\n'+source[:a]+body+source[b:]
