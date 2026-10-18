from pathlib import Path
import hashlib,json,shutil,subprocess,time
r=Path('/workspace/localization-env/paper-speed');repo=Path('/workspace/localization_zoo');tag=json.loads((r/'final_source_ready.json').read_text())['tag'];e=repo/'experiments/results/paper_pipeline_speed'
# Run only after reviewing the full comparison and importing the final profile.
assert json.loads((r/'final_comparison_ready.json').read_text())['exit_code']==0
assert json.loads((r/(tag+'_final_mesh_profile_exit.json')).read_text())['exit_code']==0
profile=e/'runs'/(tag+'_profile_accelerated_mesh_loam')
for name in ['summary.json','command.json','cpu.prof','cpu_profile.txt','poses.txt']:
 p=profile/name;assert subprocess.check_output(['git','show','HEAD:'+str(p.relative_to(repo))],cwd=repo)==p.read_bytes()
assert json.loads((e/'comparisons'/(tag+'.json')).read_text())['all_rte_equal_to_three_decimals'],'Diagnose the changed RTE before deleting inputs'
dataset=r/'kitti/dataset/sequences/07/velodyne';hashes=json.loads((e/'input_sha256.json').read_text());actual=sorted(p.name for p in dataset.glob('*.bin'));assert actual==sorted(hashes)
for name,digest in hashes.items():assert hashlib.sha256((dataset/name).read_bytes()).hexdigest()==digest,name
rawroot=r/'kitti';assert rawroot.resolve()==Path('/workspace/localization-env/paper-speed/kitti');link=repo/'dogfooding_results/paper_speed_kitti07';assert link.is_symlink() and link.resolve()==dataset.resolve()
started=time.time();link.unlink();shutil.rmtree(rawroot)
receipt={'command':['/workspace/localization-env/host-venv/bin/python',str(Path(__file__))],'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),'input_files_checked':len(actual),'input_sha256_match':True,'deleted_root':str(rawroot),'deleted_symlink':str(link),'raw_root_exists_after':rawroot.exists(),'symlink_exists_after':link.is_symlink(),'started_unix':started,'finished_unix':time.time(),'exit_code':0}
(r/'cleanup_kitti07.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt,indent=2))
