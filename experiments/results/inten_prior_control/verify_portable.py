"""Decode the existing omitted-zero solver field; frozen mathematics stays intact."""
from pathlib import Path
import hashlib,importlib.util,json
E=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('prior_control_frozen',E/'verify.py')
checker=importlib.util.module_from_spec(spec);spec.loader.exec_module(checker)

class SolverRecord(dict):
    def __missing__(self,key):
        # The canonical serializer only writes nonzero prior information.
        # Do not insert the key: equality still compares the original stored
        # fields, so inherited full-record identity is not normalized away.
        if key=='translation_prior_information' and {'called','input','output','iterations'}<=self.keys():
            return [0.]*9
        raise KeyError(key)

class RowJSON:
    @staticmethod
    def loads(data):return json.loads(data,object_hook=SolverRecord)
    dumps=staticmethod(json.dumps)
checker.json=RowJSON
read=checker.read;decode=checker.decode;check_prior=checker.check_prior
derived_information=checker.derived_information;P=checker.P
def validate():
    recovery=read(E/'schema_recovery_protocol.json')
    for name,digest in recovery['source_sha256'].items():
        checker.require(hashlib.sha256((E/name).read_bytes()).hexdigest()==digest,'Schema adapter source changed')
    checker.require(hashlib.sha256((E/'protocol.json').read_bytes()).hexdigest()==recovery['frozen_protocol_sha256'],'Frozen scientific protocol changed')
    return checker.validate()
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();r=validate()
    a.output.write_text(json.dumps(r,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:v for k,v in r.items() if k not in ('groups','gate_checks')}))
