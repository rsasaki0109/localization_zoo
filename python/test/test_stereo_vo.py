import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import numpy as np
from stereo_vo import StereoVO, triangulate, estimate_motion

class StereoTests(unittest.TestCase):
    K = np.array([[400., 0, 320], [0, 400, 240], [0, 0, 1]])

    def test_metric_depth_and_invalid_disparity(self):
        xyz, valid = triangulate([[340, 240], [320, 240]], [[320, 240], [325, 240]], self.K, .2)
        np.testing.assert_allclose(xyz[0], [.2, 0, 4])
        self.assertEqual(valid.tolist(), [True, False])

    def test_known_motion_from_image_projections(self):
        rng = np.random.default_rng(19)
        xyz = rng.uniform([-2, -1, 4], [2, 1, 10], (100, 3))
        moved = xyz + [.1, -.02, .03]
        uv = moved @ self.K.T
        uv = uv[:, :2] / uv[:, 2:]
        T, n = estimate_motion(xyz, uv, self.K)
        self.assertGreater(n, 90)
        np.testing.assert_allclose(T[:3, 3], [.1, -.02, .03], atol=1e-5)
        np.testing.assert_allclose(np.linalg.inv(T)[:3, 3], [-.1, .02, -.03], atol=1e-5)

    def test_blank_images_report_loss_not_fake_pose(self):
        vo = StereoVO(self.K, .2)
        blank = np.zeros((480, 640), np.uint8)
        self.assertEqual(vo.process(blank, blank, 1)['status'], 'lost')
        self.assertIsNone(vo.process(blank, blank, 2)['pose'])
        vo.reset()
        self.assertFalse(vo.lost)

    def test_reject_invalid_calibration(self):
        with self.assertRaises(ValueError): StereoVO(self.K, 0)


class StereoImageTest(unittest.TestCase):
    def test_images_produce_metric_camera_motion(self):
        K = StereoTests.K
        rng = np.random.default_rng(42)
        points = []
        for v in range(70, 430, 50):
            for u in range(80, 600, 50):
                z = rng.uniform(4, 9)
                points.append(((u-320)*z/400, (v-240)*z/400, z,
                               rng.integers(0, 256, (19, 19), dtype=np.uint8)))
        def render(camera_x):
            im = np.zeros((480,640),np.uint8)
            for x,y,z,patch in points:
                u=int(round(400*(x-camera_x)/z+320));v=int(round(400*y/z+240))
                im[v-9:v+10,u-9:u+10]=patch
            return im
        vo=StereoVO(K,.2)
        self.assertEqual(vo.process(render(0),render(.2),0)['status'],'initialized')
        result=vo.process(render(.06),render(.26),1)
        self.assertEqual(result['status'],'tracking')
        np.testing.assert_allclose(result['pose'][:3,3],[.06,0,0],atol=.025)

if __name__ == '__main__': unittest.main()
