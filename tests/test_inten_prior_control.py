from pathlib import Path
import copy,importlib.util,unittest
import numpy as np
from unittest.mock import patch

E=Path(__file__).resolve().parents[1]/'experiments/results/inten_prior_control'
s=importlib.util.spec_from_file_location('prior_control_checks',E/'verify_portable.py')
v=importlib.util.module_from_spec(s);s.loader.exec_module(v)

class PriorControlTest(unittest.TestCase):
    def example(self):
        information=np.diag([2.,.1,0.]);prediction=[.3,.2,-.1];t=[.5,-.4,.2]
        H=np.diag([4.,5.,6.,7.,8.,9.]);g=np.array([.1,.2,.3,.4,.5,.6])
        # Explicit left-increment Jacobian [I,-[t]x], independent of checker.cross.
        J=np.array([[1,0,0,0,.2,.4],[0,1,0,-.2,0,.5],[0,0,1,-.4,-.5,0]],float)
        pose=np.eye(4);pose[:3,3]=t;seed=np.eye(4);seed[:3,3]=prediction
        record=dict(mode=14,reference_mode=6,information=information.ravel().tolist(),prediction=prediction,t=t,
                    H_before_prior=H.ravel().tolist(),g_before_prior=g.tolist())
        solver=dict(input=seed.ravel().tolist(),translation_prior_information=information.ravel().tolist())
        reference=dict(translation_prior_information=information.ravel().tolist())
        step=dict(index=0,linearization_pose=pose.ravel().tolist(),H=(H+J.T@information@J).ravel().tolist(),
                  g=(g+J.T@information@(np.array(t)-prediction)).tolist())
        return record,solver,reference,step
    def test_complete_factorial_controls_masks_priors_updates_and_report(self):
        actual=v.validate();v.P.V.V.V.equivalent(actual,v.read(E/'report/summary.json'))
        self.assertEqual(actual['native_outcomes'],31*18)
        self.assertTrue(actual['all_434_inherited_complete_records_exact'])
        self.assertTrue(actual['unfiltered_common_prior_negative_controls_exact'])
        self.assertTrue(actual['all_fixed_prior_contributions_and_centers_checked'])
        self.assertEqual(actual['progression_gate_passed'],all(c['passed'] for c in actual['gate_checks']))
    def test_fixed_information_and_left_increment_contributions(self):
        proof=v.check_prior(*self.example())
        self.assertEqual(proof['max_H_error'],0.);self.assertEqual(proof['max_g_error'],0.)
    def test_independent_schur_information_uses_existing_fraction(self):
        H=np.diag([1.,100.,4.,10.,10.,10.])
        np.testing.assert_allclose(v.derived_information(H,np.zeros(3)),np.diag([2.,0.,0.]),rtol=1e-10,atol=1e-12)
    def test_changed_reference_arm_is_rejected(self):
        data=self.example();data[0]['reference_mode']=7
        with self.assertRaisesRegex(ValueError,'reference arm'):v.check_prior(*data)
    def test_changed_information_is_rejected(self):
        data=self.example();data[0]['information'][0]+=1.
        with self.assertRaisesRegex(ValueError,'information changed'):v.check_prior(*data)
    def test_changed_prior_center_is_rejected(self):
        data=self.example();data[0]['prediction'][0]+=.1
        with self.assertRaisesRegex(ValueError,'center changed'):v.check_prior(*data)
    def test_omitted_H_contribution_is_rejected(self):
        data=self.example();data[3]['H']=data[0]['H_before_prior'].copy()
        with self.assertRaisesRegex(ValueError,'H contribution'):v.check_prior(*data)
    def test_omitted_g_contribution_is_rejected(self):
        data=self.example();data[3]['g']=data[0]['g_before_prior'].copy()
        with self.assertRaisesRegex(ValueError,'g contribution'):v.check_prior(*data)
    def test_omitted_zero_prior_preserves_original_record_identity(self):
        record=v.SolverRecord(called=True,input=[],output=[],iterations=[])
        self.assertEqual(record['translation_prior_information'],[0.]*9)
        self.assertNotIn('translation_prior_information',record)
        self.assertEqual(record,dict(called=True,input=[],output=[],iterations=[]))
        with self.assertRaises(KeyError):record['unknown_field']
        with self.assertRaises(KeyError):v.SolverRecord()['translation_prior_information']
    def test_changed_xz_measurement_is_rejected_before_decoding(self):
        path=E/'groups/old/native.stdout.jsonl.xz';original=Path.read_bytes
        data=bytearray(original(path));data[len(data)//2]^=1
        def changed(p):return bytes(data) if p==path else original(p)
        with patch.object(Path,'read_bytes',changed):
            with self.assertRaisesRegex(ValueError,'Stored measurement hash'):v.decode(E/'groups/old','native.stdout.jsonl')
