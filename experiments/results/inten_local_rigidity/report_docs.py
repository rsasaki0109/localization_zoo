"""Render every inherited case and native cluster from complete committed evidence."""
from pathlib import Path
import csv,hashlib,importlib.util,io,json,tempfile
import numpy as np
E=Path(__file__).resolve().parent
# Installed copy lives alongside the committed report reader and analyzer.
def module(name,path):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
reader=module('rigidity_case_reader',E/'report_io.py')
analyzer=module('rigidity_case_geometry',E/'analyze.py')
read=lambda p:json.loads(p.read_text())
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
number=lambda v:'undefined' if v is None else format(float(v),'.9g')
CLASSES=('edge','ground','facade')

def boundary_points(point_groups,boundary):
 key=tuple(boundary[k] for k in ('frame','mode','translation_only','iteration','feature_class'))
 rows=point_groups.get(key,[])
 assert len(rows)==sum(g['count'] for g in boundary['groups'] if g['partition']=='all'),'Missing native-valid boundary points'
 return rows

def metrics(rows):
 scores=[float(r['mean_absolute_length_error_m']) for r in rows if r['mean_absolute_length_error_m']!='']
 g=[float(r['axis_g']) for r in rows if r['axis_g']!='']
 h=[float(r['axis_H']) for r in rows if r['axis_H']!='']
 return dict(count=len(rows),centers_with_relations=len(scores),mean_center_absolute=float(np.mean(scores)) if scores else None,
  median_center_absolute=float(np.median(scores)) if scores else None,minimum_center_absolute=min(scores) if scores else None,
  maximum_center_absolute=max(scores) if scores else None,axis_g=sum(g) if g else None,positive_axis_g=sum(x for x in g if x>0) if g else None,
  negative_axis_g=sum(x for x in g if x<0) if g else None,axis_H=sum(h) if h else None)

