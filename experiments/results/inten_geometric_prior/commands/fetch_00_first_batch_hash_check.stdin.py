import json,hashlib
from pathlib import Path
r=Path('experiments/results/inten_geometric_prior')
original=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_facade_pca/sequences/00/inputs.json')
pins=json.loads(original.read_text())['files'];raw=Path('/tmp/localization-zoo-inten-geometric-prior-20261023/raw/00/dataset/sequences/00/velodyne');rows=[]
for i in range(64):
 name=f'{i:06d}.bin';data=(raw/name).read_bytes();digest=hashlib.sha256(data).hexdigest();assert digest==pins[name]['sha256'] and len(data)==pins[name]['bytes'];rows.append(dict(frame=i,sha256=digest,bytes=len(data)))
(r/'fetch_00_first_batch_hash_check.json').write_text(json.dumps(dict(all_frozen_inputs_match=True,input_manifest_sha256=hashlib.sha256(original.read_bytes()).hexdigest(),frames=rows),indent=2)+'\n')
print('All selected frames match frozen input hashes and sizes')
