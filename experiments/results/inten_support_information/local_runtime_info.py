import json,platform,sys,numpy,scipy
print(json.dumps(dict(python=sys.version,executable=sys.executable,numpy=numpy.__version__,scipy=scipy.__version__,platform=platform.platform(),machine=platform.machine()),indent=2))
