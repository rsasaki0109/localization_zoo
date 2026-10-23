#!/usr/bin/env python3
"""Record official local/central ZIP lengths without downloading a scan."""
import json
from pathlib import Path
import sys
import urllib.request
sys.path.insert(0,str(Path(__file__).resolve().parents[3]/'evaluation/scripts'))
import fetch_kitti_odometry_sequences as f
from receipt import save
with f.RemoteZip(f.DEFAULT_URL) as archive:
    infos=f.select_frame_infos(f.select_sequence_infos(archive.infolist(),'00'),list(range(64)))
info=infos[0]
start=info.header_offset
request=urllib.request.Request(f.DEFAULT_URL,headers={'Range':f'bytes={start}-{start+29}'})
with urllib.request.urlopen(request,timeout=120) as response:
    data=response.read()
    assert response.status==206 and len(data)==30
    fields=f.LOCAL_FILE_HEADER.unpack(data)
    row={'url':f.DEFAULT_URL,'requested_range':request.get_header('Range'),
         'status':response.status,'content_range':response.headers.get('Content-Range'),
         'content_length':response.headers.get('Content-Length'),'local_header_hex':data.hex(),
         'filename':info.filename,'header_offset':start,'payload_bytes':info.compress_size,
         'central_extra_length':len(info.extra),'local_extra_length':fields[-1],
         'central_filename_length':len(info.filename.encode()),'local_filename_length':fields[-2],
         'selected_frame_run_lengths':[len(run) for run in f.contiguous_frame_runs(infos)]}
    save(Path(__file__).with_name('kitti_00_range_probe.json'),row)
    print(json.dumps(row))
