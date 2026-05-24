import argparse
import importlib.util
import json
import struct
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock


REPO_ROOT = Path(__file__).resolve().parents[1]


def load_script_module(module_name: str, relative_path: str):
    script_path = REPO_ROOT / relative_path
    spec = importlib.util.spec_from_file_location(module_name, script_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module from {script_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module


def write_manifest(
    path: Path,
    *,
    binary: str,
    pcd_dir: str,
    gt_csv: str,
    variants: list[dict],
    variant_timeout_seconds: float | None = None,
) -> dict:
    problem = {
        "id": "test_problem",
        "title": "Test problem",
        "question": "Does the runner keep its contract?",
        "same_metrics": ["ate_m", "fps"],
        "dataset": {
            "pcd_dir": pcd_dir,
            "gt_csv": gt_csv,
        },
    }
    if variant_timeout_seconds is not None:
        problem["variant_timeout_seconds"] = variant_timeout_seconds
    manifest = {
        "problem": problem,
        "stable_interface": {
            "binary": binary,
            "methods": "fake_method",
            "primary_method": "fake_method",
        },
        "variants": variants,
    }
    path.write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest


def run_runner(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["python3", "evaluation/scripts/run_experiment_matrix.py", *args],
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )


def run_script(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["python3", *args],
        cwd=REPO_ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )


class RunExperimentMatrixScriptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner_module = load_script_module(
            "test_run_experiment_matrix_module",
            "evaluation/scripts/run_experiment_matrix.py",
        )
        cls.refresh_module = load_script_module(
            "test_refresh_study_docs_module", "evaluation/scripts/refresh_study_docs.py"
        )
        cls.paper_comparison_module = load_script_module(
            "test_generate_paper_comparison_module",
            "evaluation/scripts/generate_paper_comparison.py",
        )
        cls.reproduction_module = load_script_module(
            "test_generate_reproduction_status_module",
            "evaluation/scripts/generate_reproduction_status.py",
        )
        cls.lidar_action_plan_module = load_script_module(
            "test_generate_lidar_degeneracy_action_plan_module",
            "evaluation/scripts/generate_lidar_degeneracy_action_plan.py",
        )
        cls.lidar_checks_module = load_script_module(
            "test_run_lidar_degeneracy_checks_module",
            "evaluation/scripts/run_lidar_degeneracy_checks.py",
        )
        cls.lidar_health_module = load_script_module(
            "test_run_lidar_degradation_health_module",
            "evaluation/scripts/run_lidar_degradation_health.py",
        )
        cls.lidar_summary_module = load_script_module(
            "test_summarize_lidar_degeneracy_health_module",
            "evaluation/scripts/summarize_lidar_degeneracy_health.py",
        )
        cls.lidar_risk_module = load_script_module(
            "test_calibrate_lidar_degeneracy_risk_module",
            "evaluation/scripts/calibrate_lidar_degeneracy_risk.py",
        )
        cls.ct_icp_guarded_module = load_script_module(
            "test_simulate_ct_icp_guarded_trajectory_module",
            "evaluation/scripts/simulate_ct_icp_guarded_trajectory.py",
        )
        cls.ct_icp_fallback_module = load_script_module(
            "test_compare_ct_icp_fallback_sources_module",
            "evaluation/scripts/compare_ct_icp_fallback_sources.py",
        )
        cls.ct_icp_composite_module = load_script_module(
            "test_build_ct_icp_guarded_composite_module",
            "evaluation/scripts/build_ct_icp_guarded_composite.py",
        )
        cls.fixed_map_ndt_failure_module = load_script_module(
            "test_summarize_fixed_map_ndt_failure_modes_module",
            "evaluation/scripts/summarize_fixed_map_ndt_failure_modes.py",
        )
        cls.fixed_map_ndt_publish_guard_module = load_script_module(
            "test_build_fixed_map_ndt_publish_guard_module",
            "evaluation/scripts/build_fixed_map_ndt_publish_guard.py",
        )

    @staticmethod
    def write_lidar_gate_reports(
        tmp: Path,
        *,
        method_decision: str,
        risk_decision: str,
    ) -> tuple[Path, Path]:
        method_health_json = tmp / "method_health_comparison.json"
        risk_calibration_json = tmp / "risk_gt_calibration.json"
        policy = {"policy_version": "lidar_degeneracy_triage_v4"}
        method_reasons = {
            "pass": ["ok_no_risk"],
            "watch": ["low_convergence"],
            "investigate": ["cross_method_suspicious"],
            "fail": ["low_acceptance"],
        }.get(method_decision, ["unknown_reason"])
        risk_reasons = {
            "pass": ["ok_no_risk"],
            "watch": ["low_convergence"],
            "investigate": ["path_disagrees_with_healthy_median"],
            "fail": ["nonfinite_pose"],
        }.get(risk_decision, ["unknown_reason"])
        method_health_json.write_text(
            json.dumps(
                {
                    "policy": policy,
                    "sequences": [
                        {
                            "sequence": "demo_sequence",
                            "windows": [
                                {
                                    "name": "demo_window",
                                    "start": 0,
                                    "end": 10,
                                    "methods": {
                                        "demo_method": {
                                            "policy_decision": method_decision,
                                            "policy_reasons": method_reasons,
                                        }
                                    },
                                }
                            ],
                        }
                    ],
                },
                indent=2,
            )
            + "\n"
        )
        risk_calibration_json.write_text(
            json.dumps(
                {
                    "policy": policy,
                    "proxy_records": [
                        {
                            "sequence": "demo_sequence",
                            "window": "demo_window",
                            "method": "demo_method",
                            "policy_decision": risk_decision,
                            "risk_reasons": risk_reasons,
                        }
                    ],
                },
                indent=2,
            )
            + "\n"
        )
        return method_health_json, risk_calibration_json

    def test_reuse_aggregates_rebuilds_docs_without_local_data(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            manifest_path = tmp / "demo_matrix.json"
            output_dir = tmp / "results"
            docs_dir = tmp / "docs"
            output_dir.mkdir()

            manifest = write_manifest(
                manifest_path,
                binary="build/evaluation/pcd_dogfooding",
                pcd_dir="missing/pcd_dir",
                gt_csv="missing/gt.csv",
                variants=[
                    {
                        "id": "fast",
                        "label": "Fast",
                        "design_style": "speed",
                        "intent": "Reuse an existing aggregate.",
                        "args": ["--fast-mode"],
                    }
                ],
            )

            aggregate = {
                "generated_at": "2026-04-15T00:00:00+00:00",
                "manifest_path": str(manifest_path),
                "problem": manifest["problem"],
                "stable_interface": manifest["stable_interface"],
                "dataset": {
                    "pcd_dir": "experiments/fake_pcd",
                    "gt_csv": "experiments/fake_gt.csv",
                    "extra_args": ["--landmarks-csv", "experiments/fake_landmarks.csv"],
                },
                "variant_timeout_seconds": None,
                "status": "ready",
                "blocker": "",
                "next_step": "",
                "variants": [
                    {
                        "id": "fast",
                        "label": "Fast",
                        "design_style": "speed",
                        "intent": "Reuse an existing aggregate.",
                        "args": ["--fast-mode"],
                        "command": "build/evaluation/pcd_dogfooding ...",
                        "summary_path": "experiments/results/runs/demo_matrix/fast/summary.json",
                        "log_path": "experiments/results/runs/demo_matrix/fast/run.log",
                        "status": "OK",
                        "ate_m": 1.25,
                        "fps": 22.5,
                        "time_ms": 2000.0,
                        "frames": 45,
                        "note": "",
                        "benchmark_score": 100.0,
                        "readability_score": 4.4,
                        "readability_note": "Simple flag surface.",
                        "extensibility_score": 4.6,
                        "extensibility_note": "No extra tuning knobs.",
                        "decision": "Adopt as current default",
                        "decision_reason": "Best combined benchmark score.",
                    }
                ],
            }
            (output_dir / "demo_matrix.json").write_text(json.dumps(aggregate, indent=2) + "\n")

            completed = run_runner(
                "--manifest",
                str(manifest_path),
                "--output-dir",
                str(output_dir),
                "--docs-dir",
                str(docs_dir),
                "--reuse-aggregates",
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"runner failed unexpectedly:\n{completed.stdout}",
            )

            index = json.loads((output_dir / "index.json").read_text())
            self.assertEqual(len(index["problems"]), 1)
            self.assertEqual(index["problems"][0]["status"], "ready")
            self.assertEqual(index["problems"][0]["current_default"], "fast")
            self.assertTrue((docs_dir / "experiments.md").exists())
            self.assertTrue((docs_dir / "decisions.md").exists())
            self.assertTrue((docs_dir / "interfaces.md").exists())

    def test_variant_result_from_summary_reads_optional_rpe_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            summary_path = tmp / "summary.json"
            summary_path.write_text(
                json.dumps(
                    {
                        "methods": [
                            {
                                "name": "fake_method",
                                "status": "OK",
                                "ate_m": 1.25,
                                "rpe_trans_pct": 2.5,
                                "rpe_rot_deg_per_m": 0.03125,
                                "frames": 45,
                                "time_ms": 2000.0,
                                "fps": 22.5,
                                "note": "",
                            }
                        ]
                    },
                    indent=2,
                )
                + "\n"
            )

            result = self.runner_module.variant_result_from_summary(
                variant={
                    "id": "fast",
                    "label": "Fast",
                    "design_style": "speed",
                    "intent": "Read summary metrics.",
                    "args": ["--fast-mode"],
                },
                primary_method="fake_method",
                binary=REPO_ROOT / "build/evaluation/pcd_dogfooding",
                pcd_dir=REPO_ROOT / "dogfooding_results/kitti_raw_0009_200",
                gt_csv=REPO_ROOT / "experiments/reference_data/kitti_raw_0009_200_gt.csv",
                methods="fake_method",
                dataset_extra_args=[],
                summary_path=summary_path,
                log_path=tmp / "run.log",
            )

            self.assertEqual(result.ate_m, 1.25)
            self.assertEqual(result.rpe_trans_pct, 2.5)
            self.assertEqual(result.rpe_rot_deg_per_m, 0.03125)
            self.assertEqual(result.fps, 22.5)

    def test_variant_timeout_marks_problem_skipped_and_records_timeout(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            manifest_path = tmp / "timeout_matrix.json"
            output_dir = tmp / "results"
            docs_dir = tmp / "docs"
            pcd_dir = tmp / "pcd"
            gt_csv = tmp / "gt.csv"
            binary_path = tmp / "slow_binary.sh"

            output_dir.mkdir()
            pcd_dir.mkdir()
            gt_csv.write_text("timestamp,x,y,z,qx,qy,qz,qw\n")
            binary_path.write_text(
                textwrap.dedent(
                    """\
                    #!/usr/bin/env bash
                    sleep 5
                    """
                )
            )
            binary_path.chmod(0o755)

            write_manifest(
                manifest_path,
                binary=str(binary_path),
                pcd_dir=str(pcd_dir),
                gt_csv=str(gt_csv),
                variants=[
                    {
                        "id": "default",
                        "label": "Default",
                        "design_style": "baseline",
                        "intent": "Force a timeout.",
                        "args": [],
                    }
                ],
            )

            completed = run_runner(
                "--manifest",
                str(manifest_path),
                "--output-dir",
                str(output_dir),
                "--docs-dir",
                str(docs_dir),
                "--variant-timeout-seconds",
                "0.1",
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"runner failed unexpectedly:\n{completed.stdout}",
            )

            aggregate = json.loads((output_dir / "timeout_matrix.json").read_text())
            self.assertEqual(aggregate["status"], "skipped")
            self.assertIn("Timed out after 0.1 seconds.", aggregate["blocker"])
            self.assertEqual(len(aggregate["variants"]), 1)
            self.assertEqual(aggregate["variants"][0]["status"], "timeout_budget")
            self.assertEqual(aggregate["variants"][0]["time_ms"], 100.0)
            self.assertIn("Timed out after 0.1 seconds.", aggregate["variants"][0]["note"])

            run_log = output_dir / "runs" / "timeout_matrix" / "default" / "run.log"
            self.assertTrue(run_log.exists())
            self.assertIn("Timed out after 0.1 seconds.", run_log.read_text())
            self.assertFalse(
                (output_dir / "runs" / "timeout_matrix" / "default" / "summary.json").exists()
            )

            index = json.loads((output_dir / "index.json").read_text())
            self.assertEqual(index["problems"][0]["status"], "skipped")
            self.assertIsNone(index["problems"][0]["current_default"])

    def test_refresh_study_docs_adds_reuse_flags_by_default(self) -> None:
        commands: list[list[str]] = []
        with mock.patch.object(
            self.refresh_module,
            "parse_args",
            return_value=argparse.Namespace(
                rerun=False,
                manifest=["experiments/demo_matrix.json"],
            ),
        ), mock.patch.object(
            self.refresh_module,
            "run",
            side_effect=lambda cmd: commands.append(cmd),
        ):
            self.refresh_module.main()

        self.assertEqual(commands[0][:4], [
            "python3",
            "evaluation/scripts/run_experiment_matrix.py",
            "--reuse-existing",
            "--reuse-aggregates",
        ])
        self.assertEqual(commands[0][-2:], ["--manifest", "experiments/demo_matrix.json"])
        self.assertEqual(
            commands[1:],
            [
                ["python3", "evaluation/scripts/generate_publication_docs.py"],
                ["python3", "evaluation/scripts/export_paper_assets.py"],
                ["python3", "evaluation/scripts/generate_paper_comparison.py"],
                ["python3", "evaluation/scripts/generate_reproduction_status.py"],
                ["python3", "evaluation/scripts/generate_variant_analysis.py"],
            ],
        )

    def test_refresh_study_docs_omits_reuse_flags_for_rerun(self) -> None:
        commands: list[list[str]] = []
        with mock.patch.object(
            self.refresh_module,
            "parse_args",
            return_value=argparse.Namespace(rerun=True, manifest=[]),
        ), mock.patch.object(
            self.refresh_module,
            "run",
            side_effect=lambda cmd: commands.append(cmd),
        ):
            self.refresh_module.main()

        self.assertEqual(
            commands[0],
            ["python3", "evaluation/scripts/run_experiment_matrix.py"],
        )

    def test_generate_reproduction_status_render_contains_scope_and_next_step(self) -> None:
        markdown = self.reproduction_module.render_markdown(
            {
                "methods": {
                    "demo_method": {
                        "paper": "Example et al., Demo Method 2026",
                        "method_readme": "papers/demo_method/README.md",
                        "repo_scope_label": "Compact baseline",
                        "repo_scope_summary": "Small benchmark-oriented implementation.",
                        "reproduction_status_label": "Benchmark-comparable only",
                        "reproduction_status_summary": "Not faithful enough for direct paper claims.",
                        "numeric_comparison_label": "Indirect only",
                        "numeric_comparison_summary": "Metrics differ.",
                        "main_blocker": "No paper-metric rerun.",
                        "next_step": "Add the original metric.",
                        "reported_dataset": "Demo dataset",
                        "reported_metric": "Demo metric",
                        "notes": "Demo note."
                    }
                }
            },
            "2026-04-15T00:00:00+00:00",
        )
        self.assertIn("# Reproduction Status", markdown)
        self.assertIn("Benchmark-comparable only", markdown)
        self.assertIn("No paper-metric rerun.", markdown)
        self.assertIn("Add the original metric.", markdown)
        self.assertIn("papers/demo_method/README.md", markdown)

    def test_gather_repo_defaults_preserves_optional_rpe_metrics(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            aggregate_path = tmp / "demo_aggregate.json"
            aggregate_path.write_text(
                json.dumps(
                    {
                        "stable_interface": {"methods": "litamin2"},
                        "dataset": {"pcd_dir": "dogfooding_results/kitti_seq_00_108"},
                        "variants": [
                            {
                                "id": "paper",
                                "label": "Paper-like",
                                "decision": "Adopt as current default",
                                "ate_m": 1.5,
                                "rpe_trans_pct": 2.75,
                                "rpe_rot_deg_per_m": 0.012345,
                                "fps": 15.0,
                            }
                        ],
                    },
                    indent=2,
                )
                + "\n"
            )

            with mock.patch.object(self.paper_comparison_module, "REPO_ROOT", tmp):
                defaults = self.paper_comparison_module.gather_repo_defaults(
                    {
                        "problems": [
                            {
                                "status": "ready",
                                "aggregate_path": aggregate_path.name,
                            }
                        ]
                    }
                )

            self.assertEqual(defaults["litamin2"][0]["ate_m"], 1.5)
            self.assertEqual(defaults["litamin2"][0]["rpe_trans_pct"], 2.75)
            self.assertEqual(defaults["litamin2"][0]["rpe_rot_deg_per_m"], 0.012345)
            self.assertEqual(defaults["litamin2"][0]["fps"], 15.0)

    def test_lidar_degeneracy_check_runner_smoke(self) -> None:
        completed = run_script("evaluation/scripts/run_lidar_degeneracy_checks.py")

        self.assertEqual(completed.returncode, 0, msg=completed.stdout)
        self.assertIn("[ok] lidar degeneracy checks passed", completed.stdout)

    def test_lidar_degradation_health_uses_matcher_correspondence_default(self) -> None:
        argv = [
            "run_lidar_degradation_health.py",
            "windows.json",
            "out",
            "--method",
            "kiss_keyframe",
        ]
        with mock.patch.object(sys, "argv", argv):
            args = self.lidar_health_module.parse_args()

        self.assertEqual(args.kiss_min_correspondences, 80)
        self.assertEqual(args.min_keyframe_correspondences, 1000)

    def test_lidar_degradation_health_uses_geometry_correspondence_default(self) -> None:
        argv = [
            "run_lidar_degradation_health.py",
            "windows.json",
            "out",
            "--method",
            "geometry_icp",
        ]
        with mock.patch.object(sys, "argv", argv):
            args = self.lidar_health_module.parse_args()

        self.assertEqual(args.geometry_min_correspondences, 40)

    def test_lidar_degradation_health_keeps_low_motion_regularizer_disabled_by_default(self) -> None:
        argv = [
            "run_lidar_degradation_health.py",
            "windows.json",
            "out",
            "--method",
            "intensity_bev",
        ]
        with mock.patch.object(sys, "argv", argv):
            args = self.lidar_health_module.parse_args()

        self.assertEqual(args.intensity_bev_low_motion_score_margin, 0.0)
        self.assertFalse(args.intensity_bev_low_motion_stress_only)

    def test_lidar_degeneracy_check_runner_policy_gate_passes_clean_reports(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            method_health_json, risk_calibration_json = self.write_lidar_gate_reports(
                Path(tmpdir),
                method_decision="pass",
                risk_decision="watch",
            )

            completed = run_script(
                "evaluation/scripts/run_lidar_degeneracy_checks.py",
                "--enforce-policy",
                "--method-health-json",
                str(method_health_json),
                "--risk-calibration-json",
                str(risk_calibration_json),
            )

        self.assertEqual(completed.returncode, 0, msg=completed.stdout)
        self.assertIn("[ok] lidar degeneracy policy gate passed", completed.stdout)

    def test_lidar_degeneracy_check_runner_policy_gate_fails_bad_reports(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            gate_output_dir = Path(tmpdir) / "gate_report"
            method_health_json, risk_calibration_json = self.write_lidar_gate_reports(
                Path(tmpdir),
                method_decision="fail",
                risk_decision="investigate",
            )

            completed = run_script(
                "evaluation/scripts/run_lidar_degeneracy_checks.py",
                "--enforce-policy",
                "--method-health-json",
                str(method_health_json),
                "--risk-calibration-json",
                str(risk_calibration_json),
                "--policy-gate-output-dir",
                str(gate_output_dir),
            )
            gate_report = json.loads((gate_output_dir / "policy_gate_report.json").read_text())
            gate_markdown = (gate_output_dir / "policy_gate_report.md").read_text()

        self.assertEqual(completed.returncode, 1, msg=completed.stdout)
        self.assertIn("[fail] lidar degeneracy policy gate failed", completed.stdout)
        self.assertIn("method_health_comparison: fail rows 1 > 0", completed.stdout)
        self.assertIn("risk_gt_calibration: investigate rows 1 > 0", completed.stdout)
        self.assertIn("[gate] top offenders", completed.stdout)
        self.assertIn("decision=fail reasons=low_acceptance", completed.stdout)
        self.assertEqual(gate_report["offender_count"], 2)
        self.assertEqual(
            gate_report["policy"]["path"],
            "evaluation/config/lidar_degeneracy_triage_policy.json",
        )
        self.assertEqual(
            gate_report["reports"]["method_health_comparison"]["counts"]["fail"],
            1,
        )
        self.assertEqual(gate_report["offenders"][0]["policy_decision"], "fail")
        self.assertEqual(gate_report["offenders"][0]["policy_reasons"], ["low_acceptance"])
        self.assertIn("| `method_health_comparison` | `demo_sequence` |", gate_markdown)

    def test_lidar_degeneracy_check_runner_can_include_fixed_map_ndt_audit(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            gate_output_dir = tmp / "gate_report"
            method_health_json, risk_calibration_json = self.write_lidar_gate_reports(
                tmp,
                method_decision="pass",
                risk_decision="watch",
            )
            fixed_map_audit_json = tmp / "fixed_map_ndt_failure_audit.json"
            fixed_map_audit_json.write_text(
                json.dumps(
                    {
                        "policy": {"policy_version": "lidar_degeneracy_triage_v4"},
                        "rows": [
                            {
                                "sequence": "02",
                                "variant": "seed_ct_icp",
                                "seed_source": "ct_icp",
                                "frames": 108,
                                "policy_decision": "fail",
                                "policy_reasons": ["accepted_bad_localization"],
                            }
                        ],
                    }
                )
                + "\n"
            )

            completed = run_script(
                "evaluation/scripts/run_lidar_degeneracy_checks.py",
                "--enforce-policy",
                "--method-health-json",
                str(method_health_json),
                "--risk-calibration-json",
                str(risk_calibration_json),
                "--include-fixed-map-ndt-audit",
                "--fixed-map-ndt-audit-json",
                str(fixed_map_audit_json),
                "--policy-gate-output-dir",
                str(gate_output_dir),
            )
            gate_report = json.loads((gate_output_dir / "policy_gate_report.json").read_text())

        self.assertEqual(completed.returncode, 1, msg=completed.stdout)
        self.assertIn("fixed_map_ndt_failure_audit: fail rows 1 > 0", completed.stdout)
        self.assertEqual(
            gate_report["reports"]["fixed_map_ndt_failure_audit"]["counts"]["fail"],
            1,
        )
        self.assertEqual(gate_report["offenders"][0]["report"], "fixed_map_ndt_failure_audit")
        self.assertEqual(gate_report["offenders"][0]["method"], "fixed_map_ndt:ct_icp")

    def test_fixed_map_ndt_audit_policy_records_normalize_rows(self) -> None:
        records = self.lidar_checks_module.fixed_map_ndt_audit_policy_records(
            {
                "rows": [
                    {
                        "sequence": "08",
                        "variant": "seed_scan_context_stride_5",
                        "seed_source": "scan_context",
                        "frames": 108,
                        "policy_decision": "watch",
                        "policy_reasons": ["global_initializer_near_basin"],
                    }
                ]
            }
        )

        self.assertEqual(records[0]["sequence"], "kitti_seq_08")
        self.assertEqual(records[0]["window"], "seed_scan_context_stride_5")
        self.assertEqual(records[0]["start"], 0)
        self.assertEqual(records[0]["end"], 108)
        self.assertEqual(records[0]["method"], "fixed_map_ndt:scan_context")
        self.assertEqual(records[0]["policy_reasons"], ["global_initializer_near_basin"])

    def test_lidar_degeneracy_action_plan_classifies_reason_templates(self) -> None:
        classify = self.lidar_action_plan_module.classify_reason

        self.assertEqual(classify("low_acceptance"), "local_matcher_failure")
        self.assertEqual(classify("all_pairs_failed"), "local_matcher_failure")
        self.assertEqual(classify("ct_icp_internal_convergence_low"), "diagnostic_watch")
        self.assertEqual(classify("low_motion_margin_dominant"), "false_confidence_risk")
        self.assertEqual(classify("motion_margin_dominant"), "false_confidence_risk")
        self.assertEqual(classify("overlap_tail"), "false_confidence_risk")
        self.assertEqual(
            classify("path_disagrees_with_healthy_median"),
            "cross_method_disagreement",
        )
        self.assertEqual(classify("nonfinite_pose"), "hard_numerical_failure")
        self.assertEqual(classify("accepted_bad_localization"), "wrong_pose_acceptance")
        self.assertEqual(classify("unfiltered_ndt_score_trap"), "global_hypothesis_selection")
        self.assertEqual(classify("rejected_bad_seed_detectable"), "detectable_bad_seed")
        self.assertEqual(classify("bad_localization"), "map_localization_gap")
        self.assertEqual(classify("future_reason"), "unclassified_policy_reason")

    def test_lidar_degeneracy_action_plan_groups_fixed_map_ndt_reasons(self) -> None:
        gate_report = {
            "policy": {"policy_version": "lidar_degeneracy_triage_v4"},
            "offender_count": 2,
            "offenders": [
                {
                    "report": "fixed_map_ndt_failure_audit",
                    "sequence": "kitti_seq_02",
                    "window": "seed_ct_icp",
                    "start": None,
                    "end": 108,
                    "method": "fixed_map_ndt:ct_icp",
                    "policy_decision": "fail",
                    "policy_reasons": ["accepted_bad_localization"],
                },
                {
                    "report": "fixed_map_ndt_failure_audit",
                    "sequence": "kitti_seq_02",
                    "window": "seed_scan_context_stride_5_topk_10_unfiltered",
                    "start": None,
                    "end": 108,
                    "method": "fixed_map_ndt:scan_context",
                    "policy_decision": "fail",
                    "policy_reasons": [
                        "accepted_bad_localization",
                        "unfiltered_ndt_score_trap",
                    ],
                },
            ],
        }

        plan = self.lidar_action_plan_module.build_action_plan(gate_report)

        categories = [item["category"] for item in plan["action_items"]]
        self.assertEqual(categories[0], "wrong_pose_acceptance")
        self.assertIn("global_hypothesis_selection", categories)
        scan_context_items = [
            item
            for item in plan["action_items"]
            if item["method"] == "fixed_map_ndt:scan_context"
        ]
        self.assertEqual(len(scan_context_items), 2)
        self.assertEqual(plan["summary"]["category_rows"]["wrong_pose_acceptance"], 2)
        self.assertEqual(plan["summary"]["category_rows"]["global_hypothesis_selection"], 1)

    def test_lidar_degeneracy_ct_icp_diagnostic_watch_is_not_product_failure(self) -> None:
        summarize = self.lidar_summary_module
        calibrate = self.lidar_risk_module
        diagnostic_row = {
            "accepted_rate": 1.0,
            "converged_rate": 0.1,
            "flags": "ct_icp_internal_convergence_low",
        }

        self.assertEqual(summarize.health_state(diagnostic_row), "diagnostic_watch")
        self.assertEqual(
            summarize.failure_awareness("nominal", "diagnostic_watch"),
            "diagnostic_watch",
        )
        self.assertFalse(
            summarize.product_local_risk(
                {**diagnostic_row, "health_state": "diagnostic_watch"}
            )
        )
        self.assertEqual(
            calibrate.risk_bucket(
                {
                    **diagnostic_row,
                    "health_state": "diagnostic_watch",
                    "risk_state": "diagnostic_watch",
                }
            ),
            "diagnostic_watch",
        )
        clear_status = summarize.ct_icp_watch_clear_status(
            {
                **diagnostic_row,
                "health_state": "diagnostic_watch",
                "ct_icp_refinement_gate_rate": 1.0,
                "ct_icp_iterations_mean": 7.9,
                "ct_icp_max_iterations": 8,
                "path_vs_healthy_median": 1.0,
                "path_vs_all_median": 1.0,
            }
        )
        self.assertFalse(clear_status["watch_clear_candidate"])
        self.assertEqual(clear_status["watch_clear_blockers"], ["iterations_pinned"])
        self.assertEqual(clear_status["watch_action"], "optimizer_retry")
        clear_candidate = summarize.ct_icp_watch_clear_status(
            {
                **diagnostic_row,
                "health_state": "diagnostic_watch",
                "ct_icp_refinement_gate_rate": 1.0,
                "ct_icp_iterations_mean": 6.5,
                "ct_icp_max_iterations": 8,
                "path_vs_healthy_median": 1.0,
                "path_vs_all_median": 1.0,
            }
        )
        self.assertTrue(clear_candidate["watch_clear_candidate"])
        self.assertEqual(clear_candidate["watch_clear_blockers"], [])
        self.assertEqual(clear_candidate["watch_action"], "clear_candidate")
        fallback_status = summarize.ct_icp_watch_clear_status(
            {
                **diagnostic_row,
                "health_state": "diagnostic_watch",
                "ct_icp_refinement_gate_rate": 1.0,
                "ct_icp_iterations_mean": 6.5,
                "ct_icp_max_iterations": 8,
                "path_vs_healthy_median": 3.0,
                "path_vs_all_median": 3.5,
            }
        )
        self.assertEqual(fallback_status["watch_action"], "fallback_required")
        reject_status = summarize.ct_icp_watch_clear_status(
            {
                **diagnostic_row,
                "health_state": "diagnostic_watch",
                "ct_icp_refinement_gate_rate": 0.95,
                "ct_icp_iterations_mean": 6.5,
                "ct_icp_max_iterations": 8,
                "path_vs_healthy_median": 1.0,
                "path_vs_all_median": 1.0,
            }
        )
        self.assertEqual(reject_status["watch_action"], "reject_or_retry")
        reference_missing_status = summarize.ct_icp_watch_clear_status(
            {
                **diagnostic_row,
                "health_state": "diagnostic_watch",
                "ct_icp_refinement_gate_rate": 1.0,
                "ct_icp_iterations_mean": 6.5,
                "ct_icp_max_iterations": 8,
                "path_vs_healthy_median": None,
                "path_vs_all_median": None,
            }
        )
        self.assertEqual(reference_missing_status["watch_action"], "reference_missing")
        self.assertEqual(
            summarize.ct_icp_production_guard(
                {
                    **diagnostic_row,
                    "health_state": "diagnostic_watch",
                    "risk_state": "diagnostic_watch",
                    "watch_action": "fallback_required",
                }
            )["ct_icp_guard_decision"],
            "fallback_to_prior",
        )
        self.assertEqual(
            summarize.ct_icp_production_guard(
                {
                    "accepted_rate": 1.0,
                    "health_state": "ok",
                    "risk_state": "ok",
                    "flags": "ok",
                }
            )["ct_icp_guard_decision"],
            "use_refined",
        )
        self.assertEqual(
            summarize.ct_icp_production_guard(
                {
                    "accepted_rate": 0.8,
                    "health_state": "suspicious",
                    "risk_state": "suspicious",
                    "flags": "ct_icp_internal_convergence_low",
                }
            )["ct_icp_guard_decision"],
            "reject_or_retry",
        )

        partial_acceptance_row = {
            "accepted_rate": 0.655,
            "converged_rate": 0.0,
            "flags": "ct_icp_internal_convergence_low",
        }
        self.assertEqual(summarize.health_state(partial_acceptance_row), "suspicious")

    def test_ct_icp_guarded_trajectory_uses_velocity_prior(self) -> None:
        guarded = self.ct_icp_guarded_module
        payload = {
            "method": "ct_icp_window_odometry",
            "parameters": {"max_step_translation": 1.0, "max_step_yaw_deg": 10.0},
            "pairs": [
                {
                    "index": 1,
                    "accepted": True,
                    "dx_curr_to_prev_m": 0.2,
                    "dy_curr_to_prev_m": 0.0,
                    "yaw_curr_to_prev_deg": 2.0,
                    "used_dx_curr_to_prev_m": 0.2,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 2.0,
                },
                {
                    "index": 2,
                    "accepted": True,
                    "dx_curr_to_prev_m": 2.0,
                    "dy_curr_to_prev_m": 0.0,
                    "yaw_curr_to_prev_deg": 30.0,
                    "used_dx_curr_to_prev_m": 2.0,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 30.0,
                },
                {
                    "index": 3,
                    "accepted": True,
                    "dx_curr_to_prev_m": 0.1,
                    "dy_curr_to_prev_m": 0.0,
                    "yaw_curr_to_prev_deg": 1.0,
                    "used_dx_curr_to_prev_m": 0.1,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 1.0,
                },
            ],
        }

        out = guarded.simulate_guarded_payload(
            result_payload=payload,
            guard_decision="fallback_to_prior",
            source_result_json=Path("demo.json"),
        )

        self.assertEqual(out["summary"]["velocity_prior_pairs"], 3)
        self.assertEqual(out["summary"]["prior_bootstrap_pairs"], 1)
        self.assertEqual(out["summary"]["hold_pairs"], 0)
        self.assertAlmostEqual(out["summary"]["guarded_path_length_m"], 0.6)
        self.assertLess(
            out["summary"]["guarded_step_delta_max_m"],
            out["summary"]["original_step_delta_max_m"],
        )
        self.assertEqual(out["pairs"][0]["guard_source"], "velocity_bootstrap")
        self.assertEqual(out["pairs"][1]["guard_source"], "velocity_prior")

    def test_ct_icp_fallback_bakeoff_selects_eligible_peer(self) -> None:
        fallback = self.ct_icp_fallback_module
        candidates = [
            fallback.mark_candidate_eligibility(
                {
                    "source": "ct_icp_original",
                    "source_kind": "ct_icp_refined",
                    "health_state": "diagnostic_watch",
                    "risk_state": "diagnostic_watch",
                    "accepted_rate": 1.0,
                    "path_status": "path_high",
                    "reference_error": 1.0,
                }
            ),
            fallback.mark_candidate_eligibility(
                {
                    "source": "self_velocity",
                    "source_kind": "internal_prior",
                    "health_state": "diagnostic_watch",
                    "risk_state": "diagnostic_watch",
                    "accepted_rate": 1.0,
                    "path_status": "path_high",
                    "reference_error": 0.8,
                }
            ),
            fallback.mark_candidate_eligibility(
                {
                    "source": "geometry_icp",
                    "source_kind": "external_peer",
                    "health_state": "ok",
                    "risk_state": "ok",
                    "accepted_rate": 1.0,
                    "path_status": "reference_consistent",
                    "reference_error": 0.2,
                }
            ),
            fallback.mark_candidate_eligibility(
                {
                    "source": "intensity_bev",
                    "source_kind": "external_peer",
                    "health_state": "suspicious",
                    "risk_state": "suspicious",
                    "accepted_rate": 1.0,
                    "path_status": "reference_consistent",
                    "reference_error": 0.1,
                }
            ),
        ]

        selected = fallback.select_best_candidate(candidates)

        self.assertIsNotNone(selected)
        self.assertEqual(selected["source"], "geometry_icp")
        self.assertFalse(candidates[0]["eligible"])
        self.assertIn("guard_rejects_refined", candidates[0]["eligibility_blockers"])
        self.assertFalse(candidates[1]["eligible"])
        self.assertIn("path_not_reference_consistent", candidates[1]["eligibility_blockers"])
        self.assertFalse(candidates[3]["eligible"])
        self.assertIn("source_health_not_ok", candidates[3]["eligibility_blockers"])

    def test_ct_icp_guarded_composite_uses_selected_fallback(self) -> None:
        composite = self.ct_icp_composite_module
        guarded = self.ct_icp_guarded_module
        ct_payload = {
            "method": "ct_icp_window_odometry",
            "pairs": [
                {
                    "index": 1,
                    "used_dx_curr_to_prev_m": 5.0,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 0.0,
                },
                {
                    "index": 2,
                    "used_dx_curr_to_prev_m": 5.0,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 0.0,
                },
            ],
        }
        self_velocity_payload = {
            "pairs": [
                {
                    "index": 1,
                    "guarded_dx_curr_to_prev_m": 0.5,
                    "guarded_dy_curr_to_prev_m": 0.0,
                    "guarded_yaw_curr_to_prev_deg": 0.0,
                },
                {
                    "index": 2,
                    "guarded_dx_curr_to_prev_m": 0.5,
                    "guarded_dy_curr_to_prev_m": 0.0,
                    "guarded_yaw_curr_to_prev_deg": 0.0,
                },
            ],
        }
        selected_payload = {
            "pairs": [
                {
                    "index": 1,
                    "used_dx_curr_to_prev_m": 1.0,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 0.0,
                },
                {
                    "index": 2,
                    "used_dx_curr_to_prev_m": 1.0,
                    "used_dy_curr_to_prev_m": 0.0,
                    "used_yaw_curr_to_prev_deg": 0.0,
                },
            ],
        }

        out = composite.build_composite_payload(
            guarded_module=guarded,
            ct_payload=ct_payload,
            self_velocity_payload=self_velocity_payload,
            selected_source="geometry_icp",
            selected_payload=selected_payload,
            guard_decision="fallback_to_prior",
            healthy_path_median=2.0,
            all_path_median=2.0,
            source_result_json=Path("ct.json"),
            selected_result_json=Path("geometry.json"),
        )

        self.assertEqual(out["summary"]["source_pair_counts"], {"geometry_icp": 2})
        self.assertEqual(out["summary"]["fallback_pair_rate"], 1.0)
        self.assertEqual(out["summary"]["missing_selected_pairs"], 0)
        self.assertEqual(out["summary"]["path_status"], "reference_consistent")
        self.assertAlmostEqual(out["summary"]["path_length_m"], 2.0)
        self.assertEqual(out["pairs"][0]["composite_source"], "geometry_icp")

    def test_fixed_map_ndt_failure_audit_flags_silent_bad_accepts(self) -> None:
        audit = self.fixed_map_ndt_failure_module
        row = {
            "status": "ok",
            "seed_source": "ct_icp",
            "source_class": "ct_icp_prior",
            "ate_m": 8.3,
            "rpe_trans_pct": 2.3,
            "fps": 12.0,
            "accepted_rate": 0.91,
        }

        decision = audit.classify_row(row)

        self.assertEqual(decision["policy_decision"], "fail")
        self.assertIn("accepted_bad_localization", decision["policy_reasons"])

    def test_fixed_map_ndt_failure_audit_distinguishes_detectable_rejects(self) -> None:
        audit = self.fixed_map_ndt_failure_module
        row = {
            "status": "ok",
            "seed_source": "velocity",
            "source_class": "dead_reckoning_prior",
            "ate_m": 55.0,
            "rpe_trans_pct": 90.0,
            "fps": 12.0,
            "accepted_rate": 0.0,
        }

        decision = audit.classify_row(row)

        self.assertEqual(decision["policy_decision"], "investigate")
        self.assertEqual(decision["policy_reasons"], ["rejected_bad_seed_detectable"])

    def test_fixed_map_ndt_failure_audit_parses_scan_context_note(self) -> None:
        audit = self.fixed_map_ndt_failure_module
        note = (
            "Fixed-map NDT: target map built once from GT/reference poses "
            "(stride=5, map_points=44000). Seed source=scan_context, "
            "accepted=98/107, rejected=9, seed fallbacks=66. "
            "ScanContext hits=41/107, top_k=10, "
            "NDT candidates evaluated=41, mean distance=0.0756."
        )

        metadata = audit.metadata_from_note(note)

        self.assertEqual(metadata["map_stride"], 5)
        self.assertEqual(metadata["map_points"], 44000)
        self.assertEqual(metadata["accepted"], 98)
        self.assertEqual(metadata["attempted"], 107)
        self.assertAlmostEqual(metadata["accepted_rate"], 98 / 107)
        self.assertEqual(metadata["scan_context_hits"], 41)
        self.assertEqual(metadata["scan_context_attempts"], 107)
        self.assertEqual(metadata["ndt_candidate_evals"], 41)
        self.assertAlmostEqual(metadata["scan_context_mean_distance"], 0.0756)

    def test_fixed_map_ndt_publish_guard_blocks_wrong_pose_acceptance(self) -> None:
        guard = self.fixed_map_ndt_publish_guard_module.publish_guard(
            {
                "policy_decision": "fail",
                "policy_reasons": ["accepted_bad_localization"],
                "seed_source": "ct_icp",
                "source_class": "ct_icp_prior",
            }
        )

        self.assertEqual(guard["publish_decision"], "block_publish")
        self.assertFalse(guard["allow_pose_publish"])
        self.assertEqual(guard["safety_state"], "accepted_wrong_pose")
        self.assertEqual(guard["required_gate"], "second_pose_verifier")

    def test_fixed_map_ndt_publish_guard_marks_score_trap(self) -> None:
        guard = self.fixed_map_ndt_publish_guard_module.publish_guard(
            {
                "policy_decision": "fail",
                "policy_reasons": [
                    "accepted_bad_localization",
                    "unfiltered_ndt_score_trap",
                ],
                "seed_source": "scan_context",
                "source_class": "global_initializer",
            }
        )

        self.assertEqual(guard["publish_decision"], "block_publish")
        self.assertFalse(guard["allow_pose_publish"])
        self.assertEqual(guard["safety_state"], "global_candidate_score_trap")
        self.assertEqual(
            guard["required_gate"], "calibrated_global_hypothesis_verifier"
        )

    def test_fixed_map_ndt_publish_guard_keeps_oracle_rows_lab_only(self) -> None:
        guard = self.fixed_map_ndt_publish_guard_module.publish_guard(
            {
                "policy_decision": "pass",
                "policy_reasons": ["ok_no_risk"],
                "seed_source": "gt",
                "source_class": "oracle_seed",
            }
        )

        self.assertEqual(guard["publish_decision"], "lab_only")
        self.assertFalse(guard["allow_pose_publish"])
        self.assertEqual(guard["required_gate"], "non_oracle_runtime_initializer")

    def test_lidar_degeneracy_action_plan_prioritizes_failures(self) -> None:
        gate_report = {
            "policy": {"policy_version": "lidar_degeneracy_triage_v4"},
            "offender_count": 3,
            "offenders": [
                {
                    "report": "method_health_comparison",
                    "sequence": "demo_sequence",
                    "window": "false_confidence",
                    "start": 20,
                    "end": 30,
                    "method": "intensity_bev",
                    "policy_decision": "investigate",
                    "policy_reasons": ["motion_margin_dominant"],
                },
                {
                    "report": "method_health_comparison",
                    "sequence": "demo_sequence",
                    "window": "local_fail",
                    "start": 0,
                    "end": 10,
                    "method": "geometry_icp",
                    "policy_decision": "fail",
                    "policy_reasons": ["low_acceptance"],
                },
                {
                    "report": "risk_gt_calibration",
                    "sequence": "demo_sequence",
                    "window": "local_fail",
                    "start": 0,
                    "end": 10,
                    "method": "geometry_icp",
                    "policy_decision": "fail",
                    "policy_reasons": ["low_acceptance"],
                },
            ],
        }

        plan = self.lidar_action_plan_module.build_action_plan(gate_report)

        self.assertEqual(plan["summary"]["action_items"], 2)
        self.assertEqual(plan["action_items"][0]["policy_decision"], "fail")
        self.assertEqual(plan["action_items"][0]["category"], "local_matcher_failure")
        self.assertEqual(plan["action_items"][0]["method"], "geometry_icp")
        self.assertEqual(plan["action_items"][0]["offender_rows"], 2)
        self.assertEqual(plan["action_items"][1]["category"], "false_confidence_risk")

    def test_generate_lidar_degeneracy_action_plan_writes_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            gate_report_path = tmp / "policy_gate_report.json"
            output_dir = tmp / "action_plan"
            gate_report_path.write_text(
                json.dumps(
                    {
                        "policy": {"policy_version": "lidar_degeneracy_triage_v4"},
                        "offender_count": 1,
                        "offenders": [
                            {
                                "report": "method_health_comparison",
                                "sequence": "demo_sequence",
                                "window": "demo_window",
                                "start": 0,
                                "end": 10,
                                "method": "kiss_keyframe",
                                "policy_decision": "fail",
                                "policy_reasons": ["all_pairs_failed", "low_acceptance"],
                            }
                        ],
                    },
                    indent=2,
                )
                + "\n"
            )

            completed = run_script(
                "evaluation/scripts/generate_lidar_degeneracy_action_plan.py",
                "--gate-report",
                str(gate_report_path),
                "--output-dir",
                str(output_dir),
            )
            action_plan = json.loads((output_dir / "policy_gate_action_plan.json").read_text())
            markdown = (output_dir / "policy_gate_action_plan.md").read_text()

        self.assertEqual(completed.returncode, 0, msg=completed.stdout)
        self.assertEqual(action_plan["action_items"][0]["category"], "local_matcher_failure")
        self.assertEqual(action_plan["action_items"][0]["method"], "kiss_keyframe")
        self.assertIn("Repair local matcher acceptance", markdown)


class RunMultimodalStudyScriptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.multimodal_module = load_script_module(
            "test_run_multimodal_study_module",
            "evaluation/scripts/run_multimodal_study.py",
        )

    def test_run_multimodal_study_runs_prepare_and_runner_with_overrides(self) -> None:
        commands: list[list[str]] = []
        manifests = [
            "experiments/okvis_kitti_raw_0009_matrix.json",
            "experiments/okvis_kitti_raw_0061_matrix.json",
            "experiments/okvis_kitti_raw_0009_full_matrix.json",
            "experiments/okvis_kitti_raw_0061_full_matrix.json",
        ]
        args = argparse.Namespace(
            method=["okvis"],
            sequence=[],
            include_full=True,
            skip_prepare_inputs=False,
            kitti_root="/data/kitti_raw",
            camera_dir="image_03",
            output_dir="/tmp/mm_results",
            docs_dir="/tmp/mm_docs",
            reuse_existing=True,
            variant_timeout_seconds=42.5,
            list_manifests=False,
        )
        with mock.patch.object(
            self.multimodal_module,
            "parse_args",
            return_value=args,
        ), mock.patch.object(
            self.multimodal_module,
            "manifest_paths",
            return_value=manifests,
        ) as manifest_paths_mock, mock.patch.object(
            self.multimodal_module,
            "run",
            side_effect=lambda cmd: commands.append(cmd),
        ):
            rc = self.multimodal_module.main()

        self.assertEqual(rc, 0)
        manifest_paths_mock.assert_called_once_with(
            ["okvis"],
            [
                "kitti_raw_0009_200",
                "kitti_raw_0061_200",
                "kitti_raw_0009_full",
                "kitti_raw_0061_full",
            ],
        )
        self.assertEqual(
            commands[0],
            [
                sys.executable,
                "evaluation/scripts/prepare_kitti_multimodal_inputs.py",
                "--kitti-root",
                "/data/kitti_raw",
                "--camera-dir",
                "image_03",
                "--sequence",
                "kitti_raw_0009_200",
                "--sequence",
                "kitti_raw_0061_200",
                "--sequence",
                "kitti_raw_0009_full",
                "--sequence",
                "kitti_raw_0061_full",
            ],
        )
        self.assertEqual(
            commands[1],
            [
                sys.executable,
                "evaluation/scripts/run_experiment_matrix.py",
                "--output-dir",
                "/tmp/mm_results",
                "--docs-dir",
                "/tmp/mm_docs",
                "--reuse-existing",
                "--variant-timeout-seconds",
                "42.5",
                "--manifest",
                "experiments/okvis_kitti_raw_0009_matrix.json",
                "--manifest",
                "experiments/okvis_kitti_raw_0061_matrix.json",
                "--manifest",
                "experiments/okvis_kitti_raw_0009_full_matrix.json",
                "--manifest",
                "experiments/okvis_kitti_raw_0061_full_matrix.json",
            ],
        )

    def test_run_multimodal_study_lists_manifests_without_running(self) -> None:
        manifests = [
            "experiments/okvis_kitti_raw_0009_matrix.json",
            "experiments/okvis_kitti_raw_0061_matrix.json",
        ]
        args = argparse.Namespace(
            method=["okvis"],
            sequence=["kitti_raw_0009_200", "kitti_raw_0061_200"],
            include_full=False,
            skip_prepare_inputs=False,
            kitti_root="/tmp/kitti_real",
            camera_dir="image_02",
            output_dir="/tmp/mm_results",
            docs_dir="/tmp/mm_docs",
            reuse_existing=False,
            variant_timeout_seconds=None,
            list_manifests=True,
        )
        with mock.patch.object(
            self.multimodal_module,
            "parse_args",
            return_value=args,
        ), mock.patch.object(
            self.multimodal_module,
            "manifest_paths",
            return_value=manifests,
        ), mock.patch.object(
            self.multimodal_module,
            "run",
        ) as run_mock, mock.patch(
            "builtins.print"
        ) as print_mock:
            rc = self.multimodal_module.main()

        self.assertEqual(rc, 0)
        run_mock.assert_not_called()
        print_mock.assert_any_call("experiments/okvis_kitti_raw_0009_matrix.json")
        print_mock.assert_any_call("experiments/okvis_kitti_raw_0061_matrix.json")

    def test_manifest_paths_raises_for_missing_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            experiments_dir = tmp / "experiments"
            experiments_dir.mkdir()
            existing = experiments_dir / "okvis_kitti_raw_0009_matrix.json"
            existing.write_text("{}\n")

            with mock.patch.object(self.multimodal_module, "REPO_ROOT", tmp):
                paths = self.multimodal_module.manifest_paths(
                    ["okvis"],
                    ["kitti_raw_0009_200"],
                )
                self.assertEqual(
                    paths,
                    ["experiments/okvis_kitti_raw_0009_matrix.json"],
                )
                with self.assertRaises(FileNotFoundError):
                    self.multimodal_module.manifest_paths(
                        ["okvis"],
                        ["kitti_raw_0061_200"],
                    )


class SyntheticVisualObservationGeneratorTests(unittest.TestCase):
    def test_generator_reproduces_committed_mcd_multimodal_fixture(self) -> None:
        sequence_dir = REPO_ROOT / "evaluation" / "fixtures" / "mcd_kth_smoke"
        gt_csv = REPO_ROOT / "evaluation" / "fixtures" / "mcd_kth_smoke_gt.csv"
        expected_landmarks = sequence_dir / "landmarks.csv"
        expected_observations = sequence_dir / "visual_observations.csv"

        with tempfile.TemporaryDirectory() as tmpdir:
            output_dir = Path(tmpdir)
            completed = run_script(
                "evaluation/scripts/generate_synthetic_visual_observations.py",
                "--sequence-dir",
                str(sequence_dir),
                "--gt-csv",
                str(gt_csv),
                "--output-dir",
                str(output_dir),
                "--start-frame",
                "0",
                "--max-frames",
                "3",
                "--anchor-stride",
                "2",
                "--depths",
                "10,16",
                "--laterals=-3,0,3",
                "--heights",
                "1.5",
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"generator failed unexpectedly:\n{completed.stdout}",
            )
            self.assertIn("[done] wrote 12 landmarks", completed.stdout)
            self.assertIn("[done] wrote 36 observations", completed.stdout)
            self.assertIn("frames=3", completed.stdout)

            self.assertEqual(
                (output_dir / "landmarks.csv").read_text(),
                expected_landmarks.read_text(),
            )
            self.assertEqual(
                (output_dir / "visual_observations.csv").read_text(),
                expected_observations.read_text(),
            )


class PrepareKittiOdometryInputsTests(unittest.TestCase):
    def write_bin_frame(self, path: Path, points: list[tuple[float, float, float, float]]) -> None:
        with path.open("wb") as handle:
            for point in points:
                handle.write(struct.pack("ffff", *point))

    def test_prepare_kitti_odometry_inputs_exports_window_and_full(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            kitti_root = tmp / "kitti"
            velodyne_dir = kitti_root / "sequences" / "00" / "velodyne"
            poses_dir = kitti_root / "poses"
            output_root = tmp / "dogfooding_results"
            reference_data_dir = tmp / "reference_data"
            velodyne_dir.mkdir(parents=True)
            poses_dir.mkdir(parents=True)

            for idx in range(3):
                self.write_bin_frame(
                    velodyne_dir / f"{idx:06d}.bin",
                    [
                        (1.0 + idx, 0.0, 0.0, 0.5),
                        (0.0, 1.0 + idx, 0.0, 0.6),
                    ],
                )

            (poses_dir / "00.txt").write_text(
                "\n".join(
                    [
                        "1 0 0 0 0 1 0 0 0 0 1 0",
                        "1 0 0 1 0 1 0 0 0 0 1 0",
                        "1 0 0 2 0 1 0 0 0 0 1 0",
                    ]
                )
                + "\n"
            )
            (kitti_root / "sequences" / "00" / "calib.txt").write_text(
                "Tr: 1 0 0 0 0 1 0 0 0 0 1 0\n"
            )

            completed = run_script(
                "evaluation/scripts/prepare_kitti_odometry_inputs.py",
                "--kitti-root",
                str(kitti_root),
                "--sequence",
                "00",
                "--window-size",
                "2",
                "--include-full",
                "--output-root",
                str(output_root),
                "--reference-data-dir",
                str(reference_data_dir),
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"prepare script failed unexpectedly:\n{completed.stdout}",
            )
            self.assertIn("kitti_seq_00_2: 2 PCD frames, 2 GT poses", completed.stdout)
            self.assertIn("kitti_seq_00_full: 3 PCD frames, 3 GT poses", completed.stdout)

            window_dir = output_root / "kitti_seq_00_2"
            full_dir = output_root / "kitti_seq_00_full"
            self.assertTrue((window_dir / "00000000" / "cloud.pcd").exists())
            self.assertTrue((window_dir / "00000001" / "cloud.pcd").exists())
            self.assertTrue((full_dir / "00000002" / "cloud.pcd").exists())
            self.assertTrue((window_dir / "frame_timestamps.csv").exists())
            self.assertTrue((full_dir / "frame_timestamps.csv").exists())

            self.assertEqual(
                len((reference_data_dir / "kitti_seq_00_2_gt.csv").read_text().splitlines()),
                3,
            )
            self.assertEqual(
                len((reference_data_dir / "kitti_seq_00_full_gt.csv").read_text().splitlines()),
                4,
            )


class PrepareMulranInputsTests(unittest.TestCase):
    def write_bin_frame(self, path: Path, points: list[tuple[float, float, float, float]]) -> None:
        with path.open("wb") as handle:
            for point in points:
                handle.write(struct.pack("ffff", *point))

    def test_prepare_mulran_inputs_exports_window_and_full(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            seq = tmp / "DCC01"
            ouster = seq / "Ouster"
            ouster.mkdir(parents=True)

            stamps = [
                "1000000000000000000",
                "2000000000000000000",
                "3000000000000000000",
            ]
            for stem in stamps:
                self.write_bin_frame(
                    ouster / f"{stem}.bin",
                    [
                        (1.0, 0.0, 0.0, 0.5),
                        (0.0, 1.0, 0.0, 0.6),
                    ],
                )

            (seq / "global_pose.csv").write_text(
                "timestamp,x,y,z,qx,qy,qz,qw\n"
                "1000000000000000000,0,0,0,0,0,0,1\n"
                "2000000000000000000,1,0,0,0,0,0,1\n"
                "3000000000000000000,2,0,0,0,0,0,1\n"
            )

            output_root = tmp / "dogfooding_results"
            reference_data_dir = tmp / "reference_data"

            completed = run_script(
                "evaluation/scripts/prepare_mulran_inputs.py",
                "--sequence-root",
                str(seq),
                "--max-frames",
                "2",
                "--include-full",
                "--output-root",
                str(output_root),
                "--reference-data-dir",
                str(reference_data_dir),
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"prepare_mulran failed unexpectedly:\n{completed.stdout}",
            )
            self.assertIn("mulran_dcc01_2: 2 PCD frames, 2 GT poses", completed.stdout)
            self.assertIn("mulran_dcc01_full: 3 PCD frames, 3 GT poses", completed.stdout)

            win_dir = output_root / "mulran_dcc01_2"
            full_dir = output_root / "mulran_dcc01_full"
            self.assertTrue((win_dir / "00000000" / "cloud.pcd").exists())
            self.assertTrue((win_dir / "00000001" / "cloud.pcd").exists())
            self.assertTrue((full_dir / "00000002" / "cloud.pcd").exists())
            self.assertTrue((win_dir / "frame_timestamps.csv").exists())

            gt_win = reference_data_dir / "mulran_dcc01_2_gt.csv"
            self.assertEqual(len(gt_win.read_text().splitlines()), 3)
            gt_full = reference_data_dir / "mulran_dcc01_full_gt.csv"
            self.assertEqual(len(gt_full.read_text().splitlines()), 4)


MINI_PCD_ASCII = """# .PCD v0.7 - Point Cloud Data file format
VERSION 0.7
FIELDS x y z
SIZE 4 4 4
TYPE F F F
COUNT 1 1 1
WIDTH 1
HEIGHT 1
VIEWPOINT 0 0 0 1 0 0 0
POINTS 1
DATA ascii
0 0 0
"""


class PrepareNewerCollegeInputsTests(unittest.TestCase):
    def test_prepare_newer_college_inputs_exports_window_and_full(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            seq = tmp / "01_short_experiment"
            lidar = seq / "os1_cloud_node" / "points"
            lidar.mkdir(parents=True)

            for stem in ("1000", "2000", "3000"):
                (lidar / f"{stem}.pcd").write_text(MINI_PCD_ASCII)

            (seq / "ground_truth.csv").write_text(
                "sec,nsec,x,y,z,qx,qy,qz,qw\n"
                "0,1000,0,0,0,0,0,0,1\n"
                "0,2000,1,0,0,0,0,0,1\n"
                "0,3000,2,0,0,0,0,0,1\n"
            )

            output_root = tmp / "dogfooding_results"
            reference_data_dir = tmp / "reference_data"

            completed = run_script(
                "evaluation/scripts/prepare_newer_college_inputs.py",
                "--sequence-dir",
                str(seq),
                "--max-frames",
                "2",
                "--include-full",
                "--output-root",
                str(output_root),
                "--reference-data-dir",
                str(reference_data_dir),
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=f"prepare_newer_college failed unexpectedly:\n{completed.stdout}",
            )
            self.assertIn(
                "newer_college_01_short_experiment_2: 2 PCD frames, 2 GT poses",
                completed.stdout,
            )
            self.assertIn(
                "newer_college_01_short_experiment_full: 3 PCD frames, 3 GT poses",
                completed.stdout,
            )

            win_dir = output_root / "newer_college_01_short_experiment_2"
            full_dir = output_root / "newer_college_01_short_experiment_full"
            self.assertTrue((win_dir / "00000000" / "cloud.pcd").exists())
            self.assertTrue((win_dir / "00000001" / "cloud.pcd").exists())
            self.assertTrue((full_dir / "00000002" / "cloud.pcd").exists())
            self.assertTrue((win_dir / "frame_timestamps.csv").exists())


if __name__ == "__main__":
    unittest.main()
