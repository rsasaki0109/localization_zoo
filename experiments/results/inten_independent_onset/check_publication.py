#!/usr/bin/env python3
"""Regenerate in a disposable copy; historical evidence is read-only input."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys
from receipt import save, sha


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--worktree',type=Path,required=True)
    p.add_argument('--scratch',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a=p.parse_args(); src=a.source_root; dst=a.scratch
    dst.mkdir(parents=True,exist_ok=False)
    for name in ['docs','evaluation/scripts','evaluation/data','tests']:
        shutil.copytree(src/name,dst/name,ignore=shutil.ignore_patterns('__pycache__'))
    # Only generators' writable directories get copies. Large archived run
    # directories are linked for reading, never targets of the selected commands.
    for source in src.iterdir():
        target=dst/source.name
        if source.name=='.git' or target.exists(): continue
        if source.name=='experiments':
            target.mkdir()
            for child in source.iterdir():
                dest=target/child.name
                if child.name=='results':
                    dest.mkdir()
                    for item in child.iterdir():
                        if item.is_file(): shutil.copy2(item,dest/item.name)
                        else: (dest/item.name).symlink_to(item,target_is_directory=True)
                elif child.is_file(): shutil.copy2(child,dest)
                else: dest.symlink_to(child,target_is_directory=True)
        elif source.is_file(): shutil.copy2(source,target)
        else: target.symlink_to(source,target_is_directory=True)
    for child in (src/'evaluation').iterdir():
        dest=dst/'evaluation'/child.name
        if not dest.exists(): dest.symlink_to(child,target_is_directory=child.is_dir())
    for name in ['docs/roadmap.md','tests/test_inten_independent_onset.py']:
        shutil.copy2(a.worktree/name,dst/name)
    study=Path('experiments/results/inten_independent_onset')
    (dst/study).symlink_to(a.worktree/study,target_is_directory=True)
    commands=[['evaluation/scripts/refresh_study_docs.py'],
              ['evaluation/scripts/generate_full_variant_table.py'],
              ['evaluation/scripts/generate_paper_ratio_table.py'],
              ['evaluation/scripts/validate_showcase.py'],
              ['evaluation/scripts/generate_leaderboard.py','--check'],
              ['evaluation/scripts/generate_paper_ready_evidence.py','--check']]
    for pattern in ['test_inten_independent_onset.py','test_full_variant_table.py',
                    'test_paper_ratio_table.py','test_diagnose_kitti_steps.py','test_external_odometry_evaluator.py']:
        commands.append(['-m','unittest','discover','-s','tests','-p',pattern])
    a.output.mkdir(parents=True,exist_ok=False); receipts=[]
    for index,argv in enumerate(commands):
        result=subprocess.run([sys.executable,*argv],cwd=dst,capture_output=True,text=True)
        (a.output/f'{index:02d}.stdout').write_text(result.stdout)
        (a.output/f'{index:02d}.stderr').write_text(result.stderr)
        receipts.append({'argv':[sys.executable,*argv],'cwd':str(dst),'exit_code':result.returncode})
        save(a.output/'commands.json',receipts)
        print(f'{argv}: {result.returncode}',flush=True)
    protected=['full_variant_results.csv','full_variant_results.tex','paper_ratio_table.csv','paper_ratio_table.tex']
    same={name:sha(src/'docs/assets/paper'/name)==sha(dst/'docs/assets/paper'/name) for name in protected}
    save(a.output/'summary.json',{'commands_all_passed':all(r['exit_code']==0 for r in receipts),
        'regenerated_tables_byte_equal':same,
        'historical_source_unchanged':subprocess.check_output(['git','status','--porcelain'],cwd=src,text=True)==''})
    assert all(r['exit_code']==0 for r in receipts)
    assert all(same.values()), 'diagnostic study must not change benchmark tables'


if __name__=='__main__':
    main()
