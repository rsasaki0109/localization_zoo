import sys,json,hashlib
from pathlib import Path
import numpy as np,scipy
from scipy.spatial import _ckdtree
data=dict(executable=sys.executable,python=sys.version,numpy=np.__version__,scipy=scipy.__version__,numpy_file=np.__file__,scipy_file=scipy.__file__,ckdtree_file=_ckdtree.__file__,ckdtree_sha256=hashlib.sha256(Path(_ckdtree.__file__).read_bytes()).hexdigest())
print(json.dumps(data,indent=2))
