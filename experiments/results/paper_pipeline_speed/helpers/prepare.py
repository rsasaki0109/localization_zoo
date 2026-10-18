from pathlib import Path
import hashlib,json,os,shutil,subprocess,sys,time,socket,urllib.parse
root=Path('/workspace/localization_zoo')
out=Path('/workspace/localization-env/paper-speed')
docker=['env','-u','DOCKER_HOST','-u','DOCKER_CONTEXT','-u','DOCKER_TLS','-u','DOCKER_TLS_VERIFY','-u','DOCKER_CERT_PATH','docker','--host=unix:///var/run/docker.sock']
def run(label, command):
    started=time.time()
    child_env=os.environ.copy()
    for name in ['HTTP_PROXY','HTTPS_PROXY','http_proxy','https_proxy']:
        value=child_env.get(name)
        if value:
            parsed=urllib.parse.urlsplit(value)
            if parsed.hostname=='proxy':
                child_env[name]=urllib.parse.urlunsplit(parsed._replace(netloc=parsed.netloc.replace('proxy',socket.gethostbyname('proxy'))))
    with (out/(label+'.log')).open('w') as log:
        result=subprocess.run(command,cwd=root,stdout=log,stderr=subprocess.STDOUT,env=child_env)
    receipt={'label':label,'command':command,'cwd':str(root),'started_unix':started,'finished_unix':time.time(),'exit_code':result.returncode,'log':label+'.log'}
    (out/(label+'.json')).write_text(json.dumps(receipt,indent=2)+'\n')
    print(json.dumps(receipt),flush=True)
    if result.returncode: raise SystemExit(result.returncode)
if sys.argv[1]=='fetch':
    run('fetch_kitti07',['/workspace/localization-env/host-venv/bin/python','evaluation/scripts/fetch_kitti_odometry_sequences.py','--output-root',str(out/'kitti'),'--sequence','07'])
    scans=sorted((out/'kitti/dataset/sequences/07/velodyne').glob('*.bin'))
    (out/'input_sha256.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in scans},indent=2)+'\n')
    link=root/'dogfooding_results/paper_speed_kitti07'
    if link.exists() or link.is_symlink(): raise RuntimeError(f'Unexpected preexisting dataset path {link}')
    link.symlink_to(out/'kitti/dataset/sequences/07/velodyne',target_is_directory=True)
    print('KITTI07 scans ready; hashes saved',flush=True)
elif sys.argv[1]=='tools':
    run('install_profiler',docker+['exec','-e','HTTP_PROXY','-e','HTTPS_PROXY','-e','http_proxy','-e','https_proxy','-e','NO_PROXY','-e','no_proxy','localization-zoo-dev','sh','-c','apt-get -o Acquire::http::Proxy="$HTTP_PROXY" -o Acquire::https::Proxy="$HTTPS_PROXY" update && DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::http::Proxy="$HTTP_PROXY" -o Acquire::https::Proxy="$HTTPS_PROXY" install -y google-perftools libgoogle-perftools-dev'])
    run('baseline_build',docker+['exec','-w',str(root),'localization-zoo-dev','cmake','--build','build','--target','pcd_dogfooding','-j2'])
    baseline=out/'baseline_pcd_dogfooding'
    shutil.copy2(root/'build/evaluation/pcd_dogfooding',baseline)
    commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip()
    metadata={'source_commit':commit,'binary_path':str(baseline),'binary_sha256':hashlib.sha256(baseline.read_bytes()).hexdigest(),'source_unchanged':not subprocess.check_output(['git','status','--porcelain'],cwd=root,text=True).strip()}
    (out/'baseline_binary.json').write_text(json.dumps(metadata,indent=2)+'\n')
    print(json.dumps(metadata),flush=True)
else: raise ValueError(sys.argv[1])
