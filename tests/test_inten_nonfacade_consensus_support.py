"""Guard evidence must retain harmful acceptance and lost necessary corrections."""
from pathlib import Path
import unittest,importlib.util,json,copy
ROOT=Path(__file__).resolve().parents[1];E=ROOT/'experiments/results/inten_nonfacade_consensus_support'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
class CommonSupportTest(unittest.TestCase):
    def test_complete_native_and_passive_controls_reconstruct_failed_gate(self):
        result=module('support_all_evidence',E/'verify.py').validate(history=False)
        self.assertEqual(result['exit_code'],0)
        self.assertFalse(result['gate_passed'])
        self.assertTrue(any(r['group']=='02' and r['frame']==1892 for r in result['failed_gate_checks']))
    def test_fully_supported_wrong_pose_and_lost_rescue_are_not_hidden(self):
        cases=json.loads((E/'groups/02/result.json').read_text())['cases']
        bad=next(c for c in cases if c['frame']==1892)
        self.assertTrue(bad['support']['allowed'])
        self.assertEqual(bad['support']['missing_field'],0)
        self.assertTrue(bad['metrics']['2']['bad_length'])
        self.assertFalse(bad['metrics']['0']['bad_length'])
        necessary=next(c for c in cases if c['frame']==1891)
        self.assertFalse(necessary['guard_applied'])
        self.assertTrue(necessary['metrics']['2']['bad_length'])
        self.assertFalse(necessary['metrics']['1']['bad_length'])
    def test_checker_rejects_tampered_primitive_and_falsely_accepted_guard(self):
        v=module('support_corruption_checks',E/'validate.py')
        rows=list(map(json.loads,v.decode(E/'groups/02','native.stdout.jsonl').splitlines()))
        row=next(r for r in rows if r['type']=='terminal' and r['frame']==1892 and r['mode']==2)
        ps=[p for p in map(json.loads,v.decode(E/'groups/02','primitives.jsonl').splitlines()) if p['frame']==1892]
        v.check_support(row,ps)
        altered=copy.deepcopy(ps);next(p for p in altered if p['kind']=='line')['changed']+=.01
        with self.assertRaises(ValueError):v.check_support(row,altered)
        altered=copy.deepcopy(row);altered['support']['allowed']=False
        with self.assertRaises(ValueError):v.check_support(altered,ps)
