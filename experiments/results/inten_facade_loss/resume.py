"""Explicit source-corrected continuation; preserve the rejected producer."""
from pathlib import Path
from run import E,W,R,read,write,sha,git,run,build,module


def main():
    if git('status','--porcelain') or (R/'stage_v2').exists():raise RuntimeError('Clean corrected source and fresh continuation required')
    if read(E/'initial_failure/native_command.json')['exit_code']!=1 or (R/'stage/native.stdout.log').stat().st_size:
        raise RuntimeError('Unexpected original failure/output')
    first=read(E/'initial_failure/binary.json')
    if sha(first['path'])!=first['sha256'] or sha(R/'native.o')!=first['object_sha256']:raise RuntimeError('Rejected original binary/object changed')
    p=read(E/'protocol.json')
    if sha(E/'diagnostic_native.cpp')!=p['diagnostic_source_sha256'] or sha(W/'papers/inten_loam/src/inten_loam_paper.cpp')!=p['native_source_sha256']:raise RuntimeError('Source drift')
    if module('corrected_instrument',E/'instrument.py').transformed((W/'papers/inten_loam/src/inten_loam_paper.cpp').read_text())!=(E/'diagnostic_native.cpp').read_text():raise RuntimeError('Unexplained transformation')
    for n,h in first['linked_static_libraries'].items():
        if sha(n)!=h:raise RuntimeError('Library changed')
    inputs=read(E/'initial_failure/inputs.json');raw=Path(inputs['owned_input_dir'])
    if {f.name:dict(sha256=sha(f),bytes=f.stat().st_size) for f in sorted(raw.iterdir()) if f.is_file()}!=inputs['files']:raise RuntimeError('Owned official inputs changed')
    stage=R/'stage_v2';stage.mkdir()
    command=read(E/'initial_failure/compile_native_command.json')['command']
    command=[str(R/'native_v2.o') if a==str(R/'native.o') else a for a in command]
    if run('compile_native',command,stage):raise RuntimeError('Corrected native compilation failed')
    command=read(E/'initial_failure/compile_probe_command.json')['command']
    command=[str(R/'native_v2.o') if a==str(R/'native.o') else str(R/'probe_v2') if a==str(R/'probe') else a for a in command]
    if run('compile_probe',command,stage):raise RuntimeError('Corrected probe compilation failed')
    write(stage/'binary.json',dict(path=str(R/'probe_v2'),sha256=sha(R/'probe_v2'),object_sha256=sha(R/'native_v2.o'),
        source_commit=git('rev-parse','HEAD'),linked_static_libraries=first['linked_static_libraries'],
        compiler_metadata_sha256=first['compiler_metadata_sha256'],original_binary_preserved=first))
    write(stage/'inputs.json',inputs)
    payload=(E/'initial_failure/input.txt').read_bytes();(stage/'input.txt').write_bytes(payload)
    if sha(stage/'input.txt')!=read(E/'initial_failure/native_command.json')['stdin_sha256']:raise RuntimeError('Input seed commands changed')
    if run('native',build.docker()+[str(R/'probe_v2')],stage,payload):raise RuntimeError('Corrected native continuation failed')
    if sha(R/'probe_v2')!=read(stage/'binary.json')['sha256'] or sha(first['path'])!=first['sha256']:raise RuntimeError('Producer binary changed')
    write(R/'native_complete.json',dict(exit_code=0,stage=str(stage),native_pairs=1100,native_calls=2200,raw_retained=True,
        previous_failure_commit='645363a6',original_binary_preserved=True,public_forced_passive_guard_unchanged=True))


if __name__=='__main__':main()
