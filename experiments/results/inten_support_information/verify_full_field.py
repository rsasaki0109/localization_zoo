"""Supply the original all-point field input to the unchanged frozen reconstruction."""
from pathlib import Path
import hashlib,importlib.util,json
E=Path(__file__).resolve().parent;W=E.parents[2]
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
checker=module('support_frozen_full_field',E/'verify.py')
inputs=module('support_full_field_inputs',E/'field_inputs.py')
current_group=None;node_fields={}
original_reconstruct=checker.reconstruct;original_group=checker.group_result
def reconstruct(record,clouds,excluded,pca_cache,field_cache):
    if record['class']==3:
        group=node_fields[current_group];frame=record['frame']
        checker.require(frame in group,'Unbound all-point field')
        # setdefault in the frozen checker now receives its true build input.
        # The obsolete reflector-only temporary is ignored, not used or mixed.
        field_cache[frame-1]=group[frame]
    return original_reconstruct(record,clouds,excluded,pca_cache,field_cache)
def group_result(group):
    global current_group
    current_group=group
    node_fields[group]={f:inputs.F.node_map(field) for f,field in inputs.fields(group).items()}
    try:return original_group(group)
    finally:node_fields.pop(group,None);current_group=None
checker.reconstruct=reconstruct;checker.group_result=group_result
def validate():
    recovery=json.loads((E/'field_source_recovery_protocol.json').read_text())
    assert hashlib.sha256((E/'protocol.json').read_bytes()).hexdigest()==recovery['frozen_protocol_sha256']
    for name,digest in recovery['source_sha256'].items():
        assert hashlib.sha256((W/name).read_bytes()).hexdigest()==digest,('Input recovery source changed',name)
    audit=E/'checks/field_rebuild'
    assert json.loads((audit/'field_rebuild_command.json').read_text())['exit_code']==0
    summary=json.loads((audit/'summary.json').read_text());assert summary['exit_code']==0
    for group,report in summary['groups'].items():
        assert report['feature_cache_sha256']==recovery['field_groups'][group]['feature_cache_sha256']
        expected=json.loads((E/'protocol.json').read_text())['input_pins'][group]['frames']
        assert [c['frame'] for c in report['cases']]==expected
        assert all(c['all_population_counts_exact'] and c['max_abs_intensity_sum_difference']==0. for c in report['cases'])
    return checker.validate()
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();result=validate()
    a.output.write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in result.items() if k!='groups'}))
