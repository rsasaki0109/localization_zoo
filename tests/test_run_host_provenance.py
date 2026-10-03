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
    out = sys.argv[sys.argv.index("--summary-json") + 1]
    json.dump({{"methods": [{{"name": "Demo", "status": "OK", "ate_m": 0.5, "fps": 10.0,
                             "time_ms": 100.0, "frames": 1, "note": ""}}]}}, open(out, "w"))
    """
)


@unittest.skipIf(os.name == "nt", "fake benchmark binary uses a shebang")
class RunHostProvenanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_script_module(
            "test_run_host_provenance_runner", "evaluation/scripts/run_experiment_matrix.py"
        )

    def run_variant(self, tmp: Path, reuse_existing: bool):
        binary = tmp / "fake_benchmark"
        binary.write_text(FAKE_BINARY.format(python=sys.executable))
        binary.chmod(binary.stat().st_mode | stat.S_IEXEC)
        variant = {"id": "v", "label": "V", "design_style": "balanced", "intent": "", "args": []}
        return self.runner.run_variant(
            manifest_stem="demo",
            binary=binary,
            pcd_dir=tmp,
            gt_csv=tmp / "gt.csv",
            methods="demo",
            dataset_extra_args=[],
            primary_method="Demo",
            output_dir=tmp,
            variant=variant,
            reuse_existing=reuse_existing,
            timeout_seconds=30,
        )

    def test_fresh_run_records_host_and_reuse_reads_it_back(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            fresh = self.run_variant(tmp, reuse_existing=False)
            host_path = tmp / "runs" / "demo" / "v" / "host.json"
            self.assertTrue(host_path.is_file())
            reused = self.run_variant(tmp, reuse_existing=True)

        self.assertEqual(fresh.host, self.runner.capture_run_host())
        self.assertIn("cpu", fresh.host)
        self.assertNotIn("hostname", fresh.host)
        self.assertEqual(reused.host, fresh.host)
        self.assertEqual(self.runner.variant_result_to_dict(fresh)["host"], fresh.host)

    def test_reused_run_without_record_omits_host_field(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            self.run_variant(tmp, reuse_existing=False)
            (tmp / "runs" / "demo" / "v" / "host.json").unlink()
            reused = self.run_variant(tmp, reuse_existing=True)

        self.assertIsNone(reused.host)
        self.assertNotIn("host", self.runner.variant_result_to_dict(reused))


if __name__ == "__main__":
    unittest.main()
