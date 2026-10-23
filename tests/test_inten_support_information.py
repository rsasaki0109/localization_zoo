from pathlib import Path
import copy,importlib.util,json,sys,tempfile,unittest
from unittest.mock import patch
import numpy as np

E=Path(__file__).resolve().parents[1]/'experiments/results/inten_support_information'
def module(name,path):
    s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
v=module('support_checked_mixed_parallel_field',E/'verify_mixed_parallel.py')
io=module('support_report_reader',E/'report_io.py')

class SupportInformationTest(unittest.TestCase):
    def line_fixture(self):
        blank=np.empty((0,6))
        target=np.array([[0.,0.,0.,0.,0.,0.],[0.,0.,1.,1.,0.,0.]])
        source=np.array([[.2,0.,.1,0.,0.,0.]])
        clouds={0:[target,blank,blank,blank],1:[source,blank,blank,blank]}
        J=np.array([1.,0.,0.,0.,.1,0.])
        system=dict(H=(.5*np.outer(J,J)).ravel().tolist(),g=(.1*J).tolist(),count=1,cost=.02)
        zero=dict(H=[0.]*36,g=[0.]*6,count=0,cost=0.)
        record=dict(frame=1,mode=6,translation_only=False,iteration=0,feature_class=0,
            pose=np.eye(4).ravel().tolist(),rows=[[True,True,[0,1]]],
            all=copy.deepcopy(system),kept=copy.deepcopy(system),removed=zero)
        record['class']=record.pop('feature_class')
        return record,clouds
    def test_complete_histories_native_queries_and_information_report(self):
        with tempfile.TemporaryDirectory(prefix='support_complete_check_') as folder:
            try:actual=v.validate_in(Path(folder))
            except Exception:
                for path in Path(folder).glob('*.stderr.log'):
                    print(path.name+'\n'+path.read_text(),file=sys.stderr)
                raise
        v.inputs.F.equivalent(actual,io.read_report())
        self.assertTrue(actual['all_558_historical_native_outcomes_exact'])
        self.assertEqual(sum(g['native_outcomes'] for g in actual['groups'].values()),31*18)
        self.assertEqual(set(actual['groups']),{'old','01','02','08'})
    def test_known_line_huber_and_kept_removed_partition(self):
        record,clouds=self.line_fixture()
        rebuilt,counts=v.checker.reconstruct(record,clouds,set(),{},{} )
        self.assertEqual(rebuilt['all']['count'],1)
        self.assertEqual(counts['valid_removed'],0)
        self.assertAlmostEqual(rebuilt['all']['H'][0,0],.5)
        self.assertAlmostEqual(rebuilt['all']['g'][4],.01)
        self.assertIsNone(rebuilt['all']['H'].base)
        self.assertIsNone(rebuilt['all']['g'].base)
    def test_native_edge_arithmetic_witness_passes_original_thresholds(self):
        for directory,stem in (('equation_diagnosis','old_247_6_0_16_0'),('ordered_equation_diagnosis','08_45_15_1_3_0')):
            folder=E/'checks'/directory
            record=json.loads((folder/(stem+'.query.json')).read_text())
            data=np.load(folder/(stem+'.clouds.npz'));blank=np.empty((0,6))
            clouds={record['frame']:[data['source'],blank,blank,blank],record['frame']-1:[data['target'],blank,blank,blank]}
            actual,_=v.checker.reconstruct(record,clouds,set(),{},{} )
            np.testing.assert_allclose(actual['kept']['H'],np.array(record['kept']['H']).reshape(6,6),rtol=1e-10,atol=1e-12)
        self.assertEqual(v.sum3(np.array([1e16,-1e16,1.])),1.)
    def test_invalid_reciprocal_reject_is_not_a_lost_valid_residual(self):
        record,clouds=self.line_fixture()
        clouds[0][0][:,3]=0
        clouds[1][0]=np.vstack((clouds[1][0],np.zeros((1,6))))
        record['rows']=[[False,False,[0,1]],[False,True,[0,1]]]
        for name in ('all','kept','removed'):record[name]=dict(H=[0.]*36,g=[0.]*6,count=0,cost=0.)
        _,counts=v.checker.reconstruct(record,clouds,set(),{},{} )
        self.assertEqual(counts['invalid'],2)
        self.assertEqual(counts['invalid_reciprocal_rejections'],1)
        self.assertEqual(counts['valid_removed'],0)
    def test_changed_native_equation_is_rejected(self):
        record,clouds=self.line_fixture();record['all']['H'][0]+=1.
        with self.assertRaisesRegex(ValueError,'H reconstruction differs'):
            v.checker.reconstruct(record,clouds,set(),{},{} )
    def test_changed_neighbour_population_is_rejected(self):
        record,clouds=self.line_fixture();record['rows'][0][2]=[0]
        with self.assertRaisesRegex(ValueError,'neighbour population'):
            v.checker.reconstruct(record,clouds,set(),{},{} )
    def test_centered_translation_information_is_origin_consistent(self):
        t=np.array([.3,-.4,.2]);B=np.eye(6)
        B[:3,3:]=[[0.,-.2,-.4],[.2,0.,-.3],[.4,.3,0.]]
        H0=np.diag([5.,7.,11.,2.,3.,4.]);g0=np.arange(1.,7.)
        inverse=np.linalg.inv(B);H=inverse.T@H0@inverse;g=inverse.T@g0
        axis=np.array([1.,2.,3.])/np.sqrt(14.)
        actual=v.checker.centered(H,g,t,axis)
        np.testing.assert_allclose(actual['eigenvalues'],[5.,7.,11.],rtol=1e-10,atol=1e-12)
        self.assertAlmostEqual(actual['longitudinal_information'],132./14.)
        self.assertAlmostEqual(actual['longitudinal_rhs'],np.sqrt(14.))
    def test_all_point_lround_includes_exact_halves_but_not_previous_float(self):
        points=np.array([[np.nextafter(.15,0.),.15,-.15,0.,0.,.25],
                         [.15,np.nextafter(-.15,0.),np.nextafter(-.15,-np.inf),0.,0.,.5]])
        self.assertEqual(v.inputs.rounded_nodes(points),{(0,1,-1):(.25,1),(1,0,-1):(.5,1)})
    def test_changed_full_cloud_cache_binding_is_rejected(self):
        path=E.parent/'inten_intensity_continuity/groups/old/input_sources.json'
        original=Path.read_text
        def changed(p,*args,**kwargs):
            text=original(p,*args,**kwargs)
            if p==path:
                row=json.loads(text);row['feature_cache_sha256']='0'*64;return json.dumps(row)
            return text
        with patch.object(Path,'read_text',changed):
            with self.assertRaises(AssertionError):v.inputs.fields('old')
    def test_changed_native_extra_is_rejected_before_decode(self):
        path=E/'groups/old/support.extra.jsonl.xz';original=Path.read_bytes
        data=bytearray(original(path));data[len(data)//2]^=1
        with patch.object(Path,'read_bytes',lambda p:bytes(data) if p==path else original(p)):
            with self.assertRaisesRegex(ValueError,'Stored evidence changed'):v.checker.decode(E/'groups/old','support.extra.jsonl')
    def test_changed_parent_reference_is_rejected_before_reconstruction(self):
        reference=json.loads((E/'groups/old/native.stdout.reference.json').read_text())
        reference['parent_archive_sha256']='0'*64
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder);(path/'native.stdout.reference.json').write_text(json.dumps(reference))
            with self.assertRaisesRegex(ValueError,'Parent archive changed'):v.checker.verify_transport(path,[])

if __name__=='__main__':unittest.main()
