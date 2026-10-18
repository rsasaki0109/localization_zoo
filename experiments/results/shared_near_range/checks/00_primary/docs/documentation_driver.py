from pathlib import Path
import json,subprocess
repo=Path('/workspace/localization_zoo');E=repo/'experiments/results/shared_near_range'
commit='485fa00601644731827dae601eb3b43c7543efeb'
report=json.loads(subprocess.check_output(['git','show',commit+':experiments/results/shared_near_range/sequences/00/audit/report.json'],cwd=repo))
paths={'A-LOAM':'aloam','F-LOAM':'floam','LeGO-LOAM':'lego_loam','Mesh-LOAM':'mesh_loam','VLOM':'vlom'}
for method,folder in paths.items():
 row=next(r for r in report['rows'] if r['method']==method);assert row['audit_status']=='drift'
 p=repo/'papers'/folder/'README.md';text=p.read_text();assert '<!-- SHARED_NEAR_RANGE_00_PRIMARY -->' not in text
 block=f'''
<!-- SHARED_NEAR_RANGE_00_PRIMARY -->
## Shared-loader replay: KITTI 00 comparison in progress

The full 00 replay of the stored `{row['variant_id']}` arguments completes all
{row['frames']} frames, with the optional near-range cut still off. The historical
100 m RPE changes from **{row['stored_rpe_trans_pct']:.6f}%** to
**{row['rerun_rpe_trans_pct']:.6f}%**; ATE changes from **{row['stored_ate_m']:.6f} m**
to **{row['rerun_ate_m']:.6f} m**. RPE worsens. ATE {'worsens too' if row['rerun_ate_m']>row['stored_ate_m'] else 'improves'}, so these outcomes are retained separately.
The native process exits 0, but the historical audit exits **1**.

The frozen pre-extraction comparison is still running. The cause of this
historical difference and whether extraction affects this cell remain
**unresolved** until that comparison completes. No setting is reselected;
the stored aggregate and published paper comparison are preserved.

[Completed candidate commands, actual exits, logs and summaries](../../experiments/results/shared_near_range/sequences/00/audit/)
were committed at `{commit}`.
[Protocol and current scope](../../experiments/results/shared_near_range/README.md)
distinguish these completed runs from a cleaned sequence checkpoint: 00 input
cleanup and the whole-zoo audit are incomplete. Defaults remain off.
<!-- /SHARED_NEAR_RANGE_00_PRIMARY -->
'''
 p.write_text(text.rstrip()+'\n'+block)
print('All five numeric reproduction differences documented from committed primary evidence.')
