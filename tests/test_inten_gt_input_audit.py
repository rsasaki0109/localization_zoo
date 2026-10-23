from pathlib import Path
import importlib.util,unittest
E=Path(__file__).resolve().parents[1]/'experiments/results/inten_gt_input_audit'
s=importlib.util.spec_from_file_location('gt_input_audit',E/'verify.py');v=importlib.util.module_from_spec(s);s.loader.exec_module(v)
class OfficialGTInputAuditTest(unittest.TestCase):
    def test_every_official_sequence_regenerates_without_motion_correction(self):
        r=v.validate();self.assertEqual(r['exit_code'],0)
        self.assertEqual([x['sequence'] for x in r['sequences']],[f'{i:02d}' for i in range(11)])
        self.assertTrue(r['no_gt_correction'])
    def data(self):return b'timestamp,x,y,z,roll,pitch,yaw\n0,0,0,0,0,0,0\n1,1,0,0,0,0,0\n'
    def test_changed_translation_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'Numeric GT mismatch'):v.compare(self.data(),self.data().replace(b'1,1,0',b'1,2,0'))
    def test_changed_rotation_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'Numeric GT mismatch'):v.compare(self.data(),self.data().replace(b'1,1,0,0,0,0,0',b'1,1,0,0,0,0,0.1'))
    def test_frame_shift_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'frame IDs'):v.compare(self.data(),self.data().replace(b'\n1,1,0',b'\n2,1,0'))
