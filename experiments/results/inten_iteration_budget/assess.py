"""Read all frozen baseline terminal traces; no new estimator execution or GT."""
from common import *
import numpy as np
V=module('budget_old_decode',E.parent/'inten_intensity_consistent/verify.py')
def main():
    rows=[];sources={}
    for group in ('old','01','02','08'):
        folder=E.parent/'inten_intensity_consistent/groups'/group
        sources[group]=sha(folder/'archive.json')
        for line in V.V.decode(folder,'native.stdout.jsonl').splitlines():
            row=json.loads(line)
            if row['type']!='terminal':continue
            t=row['trace']['s2s'];steps=t['iterations']
            rows.append(dict(group=group,frame=row['frame'],mode=row['mode'],stop=t['stop'],
                iterations=len(steps),initial=t['input'],output=t['output'],
                final_delta=steps[-1]['delta'],final_delta_norm=float(np.linalg.norm(steps[-1]['delta'])),
                final_pairs=steps[-1]['pairs']))
    write(Path(__import__('sys').argv[1]),dict(exit_code=0,parent_commit=BASE,source_archives=sources,
        baseline_only=True,no_gt_input=True,cases=rows))
if __name__=='__main__':main()
