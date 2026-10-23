"""Lossless XZ transport adapter; the prospectively frozen verifier is unchanged."""
from pathlib import Path
import hashlib,importlib.util,json,lzma
E=Path(__file__).resolve().parent
s=importlib.util.spec_from_file_location('reciprocal_frozen_verifier',E/'verify.py');checker=importlib.util.module_from_spec(s);s.loader.exec_module(checker)
V=checker.V;read=checker.read;masks=checker.masks
def decode(folder,name):
    entry=read(folder/'archive.json')['files'][name]
    if entry['storage']!='xz':return V.decode(folder,name)
    record=read(E/'transport_recovery_protocol.json')['groups'][folder.name]
    checker.require(entry['sha256']==record['raw_sha256'] and entry['bytes']==record['raw_bytes'],'Transport raw measurement pin changed')
    data=(folder/entry['path']).read_bytes()
    checker.require(hashlib.sha256(data).hexdigest()==entry['stored_sha256']==record['xz_sha256'],'Transport XZ hash changed')
    raw=lzma.decompress(data)
    checker.require(len(raw)==entry['bytes'] and hashlib.sha256(raw).hexdigest()==entry['sha256'],'Transport bytes do not restore exactly')
    return raw
checker.decode=decode
def validate():
    protocol=read(E/'transport_recovery_protocol.json')
    for name,digest in protocol['source_sha256'].items():
        checker.require(hashlib.sha256((E/name).read_bytes()).hexdigest()==digest,'Transport adapter source changed')
    checker.require(hashlib.sha256((E/'protocol.json').read_bytes()).hexdigest()==protocol['scientific_protocol_sha256'],'Scientific protocol changed during transport')
    return checker.validate()
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);args=p.parse_args();r=validate()
    args.output.write_text(json.dumps(r,indent=2,allow_nan=False)+'\n')
    print(json.dumps({k:r[k] for k in ('exit_code','native_outcomes','progression_gate_passed','motion_totals','backward_queries','max_backward_distance_error','max_solver_backward_error')}))
