from pathlib import Path
import copy,importlib.util,unittest
E=Path(__file__).resolve().parents[1]/'experiments/results/inten_local_rigidity'
spec=importlib.util.spec_from_file_location('rigidity_table_test',E/'report_docs.py')
table=importlib.util.module_from_spec(spec);spec.loader.exec_module(table)

class LocalRigidityTablesTest(unittest.TestCase):
    def test_empty_native_valid_boundary_is_explicit(self):
        report=table.reader.read_report()
        empty=[b for g in report['groups'].values() for b in g['boundaries'] if not sum(p['count'] for p in b['groups'] if p['partition']=='all')]
        self.assertTrue(empty)
        for boundary in empty:
            self.assertEqual(table.boundary_points({},boundary),[])
            self.assertEqual(table.metrics([])['count'],0)
            self.assertIsNone(table.metrics([])['median_center_absolute'])
    def test_missing_nonempty_boundary_rows_are_rejected(self):
        report=table.reader.read_report()
        boundary=next(b for g in report['groups'].values() for b in g['boundaries'] if sum(p['count'] for p in b['groups'] if p['partition']=='all'))
        with self.assertRaisesRegex(AssertionError,'Missing native-valid boundary points'):
            table.boundary_points({},boundary)

if __name__=='__main__':unittest.main()
