"""Portable replay of the frozen equations and complete adverse outcomes."""
from pathlib import Path
import argparse,hashlib,importlib.util,json,subprocess
import numpy as np
E=Path(__file__).resolve().parent;W=E.parents[2]
BASE='cd7471b76435b580ed583dac8b165b357848424e'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
read=lambda p:json.loads(Path(p).read_text())
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()

def validate(history=True):
    protocol=read(E/'protocol.json');post=read(E/'postprocessing_protocol.json')
    for name,digest in protocol['source_sha256'].items():
        if history or name.startswith('experiments/results/'):assert sha(W/name)==digest,name
    for name,digest in post['source_sha256'].items():assert sha(E/name)==digest,name
    for name,digest in post['additional_source_sha256'].items():assert sha(W/name)==digest,name
    reader=module('fixed_rotation_archive_reader',E.parent/'inten_cluster_motion/verify.py').restored
    for folder in ('controls','analysis','collection_execution','analysis_execution'):
        manifest=read(E/folder/'archive.json');assert manifest['exit_code']==0 and manifest['byte_roundtrip_verified']
        for name in manifest['files']:reader(E/folder,name)
    for label in ('mechanism_tests','compiler','compile_native','compile_probe','linked_libraries','native'):
        command=read(E/'controls'/(label+'_command.json'));assert command['exit_code']==0
        assert command['source_commit']==post['numerical_producer_commit']
    assert read(E/'collection_execution/collection_command.json')['exit_code']==0
    assert read(E/'analysis_execution/analysis_command.json')['exit_code']==0
    instrument=module('fixed_rotation_instrument_verify',E/'instrument.py')
    assert instrument.transformed((E/'reference_native.cpp').read_text())==(E/'diagnostic_native.cpp').read_text()
    sources=read(E/'controls/input_sources.json');assert sources['no_raw_scans_acquired'] and sources['no_gt_in_native_input']
    data={}
    for source in sources['files']:
        manifest=W/source['archive_manifest'];assert sha(manifest)==source['archive_manifest_sha256']
        value=reader(manifest.parent,source['name']);assert hashlib.sha256(value).hexdigest()==source['sha256']
        data[source['label']]=value
    previous={(v['frame'],v['mode']):v for v in map(json.loads,data['previous_native'].splitlines())}
    profiles=[v for v in map(json.loads,data['profiles'].splitlines()) if v['type']=='profile']
    native=list(map(json.loads,reader(E/'controls','native.stdout.jsonl').splitlines()))
    truth={v['frame']:np.array(v['gt_relative']).reshape(4,4) for v in json.loads(data['offline_truth'])['cases']}
    evaluate=module('fixed_rotation_equation_verify',E/'equations.py').evaluate
    actual=evaluate(protocol,previous,profiles,read(E/'controls/selections.json')['frames'],native,
                    map(json.loads,reader(E/'controls','residuals.jsonl').splitlines()),truth)
    committed=read(E/'analysis/report.json')
    # Permit platform-level float differences, never outcome/selection changes.
    def compare(a,b):
        assert type(a)==type(b),(type(a),type(b))
        if isinstance(a,dict):
            assert set(a)==set(b)
            for k in a:
                if k=='rotation_error_mrad':
                    # acos near identity amplifies last-bit matrix-product drift.
                    # Native rotations remain bitwise checked in evaluate().
                    assert np.isclose(a[k],b[k],rtol=1e-8,atol=1e-6),(a[k],b[k])
                else:compare(a[k],b[k])
        elif isinstance(a,list):
            assert len(a)==len(b)
            for x,y in zip(a,b):compare(x,y)
        elif isinstance(a,float):assert np.isclose(a,b,rtol=1e-8,atol=1e-9),(a,b)
        else:assert a==b,(a,b)
    compare(actual,committed)
    if history:
        changed=subprocess.check_output(['git','diff','--name-only',BASE,'--'],cwd=W,text=True).splitlines()
        for name in changed:
            assert name.startswith('experiments/results/inten_fixed_rotation/') or name in ('tests/test_inten_fixed_rotation_study.py','docs/roadmap.md','papers/inten_loam/README.md'),name
        before=subprocess.check_output(['git','show',BASE+':papers/inten_loam/README.md'],cwd=W)
        assert (W/'papers/inten_loam/README.md').read_bytes().startswith(before)
    return dict(exit_code=0,native_calls=len(native),all_equations_and_original_controls_reconstructed=True,
                all_translation_only_rotations_bitwise_original=True,candidate_gate_passed=actual['candidate_gate_passed'],
                no_raw_acquisition=True,history_and_live_production_pins_checked=history)

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();args.output.write_text(json.dumps(validate(),indent=2)+'\n')
    print('Fixed-rotation factorial, actual equations, all outcomes and preservation passed')
