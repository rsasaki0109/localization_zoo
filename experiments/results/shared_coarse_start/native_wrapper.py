#!/usr/bin/env python3
"""Experiment-only output isolation; forwards all native arguments unchanged."""
from pathlib import Path
import hashlib,json,os,subprocess,sys,time

args=sys.argv[1:]
summary=Path(args[args.index('--summary-json')+1]).resolve()
binary=Path(os.environ.get('ZOO_COARSE_NATIVE_BINARY','/workspace/localization-env/shared-coarse-start/candidate_pcd_dogfooding'))
native_cwd=summary.parent/(summary.stem+'_native')
native_cwd.mkdir()
(native_cwd/'dogfooding_results').mkdir()
digest=hashlib.sha256()
with binary.open('rb') as stream:
    for chunk in iter(lambda:stream.read(1024*1024),b''):digest.update(chunk)
command=[str(binary),*args]
start=time.time()
record={'command':command,'cwd':str(native_cwd),'binary_sha256':digest.hexdigest(),'started_unix':start,
        'environment':{key:os.environ.get(key) for key in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS')}}
receipt=summary.with_name(summary.stem+'_native_command.json')
try:
    process=subprocess.run(command,cwd=native_cwd)
    record.update(exit_code=process.returncode,finished_unix=time.time())
except OSError as error:
    record.update(exit_code=127,error=str(error),finished_unix=time.time())
receipt.write_text(json.dumps(record,indent=2)+'\n')
raise SystemExit(record['exit_code'])
