"""Reject false safeguard promotion and corrupted native control evidence."""
from pathlib import Path
import importlib.util,unittest,json,gzip,copy
ROOT=Path(__file__).resolve().parents[1];E=ROOT/'experiments/results/inten_zero_consensus_control'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
class ZeroConsensusTest(unittest.TestCase):
    def test_all_cases_reconstruct_and_failed_gate_is_retained(self):
        result=module('zero_verify_all',E/'verify.py').validate(history=False)
        self.assertEqual(result['exit_code'],0)
        self.assertFalse(result['gate_passed'])
        failed={(r['group'],r['frame']) for r in result['failed_gate_checks']}
        self.assertTrue({('old',200),('old',202),('old',203)} <= failed)
    def test_native_checker_rejects_wrong_selection_mask_and_rotation(self):
        check=module('zero_verify_corruption',E/'check.py')
        rows=[json.loads(line) for line in gzip.decompress((E/'groups/old/native.stdout.jsonl.parts/part-00000.gz').read_bytes()).splitlines()]
        trace=next(r['trace'] for r in rows if r['type']=='terminal' and r['frame']==204 and r['mode']==2)
        check.check_frame(trace,2)
        for kind in ('selection','mask','rotation'):
            changed=copy.deepcopy(trace);c=changed['s2s_consensus']
            if kind=='selection':c['best_index']=1
            elif kind=='mask':c['excluded_clusters'].append(999999)
            else:c['refinement']['output'][0]+=1e-4
            with self.assertRaises(AssertionError):check.check_frame(changed,2)
