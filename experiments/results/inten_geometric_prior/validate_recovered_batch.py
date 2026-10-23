"""Validate the recovered raw batch against frozen input and binary pins."""
import hashlib,json,subprocess
from pathlib import Path
r=Path(__file__).resolve().parent
raw=Path('/tmp/localization-zoo-inten-geometric-prior-20261023/raw/01/dataset/sequences/01/velodyne')
original=Path('/workspace/localization-env/branch-cleanup-20261022/worktree/experiments/results/inten_facade_pca/sequences/01/inputs.json')
pins=json.loads(original.read_text())['files']
checked=[]
for index in range(64):
    name=f'{index:06d}.bin';data=(raw/name).read_bytes()
    digest=hashlib.sha256(data).hexdigest()
    assert digest==pins[name]['sha256'] and len(data)==pins[name]['bytes'],name
    checked.append({'name':name,'sha256':digest,'bytes':len(data)})
binary=json.loads((r/'binaries.json').read_text())
actual=subprocess.check_output(['docker','exec','localization-zoo-dev','sha256sum',binary['runner_container_path']],text=True).split()[0]
assert actual==binary['runner_sha256']
print(json.dumps({'inputs':checked,'binary_sha256':actual,'passed':True},indent=2))
