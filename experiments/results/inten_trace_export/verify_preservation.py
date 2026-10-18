"""Verify published evidence and the narrow runner-only implementation scope."""
from pathlib import Path
import argparse,hashlib,json,subprocess

E=Path(__file__).resolve().parent
REPO=E.parents[2]
P=json.loads((E/'protocol.json').read_text())
NATIVE=['common','evaluation/src','evaluation/test','evaluation/CMakeLists.txt',
        ':(glob)papers/*/src/**',':(glob)papers/*/include/**',':(glob)papers/*/test/**',
        ':(glob)papers/*/CMakeLists.txt','CMakeLists.txt']
ALLOWED=['evaluation/CMakeLists.txt','evaluation/src/pcd_dogfooding.cpp',
         'evaluation/src/inten_trace_jsonl.h','evaluation/test/test_inten_trace_jsonl.cpp']


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--base-commit', help='Explicit integration baseline; defaults to the original study base.')
    parser.add_argument('--native-source-commit', help='Explicit integration source; defaults to the original source.')
    a=parser.parse_args(); failures=[]; base=a.base_commit or P['base_commit']
    source_commit=a.native_source_commit or P['native_source_commit']
    expected=json.loads((REPO/'experiments/results/shared_near_range/historical_aggregate_sha256.json').read_text())
    actual={str(p.relative_to(REPO)):hashlib.sha256(p.read_bytes()).hexdigest()
            for p in (REPO/'experiments/results').glob('*_matrix.json')}
    if actual != expected: failures.append('Historical aggregate inventory changed')
    publication={}
    for path in ['README.md','docs/index.html','docs/assets/paper/paper_ratio_table.csv','evaluation/data/paper_reported_numbers.json']:
        before=subprocess.check_output(['git','show',base+':'+path],cwd=REPO)
        now=(REPO/path).read_bytes()
        publication[path]=dict(identical=before==now,sha256=hashlib.sha256(now).hexdigest())
        if before != now: failures.append('Published evidence changed: '+path)
    prior=subprocess.run(['git','diff','--exit-code',base,'--','experiments/results','docs/benchmarks',
                          ':(exclude)experiments/results/inten_trace_export',
                          ':(exclude)experiments/results/index.json'],cwd=REPO,capture_output=True)
    if prior.returncode: failures.append('Prior study evidence changed')
    changed=subprocess.check_output(['git','diff','--name-only',base,'--',*NATIVE],cwd=REPO,text=True).splitlines()
    if sorted(changed)!=sorted(ALLOWED):failures.append('Native changes outside allowed runner/header/test/CMake files')
    source=subprocess.run(['git','diff','--exit-code',source_commit,'--',*NATIVE],cwd=REPO,capture_output=True)
    if source.returncode: failures.append('Pinned candidate native source changed')
    report=dict(exit_code=int(bool(failures)),base_commit=base,native_source_commit=source_commit,
                aggregate_count=len(expected),aggregate_hashes_match=actual==expected,publication=publication,
                prior_evidence_exit_code=prior.returncode,native_source_identity_exit_code=source.returncode,
                changed_native_files=changed,allowed_native_files=ALLOWED,failures=failures)
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(exit_code=report['exit_code'],failures=failures)))
    return report['exit_code']


if __name__=='__main__':raise SystemExit(main())
