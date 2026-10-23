"""Preserve the intentional slow07 interruption and its still-owned official inputs."""
from pathlib import Path
import hashlib,importlib.util,json,shutil,subprocess,sys,time
E=Path(__file__).resolve().parent;W=E.parents[2];R=W.parent
def read(p):return json.loads(p.read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    assert not (R/'collector.lock').exists()
    assert read(E/'collection_failure/command.json')['exit_code']==1
    stage=R/'full_staging/07'
    assert read(stage/'baseline/native_command.json')['exit_code']==0
    code=read(stage/'candidate/native_command.json')['exit_code'];assert code in (130,-2)
    intent=read(R/'acceleration_pause_request.json')
    raw=Path(intent['matched_raw']);expected=read(stage/'inputs.json')['files']
    actual={p.name:dict(bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(raw.iterdir()) if p.is_file()}
    assert actual==expected
    spec=importlib.util.spec_from_file_location('interruption_archive',E/'archive.py');a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
    target=E/'collection_interruption'
    a.archive(stage,target/'partial_seq_07')
    a.archive(R/'backups',target/'backups')
    for name in ('acceleration_pause_request.json','failure.json','active.json'):shutil.copyfile(R/name,target/name)
    receipt=dict(exit_code=0,command=[sys.executable,str(Path(__file__).resolve())],cwd=str(W),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),observed_unix=time.time(),
      scope='Archival only; intentional performance interruption is not a completed scientific07 result.',
      original_wrapper_actual_exit=1,original_candidate_actual_exit=code,original_baseline_actual_exit=0,
      all_original_bytes_roundtripped=True,owned_raw=str(raw),owned_raw_hashes_verified=True,raw_deleted=False,
      no_native_rerun=True,no_accuracy_setting_change=True,slow_full01_negative_preserved=True,
      next='Transfer the same officially fetched07 input to a frozen exact accelerated producer; require every full01 numerical trace/pose/GT/metric identical. Archive/commit/push new07 before raw deletion and any other acquisition.')
    (target/'result.json').write_text(json.dumps(receipt,indent=2)+'\n')
    assert {p.name:dict(bytes=p.stat().st_size,sha256=sha(p)) for p in sorted(raw.iterdir()) if p.is_file()}==expected
    print('Intentional interruption, all partial07/backups and owned official-input hashes preserved')
if __name__=='__main__':main()
