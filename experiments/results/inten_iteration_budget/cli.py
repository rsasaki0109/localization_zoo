"""Actual production CLI: default, explicit default, extended budget, invalid input."""
from common import *
from build import docker

def main():
    assert not git('status','--porcelain')
    stage=R/'cli_stage';stage.mkdir()
    binary=R/'pcd_dogfooding'
    assert sha(binary)==read(E/'checks/build/binaries.json')['binaries']['pcd_dogfooding']['sha256']
    fixture=W/'evaluation/fixtures/mcd_kth_smoke';truth=W/'evaluation/fixtures/mcd_kth_smoke_gt.csv'
    write(stage/'inputs.json',dict(binary_sha256=sha(binary),truth_sha256=sha(truth),
        fixture_sha256={str(p.relative_to(W)):sha(p) for p in sorted(fixture.rglob('*')) if p.is_file()}))
    for label,budget in (('default',None),('explicit',10),('extended',100),('invalid_fraction',1.5)):
        command=[str(binary),str(fixture),str(truth),'3','--methods','inten_loam',
            '--inten-loam-paper-odometry','--no-gt-seed','--paper-pipeline-accelerated',
            '--summary-json',str(stage/(label+'.summary.json')),
            '--inten-loam-paper-trace-jsonl',str(stage/(label+'.trace.jsonl'))]
        if budget is not None:command+=['--inten-loam-paper-set','s2s_iterations='+str(budget)]
        # Disable core dumps for the intentional invalid-input exception only.
        actual=docker()+['bash','-c','ulimit -c 0; exec "$@"','--',*command]
        code=run(label,actual,stage)
        assert (code!=0) if label=='invalid_fraction' else (code==0)
    module('budget_cli_archive',E/'archive.py').archive(stage,E/'checks/cli')
if __name__=='__main__':main()
