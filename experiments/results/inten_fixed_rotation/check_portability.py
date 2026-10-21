"""Record checks after angle-roundoff portability and generated-file cleanup."""
import subprocess
from common import E,W,R,PY,BASE,read,write,run,module

def main():
    assert not subprocess.check_output(['git','status','--porcelain'],cwd=W)
    folder=R/'portability_checks_corrected'
    commands=[('python',[PY,'-m','unittest','discover','-s','tests']),
              ('preservation',[PY,str(E/'verify.py'),'--output',str(folder/'preservation.json')]),
              ('whitespace',['git','diff',BASE,'--check'])]
    codes={name:run(name,command,folder) for name,command in commands}
    assert not subprocess.check_output(['git','diff','0e2ab939','--',str(E/'controls'),str(E/'analysis')],cwd=W)
    write(folder/'result.json',dict(exit_code=int(any(codes.values())),codes=codes,native_equations_metrics_gate_unchanged=True,
          bytecode_untracked_only=True,native_bitwise_rotation_checks_unchanged=True,
          offline_angle_tolerance_reason='acos near identity amplifies last-bit platform matrix multiplication differences; match inherited absolute1e-6mrad tolerance, retain exact native rotation and gate decisions'))
    module('fixed_rotation_portability_archive',E/'archive.py').archive(folder,E/'checks/portability_corrected')
    assert not any(codes.values())

if __name__=='__main__':main()
