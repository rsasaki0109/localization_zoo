"""Regenerate canonical study docs in a private copy, preserving older results."""
from pathlib import Path
import json,shutil
import common
E,W,S=common.E,common.W,common.S

def main():
    stage=S/'refresh_check';stage.mkdir();shadow=stage/'repository';shadow.mkdir()
    for name in ('evaluation','docs','papers'):
        shutil.copytree(W/name,shadow/name,ignore=shutil.ignore_patterns('__pycache__'))
    shutil.copy2(W/'README.md',shadow/'README.md')
    experiments=shadow/'experiments';experiments.mkdir();results=experiments/'results';results.mkdir()
    for source in (W/'experiments').glob('*.json'):shutil.copy2(source,experiments/source.name)
    for source in (W/'experiments/results').glob('*.json'):shutil.copy2(source,results/source.name)
    index=common.read(W/'experiments/results/index.json')
    for entry in index['problems']:
        for name in ('manifest_path','aggregate_path'):
            path=Path(entry[name]);assert not path.is_absolute() and '..' not in path.parts
            destination=shadow/path;destination.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(W/path,destination)
    codes={}
    for label,script in (('refresh','refresh_study_docs.py'),('full_variant','generate_full_variant_table.py'),('paper_ratio','generate_paper_ratio_table.py')):
        commands=common.PREFIX+[str(shadow/'evaluation/scripts'/script)]
        codes[label]=common.run(label,commands,stage)
        if codes[label]:break
    result=dict(exit_code=1,commands=codes,shadow_repository=str(shadow))
    try:
        assert len(codes)==3 and all(v==0 for v in codes.values())
        actual=common.read(results/'index.json');actual.pop('generated_at',None);expected=dict(index);expected.pop('generated_at',None)
        assert actual==expected,'Canonical index changed beyond generated_at'
        preserved={}
        for path in sorted((shadow/'docs/assets/paper').glob('*.csv')):
            relative=path.relative_to(shadow);assert path.read_bytes()==(W/relative).read_bytes(),str(relative)
            preserved[str(relative)]=common.sha(path)
        assert (shadow/'README.md').read_bytes()==(W/'README.md').read_bytes(),'Generated root README differs'
        for entry in index['problems']:
            path=Path(entry['aggregate_path']);assert (shadow/path).read_bytes()==(W/path).read_bytes(),str(path)
        result.update(exit_code=0,all_canonical_numeric_csvs_exact=preserved,all_indexed_aggregates_exact=True,
            index_exact_except_generated_at=True,root_README_exact=True,
            policy='Complete canonical driver and both required paper-table generators execute in an isolated copy. Generated timestamps/plot metadata stay private; no historical committed result is rewritten.')
    except Exception as error:result['error']=str(error)
    common.write(stage/'result.json',result)
    # Archive receipts and complete logs; generated copied repository remains private.
    archive=stage/'receipts';archive.mkdir()
    for path in stage.iterdir():
        if path.is_file():shutil.copy2(path,archive/path.name)
    common.module('rigidity_refresh_archive',common.P/'archive.py').archive(archive,E/'checks/canonical_refresh')
    return result['exit_code']
if __name__=='__main__':raise SystemExit(main())
