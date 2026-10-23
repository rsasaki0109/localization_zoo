import urllib.request,urllib.error,json,hashlib
url='https://s3.eu-central-1.amazonaws.com/avg-kitti/data_odometry_velodyne.zip'
rows=[]
for requested in ['bytes=9936447580-9936447707','bytes=-65536']:
 req=urllib.request.Request(url,headers={'Range':requested})
 try: response=urllib.request.urlopen(req,timeout=20)
 except urllib.error.HTTPError as error: response=error
 with response:
  data=response.read(65536 if response.status==206 else 1024)
  row={'requested_range':requested,'status':response.status,'content_range':response.headers.get('Content-Range'),'bytes_read':len(data),'sha256':hashlib.sha256(data).hexdigest()}
  if response.status!=206:row['body']=data.decode(errors='replace')
  rows.append(row)
print(json.dumps(rows,indent=2))
raise SystemExit(0 if all(r['status']==206 for r in rows) else 1)
