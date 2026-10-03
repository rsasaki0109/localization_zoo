import json
import tempfile
import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module


class CaptureBenchmarkEnvironmentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_script_module(
            "capture_benchmark_environment", "evaluation/scripts/capture_benchmark_environment.py"
        )

    def test_collects_flat_host_records_only(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            evidence = root / "evaluation" / "data"
            evidence.mkdir(parents=True)
            (evidence / "a.json").write_text(json.dumps({
                "host": {"cpu": "Test CPU", "nested": {"x": 1}},
                "dependencies": {"runtime": "Docker"},
            }))
            (evidence / "b.json").write_text(json.dumps({"no_host": True}))
            (evidence / "broken.json").write_text("{")
            records = self.module.collect_recorded_hosts(evidence, root)

        self.assertEqual(records, [{
            "evidence": "evaluation/data/a.json",
            "host": {"cpu": "Test CPU"},
            "runtime": "Docker",
        }])

    def test_markdown_marks_missing_values_and_provenance(self) -> None:
        host = self.module.capture_host()
        host["libraries"]["Ceres Solver"] = ""
        markdown = self.module.render_markdown({
            "generated_at": "2026-01-01T00:00:00+00:00",
            "capture_host": host,
            "recorded_hosts": [],
            "provenance_note": "boundary text",
        })
        self.assertIn("| Ceres Solver | not found |", markdown)
        self.assertIn("## Provenance boundary", markdown)
        self.assertIn("boundary text", markdown)


if __name__ == "__main__":
    unittest.main()
