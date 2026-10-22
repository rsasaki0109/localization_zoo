"""A sparse native cubic field can jump while its within-cell Jacobian is correct."""
from pathlib import Path
import copy,importlib.util,json,unittest
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
E=ROOT/'experiments/results/inten_intensity_continuity'
spec=importlib.util.spec_from_file_location('intensity_continuity_verify',E/'verify.py')
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)

class IntensityContinuityTest(unittest.TestCase):
    def test_complete_native_evidence_and_parent_controls_reconstruct(self):
        result,details=v.validate();v.check_recorded(result,details)
        self.assertEqual(result['exit_code'],0)
        self.assertEqual(result['totals']['fields'],31)
        self.assertEqual(result['totals']['endpoint_controls'],17658)
        self.assertGreater(result['totals']['jumps_above_1e_8'],0)

    def test_independent_polynomial_reproduces_dense_affine_field(self):
        coefficients=np.array([.02,.03,-.01]);knot=.3
        nodes={(i,j,k):.4+coefficients@np.array([i,j,k]) for i in range(-3,4) for j in range(-3,4) for k in range(-3,4)}
        x=np.array([[.31,.17,-.24],[-.15,.02,.05]])
        valid,value,gradient,_,_=v.evaluate(nodes,x,knot)
        self.assertTrue(valid.all())
        np.testing.assert_allclose(value,.4+(x/knot)@coefficients,rtol=1e-12,atol=1e-12)
        np.testing.assert_allclose(gradient,np.tile(coefficients/knot,(2,1)),rtol=1e-12,atol=1e-12)

    def test_sparse_constant_dense_and_invalid_controls_are_retained(self):
        s=json.loads((E/'report/summary.json').read_text())
        for r in s['synthetic']:
            if r['field'].startswith('sparse_ramp'):
                self.assertEqual(r['both_valid'],1)
                self.assertGreater(r['max_abs_jump'],.02)
                self.assertLess(r['max_interior_derivative_abs_error'],1e-8)
            elif r['field'].startswith('invalid_three'):
                self.assertEqual(r['neither_valid'],1)
            else:self.assertLessEqual(r['max_abs_jump'],1e-12)
        for group in ('old','01'):
            zero=[r for r in s['groups'][group] if r['boundaries']==0]
            self.assertTrue(zero)
            self.assertTrue(all(r['paths']>0 and r['endpoint_controls']>0 for r in zero))

    def test_checker_rejects_tampered_native_value_or_gradient(self):
        rows=[json.loads(s) for s in v.decode(E/'groups/synthetic','native.stdout.jsonl').splitlines()]
        field=next(r for r in rows if r['type']=='field' and r['field']=='sparse_ramp_0')
        path=next(r for r in rows if r['type']=='path' and r['field']=='sparse_ramp_0')
        v.check_field(field,[path])
        changed=copy.deepcopy(path);changed['boundaries'][0]['samples'][-1]['value']+=.01
        with self.assertRaisesRegex(ValueError,'intensity reconstruction'):v.check_field(field,[changed])
        changed=copy.deepcopy(path);changed['interior'][0]['gradient'][0]+=.01
        with self.assertRaisesRegex(ValueError,'gradient reconstruction'):v.check_field(field,[changed])
