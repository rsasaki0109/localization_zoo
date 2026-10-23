"""Pose-free local distance preservation, conditional on frozen native anchors."""
import numpy as np

K = 10

def neighbors(points, k=K):
    """Nearest other source points, with distance/index ties fixed prospectively."""
    points = np.asarray(points, dtype=np.float64).reshape(-1,3)
    assert isinstance(k,int) and k>=0 and np.isfinite(points).all()
    delta = points[:,None,:]-points[None,:,:]
    squared = np.sum(delta*delta, axis=2)
    np.fill_diagonal(squared, np.inf)
    return np.argsort(squared, axis=1, kind='stable')[:,:min(k,max(0,len(points)-1))]

def measure(source, target, anchors, valid, near):
    """Keep the same all-valid neighbor population for every center partition."""
    source = np.asarray(source, float).reshape(-1,3)
    target = np.asarray(target, float).reshape(-1,3)
    anchors = np.asarray(anchors, int); valid = np.asarray(valid, bool)
    near=np.asarray(near,int)
    assert near.ndim==2 and np.isfinite(source).all() and np.isfinite(target).all()
    assert len(source)==len(anchors)==len(valid)==len(near)
    assert np.all((near>=0)&(near<len(source)))
    assert not np.any(near==np.arange(len(source))[:,None])
    assert np.all((anchors[valid]>=0)&(anchors[valid]<len(target)))
    center = np.repeat(np.arange(len(source)), near.shape[1])
    other = near.ravel()
    eligible = valid[center]&valid[other]
    center = center[eligible]; other = other[eligible]
    ds = np.linalg.norm(source[center]-source[other], axis=1)
    dt = np.linalg.norm(target[anchors[center]]-target[anchors[other]], axis=1)
    error = dt-ds
    count = np.bincount(center, minlength=len(source))
    absolute = np.bincount(center, weights=np.abs(error), minlength=len(source))
    maximum = np.zeros(len(source)); np.maximum.at(maximum, center, np.abs(error))
    mean = np.divide(absolute, count, out=np.full(len(source),np.nan), where=count>0)
    return dict(center=center, other=other, source_length=ds, target_length=dt,
        error=error, count=count, mean_absolute=mean, maximum_absolute=maximum,
        possible_relations=int(valid.sum())*near.shape[1])

def summarize(values, center_mask):
    center_mask = np.asarray(center_mask, bool)
    active = center_mask[values['center']]
    ds = values['source_length'][active]; dt = values['target_length'][active]
    error = values['error'][active]; absolute = np.abs(error)
    scores = values['mean_absolute'][center_mask]
    scores = scores[np.isfinite(scores)]
    return dict(centers=int(center_mask.sum()), centers_with_relations=len(scores), relations=len(error),
        zero_source_length=int((ds==0).sum()), source_length_sum=float(ds.sum()),
        target_length_sum=float(dt.sum()), signed_sum=float(error.sum()),
        absolute_sum=float(absolute.sum()), squared_sum=float(np.sum(error*error)),
        maximum_absolute=float(absolute.max()) if len(error) else None,
        center_mean_absolute=list(map(float,scores)))

def finish(parts):
    result={k:sum(p[k] for p in parts) for k in ('centers','centers_with_relations','relations','zero_source_length',
        'source_length_sum','target_length_sum','signed_sum','absolute_sum','squared_sum')}
    maxima=[p['maximum_absolute'] for p in parts if p['maximum_absolute'] is not None]
    scores=np.array([v for p in parts for v in p['center_mean_absolute']],float)
    n=result['relations']; length=result['source_length_sum']
    result.update(mean_absolute=result['absolute_sum']/n if n else None,
        rms=float(np.sqrt(result['squared_sum']/n)) if n else None,
        length_normalized_absolute=result['absolute_sum']/length if length else None,
        maximum_absolute=max(maxima) if maxima else None,
        center_mean_absolute_quantiles=np.quantile(scores,[0.,.25,.5,.75,1.],method='linear').tolist() if len(scores) else None)
    return result
