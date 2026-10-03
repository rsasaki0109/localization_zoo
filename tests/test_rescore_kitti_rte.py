import argparse
import json
import os
import stat
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module

FAKE_BINARY = textwrap.dedent(
    """\
    #!{python}
    import json, sys
    args = sys.argv
    out = args[args.index("--summary-json") + 1]
    json.dump({{"argv": args, "methods": [{{"name": "Demo (fast)", "rpe_trans_pct": 1.0,
               "kitti_rte_trans_pct": 1.2, "kitti_rte_rot_deg_per_100m": 0.4,
               "frames": 10, "fps": 5.0}}]}}, open(out, "w"))
    """
)


@unittest.skipIf(os.name == "nt", "fake benchmark binary uses a shebang")
class RescoreKittiRteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module("rescore_kitti_rte", "evaluation/scripts/rescore_kitti_rte.py")

    def test_rescore_row_replays_variant_and_reports_determinism(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            binary = tmp / "fake"
            binary.write_text(FAKE_BINARY.format(python=sys.executable))
            binary.chmod(binary.stat().st_mode | stat.S_IEXEC)
            aggregate = tmp / "demo_matrix.json"
            aggregate.write_text(json.dumps({
                "stable_interface": {"methods": "demo", "primary_method": "Demo"},
                "dataset": {"pcd_dir": "dogfooding_results/kitti_seq_07_full", "gt_csv": "gt.csv",
                            "extra_args": ["--shared"]},
                "variants": [{"id": "best", "args": ["--knob", "3"], "rpe_trans_pct": 0.9}],
            }))
            row = {"method": "demo", "sequence": "07", "repo_best_aggregate": str(aggregate),
                   "repo_best_variant": "best"}
            args = argparse.Namespace(binary=binary, data_root=tmp / "data", work_dir=tmp / "work")
            args.work_dir.mkdir()
            item = self.module.rescore_row(row, args)
            argv = json.loads(next(args.work_dir.glob("*.json")).read_text())["argv"]

        self.assertEqual(item["kitti_rte_trans_pct"], 1.2)
        self.assertAlmostEqual(item["rerun_rpe_abs_delta"], 0.1)
        self.assertEqual(argv[1], str(tmp / "data" / "dogfooding_results/kitti_seq_07_full"))
        self.assertEqual(argv[-3:], ["--shared", "--knob", "3"])


if __name__ == "__main__":
    unittest.main()
