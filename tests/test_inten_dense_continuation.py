"""Every independent dense score and selector outcome remains reviewable."""
from pathlib import Path
import copy,importlib.util,json,unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1];E=ROOT/'experiments/results/inten_dense_continuation'
spec=importlib.util.spec_from_file_location('dense_continuation_verify',E/'verify.py');v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)

class DenseContinuationTest(unittest.TestCase):
    def test_every_cost_and_decision_reconstructs_and_report_matches(self):
        actual=v.validate();v.V.V.V.equivalent(actual,v.read(E/'report/summary.json'))
        self.assertEqual(actual['pose_scores'],31*12)
        self.assertTrue(actual['all_pose_inputs_bound_to_committed_estimator_outputs'])
        self.assertTrue(actual['all_native_costs_independently_reconstructed'])

    def test_predeclared_input_pins_and_every_case_are_retained(self):
        p=v.read(E/'protocol.json');s=v.read(E/'report/summary.json')
        self.assertEqual(s['progression_gate_passed'],all(c['passed'] for c in s['gate_checks']))
        for group,pin in p['input_pins'].items():
            meta=v.read(E/'groups'/group/'input_sources.json')
            self.assertEqual(meta['parent_archive_sha256'],pin['parent_archive_sha256'])
            self.assertEqual(meta['frames'],pin['frames'])
            self.assertEqual([c['frame'] for c in s['groups'][group]['cases']],pin['frames'])
            self.assertTrue(all(set(c['scores'])=={str(m) for m in range(12)} for c in s['groups'][group]['cases']))

    def test_both_directions_and_numeric_margin_are_required(self):
        def score(x,y):return {side:dict(valid=True,mean=value,population=10,within_gate=10) for side,value in (('forward',x),('reverse',y))}
        base=score(.1,.1)
        self.assertTrue(v.choose(base,score(.05,.05)))
        self.assertFalse(v.choose(base,score(.05,.2)))
        self.assertFalse(v.choose(base,score(.1-1e-14,.05)))
        invalid=score(.05,.05);invalid['reverse']['valid']=False
        self.assertFalse(v.choose(base,invalid))

    def mutate_native(self,kind):
        folder=E/'groups/old';rows=[json.loads(r) for r in v.decode(folder,'native.stdout.jsonl').splitlines()]
        if kind=='decision':
            row=next(r for r in rows if r['type']=='decision');row['selected_arms'][0]=6 if row['selected_arms'][0]==0 else 0
        else:
            row=next(r for r in rows if r['type']=='score')
            if kind=='cost':row['forward']['mean']+=.01
            else:row['forward']['population']-=1
        data=('\n'.join(json.dumps(r) for r in rows)+'\n').encode();original=v.decode
        def altered(f,n):return data if f==folder and n=='native.stdout.jsonl' else original(f,n)
        return altered

    def test_checker_rejects_changed_cost(self):
        with patch.object(v,'decode',self.mutate_native('cost')):
            with self.assertRaisesRegex(ValueError,'cost reconstruction'):v.group_result('old')

    def test_checker_rejects_changed_population(self):
        with patch.object(v,'decode',self.mutate_native('population')):
            with self.assertRaisesRegex(ValueError,'population/coverage'):v.group_result('old')

    def test_checker_rejects_changed_selection(self):
        with patch.object(v,'decode',self.mutate_native('decision')):
            with self.assertRaisesRegex(ValueError,'decision differs'):v.group_result('old')
