from pathlib import Path
import ast,hashlib,json,os,shutil,subprocess,time
R=Path(__file__).resolve().parent;W=R/'worktree';E=W/'experiments/results/inten_map_plane_spread';PY='/workspace/localization-env/host-venv/bin/python'
ENV=dict(os.environ,PATH=str(Path(PY).parent)+os.pathsep+os.environ['PATH'],PYTHONDONTWRITEBYTECODE='1',OMP_NUM_THREADS='1',OPENBLAS_NUM_THREADS='1')
def read(p):return json.loads(p.read_text())
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.write_text(json.dumps(v,indent=2,allow_nan=False)+'\n')
def run(label,argv):
    started=time.time()
    with (R/(label+'.stdout')).open('xb') as out,(R/(label+'.stderr')).open('xb') as err:x=subprocess.run(argv,cwd=W,env=ENV,stdout=out,stderr=err)
    write(R/(label+'_command.json'),dict(command=argv,cwd=str(W),source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=W,text=True).strip(),started_unix=started,finished_unix=time.time(),exit_code=x.returncode,environment={k:ENV[k] for k in ('OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','PYTHONDONTWRITEBYTECODE')}));print(label,x.returncode,flush=True)
    if x.returncode:raise RuntimeError(label+' failed; retain everything and stop')
while not (R/'spread_driver.json').exists():time.sleep(2)
assert read(R/'spread_driver.json')['exit_code']==0 and read(R/'spread_complete.json')['exit_code']==0
old=W/'experiments/results/inten_facade_pca';pins=read(old/'binaries.json');spread=read(E/'spread/binary.json')
write(E/'binaries.json',dict(runner=pins['runner'],spread_probe={k:spread[k] for k in ('path','sha256','source_commit')},linked_static_libraries=pins['linked_static_libraries'],scope='Same immutable original159 runner in BOTH full arms. Existing actualCLI spread option only. Separate transformed diagnostic producer never used for accuracy.'))
oldbuild=Path('/workspace/localization-env/inten-facade-pca/build')
tests={label:dict(path=str(path),sha256=sha(path)) for label,path in [('inten',oldbuild/'papers/inten_loam/test_inten_loam'),('paper',oldbuild/'papers/inten_loam/test_inten_loam_paper'),('trace',oldbuild/'evaluation/test_inten_trace_jsonl')]}
write(E/'producer_provenance.json',dict(native_source_commit='9a01528b05128794df962b52ace59e727e834417',inherited_binary_record=str((old/'binaries.json').relative_to(W)),inherited_binary_record_sha256=sha(old/'binaries.json'),test_binaries=tests,diagnostic_producer_record=str((E/'spread/binary.json').relative_to(W)),policy='Production source unchanged from the actual original binary producer. Whole final project CI required. All original binaries, headers, libraries, worktrees and builds preserved.'))
for p in E.glob('*.py'):ast.parse(p.read_text())
subprocess.run(['git','add',str(E),*[str(W/m['path']) for m in read(E/'protocol.json')['manifests'].values()]],cwd=W,check=True)
subprocess.run(['git','diff','--cached','--check'],cwd=W,check=True)
subprocess.run(['git','commit','-m','Freeze uniform full KITTI plane-spread comparison and exact original native producer'],cwd=W,check=True)
run('setup',[PY,str(E/'validate_setup.py')])
run('collector',[PY,str(E/'collect.py')])
write(R/'start_complete.json',dict(exit_code=0))
