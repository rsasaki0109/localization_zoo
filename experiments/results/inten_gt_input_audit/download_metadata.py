from pathlib import Path
import hashlib,json,urllib.request,zipfile
R=Path(__file__).resolve().parent
R.joinpath('official').mkdir(exist_ok=True)
records=[]
for name in ('data_odometry_poses.zip','data_odometry_calib.zip'):
    url='https://s3.eu-central-1.amazonaws.com/avg-kitti/'+name
    target=R/'official'/name
    with urllib.request.urlopen(url,timeout=60) as response:
        raw=response.read(16*1024*1024+1)
        assert len(raw)<=16*1024*1024,'Metadata exceeds bounded download'
        final_url=response.geturl()
    with target.open('xb') as stream:stream.write(raw)
    with zipfile.ZipFile(target) as archive:
        assert archive.testzip() is None
        members={i.filename:dict(bytes=i.file_size,crc32=i.CRC) for i in archive.infolist() if not i.is_dir()}
    records.append(dict(request_url=url,final_url=final_url,path=str(target),bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest(),members=members))
R.joinpath('download_manifest.json').write_text(json.dumps(dict(exit_code=0,no_sensor_scans=True,archives=records),indent=2)+'\n')
print('Official pose and calibration archives downloaded and CRC checked; no sensor scans.')
