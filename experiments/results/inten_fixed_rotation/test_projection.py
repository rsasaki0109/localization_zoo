"""Conditional translation differs from truncating a coupled joint update."""
import unittest
import numpy as np

class ProjectionTest(unittest.TestCase):
    def test_conditional_block_not_marginalized_or_truncated(self):
        A=np.array([[3.,1.,0.,2.],[0.,2.,1.,1.],[1.,0.,3.,2.],[0.,1.,1.,4.]])
        H=A.T@A+1e-6*np.eye(4);g=np.array([1.,-2.,3.,-4.])
        conditional=np.linalg.solve(H[:3,:3],-g[:3]);joint=np.linalg.solve(H,-g)
        self.assertFalse(np.allclose(conditional,joint[:3]))
        self.assertLess(np.linalg.norm(H[:3,:3]@conditional+g[:3]),1e-12)
        self.assertLess(g[:3]@conditional,0.)
    def test_fixed_rotation_translation_has_additive_update(self):
        angle=.3;R=np.array([[np.cos(angle),-np.sin(angle),0.],[np.sin(angle),np.cos(angle),0.],[0.,0.,1.]])
        T=np.eye(4);T[:3,:3]=R;T[:3,3]=[4.,-2.,1.]
        before=T.copy();delta=np.array([.2,.1,-.3]);T[:3,3]+=delta
        np.testing.assert_array_equal(T[:3,:3],before[:3,:3])
        np.testing.assert_array_equal(T[:3,3],before[:3,3]+delta)

if __name__=='__main__':unittest.main()
