#!/usr/bin/env python3
"""Generate publication-planning docs from experiment aggregates."""

from __future__ import annotations

import json
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from collections import Counter
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[2]
RESULTS_DIR = REPO_ROOT / "experiments" / "results"
DOCS_DIR = REPO_ROOT / "docs"


@dataclass
class ProblemSummary:
    problem_id: str
    title: str
    status: str
    blocker: str
    next_step: str
    scope_decision: str
    manifest_path: str
    aggregate_path: str
    current_default: str | None
    selector: str
    dataset_pcd_dir: str
    dataset_gt_csv: str
    best_ate_m: float | None
    best_fps: float | None
    default_ate_m: float | None
    default_fps: float | None


def relpath(path: Path | str) -> str:
    candidate = Path(path)
    try:
        return str(candidate.relative_to(REPO_ROOT))
    except ValueError:
        return str(candidate)


def render_metric(value: float | None, digits: int = 3) -> str:
    if value is None:
        return "-"
    return f"{value:.{digits}f}"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def summarize_problem(entry: dict[str, Any]) -> ProblemSummary:
    aggregate_path = REPO_ROOT / entry["aggregate_path"]
    aggregate = load_json(aggregate_path)
    problem = aggregate["problem"]
    stable_interface = aggregate["stable_interface"]
    dataset = aggregate.get("dataset", {})
    variants = aggregate.get("variants", [])

    best_ate = None
    best_fps = None
    default_ate = None
    default_fps = None

    for variant in variants:
        if variant.get("status") != "OK":
            continue
        ate = variant.get("ate_m")
        fps = variant.get("fps")
        if ate is not None:
            best_ate = ate if best_ate is None else min(best_ate, ate)
        if fps is not None:
            best_fps = fps if best_fps is None else max(best_fps, fps)
        if entry.get("current_default") == variant.get("id"):
            default_ate = ate
            default_fps = fps

    return ProblemSummary(
        problem_id=entry["problem_id"],
        title=entry["title"],
        status=entry["status"],
        blocker=entry.get("blocker", ""),
        next_step=str(aggregate.get("next_step") or problem.get("next_step", "")),
        scope_decision=str(problem.get("scope_decision", "")),
        manifest_path=entry["manifest_path"],
        aggregate_path=entry["aggregate_path"],
        current_default=entry.get("current_default"),
        selector=stable_interface["methods"],
        dataset_pcd_dir=str(dataset.get("pcd_dir", "")),
        dataset_gt_csv=str(dataset.get("gt_csv", "")),
        best_ate_m=best_ate,
        best_fps=best_fps,
        default_ate_m=default_ate,
        default_fps=default_fps,
    )


def readiness_label(score: int) -> str:
    if score >= 80:
        return "High"
    if score >= 55:
        return "Medium"
    return "Low"


def is_non_imu_problem(problem: ProblemSummary) -> bool:
    return problem.selector != "ct_lio"


def unique_non_imu_datasets(problems: list[ProblemSummary]) -> set[str]:
    return {problem.dataset_pcd_dir for problem in problems if is_non_imu_problem(problem)}


def dataset_family_key(dataset_path: str) -> str:
    name = Path(dataset_path).name.lower()
    if "istanbul" in name:
        return "autoware_istanbul"
    if "hdl_400" in name:
        return "hdl_400"
    for token in (
        "kitti",
        "mulran",
        "nclt",
        "newer_college",
        "newercollege",
        "oxford",
        "boreas",
    ):
        if token in name:
            return token
    parts = [part for part in name.split("_") if part]
    if len(parts) >= 2:
        return "_".join(parts[:2])
    return name


def unique_non_imu_dataset_families(problems: list[ProblemSummary]) -> set[str]:
    return {
        dataset_family_key(problem.dataset_pcd_dir)
        for problem in problems
        if is_non_imu_problem(problem)
    }


def non_imu_dataset_family_counts(problems: list[ProblemSummary]) -> dict[str, int]:
    counter = Counter(
        dataset_family_key(problem.dataset_pcd_dir)
        for problem in problems
        if is_non_imu_problem(problem)
    )
    return dict(counter)


