"""Input-policy boundaries, independent of a benchmark's observed score."""
import ctypes
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class InputPolicyTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not shutil.which('g++'):
            raise unittest.SkipTest('g++ unavailable')
        cls.directory = tempfile.TemporaryDirectory()
        source = Path(__file__).resolve().parents[1]/'experiments/results/inten_independent_onset/preprocess.cpp'
        library = Path(cls.directory.name)/'preprocess.so'
        subprocess.run(['g++','-std=c++17','-O2','-fPIC','-shared',str(source),'-o',str(library)],check=True,capture_output=True)
        cls.function = ctypes.CDLL(str(library)).preprocess
        cls.function.argtypes = [ctypes.POINTER(ctypes.c_float),ctypes.c_size_t]
        cls.function.restype = ctypes.c_size_t

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def apply(self, rows):
        values = (ctypes.c_float*(4*len(rows)))(*(v for row in rows for v in row))
        size = self.function(values,len(rows))
        return [list(values[4*i:4*i+4]) for i in range(size)]

    def test_finite_strict_range_and_stable_order(self):
        rows = [(0,0,1,1),(0,0,120,2),(0,0,1.0001,3),(0,0,119.99,4),
                (2,0,0,float('nan')),(float('inf'),0,0,6),(0,0,float('nan'),7)]
        result = self.apply(rows)
        self.assertEqual([row[3] for row in result],[3,4])

    def test_elevation_sign_and_float_input_preservation(self):
        import math
        result = self.apply([(10,0,0,12),(0,10,0,13)])
        angle = math.radians(.17)
        self.assertAlmostEqual(result[0][0],10*math.cos(angle),places=5)
        self.assertAlmostEqual(result[0][2],10*math.sin(angle),places=7)
        self.assertEqual(result[0][0],result[1][1])
        self.assertEqual(result[0][2],result[1][2])
        self.assertEqual([row[3] for row in result],[12,13])


if __name__=='__main__':
    unittest.main()
