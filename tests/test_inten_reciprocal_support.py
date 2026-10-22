from pathlib import Path
import copy,importlib.util,unittest
import numpy as np
from unittest.mock import patch
E=Path(__file__).resolve().parents[1]/'experiments/results/inten_reciprocal_support'
s=importlib.util.spec_from_file_location('reciprocal_support_check',E/'verify_portable.py');v=importlib.util.module_from_spec(s);s.loader.exec_module(v)
class ReciprocalSupportTest(unittest.TestCase):
    def test_complete_native_controls_masks_updates_and_recorded_report(self):
        actual=v.validate();v.V.V.V.equivalent(actual,v.read(E/'report/summary.json'))
        self.assertEqual(actual['native_outcomes'],31*14)
        self.assertTrue(actual['all_372_inherited_traces_exact'])
        self.assertEqual(actual['progression_gate_passed'],all(x['passed'] for x in actual['gate_checks']))
    def example(self):
        src=np.array([[0,0,0,0,0],[1,0,0,0,0]],float);dst=np.array([[.1,0,0,0,0],[2,0,0,0,0]],float);empty=np.empty((0,5))
        record=dict(pose=np.eye(4).ravel().tolist(),classes=[[[0,0,0,True],[1,0,0,False]],[],[]])
        return record,[src,empty,empty],[dst,empty,empty]
    def test_many_to_one_is_rejected_without_gt_or_prediction_labels(self):
        r,s,t=self.example();proof=v.masks(r,s,t,set());self.assertEqual(proof['backward_queries'],2);self.assertEqual(proof['accepted'],1)
    def test_changed_reciprocal_decision_is_rejected(self):
        r,s,t=self.example();r['classes'][0][1][3]=True
        with self.assertRaisesRegex(ValueError,'mask differs'):v.masks(r,s,t,set())
    def test_changed_backward_neighbour_is_rejected(self):
        r,s,t=self.example();r['classes'][0][0][2]=1
        with self.assertRaisesRegex(ValueError,'Backward nearest'):v.masks(r,s,t,set())
    def test_omitted_query_is_rejected(self):
        r,s,t=self.example();r['classes'][0].pop()
        with self.assertRaisesRegex(ValueError,'population'):v.masks(r,s,t,set())
    def test_invented_forward_gate_rejection_is_rejected(self):
        r,s,t=self.example();r['classes'][0][0]=[0,-1,-1,False]
        with self.assertRaisesRegex(ValueError,'Dropped valid'):v.masks(r,s,t,set())
    def test_changed_xz_measurement_is_rejected_before_decoding(self):
        path=E/'groups/old/native.stdout.jsonl.xz';original=Path.read_bytes
        raw=bytearray(original(path));raw[len(raw)//2]^=1
        def changed(p):return bytes(raw) if p==path else original(p)
        with patch.object(Path,'read_bytes',changed):
            with self.assertRaisesRegex(ValueError,'XZ hash'):v.decode(E/'groups/old','native.stdout.jsonl')