def compute_track_scores(problems: list[ProblemSummary]) -> dict[str, int]:
    ready = [problem for problem in problems if problem.status == "ready"]
    nonready = [problem for problem in problems if problem.status != "ready"]
    selectors = {problem.selector for problem in ready}

    empirical = 25
    empirical += min(len(ready) * 6, 30)
    empirical += 10 if len(selectors) >= 5 else 0
    empirical += 10 if any(problem.selector == "ct_lio" for problem in ready) else 0
    empirical -= min(len(nonready) * 10, 20)
    empirical = max(0, min(empirical, 100))

    artifact = 45
    artifact += min(len(ready) * 4, 24)
    artifact += 10 if nonready else 0
    artifact += 8 if any(problem.current_default for problem in ready) else 0
    artifact = max(0, min(artifact, 100))

    method = 15
    method += 20 if any(problem.selector == "ct_lio" for problem in ready) else 0
    method += 10 if len({problem.dataset_pcd_dir for problem in ready}) >= 2 else 0
    method -= 15 if nonready else 0
    method -= 10 if len(ready) > 4 else 0
    method = max(0, min(method, 100))

    return {
        "empirical": empirical,
        "artifact": artifact,
        "method": method,
    }


def gt_lio_is_scoped_out(problems: list[ProblemSummary]) -> bool:
    for problem in problems:
        if problem.problem_id != "ct_lio_public_gt_readiness":
            continue
        decision_text = f"{problem.next_step} {problem.scope_decision}".lower()
        return (
            "scoped out" in decision_text
            or "excluded from the main study" in decision_text
            or "no action planned" in decision_text
        )
    return False


def status_count_text(problems: list[ProblemSummary]) -> str:
    counts = Counter(problem.status for problem in problems)
    order = ["ready", "blocked", "skipped"]
    parts = [f"`{counts[status]}` {status}" for status in order if counts.get(status)]
    parts.extend(
        f"`{count}` {status}" for status, count in sorted(counts.items()) if status not in order
    )
    return ", ".join(parts)


def render_current_state(problems: list[ProblemSummary]) -> list[str]:
    lines = [
        "## Current State",
        "",
        "| Problem | Status | Default | Best ATE [m] | Best FPS | Dataset |",
        "|---------|--------|---------|--------------|----------|---------|",
    ]
    for problem in problems:
        lines.append(
            f"| {problem.title} | `{problem.status}` | `{problem.current_default or '-'}` | "
            f"{render_metric(problem.best_ate_m)} | {render_metric(problem.best_fps, digits=1)} | "
            f"`{problem.dataset_pcd_dir}` |"
        )
    return lines


def render_track_section(
    *,
    name: str,
    score: int,
    primary_claim: str,
    strongest_evidence: list[str],
    gaps: list[str],
    publish_decision: str,
    next_experiments: list[str],
) -> list[str]:
    lines = [
        f"## {name}",
        "",
        f"- **Readiness**: {readiness_label(score)} ({score}/100)",
        f"- **Primary claim**: {primary_claim}",
        f"- **Decision**: {publish_decision}",
        "",
        "### Strongest Evidence",
        "",
    ]
    lines.extend(f"- {item}" for item in strongest_evidence)
    lines.extend(
        [
            "",
            "### Gaps",
            "",
        ]
    )
    lines.extend(f"- {item}" for item in gaps)
    lines.extend(
        [
            "",
            "### Next Experiments",
            "",
        ]
    )
    lines.extend(f"- {item}" for item in next_experiments)
    return lines