def render(output):
 report=reader.read_report();analyzer.bindings();parent=read(E.parent/'inten_prior_control/report/summary.json')
 primary=[];locations=[];boundaries=[];tables=[]
 for group,result in report['groups'].items():
  clouds=analyzer.geometry(group);point_groups={}
  for row in csv.DictReader(io.StringIO(reader.read_file(group+'/points.csv').decode())):
   f,m,tr,it,c=(int(row[k]) for k in ('frame','mode','translation_only','iteration','class'))
   key=f,m,bool(tr),it,c;point_groups.setdefault(key,[]).append(row)
  bycase={r['frame']:r for r in parent['groups'][group]['cases']}
  for b in result['boundaries']:
   rows=boundary_points(point_groups,b)
   parts={name:metrics([r for r in rows if name=='all' or bool(int(r['kept']))==(name=='kept')]) for name in ('all','kept','removed')}
   item=dict(group=group,**{k:b[k] for k in ('frame','mode','translation_only','iteration','feature_class','axis')},partitions=parts)
   boundaries.append(item)
   for local in b['groups']:
    selected=[r for r in rows if int(clouds[b['frame']][b['feature_class']][int(r['source_index']),4])==local['cluster'] and (local['partition']=='all' or bool(int(r['kept']))==(local['partition']=='kept'))]
    stats=metrics(selected);assert stats['count']==local['count']
    locations.append(dict(group=group,**{k:b[k] for k in ('frame','mode','translation_only','iteration','feature_class')},**local,witness=stats))
  for f in read(E/'protocol.json')['population'][group]:
   selected=[b for b in boundaries if b['group']==group and b['frame']==f and b['mode']==14 and not b['translation_only']]
   first=min(b['iteration'] for b in selected);last=max(b['iteration'] for b in selected)
   facade=next(b for b in selected if b['iteration']==first and b['feature_class']==2)
   primary.append(dict(group=group,frame=f,unchanged_length_ratio=bycase[f]['metrics']['14']['length_ratio'],first_iteration=first,last_iteration=last,
    first_facade=facade['partitions']))
  for h in result['histories']:
   a,k,r=(h['partitions'][p] for p in ('all','kept','removed'))
   tables.append('| '+' | '.join(map(str,[group+'/'+str(h['frame']),h['mode'],'refinement' if h['translation_only'] else 'joint',CLASSES[h['feature_class']],h['iterations'],a['centers'],k['relations'],r['relations'],number(k['mean_absolute']),number(r['mean_absolute']),number(k['center_mean_absolute_quantiles'][2]) if k['center_mean_absolute_quantiles'] else 'undefined',number(r['center_mean_absolute_quantiles'][2]) if r['center_mean_absolute_quantiles'] else 'undefined']))+' |')
 summary=dict(exit_code=0,report_manifest_sha256=sha(E/'report/manifest.json'),unchanged_parent_metrics_sha256=sha(E.parent/'inten_prior_control/report/summary.json'),primary=primary,boundaries=boundaries,locations=locations,
  counts={key:sum(g['counts'][key] for g in report['groups'].values()) for key in next(iter(report['groups'].values()))['counts']},
  points=sum(b['partitions']['all']['count'] for b in boundaries),histories=sum(len(g['histories']) for g in report['groups'].values()))
 (output/'cases.summary.json').write_text(json.dumps(summary,indent=2,allow_nan=False)+'\n')
 text=['# Local rigidity: every inherited case','',
  'This is a passive two-frame pair-distance equation conditional on the native assigned anchors, not static-object truth. Rigid moving objects can pass. Sampling, occlusion and cross-object neighbors can fail. Every intermediate iteration enters the complete history aggregates; signed contributions and spatial groups refer to the first and last recorded pre-update linearizations. Repeated centers/relations are not independent observations.','',
  'All source neighborhoods use the same fixed fanout and all-valid neighbor population. Kept/removed partitions split centers only. A center without an eligible neighbor has an undefined score. Native class point indices, full-precision g/H, eligible counts and discrepancy scores remain in the complete point CSVs; join pinned parent features for xyz/row/native cluster. H/g are raw axis partial derivatives with rotation fixed, not solved motion or physical covariance.','',
  'Primary overview: the predeclared arm14 joint phase, first recorded facade linearization. Length/GT is unchanged historical context; it is not used by the witness. `old` and `01` are separate inherited KITTI01 histories.','',
  '| Case | Unchanged length/GT | Facade kept | Removed | Kept center median (m) | Removed center median (m) | Removed positive gt | Removed negative gt |',
  '|---|---:|---:|---:|---:|---:|---:|---:|']
 for p in primary:
  k,r=(p['first_facade'][name] for name in ('kept','removed'))
  text.append('| '+' | '.join(map(str,[p['group']+'/'+str(p['frame']),number(p['unchanged_length_ratio']),k['count'],r['count'],number(k['median_center_absolute']),number(r['median_center_absolute']),number(r['positive_axis_g']),number(r['negative_axis_g'])]))+' |')
 text.extend(['','## Every history and geometric class','',
  'Mean absolute errors weight directed relations. Medians use per-center mean absolute errors over all iterations, and therefore have a different weighting. Neither is a motion-accuracy score or a new acceptance threshold. Complete counts/moments/quantiles and unexecuted phases remain in the lossless report.','',
  '| Case | Arm | Phase | Class | Iterations | All centers | Kept relations | Removed relations | Kept relation MAE (m) | Removed relation MAE (m) | Kept center median (m) | Removed center median (m) |',
  '|---|---:|---|---|---:|---:|---:|---:|---:|---:|---:|---:|',*tables])
 (output/'cases.md').write_text('\n'.join(text)+'\n')
 text=['# Source regions of retained and removed contributions','',
  'Every native source-cluster partition at every first/last pre-update boundary of every case and passive phase is retained. Cluster IDs are frame-local source labels, not tracked physical object IDs. Bounds/centroids are source-frame meters. Edge/ground labels may be a single cluster; exact point locations are obtained by joining full point CSV source_index to the pinned parent support_features record. Empty groups and undefined axes stay explicit.','',
  '| Case | Arm | Phase | Iteration | Class | Cluster | Partition | Count | Source centroid xyz | Source min xyz | Source max xyz | gt | Htt | Center median discrepancy (m) |',
  '|---|---:|---|---:|---|---:|---|---:|---|---|---|---:|---:|---:|']
 vector=lambda v:'undefined' if v is None else ', '.join(number(x) for x in v)
 for row in locations:
  text.append('| '+' | '.join(map(str,[row['group']+'/'+str(row['frame']),row['mode'],'refinement' if row['translation_only'] else 'joint',row['iteration'],CLASSES[row['feature_class']],row['cluster'],row['partition'],row['count'],vector(row['centroid']),vector(row['minimum']),vector(row['maximum']),number(row['axis_g']),number(row['axis_H']),number(row['witness']['median_center_absolute'])]))+' |')
 (output/'locations.md').write_text('\n'.join(text)+'\n')
 print('All cases, histories, source regions and opposing signed contributions rendered without a selector.')
def main():
 packer=module('rigidity_table_regeneration_pack',E/'pack_tables.py')
 with tempfile.TemporaryDirectory(prefix='rigidity_complete_tables_') as folder:
  render(Path(folder));packer.pack(Path(folder),E)

if __name__=='__main__':main()
