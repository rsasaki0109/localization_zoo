from pathlib import Path
import json,subprocess,time,traceback
r=Path('/workspace/localization-env/paper-speed');tag=json.loads((r/'final_source_ready.json').read_text())['tag'];repo=Path('/workspace/localization_zoo')
try:
 while not (r/'final_comparison_ready.json').exists():time.sleep(5)
 assert json.loads((r/'final_comparison_ready.json').read_text())['exit_code']==0
 data=json.loads((repo/'experiments/results/paper_pipeline_speed/comparisons'/(tag+'.json')).read_text());assert data['all_rte_equal_to_three_decimals'] and all(v['pose_matrix_max_abs_delta']==0 for v in data['methods'].values())
 assert all(data['methods'][m]['speedup']>1 for m in ['imls_slam','inten_loam']),'Review retention of a path without full-run speed benefit'
 while not (r/(tag+'_final_mesh_profile_exit.json')).exists():time.sleep(5)
 assert json.loads((r/(tag+'_final_mesh_profile_exit.json')).read_text())['exit_code']==0
 cmd=['/workspace/localization-env/host-venv/bin/python',str(r/'verify_release_candidate.py')]
 with (r/'release_checks_stdout.log').open('w') as f:result=subprocess.run(cmd,cwd=repo,stdout=f,stderr=subprocess.STDOUT)
 release=json.loads((r/'release_source_ready.json').read_text());out=r/'release_verification'/release['tag'];assert result.returncode==0 and json.loads((out/'exit.json').read_text())['exit_code']==0
 (r/'release_verification_ready.json').write_text(json.dumps({'exit_code':0,'release_source':release['source_commit'],'results':str(out),'next':'Review/import/commit profile and release verification. Then render/update numeric docs from committed full comparison, cleanup raw inputs, run release_final_validation.py, publish PR and verify exact HEAD CI.'},indent=2)+'\n')
except BaseException as err:
 (r/'release_verification_ready.json').write_text(json.dumps({'exit_code':1,'error':str(err),'finished_unix':time.time()},indent=2)+'\n');traceback.print_exc();raise
