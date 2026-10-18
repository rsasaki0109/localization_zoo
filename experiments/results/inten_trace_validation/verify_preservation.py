"""Check immutable native sources, previous evidence and published numeric artifacts."""
from pathlib import Path
import argparse,hashlib,json,subprocess
E=Path(__file__).resolve().parent;REPO=E.parents[2]
P=json.loads((E/'protocol.json').read_text())
NATIVE=['common','evaluation/src','evaluation/test','evaluation/CMakeLists.txt',
        ':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',
        ':(glob)papers/*/CMakeLists.txt','CMakeLists.txt']


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',required=True,type=Path)
    parser.add_argument('--base-commit',help='Explicit integration baseline; defaults to the original study base.')
    a=parser.parse_args();base=a.base_commit or P['base_commit'];failures=[]
    expected=json.loads((REPO/'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
    actual={str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (REPO/'experiments/results').glob('*_matrix.json')}
    if actual!=expected:failures.append('Published aggregate inventory differs')
    pub={}
    for path in ['README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json']:
        before=subprocess.check_output(['git','show',base+':'+path],cwd=REPO);now=(REPO/path).read_bytes()
        pub[path]=dict(identical=before==now,sha256=hashlib.sha256(now).hexdigest())
        if before!=now:failures.append('Published result changed: '+path)
    def check(label,args):
        r=subprocess.run(['git','diff','--exit-code',*args],cwd=REPO,capture_output=True)
        if r.returncode:failures.append(label)
        return r.returncode
    native=check('Native source/CMake changed',[base,'--',*NATIVE])
    prior=check('Prior evidence changed',[base,'--','experiments/results','docs/benchmarks',
        ':(exclude)experiments/results/inten_trace_validation',':(exclude)experiments/results/index.json'])
    tool=check('Pinned tool/test source changed',[P['tool_source_commit'],'--','evaluation/scripts/check_inten_trace.py','tests/test_inten_trace_validation.py'])
    report=dict(exit_code=int(bool(failures)),base_commit=base,tool_source_commit=P['tool_source_commit'],
                aggregate_count=len(expected),aggregate_hashes_match=actual==expected,publication=pub,
                native_source_exit_code=native,prior_evidence_exit_code=prior,tool_source_exit_code=tool,failures=failures)
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(exit_code=report['exit_code'],failures=failures)))
    return report['exit_code']


if __name__=='__main__':raise SystemExit(main())
