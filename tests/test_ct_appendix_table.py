import unittest
from pathlib import Path

from test_experiment_scripts import load_script_module

REPO_ROOT = Path(__file__).resolve().parents[1]


class CtAppendixTableTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        module = load_script_module(
            "generate_ct_appendix_table", "evaluation/scripts/generate_ct_appendix_table.py"
        )
        cls.module = module
        cls.rows = module.collect_rows(REPO_ROOT / "experiments" / "results")

    def test_sections_keep_native_synthetic_and_blocked_evidence_apart(self) -> None:
        by_section = {}
        for row in self.rows:
            by_section.setdefault(row["section"], set()).add(row["dataset"])
        self.assertEqual(set(by_section), {"A", "B", "C"})
        self.assertTrue(all("time_index" not in name for name in by_section["A"]))
        self.assertTrue(all("time_index" in name for name in by_section["B"]))
        blocked = [row for row in self.rows if row["section"] == "C"]
        self.assertEqual(len(blocked), 1)
        self.assertEqual(blocked[0]["problem_status"], "blocked")
        self.assertTrue(blocked[0]["blocker"])

    def test_one_default_per_ready_problem_and_clins_flagged_gt_seeded(self) -> None:
        ready = {row["aggregate_path"] for row in self.rows if row["problem_status"] == "ready"}
        defaults = [row["aggregate_path"] for row in self.rows if row["is_current_default"] == "true"]
        self.assertEqual(sorted(defaults), sorted(ready))
        clins = [row for row in self.rows if row["method"] == "CLINS"]
        self.assertTrue(clins and all(row["gt_seeded"] == "true" for row in clins))

    def test_tex_renders_section_headers_and_blocked_row(self) -> None:
        tex = self.module.render_tex(self.rows)
        self.assertIn(r"\textit{B. Public ROS1 HDL-400 window, synthesized per-point time}", tex)
        self.assertIn(r"CT-LIO & -- & -- & -- & -- & blocked \\", tex)
        self.assertIn("GT-seeded init", tex)


if __name__ == "__main__":
    unittest.main()
