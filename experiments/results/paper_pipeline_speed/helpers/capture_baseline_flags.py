from pathlib import Path
import json
root=Path('/workspace/localization_zoo');lines=(root/'build/build.ninja').read_text().splitlines()
paths=['papers/mesh_loam/src/mesh_loam.cpp','papers/imls_slam/src/imls_paper_odometry.cpp','papers/inten_loam/src/inten_loam_paper.cpp','evaluation/src/pcd_dogfooding.cpp']
entries={}
for path in paths:
 stem=Path(path).name+'.o:'
 for i,line in enumerate(lines):
  if line.startswith('build ') and stem in line:
   block=[]
   for row in lines[i:]:
    if block and not row.strip():break
    block.append(row)
   entries[path]=block;break
 assert path in entries,path
out={'source_commit':'134ead2f21a4611ec328a45aa543b31428d6b92c','capture_note':'Baseline Ninja configuration inspected before reconfiguration for the opt-in candidate; these are build manifest flags, no compilation is performed.','targets':entries}
Path('/workspace/localization-env/paper-speed/baseline_target_flags.json').write_text(json.dumps(out,indent=2)+'\n')
for path,rows in entries.items():print(path,[row.strip() for row in rows if row.lstrip().startswith(('FLAGS =','DEFINES ='))])
