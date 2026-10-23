"""Ambiguous geometry, independent motion, majority failure and boundary controls."""
import unittest
from consensus import select


def narrow(cluster, *indices):
    return dict(cluster=cluster, costs_m2=[0. if k in indices else 1. for k in range(121)])


class ConsensusTests(unittest.TestCase):
    def test_ambiguous_planes_support_shared_correction_and_reject_independent_motion(self):
        rows = [dict(cluster=c, costs_m2=[0.]*121) for c in range(3)]
        rows += [narrow(3, 90), narrow(4, 90), narrow(5, 20)]
        result = select(rows)
        self.assertTrue(result['usable'])
        self.assertEqual(result['applied_index'], 90)
        self.assertEqual(result['excluded_clusters'], [5])
        self.assertEqual(result, select(list(reversed(rows))))

    def test_no_majority_keeps_original_prediction_and_support(self):
        result = select([narrow(1, 0), narrow(2, 60), narrow(3, 120)])
        self.assertFalse(result['usable'])
        self.assertEqual(result['shift_m'], 0.)
        self.assertEqual(result['excluded_clusters'], [])

    def test_boundary_consensus_is_unavailable(self):
        result = select([narrow(1, 0), narrow(2, 0)])
        self.assertFalse(result['usable'])
        self.assertEqual(result['applied_index'], 60)
        self.assertEqual(result['excluded_clusters'], [])

    def test_ties_prefer_small_motion_then_negative(self):
        self.assertEqual(select([narrow(1, 40, 80)])['applied_index'], 40)
        self.assertEqual(select([dict(cluster=1, costs_m2=[0.]*121)])['applied_index'], 60)

    def test_empty_profiles_keep_original(self):
        result = select([dict(cluster=1, costs_m2=[])])
        self.assertFalse(result['usable'])
        self.assertEqual(result['shift_m'], 0.)
        self.assertEqual(result['excluded_clusters'], [])


if __name__ == '__main__':
    unittest.main()
