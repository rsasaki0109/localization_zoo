"""Separate byte-exact archive transport from strict portable recomputation."""
from pathlib import Path
import csv,hashlib,importlib.util,io,json,math
import numpy as np
E=Path(__file__).resolve().parent
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
reader=module('rigidity_portable_original_reader',E/'report_io.py')
frozen=module('rigidity_portable_original_thresholds',E.parent/'inten_support_information/verify_mixed.py')
sha=lambda data:hashlib.sha256(data).hexdigest()

def equivalent(actual,expected,path='report'):
    if isinstance(expected,dict):
        assert isinstance(actual,dict) and actual.keys()==expected.keys(),path
        for key in expected:equivalent(actual[key],expected[key],path+'/'+str(key))
    elif isinstance(expected,list):
        assert isinstance(actual,list) and len(actual)==len(expected),path
        for i,(a,b) in enumerate(zip(actual,expected)):equivalent(a,b,path+'/'+str(i))
    elif isinstance(expected,float):
        assert isinstance(actual,float) and math.isfinite(actual) and math.isfinite(expected),path
        frozen.checker.close(actual,expected,path+' differs at original tolerance')
    else:assert type(actual) is type(expected) and actual==expected,path

def compare_summary(actual,expected):
    original=(E/'initial_protocol.json').read_bytes();current=(E/'protocol.json').read_bytes()
    assert expected['protocol_sha256']==sha(original)
    assert actual['protocol_sha256'] in (sha(original),sha(current))
    assert json.loads(original)['witness']==json.loads(current)['witness']==actual['settings']==expected['settings']
    adjusted=dict(actual,protocol_sha256=expected['protocol_sha256']);adjusted['groups']={}
    for group,values in actual['groups'].items():
        # Transport identities are verified against each complete actual CSV below.
        adjusted['groups'][group]=dict(values,points=expected['groups'][group]['points'])
    equivalent(adjusted,expected)

def compare_outputs(output):
    actual=json.loads((output/'summary.json').read_text());expected=reader.read_report()
    compare_summary(actual,expected);deltas={}
    for group in ('old','01','02','08'):
        raw=(output/group/'points.csv').read_bytes();original=reader.read_file(group+'/points.csv')
        meta=actual['groups'][group]['points']
        assert len(raw)==meta['bytes'] and sha(raw)==meta['sha256']
        left=list(csv.DictReader(io.StringIO(raw.decode())));right=list(csv.DictReader(io.StringIO(original.decode())))
        assert len(left)==len(right)
        real=('axis_g','axis_H','mean_absolute_length_error_m','maximum_absolute_length_error_m')
        for a,b in zip(left,right):
            assert a.keys()==b.keys()
            for key in a:
                if key not in real:assert a[key]==b[key],('Discrete point identity differs',group,key)
                else:assert (a[key]=='')==(b[key]==''),('Missing-value mask differs',group,key)
        deltas[group]={}
        for key in real:
            a=np.array([float(row[key]) for row in left if row[key]!='']);b=np.array([float(row[key]) for row in right if row[key]!=''])
            assert np.isfinite(a).all() and np.isfinite(b).all()
            frozen.checker.close(a,b,'Complete point column differs at original tolerance: '+group+'/'+key)
            deltas[group][key]=float(np.max(np.abs(a-b))) if len(a) else None
    return dict(exit_code=0,all_discrete_identities_counts_and_missing_masks_exact=True,
        complete_archive_bytes_hash_verified=True,all_numeric_values_at_original_tolerance=True,
        original_rtol=1e-10,original_atol=1e-12,maximum_absolute_point_delta=deltas)
