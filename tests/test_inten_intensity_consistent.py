"""Validate the fixed-node field separately from every motion-gate outcome."""
from pathlib import Path
import copy, gzip, importlib.util, json, unittest
ROOT=Path(__file__).resolve().parents[1]
E=ROOT/'experiments/results/inten_intensity_consistent'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path)
    result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
v=module('consistent_evidence_verify',E/'verify.py')
c=module('consistent_recorded_check',E/'check_recorded.py')

class ConsistentIntensityTest(unittest.TestCase):
    def test_all_native_fields_and_inherited_motion_controls_reconstruct(self):
        actual,details=v.validate()
        c.check(actual,details,v.read(E/'report/summary.json'))
        self.assertEqual(actual['native_motion_calls'],31*6)
        self.assertTrue(actual['all_legacy_controls_exact'])
        self.assertEqual(actual['field_totals']['fields'],31)
        self.assertEqual(actual['field_totals']['jumps_above_1e_8'],0)

    def test_checker_rejects_changed_native_value_and_gradient(self):
        rows=[json.loads(r) for r in v.V.decode(E/'fields/synthetic','native.stdout.jsonl').splitlines()]
        field=next(r for r in rows if r['type']=='field' and r['field']=='sparse_ramp_0')
        path=next(r for r in rows if r['type']=='path' and r['field']=='sparse_ramp_0')
        v.check_field(field,[path])
        changed=copy.deepcopy(path);changed['endpoints'][0]['value']+=.01
        with self.assertRaisesRegex(ValueError,'intensity reconstruction'):v.check_field(field,[changed])
        changed=copy.deepcopy(path);changed['interior'][0]['gradient'][1]+=.01
        with self.assertRaisesRegex(ValueError,'gradient reconstruction'):v.check_field(field,[changed])

    def test_every_case_and_arm_is_retained_and_gate_is_recomputed(self):
        saved=v.read(E/'report/summary.json')
        self.assertEqual(saved['progression_gate_passed'],all(x['passed'] for x in saved['gate_checks']))
        for group,count in (('old',8),('01',6),('02',8),('08',9)):
            cases=saved['groups'][group]['cases'];self.assertEqual(len(cases),count)
            self.assertTrue(all(set(x['metrics'])=={str(m) for m in range(6)} for x in cases))

    def test_all_previous_zero_weight_holes_have_equal_valid_limits(self):
        previous=json.loads(gzip.decompress((E.with_name('inten_intensity_normalized')/'report/boundaries.json.gz').read_bytes()))
        current=json.loads(gzip.decompress((E/'report/boundaries.json.gz').read_bytes()))
        count=0
        for group,fields in previous.items():
            for field,faces in fields.items():
                for old in faces:
                    if not (old['both_valid'] and old['zero_weight_limit']):continue
                    matches=[f for f in current[group][field] if all(f[k]==old[k] for k in ('index','axis','face'))]
                    self.assertEqual(len(matches),1);self.assertTrue(matches[0]['both_valid'])
                    self.assertLessEqual(abs(matches[0]['jump']),1e-8)
                    self.assertLessEqual(matches[0]['gradient_jump'],1e-8);count+=1
        self.assertEqual(count,5)

    def saved(self):
        return v.read(E/'report/summary.json'),json.loads(gzip.decompress((E/'report/boundaries.json.gz').read_bytes()))

    def test_near_zero_maximum_witness_ties_are_validated(self):
        saved,details=self.saved();changed=copy.deepcopy(saved)
        field=changed['fields']['old'][0];old=field['largest']
        key=lambda b:tuple(b[k] for k in ('index','axis','face'))
        other=next(b for b in details['old'][field['field']] if b['both_valid'] and key(b)!=key(old) and v.V.close(abs(b['jump']),abs(old['jump'])))
        field['largest']=copy.deepcopy(other)
        self.assertEqual(c.check(changed,details,saved)['exit_code'],0)

    def test_checker_rejects_invented_face_value_and_motion_label(self):
        saved,details=self.saved()
        for change in ('face','value','motion'):
            altered=copy.deepcopy(saved)
            if change=='face':altered['fields']['old'][0]['largest']['index']=10**9
            elif change=='value':altered['fields']['old'][0]['largest']['jump']+=.01
            else:
                metric=altered['groups']['old']['cases'][0]['metrics']['4']
                metric['bad_length']=not metric['bad_length']
            with self.assertRaises(ValueError):c.check(saved,details,altered)
