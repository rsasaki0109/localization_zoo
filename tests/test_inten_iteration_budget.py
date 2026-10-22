"""Budget continuation must preserve exact controls and expose all adverse results."""
from pathlib import Path
import copy,importlib.util,json,unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];E=ROOT/'experiments/results/inten_iteration_budget'
spec=importlib.util.spec_from_file_location('budget_evidence_verify',E/'verify.py')
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)

class IterationBudgetTest(unittest.TestCase):
    def test_complete_native_controls_prefixes_equations_and_recorded_report(self):
        actual=v.validate();v.V.V.equivalent(actual,v.read(E/'report/summary.json'))
        self.assertEqual(actual['native_outcomes'],31*12)
        self.assertTrue(actual['all_186_inherited_traces_exact'])
        self.assertTrue(actual['all_extended_joint_prefixes_exact'])

    def test_each_case_and_arm_remains_and_gate_has_no_hidden_outcome_selection(self):
        s=v.read(E/'report/summary.json')
        self.assertEqual(s['progression_gate_passed'],all(c['passed'] for c in s['gate_checks']))
        for group,count in (('old',8),('01',6),('02',8),('08',9)):
            cases=s['groups'][group]['cases'];self.assertEqual(len(cases),count)
            self.assertTrue(all(set(c['metrics'])=={str(m) for m in range(12)} for c in cases))

    def test_solver_and_header_source_are_identical_to_parent(self):
        new=v.read(E/'protocol.json')['source_sha256']
        old=v.read(E.with_name('inten_intensity_consistent')/'protocol.json')['source_sha256']
        for name in ('papers/inten_loam/src/inten_loam_paper.cpp','papers/inten_loam/include/inten_loam/inten_loam_paper.h'):
            self.assertEqual(new[name],old[name])

    def old_rows(self):
        return [json.loads(r) for r in v.decode(E/'groups/old','native.stdout.jsonl').splitlines()]

    def test_checker_rejects_changed_native_delta_and_pose(self):
        rows=self.old_rows();trace=next(r['trace']['s2s'] for r in rows if r['type']=='terminal' and r['frame']==247 and r['mode']==6)
        v.check_solver(trace,100)
        changed=copy.deepcopy(trace);changed['iterations'][0]['delta'][0]+=.01
        with self.assertRaisesRegex(ValueError,'backward equation'):v.check_solver(changed,100)
        changed=copy.deepcopy(trace);changed['output'][3]+=.01
        with self.assertRaisesRegex(ValueError,'recorded update'):v.check_solver(changed,100)

    def test_checker_rejects_joint_prefix_corruption(self):
        rows=self.old_rows();r=next(r for r in rows if r['type']=='terminal' and r['frame']==190 and r['mode']==6)
        r['trace']['s2s']['iterations'][0]['weighted_squared_residual']+=.01
        raw=('\n'.join(json.dumps(r) for r in rows)+'\n').encode();original=v.decode
        def altered(folder,name):
            return raw if folder==E/'groups/old' and name=='native.stdout.jsonl' else original(folder,name)
        with patch.object(v,'decode',altered):
            with self.assertRaisesRegex(ValueError,'prefix'):v.group_result('old')

    def test_cli_proof_rejects_an_ignored_override(self):
        folder=E/'checks/cli';original=v.decode
        rows=[json.loads(r) for r in original(folder,'extended.trace.jsonl').splitlines()]
        rows[0]['params']['s2s_iterations']=10
        raw=('\n'.join(json.dumps(r) for r in rows)+'\n').encode()
        with patch.object(v,'decode',lambda f,n:raw if f==folder and n=='extended.trace.jsonl' else original(f,n)):
            with self.assertRaisesRegex(ValueError,'CLI budget'):v.check_cli()
