"""The continuous field must be checked separately from motion feasibility."""
from pathlib import Path
import copy, importlib.util, json, unittest
ROOT=Path(__file__).resolve().parents[1]
E=ROOT/'experiments/results/inten_intensity_normalized'
spec=importlib.util.spec_from_file_location('normalized_evidence_verify',E/'verify.py')
v=importlib.util.module_from_spec(spec);spec.loader.exec_module(v)

class NormalizedIntensityTest(unittest.TestCase):
    def test_initial_execution_failure_and_frozen_sources_remain_byte_verified(self):
        import gzip,hashlib
        initial=E/'attempts/initial'
        for manifest in initial.rglob('archive.json'):
            for name in v.read(manifest)['files']:v.V.decode(manifest.parent,name)
        pins=v.read(initial/'protocol.json')['source_sha256']
        sources=v.read(initial/'source_manifest.json')['files']
        self.assertEqual(set(pins),set(sources))
        for name,entry in sources.items():
            data=(initial/entry['path']).read_bytes()
            self.assertEqual(hashlib.sha256(data).hexdigest(),entry['gzip_sha256'])
            self.assertEqual(hashlib.sha256(gzip.decompress(data)).hexdigest(),pins[name])
        failure=initial/'checks/initial_reconstruction_failure'
        self.assertEqual(v.read(failure/'reconstruct_command.json')['exit_code'],1)
        self.assertIn(b'Native support validity differs',v.V.decode(failure,'reconstruct.stderr.log'))

    def test_complete_native_evidence_reconstructs_and_matches_recorded_report(self):
        actual,details=v.validate()
        v.V.equivalent(actual,v.read(E/'report/summary.json'))
        self.assertEqual(actual['native_motion_calls'],124)
        self.assertTrue(actual['all_legacy_controls_exact'])
        self.assertEqual(actual['field_totals']['fields'],31)
        self.assertEqual(actual['field_totals']['jumps_above_1e_8'],0)

    def test_checker_rejects_wrong_native_value_and_missing_denominator_derivative(self):
        rows=[json.loads(r) for r in v.V.decode(E/'fields/synthetic','native.stdout.jsonl').splitlines()]
        field=next(r for r in rows if r['type']=='field' and r['field']=='sparse_ramp_0')
        path=next(r for r in rows if r['type']=='path' and r['field']=='sparse_ramp_0')
        v.check_field(field,[path])
        changed=copy.deepcopy(path);changed['endpoints'][0]['value']+=.01
        with self.assertRaisesRegex(ValueError,'intensity reconstruction'):v.check_field(field,[changed])
        changed=copy.deepcopy(path);changed['interior'][0]['gradient'][1]+=.01
        with self.assertRaisesRegex(ValueError,'gradient reconstruction'):v.check_field(field,[changed])

    def test_motion_gate_is_recomputed_without_discarding_adverse_cases(self):
        saved=v.read(E/'report/summary.json')
        self.assertEqual(saved['progression_gate_passed'],all(c['passed'] for c in saved['gate_checks']))
        for group,count in (('old',8),('01',6),('02',8),('08',9)):
            cases=saved['groups'][group]['cases']
            self.assertEqual(len(cases),count)
            self.assertTrue(all(set(c['metrics'])=={'0','1','2','3'} for c in cases))
