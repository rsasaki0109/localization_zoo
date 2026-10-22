"""Audit every inherited endpoint without an optimizer or raw scan download."""
from common import *
from build import docker
PARENT=E.parent/'inten_nonfacade_consensus_support'
CHECK=module('continuity_parent_decode',PARENT/'validate.py')

def prepare(group,stage):
    parent=PARENT/'groups'/group
    meta=read(parent/'input_sources.json');features=Path(meta['feature_cache_private'])
    assert sha(features)==meta['feature_cache_sha256']
    rows=list(map(json.loads,CHECK.decode(parent,'native.stdout.jsonl').splitlines()))
    terminals={r['frame']:r for r in rows if r['type']=='terminal' and r['mode']==2}
    original=list(map(json.loads,CHECK.decode(PARENT/'attribution',group+'/joint_primitives.jsonl').splitlines()))
    guard=list(map(json.loads,CHECK.decode(parent,'primitives.jsonl').splitlines()))
    prior={f:[p for p in guard if p['frame']==f and p['class']==3] for f in meta['frames']}
    expected=[];commands=[]
    joint_inputs={int(line.split()[0]):list(map(float,line.split()[1:])) for line in (PARENT/'attribution'/group/'joint_input.txt').read_text().splitlines()}
    proposed_inputs={int(line.split()[0]):list(map(float,line.split()[1:])) for line in (PARENT/'attribution'/group/'refined_input.txt').read_text().splitlines()}
    for frame in meta['frames']:
        # Include untouched fallbacks as zero-displacement paths.
        a=joint_inputs[frame];b=proposed_inputs[frame];delta=[b[k]-a[k] for k in (3,7,11)]
        primitives=[p for p in original if p['frame']==frame and p['class']==3]
        if terminals[frame]['support']['evaluated']:
            assert len(primitives)==len(prior[frame])
            assert delta==terminals[frame]['support']['delta']
        else:assert not any(delta)
        for index,p in enumerate(primitives):
            previous=prior[frame][index] if prior[frame] else p
            assert p['x']==previous['x'] and p['mu']==previous['mu']
            candidate=previous['other_mu'] if prior[frame] else p['mu']
            expected.append(dict(frame=frame,index=index,x=p['x'],delta=delta,original=p['mu'],candidate=candidate,intensity=p['intensity'],gradient=p['a'],scale=p['scale']))
            commands.append(' '.join(map(str,[frame,index,*p['x'],*delta])))
    write(stage/'input_sources.json',dict(parent_group=str(parent.relative_to(W)),feature_cache_private=str(features),feature_cache_sha256=sha(features),feature_cache_bytes=features.stat().st_size,frames=meta['frames'],parent_archive_sha256=sha(parent/'archive.json'),attribution_archive_sha256=sha(PARENT/'attribution/archive.json'),no_raw_acquired=True,no_gt_in_native_input=True))
    write(stage/'expected.json',expected)
    (stage/'paths.txt').write_text('\n'.join(commands)+'\n')
    return features

def main():
    assert not git('status','--porcelain')
    for name,digest in read(E/'protocol.json')['source_sha256'].items():assert sha(W/name)==digest,name
    binary=read(E/'checks/build/binaries.json')['binaries']['probe']['sha256']
    for group in ('synthetic','old','01','02','08'):
        stage=R/('stage_'+group);stage.mkdir()
        args=[]
        if group!='synthetic':args=[str(prepare(group,stage)),str(stage/'paths.txt')]
        assert sha(R/'probe')==binary
        assert run('native',docker()+[str(R/'probe'),*args],stage,jsonl=True)==0
        assert sha(R/'probe')==binary
        module('continuity_archive',E/'archive.py').archive(stage,E/'groups'/group)
if __name__=='__main__':main()
