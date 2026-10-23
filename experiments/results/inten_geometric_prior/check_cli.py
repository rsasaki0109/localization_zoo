#!/usr/bin/env python3
"""Compare the old CLI baseline to the current disabled path on the same fixture."""
import json,subprocess
from pathlib import Path
from receipt import save,sha
root=Path.cwd();study=root/'experiments/results/inten_geometric_prior'
source='/workspace/localization-env/branch-cleanup-20261022/worktree'
scratch='/dev/inten-geometric-prior-20261023'
container='localization-zoo-dev'
out=study/'cli_fixture';out.mkdir(exist_ok=False)
commands=[]
def run(argv):
    result=subprocess.run(argv,capture_output=True,text=True)
    commands.append({'argv':argv,'exit_code':result.returncode,'stdout':result.stdout,'stderr':result.stderr})
    save(out/'commands.json',commands)
    if result.returncode:raise RuntimeError('CLI fixture command failed')
    return result.stdout
base=['docker','exec','-e','OMP_NUM_THREADS=1','-e','OPENBLAS_NUM_THREADS=1']
flags=['--methods','inten_loam','--no-gt-seed','--inten-loam-paper-odometry','--input-max-range-m','120',
       '--input-vertical-angle-correction-deg','0.17','--paper-pipeline-accelerated']
for item in ['s2m_plane_max_dist=0.05','s2s_mapped_prior=1','s2s_translation_prior_ratio=0.03','s2s_ground_pca=1','s2s_facade_pca=1']:
    flags+=['--inten-loam-paper-set',item]
for name,binary in [('frozen','/workspace/localization-env/inten-facade-pca/pcd_dogfooding'),
                    ('disabled',scratch+'/pcd_dogfooding'),('enabled',scratch+'/pcd_dogfooding')]:
    directory=scratch+'/fixture_'+name
    run(base+[container,'mkdir',directory])
    command=base+['-w',directory,container,binary,source+'/evaluation/fixtures/mcd_kth_smoke',
                 source+'/evaluation/fixtures/mcd_kth_smoke_gt.csv','3',*flags,
                 '--summary-json',directory+'/summary.json']
    if name=='enabled':command+=['--inten-loam-paper-set','s2s_geometric_prior=1']
    run(command)
    for filename,target in [('summary.json','summary.json'),('dogfooding_results/InTEn_LOAM.txt','poses.txt')]:
        (out/(name+'_'+target)).write_text(run(base+[container,'cat',directory+'/'+filename]))
assert (out/'frozen_poses.txt').read_bytes()==(out/'disabled_poses.txt').read_bytes()
old=json.loads((out/'frozen_summary.json').read_text())['methods'][0]
new=json.loads((out/'disabled_summary.json').read_text())['methods'][0]
keys=['ate_m','rpe_trans_pct','rpe_rot_deg_per_m','kitti_rte_trans_pct','frames']
same={key:old[key]==new[key] for key in keys}
for name in ['frozen','disabled','enabled']:
    summary=json.loads((out/(name+'_summary.json')).read_text())
    assert summary['num_frames']==summary['methods'][0]['frames']==3
    assert len((out/(name+'_poses.txt')).read_text().splitlines())==3
assert all(same.values())
assert 'independent KISS-ICP' in json.loads((out/'enabled_summary.json').read_text())['methods'][0]['note']
save(out/'check.json',{'exit_code':0,'disabled_poses_byte_equal':True,'disabled_metric_equal':same,
     'new_cli_accepts_option':True,'frozen_binary_sha256':sha('/workspace/localization-env/inten-facade-pca/pcd_dogfooding')})
