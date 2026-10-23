from pathlib import Path
import csv,importlib.util,io,json,tempfile,unittest
from unittest.mock import patch
import numpy as np

E=Path(__file__).resolve().parents[1]/'experiments/results/inten_local_rigidity'
def module(name,path):
    spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
r=module('rigidity_mechanism_under_test',E/'rigidity.py')
a=module('rigidity_analysis_under_test',E/'analyze.py')
reader=module('rigidity_report_under_test',E/'report_io.py')
portable=module('rigidity_portable_report_under_test',E/'portable.py')

class LocalRigidityTest(unittest.TestCase):
    def fixture(self):
        p=np.array([[0.,0.,0.],[1.,0.,0.],[0.,2.,0.],[0.,0.,3.]])
        return p,np.arange(len(p)),np.ones(len(p),bool),r.neighbors(p)
    def test_independent_rigid_frame_changes_preserve_equation(self):
        p,anchors,valid,near=self.fixture();q=p*np.array([1.2,1.,1.])
        original=r.measure(p,q,anchors,valid,near)
        R=np.array([[0.,-1.,0.],[1.,0.,0.],[0.,0.,1.]])
        S=np.array([[1.,0.,0.],[0.,0.,-1.],[0.,1.,0.]])
        changed=r.measure(p@R+[4.,-2.,9.],q@S+[-7.,5.,1.],anchors,valid,near)
        np.testing.assert_allclose(original['error'],changed['error'],rtol=1e-10,atol=1e-12)
    def test_rigid_moving_object_passes_and_is_not_static_truth(self):
        p,anchors,valid,near=self.fixture()
        q=p@np.array([[0.,-1.,0.],[1.,0.,0.],[0.,0.,1.]])+[20.,3.,0.]
        value=r.measure(p,q,anchors,valid,near)
        np.testing.assert_allclose(value['error'],0.,rtol=1e-10,atol=1e-12)
        self.assertGreater(np.linalg.norm(q-p),10.)
    def test_nonrigid_stretch_and_anchor_collision_are_visible(self):
        p,anchors,valid,near=self.fixture()
        stretch=r.measure(p,2*p,anchors,valid,near)
        np.testing.assert_allclose(stretch['error'],stretch['source_length'])
        collision=r.measure(p,p,np.zeros(len(p),int),valid,near)
        np.testing.assert_allclose(collision['error'],-collision['source_length'])
    def test_invalid_nearest_neighbor_has_no_farther_fallback(self):
        p=np.array([[0.,0.,0.],[1.,0.,0.],[3.,0.,0.]])
        value=r.measure(p,p,np.arange(3),np.array([True,False,True]),r.neighbors(p,k=1))
        self.assertEqual(value['possible_relations'],2)
        self.assertEqual(len(value['error']),0)
        self.assertTrue(np.isnan(value['mean_absolute']).all())
    def test_center_partition_does_not_refilter_neighbors(self):
        p,anchors,valid,near=self.fixture();value=r.measure(p,2*p,anchors,valid,near)
        keep=np.array([True,False,False,False])
        k=r.summarize(value,keep);removed=r.summarize(value,~keep);all_rows=r.summarize(value,valid)
        self.assertEqual(k['relations'],3)
        self.assertEqual(all_rows['relations'],k['relations']+removed['relations'])
        self.assertAlmostEqual(all_rows['absolute_sum'],k['absolute_sum']+removed['absolute_sum'])
    def test_ties_empty_and_zero_length_are_explicit(self):
        p=np.array([[0.,0.,0.],[1.,0.,0.],[-1.,0.,0.],[0.,0.,0.]])
        self.assertEqual(r.neighbors(p,k=3)[0].tolist(),[3,1,2])
        self.assertEqual(r.neighbors(np.empty((0,3))).shape,(0,0))
        self.assertEqual(r.neighbors(p[:1]).shape,(1,0))
        value=r.measure(p,p,np.arange(4),np.ones(4,bool),r.neighbors(p))
        summary=r.finish([r.summarize(value,np.ones(4,bool))])
        self.assertEqual(summary['zero_source_length'],2)
        self.assertEqual(summary['mean_absolute'],0.)
        with self.assertRaises(AssertionError):r.neighbors([[np.nan,0.,0.]])
    def test_native_boundary_point_projection_and_missing_axis(self):
        parent=E.parent/'inten_support_information/checks'
        for directory,stem in (('equation_diagnosis','old_247_6_0_16_0'),('ordered_equation_diagnosis','08_45_15_1_3_0')):
            folder=parent/directory;record=json.loads((folder/(stem+'.query.json')).read_text())
            data=np.load(folder/(stem+'.clouds.npz'));blank=np.empty((0,6))
            clouds={record['frame']:[data['source'],blank,blank,blank],record['frame']-1:[data['target'],blank,blank,blank]}
            for axis in (np.array([1.,0.,0.]),np.array([1.,2.,3.])/np.sqrt(14.)):
                g,h=a.point_terms(record,clouds,[],{},axis)
                self.assertTrue(np.isfinite(g).all() and np.isfinite(h).all())
            g,h=a.point_terms(record,clouds,[],{},None)
            self.assertTrue(np.isnan(g).all() and np.isnan(h).all())
    def test_changed_archive_is_rejected_before_decode(self):
        path=E.parent/'inten_support_information/groups/old/support.extra.jsonl.xz'
        original=Path.read_bytes;data=bytearray(original(path));data[len(data)//2]^=1
        with patch.object(Path,'read_bytes',lambda p:bytes(data) if p==path else original(p)):
            with self.assertRaises(AssertionError):next(a.stream(path.parent,'support.extra.jsonl'))
    def test_complete_derived_report_regenerates_all_histories_and_points(self):
        with tempfile.TemporaryDirectory(prefix='rigidity_complete_') as folder:
            output=Path(folder)/'complete';actual=a.analyze(output)
            self.assertEqual(portable.compare_outputs(output)['exit_code'],0)
            with self.assertRaises(AssertionError):portable.equivalent({'count':1},{'count':2})
            with self.assertRaises(ValueError):portable.equivalent({'axis_H':1.},{'axis_H':2.})
            for group in ('old','01','02','08'):
                raw=(output/group/'points.csv').read_bytes()
                rows=list(csv.DictReader(io.StringIO(raw.decode())))
                self.assertTrue(rows)
                self.assertEqual({int(row['mode']) for row in rows},{6,7,14,15})

if __name__=='__main__':unittest.main()
