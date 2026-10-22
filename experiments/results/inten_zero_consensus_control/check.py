"""Independently validate every consensus decision, projected update and acceptance."""
from pathlib import Path
import argparse,json
import numpy as np

def matrix(v):
    a=np.asarray(v,dtype=float).reshape(4,4)
    assert np.isfinite(a).all()
    return a

def check_frame(frame,mode):
    enabled = mode != 0
    c=frame.get('s2s_consensus')
    if c is None:
        assert not enabled or frame['initialization'] or frame['s2s']['returned_pairs']<30
        return None
    assert enabled and not frame['initialization'] and frame['s2s']['returned_pairs']>=30
    assert c['seed']==frame['s2s']['input'] and c['original_estimate']==frame['s2s']['output']
    bands={}
    for band in c['admissions']:
        cluster=band['cluster'];raw=band['bits_low_high_hex']
        assert type(cluster) is int and cluster>=2 and cluster not in bands and len(raw)==32
        bits=int(raw[:16],16)|(int(raw[16:],16)<<64)
        assert bits>0 and bits>>121==0
        bands[cluster]=bits
    assert list(bands)==sorted(bands) and c['valid_clusters']==len(bands)
    votes=[sum((v>>k)&1 for v in bands.values()) for k in range(121)]
    best=min(range(121),key=lambda k:(-votes[k],abs(k-60),k))
    if mode in (2, 5, 6) and bands and 2*votes[60]>len(bands): best=60
    usable=bool(bands and 0<best<120 and 2*votes[best]>len(bands))
    assert c['votes']==votes and c['best_index']==best and c['usable']==usable
    excluded=[cluster for cluster,b in bands.items() if usable and not ((b>>best)&1)]
    if mode in (3, 5, 7): excluded=[]
    assert c['excluded_clusters']==excluded
    solver=c['refinement'];original=matrix(c['original_estimate']);seed=matrix(c['seed'])
    assert solver['called']==usable
    maximum=0.;steps=0
    if usable:
        direction=seed[:3,3].copy();direction/=np.linalg.norm(direction)
        initial=matrix(solver['input'])
        assert np.array_equal(initial[:3,:3],original[:3,:3])
        assert np.allclose(initial[:3,3],seed[:3,3]+(((60 if mode in (4,6,7) else best)-60)*.05)*direction,rtol=1e-13,atol=1e-13)
        iterations=solver['iterations'];assert iterations and iterations[0]['linearization_pose']==solver['input']
        for k,it in enumerate(iterations):
            T=matrix(it['linearization_pose']);H=np.asarray(it['H']).reshape(6,6);g=np.asarray(it['g']);d=np.asarray(it['delta'])
            assert np.isfinite(H).all() and np.isfinite(g).all() and np.isfinite(d).all()
            assert np.array_equal(T[:3,:3],original[:3,:3]) and np.array_equal(d[3:],np.zeros(3))
            assert sum(it['class_pairs'])==it['pairs']
            if it['step_attempted']:
                assert it['step_finite'] and it['update_applied'] and it['pairs']>=30
                A=H[:3,:3]+1e-6*np.eye(3);delta=d[:3]
                residual=float(np.linalg.norm(A@delta+g[:3])/max(1.,np.linalg.norm(A)*np.linalg.norm(delta)+np.linalg.norm(g[:3])))
                assert residual<1e-10
                maximum=max(maximum,residual);steps+=1
                after=matrix(iterations[k+1]['linearization_pose'] if k+1<len(iterations) else solver['output'])
                assert np.array_equal(after[:3,3],T[:3,3]+delta)
            else:
                assert k==len(iterations)-1 and solver['stop']=='insufficient_pairs' and it['pairs']<30
        out=matrix(solver['output']);assert np.array_equal(out[:3,:3],original[:3,:3])
        assert solver['returned_pairs']==iterations[-1]['pairs']
        assert c['applied']==(solver['returned_pairs']>=30)
    else:
        assert not c['applied'] and not solver['iterations']
    assert frame['accepted_relative']==(solver['output'] if c['applied'] else frame['s2s']['output'])
    return dict(usable=usable,applied=c['applied'],clusters=len(bands),excluded=len(c['excluded_clusters']),steps=steps,max_backward=maximum)
