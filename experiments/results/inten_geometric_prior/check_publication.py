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
    subprocess.run(['git','clone','--shared','--no-checkout',str(a.worktree),str(dst)],check=True)
    for name in ['docs','evaluation/scripts','evaluation/data','evaluation/config','tests']:
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
    # Some legacy corruption tests compare paths to the verifier's resolved
    # directory. Copy this small study instead of linking its entire directory.
    budget=dst/'experiments/results/inten_iteration_budget'
    budget.unlink()
    shutil.copytree(src/'experiments/results/inten_iteration_budget',budget,
                    ignore=shutil.ignore_patterns('__pycache__'))
    for child in (src/'evaluation').iterdir():
        dest=dst/'evaluation'/child.name
        if not dest.exists(): dest.symlink_to(child,target_is_directory=child.is_dir())
    # Materialize tracked additions/edits relevant to publication tests. Do not
    # follow a historical symlink for writes: replace its directory first.
    for name in ['papers', 'evaluation/src']:
        folder=dst/name
        if folder.is_symlink(): folder.unlink()
        folder.mkdir(exist_ok=True)
        for child in (src/name).iterdir():
            target=folder/child.name
            if not target.exists(): target.symlink_to(child,target_is_directory=child.is_dir())
    for method in ['inten_loam', 'kiss_icp']:
        folder=dst/'papers'/method
        folder.unlink()
        folder.symlink_to(a.worktree/'papers'/method,target_is_directory=True)
    for name in ['docs/roadmap.md','tests/test_inten_geometric_prior.py',
                 'evaluation/src/pcd_dogfooding.cpp','evaluation/src/inten_trace_jsonl.h']:
        target=dst/name
        if target.is_symlink(): target.unlink()
        shutil.copy2(a.worktree/name,target)
    old_test='tests/test_inten_independent_onset.py'
    (dst/old_test).write_bytes(subprocess.check_output(['git','show','HEAD:'+old_test],cwd=a.worktree))
    for study_name in ['inten_independent_onset','inten_geometric_prior']:
        study=Path('experiments/results')/study_name
        (dst/study).symlink_to(a.worktree/study,target_is_directory=True)
    for manifest in (a.worktree/'experiments').glob('inten_kitti_seq_*_full_geometric_prior_matrix.json'):
        shutil.copy2(manifest,dst/'experiments'/manifest.name)
    commands=[['evaluation/scripts/refresh_study_docs.py'],
              ['evaluation/scripts/generate_full_variant_table.py'],
              ['evaluation/scripts/generate_paper_ratio_table.py'],
              ['evaluation/scripts/validate_showcase.py'],
              ['evaluation/scripts/generate_leaderboard.py','--check'],
              ['evaluation/scripts/generate_paper_ready_evidence.py','--check']]
    commands.append(['-m','unittest','discover','-s','tests'])
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
