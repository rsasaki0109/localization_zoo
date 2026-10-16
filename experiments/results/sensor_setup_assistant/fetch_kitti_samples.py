import hashlib,json,sys,time
from pathlib import Path
sys.path.insert(0,'/workspace/localization_zoo/evaluation/scripts')
from fetch_kitti_odometry_sequences import RemoteZip,DEFAULT_URL,select_sequence_infos,extract_sequence_range
out=Path('/workspace/localization-env/sensor-setup/kitti')
with RemoteZip(DEFAULT_URL) as archive:
 infos=sorted(select_sequence_infos(archive.infolist(),'00'),key=lambda x:x.filename)[:3]
for info in infos: extract_sequence_range(DEFAULT_URL,[info],out)
rows=[]
for i in infos:
 p=out/i.filename;rows.append({'path':str(p),'archive_entry':i.filename,'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(out/'download.json').write_text(json.dumps({'url':DEFAULT_URL,'source_script':'evaluation/scripts/fetch_kitti_odometry_sequences.py','frames':rows},indent=2)+'\n')
print(json.dumps(rows,indent=2))