def render_paper_tracks_md(
    problems: list[ProblemSummary], scores: dict[str, int], generated_at: str
) -> str:
    ready = [problem for problem in problems if problem.status == "ready"]
    non_imu_dataset_count = len(unique_non_imu_datasets(ready))
    non_imu_family_count = len(unique_non_imu_dataset_families(ready))
    family_counts = non_imu_dataset_family_counts(ready)
    smallest_family_count = min(family_counts.values()) if family_counts else 0
    ct_lio = next((problem for problem in ready if problem.selector == "ct_lio"), None)
    litamin2 = next((problem for problem in ready if problem.selector == "litamin2"), None)
    gt_lio_scoped_out = gt_lio_is_scoped_out(problems)

    lines = [
        "# Paper Tracks",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/generate_publication_docs.py`._",
        "",
        "This repository should not be pitched as \"many implementations exist here\".",
        "The paper target has to be a claim about what this experiment-driven process reveals.",
        "",
        f"Current coverage: {status_count_text(problems)} problems.",
        "",
    ]
    lines.extend(render_current_state(problems))
    lines.extend(
        [
            "",
            "## Recommendation Order",
            "",
            "1. Empirical study as the primary paper target.",
            "2. Artifact / reproducibility submission in parallel.",
            "3. Focused method paper only after a stronger multi-dataset delta exists.",
            "",
        ]
    )

    empirical_evidence = [
        f"The stable benchmark contract now covers {len(ready)} ready problems under one CLI and one summary JSON interface.",
        "Each active problem keeps at least three concrete variants alive instead of collapsing immediately to a single abstraction.",
        f"Current defaults already show non-trivial trade-offs, such as `LiTAMIN2={litamin2.current_default}` at {render_metric(litamin2.default_fps, digits=1)} FPS and `CT-LIO={ct_lio.current_default}` at {render_metric(ct_lio.default_ate_m)} m ATE on the public HDL-400 reference window."
        if litamin2 and ct_lio
        else "Current defaults already show non-trivial trade-offs across LiDAR-only and LiDAR+IMU methods.",
    ]
    empirical_gaps = [
        (
            "Most methods are still demonstrated on a single repository-stored Istanbul sequence, so external validity is weak."
            if non_imu_dataset_count <= 1
            else (
                (
                    f"LiDAR-only methods now cover {non_imu_dataset_count} repository-stored open sequences, but they still come from one dataset family."
                    if non_imu_family_count <= 1
                    else (
                        f"LiDAR-only methods now cover {non_imu_dataset_count} repository-stored open sequences across {non_imu_family_count} public dataset families, but the newer family is still represented by only one short window."
                        if smallest_family_count <= 1
                        else f"LiDAR-only methods now cover {non_imu_dataset_count} repository-stored open sequences across {non_imu_family_count} public dataset families, but external validity still rests on only {non_imu_family_count} families."
                    )
                )
            )
        ),
        (
            "Reference-based and GT-backed results are separated, and the GT-backed CT-LIO public benchmark is explicitly scoped out of the main study until independent GT appears."
            if gt_lio_scoped_out
            else "Reference-based and GT-backed results are now separated conceptually, but the GT-backed CT-LIO public benchmark is still blocked."
        ),
        "There is no paper-ready comparison against originally reported results yet.",
        "Hardware-normalized reruns and confidence intervals are not exported yet.",
    ]
    empirical_next = [
        (
            "Add at least two more open sequences that every LiDAR-only method can run on."
            if non_imu_dataset_count <= 1
            else (
                "Add at least one more open sequence so the LiDAR-only study spans three distinct public windows."
                if non_imu_dataset_count == 2
                else (
                    "Add at least one LiDAR-only sequence from a different public dataset family."
                    if non_imu_family_count <= 1
                    else (
                        "Deepen the newer dataset family with at least one more LiDAR-only sequence, or add a third family, so the study is not resting on one reference window."
                        if smallest_family_count <= 1
                        else "Add a third public dataset family, or expand the current two-family evidence with longer and less curated windows."
                    )
                )
            )
        ),
        (
            "Keep GT-backed CT-LIO out of the main evidence tables and revisit only if independent HDL-400 GT becomes available."
            if gt_lio_scoped_out
            else "Unblock or explicitly exclude GT-backed CT-LIO from the main study before writing."
        ),
        "Generate a method-by-method table comparing repository defaults, challengers, and original-paper numbers.",
        "Export paper-ready Pareto figures from `experiments/results/*.json`.",
    ]
    lines.extend(
        render_track_section(
            name="Track A: Empirical Study",
            score=scores["empirical"],
            primary_claim="Variant-first localization benchmarking reveals Pareto fronts that are hidden when repositories force one canonical implementation too early.",
            strongest_evidence=empirical_evidence,
            gaps=empirical_gaps,
            publish_decision="Primary target. This is the most defensible full-paper narrative for the current repository.",
            next_experiments=empirical_next,
        )
    )
    lines.append("")

    artifact_evidence = [
        "The repository already separates stable core (`pcd_dogfooding --summary-json`) from discardable experimental manifests.",
        "Comparison state is externalized into generated docs instead of being trapped in code comments or ad-hoc notebooks.",
        "The runner now supports `--reuse-existing`, which makes expensive comparisons reproducible without rerunning every variant.",
        "Experiment-facing and publication-facing docs can now be refreshed in one command via `python3 evaluation/scripts/refresh_study_docs.py`.",
        "Paper-ready tables and Pareto figures are now exported from aggregate JSON via `python3 evaluation/scripts/export_paper_assets.py`.",
    ]
    artifact_gaps = [
        "Container or pinned environment instructions are still missing.",
        "The public Pages site does not yet expose the experiment docs alongside the benchmark snapshot.",
        "Dataset bootstrap is still manual, so full artifact replay is not yet one-command from a clean machine.",
    ]
    artifact_next = [
        "Ship a pinned environment definition or container recipe.",
        "Publish `docs/paper_tracks.md` and `docs/paper_roadmap.md` through Pages so the artifact narrative is visible externally.",
        "Add dataset bootstrap helpers so a clean machine can reproduce the study without manual path setup.",
    ]
    lines.extend(
        render_track_section(
            name="Track B: Artifact / Reproducibility",
            score=scores["artifact"],
            primary_claim="Localization research tooling is more reproducible when the benchmark contract is stable and variant search remains explicit, comparable, and discardable.",
            strongest_evidence=artifact_evidence,
            gaps=artifact_gaps,
            publish_decision="Parallel target. This is the fastest submission path if the goal is near-term publication evidence.",
            next_experiments=artifact_next,
        )
    )
    lines.append("")

    method_evidence = [
        "CT-LIO now has a public reference-based trade-off problem with three bounded variants under one interface.",
        "LiTAMIN2 already has a measurable throughput-oriented variant family and a paper-profile family under the same evaluator.",
        "The repository can now keep weaker and stronger method variants alive without deleting the evidence trail.",
    ]
    method_gaps = [
        "No single method currently shows a strong enough multi-dataset improvement claim for a focused algorithm paper.",
        "Current CT-LIO evidence is reference-based and slow; it is not yet enough for a strong method contribution.",
        "LiTAMIN2 speedups are real inside this repository, but they are not yet benchmarked broadly enough to support a standalone claim.",
    ]
    method_next = [
        "Pick one method only after it shows repeatable improvement on at least two open sequences.",
        "Add ablations that isolate exactly one new algorithmic idea instead of a profile bundle.",
        "Treat the focused-method path as a by-product of Track A, not the current main plan.",
    ]
    lines.extend(
        render_track_section(
            name="Track C: Focused Method Paper",
            score=scores["method"],
            primary_claim="One concrete localization method variant materially improves the accuracy / throughput frontier beyond the current repository baselines.",
            strongest_evidence=method_evidence,
            gaps=method_gaps,
            publish_decision="Hold. Keep collecting evidence, but do not make this the main paper narrative yet.",
            next_experiments=method_next,
        )
    )
    return "\n".join(lines)


def render_paper_roadmap_md(
    problems: list[ProblemSummary], scores: dict[str, int], generated_at: str
) -> str:
    ready = [problem for problem in problems if problem.status == "ready"]
    nonready = [problem for problem in problems if problem.status != "ready"]
    non_imu_dataset_count = len(unique_non_imu_datasets(ready))
    non_imu_family_count = len(unique_non_imu_dataset_families(ready))
    family_counts = non_imu_dataset_family_counts(ready)
    smallest_family_count = min(family_counts.values()) if family_counts else 0
    gt_lio_scoped_out = gt_lio_is_scoped_out(problems)
    expand_public_evidence = (
        "- Add at least two additional open sequences for LiDAR-only methods."
        if non_imu_dataset_count <= 1
        else (
            "- Add at least one more open sequence for LiDAR-only methods so the study spans three public windows."
            if non_imu_dataset_count == 2
            else (
                "- Add at least one LiDAR-only sequence from a different public dataset family."
                if non_imu_family_count <= 1
                else (
                    "- Deepen the newer public dataset family with another LiDAR-only sequence, or add a third family."
                    if smallest_family_count <= 1
                    else "- Add a third public dataset family, or broaden the current two-family evidence with longer windows."
                )
            )
        )
    )
    gt_lio_phase2_task = (
        "- Keep the scoped-out GT-backed CT-LIO item in appendix/artifact docs; revisit only if independent GT appears."
        if gt_lio_scoped_out
        else "- Resolve the blocked GT-backed CT-LIO item or explicitly scope it out of the main paper."
    )
    immediate_public_task = (
        "| P0 | Add another public LiDAR-only sequence to every ready non-IMU problem. | One-sequence evidence is too thin for a paper claim. |"
        if non_imu_dataset_count <= 1
        else (
            "| P0 | Add a third public LiDAR-only sequence to every ready non-IMU problem. | Two windows are better than one, but still thin for a paper claim. |"
            if non_imu_dataset_count == 2
            else (
                "| P0 | Add a LiDAR-only sequence from a different public dataset family. | Three windows from one family are useful, but still weak on external validity. |"
                if non_imu_family_count <= 1
                else (
                    "| P0 | Add another LiDAR-only sequence from the HDL-400 family or add a third dataset family. | The study now spans multiple families, but one of them is still represented by a single short reference window. |"
                    if smallest_family_count <= 1
                    else "| P0 | Add a third public dataset family or broader windows from the current families. | The study now spans two families, but that is still a narrow basis for a paper claim. |"
                )
            )
        )
    )
    gt_lio_immediate_task = (
        "| P1 | Keep GT-backed CT-LIO scoped out of main evidence tables. | The status is decided; future refreshes should not mix blocked evidence with accepted results. |"
        if gt_lio_scoped_out
        else "| P0 | Decide the status of GT-backed CT-LIO. | The paper must separate blocked and accepted evidence cleanly. |"
    )
    lines = [
        "# Paper Roadmap",
        "",
        f"_Generated at {generated_at} by `evaluation/scripts/generate_publication_docs.py`._",
        "",
        "## Chosen Direction",
        "",
        f"- Primary path: `Track A: Empirical Study` ({readiness_label(scores['empirical'])}).",
        f"- Parallel path: `Track B: Artifact / Reproducibility` ({readiness_label(scores['artifact'])}).",
        f"- Deferred path: `Track C: Focused Method Paper` ({readiness_label(scores['method'])}).",
        "",
        "## Phase 1: Lock the Study Contract",
        "",
        f"- Keep the current `{len(ready)}` ready problems reproducible through `python3 evaluation/scripts/run_experiment_matrix.py --reuse-existing`.",
        "- Freeze the meaning of `reference-based` versus `GT-backed` results and keep them in separate tables.",
        "- Do not merge ad-hoc benchmark scripts outside the stable `pcd_dogfooding --summary-json` contract.",
        "",
        "## Phase 2: Expand Public Evidence",
        "",
        expand_public_evidence,
        gt_lio_phase2_task,
        "- Re-run every ready problem on the same hardware profile and archive the outputs under `experiments/results/`.",
        "",
        "## Phase 3: Paper Assets",
        "",
        "- Export a paper-ready table: default variant, active challengers, ATE, FPS, and contract type.",
        "- Export Pareto plots from aggregate JSON instead of manually screenshotting benchmark runs.",
        "- Add an original-paper comparison appendix with one row per implemented method family.",
        "",
        "## Phase 4: Writing Package",
        "",
        "- Draft the empirical paper around one sentence: variant-first benchmarking exposes useful Pareto fronts that single-canonical repos hide.",
        "- Submit the artifact package in parallel using the same generated docs and experiment manifests.",
        "- Keep the focused-method idea as a follow-on only if one method starts winning on multiple open sequences.",
        "",
        "## Immediate Next Tasks",
        "",
        "| Priority | Task | Why it matters |",
        "|----------|------|----------------|",
        immediate_public_task,
        gt_lio_immediate_task,
        "| P1 | Curate manuscript-facing subsets and captions from the generated paper assets. | The exports exist now; the remaining work is turning them into final paper figures and tables. |",
        "| P1 | Add dataset bootstrap helpers and a pinned environment. | The refresh entrypoint exists, but a clean-machine replay path is still missing. |",
        "| P2 | Add original-paper comparison sheets for LiTAMIN2, GICP, NDT, KISS-ICP, CT-ICP, and CT-LIO. | Needed for a stronger empirical framing. |",
        "",
        "## Current Non-Ready Problems",
        "",
    ]
    if not nonready:
        lines.append("- No non-ready problems are currently registered.")
    else:
        for problem in nonready:
            line = f"- `{problem.title}` (`{problem.status}`): {problem.blocker}"
            if problem.next_step:
                line += f" Next step: {problem.next_step}"
            lines.append(line)
    return "\n".join(lines)


def main() -> None:
    index = load_json(RESULTS_DIR / "index.json")
    problems = [summarize_problem(entry) for entry in index["problems"]]
    problems.sort(key=lambda item: item.title.lower())
    scores = compute_track_scores(problems)
    generated_at = datetime.now(timezone.utc).replace(microsecond=0).isoformat()

    DOCS_DIR.mkdir(parents=True, exist_ok=True)
    (DOCS_DIR / "paper_tracks.md").write_text(
        render_paper_tracks_md(problems, scores, generated_at) + "\n"
    )
    (DOCS_DIR / "paper_roadmap.md").write_text(
        render_paper_roadmap_md(problems, scores, generated_at) + "\n"
    )

    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_tracks.md')}")
    print(f"[done] wrote {relpath(DOCS_DIR / 'paper_roadmap.md')}")


if __name__ == "__main__":
    main()
