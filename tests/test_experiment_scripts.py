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
            self.assertEqual(aggregate["variants"][0]["status"], "TIMED_OUT")
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
