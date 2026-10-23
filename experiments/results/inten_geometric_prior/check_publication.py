#!/usr/bin/env python3
"""Regenerate in a disposable copy; historical evidence is read-only input."""
import argparse
import json
import re
from pathlib import Path
import shutil
import subprocess
import sys
from receipt import save, sha


def copy_evidence(source, target):
    # Real Python paths preserve path-sensitive corruption checks. Large data
    # stay read-only inputs; only generator-owned top-level JSON is writable.
    def copy_file(src, dst):
        if Path(src).suffix == '.py': return shutil.copy2(src, dst)
        Path(dst).symlink_to(Path(src).resolve())
        return dst
    shutil.copytree(source, target, copy_function=copy_file,
                    ignore=shutil.ignore_patterns('__pycache__'))


def main():
    p=argparse.ArgumentParser(__doc__)
    p.add_argument('--source-root',type=Path,required=True)
    p.add_argument('--worktree',type=Path,required=True)
    p.add_argument('--scratch',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--failed-log',type=Path,help='Recheck every failed case from a completed full discovery log')
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
                        else: copy_evidence(item,dest/item.name)
                elif child.is_file(): shutil.copy2(child,dest)
                else: dest.symlink_to(child,target_is_directory=True)
        elif source.is_file(): shutil.copy2(source,target)
        else: target.symlink_to(source,target_is_directory=True)
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
                 'tests/test_inten_support_information.py',
                 'tests/test_inten_support_source_binding.py',
                 'evaluation/scripts/verify_inten_support_history.py',
                 'evaluation/src/pcd_dogfooding.cpp','evaluation/src/inten_trace_jsonl.h']:
        target=dst/name
        if target.is_symlink(): target.unlink()
        shutil.copy2(a.worktree/name,target)
    old_test='tests/test_inten_independent_onset.py'
    (dst/old_test).write_bytes(subprocess.check_output(['git','show','HEAD:'+old_test],cwd=a.worktree))
    for study_name in ['inten_independent_onset','inten_geometric_prior']:
        study=Path('experiments/results')/study_name
        copy_evidence(a.worktree/study,dst/study)
    for index in range(11):
        name=f'experiments/inten_kitti_seq_{index:02d}_full_geometric_prior_matrix.json'
        (dst/name).write_bytes(subprocess.check_output(['git','show','HEAD:'+name],cwd=a.worktree))
    commands=[['evaluation/scripts/refresh_study_docs.py'],
              ['evaluation/scripts/generate_full_variant_table.py'],
              ['evaluation/scripts/generate_paper_ratio_table.py'],
              ['evaluation/scripts/validate_showcase.py'],
              ['evaluation/scripts/generate_leaderboard.py','--check'],
              ['evaluation/scripts/generate_paper_ready_evidence.py','--check']]
    failed_names=[]
    if a.failed_log:
        failed_text=a.failed_log.read_text()
        failed_names=re.findall(r'^(?:FAIL|ERROR): \S+ \(([^)]+)\)$',failed_text,re.M)
        totals=re.search(r'^FAILED \((.+)\)$',failed_text,re.M)
        assert totals and failed_names, 'Require a completed failing discovery log'
        expected=sum(int(value) for value in re.findall(r'(?:failures|errors)=(\d+)',totals[1]))
        assert len(failed_names)==expected, 'Must recheck every failed case'
        code="import sys,unittest; sys.path.insert(0,'tests'); unittest.main(module=None,argv="+repr(['unittest',*failed_names])+")"
        commands.append(['-c',code])
    else:
        commands.append(['-m','unittest','discover','-s','tests'])
    a.output.mkdir(parents=True,exist_ok=False); receipts=[]
    if a.failed_log:
        save(a.output/'rechecked_cases.json',{'original_log':str(a.failed_log),'original_log_sha256':sha(a.failed_log),'all_failed_cases':failed_names})
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
