"""Preserve every CPU-control byte as lossless edits to the original point CSVs."""
from pathlib import Path
import hashlib,json,lzma,shutil
import common,portable
E,S=common.E,common.S
digest=lambda data:hashlib.sha256(data).hexdigest()

def edits(base,actual):
    left=base.splitlines(keepends=True);right=actual.splitlines(keepends=True);assert len(left)==len(right)
    result=[]
    for index,(a,b) in enumerate(zip(left,right)):
        if a==b:continue
        prefix=0
        while prefix<min(len(a),len(b)) and a[prefix]==b[prefix]:prefix+=1
        suffix=0
        while suffix<min(len(a),len(b))-prefix and a[len(a)-suffix-1]==b[len(b)-suffix-1]:suffix+=1
        result.append(dict(line=index,prefix=prefix,suffix=suffix,insert=b[prefix:len(b)-suffix if suffix else len(b)].decode()))
    return json.dumps(result,separators=(',',':')).encode()+b'\n'

def restore(base,patch):
    lines=base.splitlines(keepends=True);seen=set()
    for row in json.loads(patch):
        index,prefix,suffix=(row[k] for k in ('line','prefix','suffix'))
        assert 0<=index<len(lines) and index not in seen;seen.add(index)
        original=lines[index];assert 0<=prefix<=len(original) and 0<=suffix<=len(original)-prefix
        lines[index]=original[:prefix]+row['insert'].encode()+(original[-suffix:] if suffix else b'')
    return b''.join(lines)

def read_control(core,name):
    folder=E/'cpu_controls';meta=common.read(folder/'manifest.json')['files'][core+'/'+name]
    data=(folder/meta['path']).read_bytes();assert digest(data)==meta['stored_sha256']
    payload=lzma.decompress(data);assert digest(payload)==meta['payload_sha256']
    if name.endswith('points.csv'):
        base=portable.reader.read_file(name);assert digest(base)==meta['base_sha256'];raw=restore(base,payload)
    else:raw=payload
    assert len(raw)==meta['bytes'] and digest(raw)==meta['sha256']
    return raw

def main():
    audit=common.read(E/'checks/cpu_audit/result.json');assert audit['exit_code']==1 and all(v==0 for v in audit['commands'].values())
    payloads={};files={};proof={}
    for core in ('Haswell','Sandybridge'):
        source=S/('cpu_'+core);control=audit['controls'][core]
        assert common.sha(source/'summary.json')==control['complete_summary_sha256']
        proof[core]=portable.compare_outputs(source)
        for name in ['summary.json']+[g+'/points.csv' for g in ('old','01','02','08')]:
            raw=(source/name).read_bytes();header={}
            if name.endswith('points.csv'):
                assert digest(raw)==control['points'][name.split('/')[0]]
                base=portable.reader.read_file(name);payload=edits(base,raw);assert restore(base,payload)==raw
                path=core+'/'+name+'.edits.json.xz';header=dict(storage='original_csv_line_edits_xz',base_sha256=digest(base))
            else:payload=raw;path=core+'/'+name+'.xz';header=dict(storage='complete_xz')
            data=lzma.compress(payload,format=lzma.FORMAT_XZ,check=lzma.CHECK_CRC64,preset=9|lzma.PRESET_EXTREME)
            assert lzma.decompress(data)==payload
            payloads[path]=data;files[core+'/'+name]=dict(path=path,bytes=len(raw),sha256=digest(raw),stored_sha256=digest(data),payload_sha256=digest(payload),**header)
    assert shutil.disk_usage(E).free>2*sum(map(len,payloads.values()))+1024*1024,'Insufficient storage for complete CPU controls and Git objects'
    target=E/'cpu_controls';target.mkdir()
    for path,data in payloads.items():
        p=target/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
    common.write(target/'manifest.json',dict(files=files,source_sha256=common.sha(Path(__file__)),original_failure_sha256=common.sha(E/'checks/cpu_audit/result.json'),
        policy='Complete original CPU-control bytes retained. Per-line prefix/suffix edits have no numeric normalization, row omission or precision reduction; full stored/payload/base/rebuilt hashes and roundtrip verified.'))
    for key in files:
        core,name=key.split('/',1);assert read_control(core,name)==(S/('cpu_'+core)/name).read_bytes()
    common.write(S/'cpu_recovery/result.json',dict(exit_code=0,complete_controls_lossless=True,controls=proof,
        original_bit_equality_failure_retained=True,all_native_data_equations_settings_and_original_tolerances_unchanged=True))
    print('Both complete CPU controls retain every byte and pass original strict numerical/discrete checks.')
if __name__=='__main__':main()
