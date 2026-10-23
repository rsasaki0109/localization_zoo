"""Keep the prospective hybrid trial one fixed configuration across KITTI."""
import json
from pathlib import Path
import unittest

class GeometricPriorManifestTest(unittest.TestCase):
    def test_all_sequences_share_settings_and_keep_no_gt_seed(self):
        root=Path(__file__).resolve().parents[1]
        variants=[]
        for i in range(11):
            seq=f'{i:02d}'
            manifest=json.loads((root/'experiments'/f'inten_kitti_seq_{seq}_full_geometric_prior_matrix.json').read_text())
            self.assertEqual(manifest['problem']['dataset']['gt_csv'],f'experiments/reference_data/kitti_seq_{seq}_full_gt.csv')
            base,candidate=manifest['variants']
            self.assertEqual(candidate['args'],base['args']+['--inten-loam-paper-set','s2s_geometric_prior=1'])
            self.assertIn('--no-gt-seed',base['args'])
            self.assertNotIn('s2s_geometric_prior=1',base['args'])
            variants.append([base['args'],candidate['args']])
        self.assertTrue(all(v==variants[0] for v in variants))

if __name__=='__main__':unittest.main()
