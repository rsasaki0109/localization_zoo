"""Validate, archive and push complete controls before deleting owned scans."""
from pathlib import Path
import shutil,subprocess
from run import E,W,R,PY,ENV,read,write,sha,git,run,module


def backup(label):
    folder=R/'backups'/label
    if run('push',['git','push','-u','origin','HEAD:refs/heads/codex/inten-facade-loss'],folder):
        raise RuntimeError('Backup failed; preserve owned scans')


def main():
    if git('status','--porcelain'):raise RuntimeError('Clean committed analysis sources required')
    if read(R/'native_complete.json')['exit_code']!=0:raise RuntimeError('Native incomplete')
    archive=module('loss_archive',E/'archive.py').archive
    stage=Path(read(R/'native_complete.json')['stage'])
    if run('analysis',[PY,str(E/'analyze.py')],R/'analysis_record'):
        archive(R/'analysis_record',E/'analysis_failure')
        if (R/'analysis').exists():archive(R/'analysis',E/'partial_analysis_failure')
        raise RuntimeError('Analysis failed; retain failure before correcting')
    archive(stage,E/'controls');archive(R/'analysis',E/'analysis');archive(R/'analysis_record',E/'analysis_execution')
    subprocess.run(['git','add','-f',str(E)],cwd=W,check=True)
    subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Commit every matched-seed facade-loss outcome and native control'],cwd=W,check=True)
    backup('raw_controls')
    inputs=read(stage/'inputs.json');raw=Path(inputs['owned_input_dir'])
    if {f.name:dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted(raw.iterdir()) if f.is_file()}!=inputs['files']:
        raise RuntimeError('Input changed before deletion')
    shutil.rmtree(raw)
    write(E/'cleanup.json',dict(exit_code=0,owned_input_dir=str(raw),hashes_verified_before_removal=True,input_dir_exists_after=raw.exists(),
        backup_before_removal_commit=git('rev-parse','HEAD'),removed_files=len(inputs['files'])))
    subprocess.run(['git','add','-f',str(E/'cleanup.json')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Record verified owned KITTI01 scan cleanup after backed-up controls'],cwd=W,check=True)
    backup('cleanup')
    archive(R/'backups',E/'backup_execution')
    subprocess.run(['git','add','-f',str(E/'backup_execution')],cwd=W,check=True)
    subprocess.run(['git','commit','-m','Retain normal backup receipts for facade-loss controls and cleanup'],cwd=W,check=True)
    print('All diagnostic controls validated, archived, committed and backed up; owned scans removed',flush=True)


if __name__=='__main__':main()
