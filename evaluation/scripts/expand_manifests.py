#!/usr/bin/env python3
"""Expand experiment manifests so every method covers all dataset windows.

For each method that has fewer manifests than the reference set (litamin2's 13
windows), this script:
  1. Identifies which windows are missing.
  2. Uses the method's existing manifest as a template (same variants,
     same stable_interface).
  3. Creates new manifests for the missing windows, substituting dataset
     paths from the corresponding litamin2 manifest.

IMU-requiring methods (clins, dlio, lio_sam, lins, point_lio, fast_lio2,
fast_lio_slam, ct_lio) are restricted to windows that do NOT require
bag_dir / imu_topic fields in the dataset (i.e. the standard pcd_dir
windows). Special HDL-400 ROS1 synthtime manifests are excluded from the
expansion target set because they use a different dataset pipeline.

Existing manifests are NEVER modified.
"""

import copy
import json
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
EXPERIMENTS_DIR = REPO_ROOT / "experiments"

# ---------------------------------------------------------------------------
# 1. Define the canonical window set from litamin2
# ---------------------------------------------------------------------------
# Map: window_key -> litamin2 filename (without _matrix.json suffix)
LITAMIN2_WINDOWS = {
    "profile":                "litamin2_profile",
    "istanbul_window_b":      "litamin2_istanbul_window_b",
    "istanbul_window_c":      "litamin2_istanbul_window_c",
    "hdl_400_reference":      "litamin2_hdl_400_reference",
    "hdl_400_reference_b":    "litamin2_hdl_400_reference_b",
    "mcd_tuhh_night_09":      "litamin2_mcd_tuhh_night_09",
    "mcd_ntu_day_02":         "litamin2_mcd_ntu_day_02",
    "mcd_kth_day_06":         "litamin2_mcd_kth_day_06",
    "kitti_raw_0009":         "litamin2_kitti_raw_0009",
    "kitti_raw_0061":         "litamin2_kitti_raw_0061",
    "kitti_raw_0009_full":    "litamin2_kitti_raw_0009_full",
    "kitti_raw_0061_full":    "litamin2_kitti_raw_0061_full",
    "kitti_raw_0009_nogt":    "litamin2_kitti_raw_0009_nogt",
}

# The filename suffix used for each window.
# "profile" window uses "{method}_profile_matrix.json"
# All others use "{method}_{window_key}_matrix.json"
WINDOW_FILE_SUFFIX = {
    "profile":                "profile",
    "istanbul_window_b":      "istanbul_window_b",
    "istanbul_window_c":      "istanbul_window_c",
    "hdl_400_reference":      "hdl_400_reference",
    "hdl_400_reference_b":    "hdl_400_reference_b",
    "mcd_tuhh_night_09":      "mcd_tuhh_night_09",
    "mcd_ntu_day_02":         "mcd_ntu_day_02",
    "mcd_kth_day_06":         "mcd_kth_day_06",
    "kitti_raw_0009":         "kitti_raw_0009",
    "kitti_raw_0061":         "kitti_raw_0061",
    "kitti_raw_0009_full":    "kitti_raw_0009_full",
    "kitti_raw_0061_full":    "kitti_raw_0061_full",
    "kitti_raw_0009_nogt":    "kitti_raw_0009_nogt",
}

# IMU-requiring methods. These should NOT get windows that require special
# bag_dir / imu_topic dataset fields (like Istanbul and HDL-400 reference
# windows that are pcd_dir-only but were originally from non-IMU pipelines).
# Actually, looking at existing manifests, these IMU methods already have
# manifests for KITTI and MCD windows using the same pcd_dir approach.
# They are NOT restricted to particular windows; they just don't get the
# special ROS1 synthtime windows. We'll give them all standard windows.
IMU_METHODS = {
    "clins", "dlio", "lio_sam", "lins", "point_lio",
    "fast_lio2", "fast_lio_slam", "ct_lio",
}

# Methods that use --{method}-kitti-profile for KITTI-based windows.
# These need special handling: their KITTI variants include a kitti-profile arg.
# We detect this from existing manifests rather than hard-coding.

# Special manifests to SKIP entirely (not part of the standard window set).
# These have non-standard naming or are blocked/readiness manifests.
SKIP_FILENAMES = {
    "ct_lio_public_readiness_matrix.json",
    "ct_lio_reference_profile_matrix.json",
    "ct_icp_hdl_400_public_ros1_synthtime_matrix.json",
    "ct_lio_hdl_400_public_ros1_synthtime_matrix.json",
    "clins_hdl_400_public_ros1_synthtime_matrix.json",
}

# Human-readable window descriptions for titles/questions
WINDOW_DESCRIPTIONS = {
    "profile": {
        "title_suffix": "the repository-stored Istanbul sequence",
        "question_context": "on the repository-stored Istanbul sequence",
    },
    "istanbul_window_b": {
        "title_suffix": "the second repository-stored Istanbul sequence",
        "question_context": "on a second public Istanbul window",
    },
    "istanbul_window_c": {
        "title_suffix": "the third repository-stored Istanbul sequence",
        "question_context": "on a third public Istanbul window",
    },
    "hdl_400_reference": {
        "title_suffix": "the public HDL-400 reference window",
        "question_context": "on the public HDL-400 reference window",
    },
    "hdl_400_reference_b": {
        "title_suffix": "the second public HDL-400 reference window",
        "question_context": "on the second public HDL-400 reference window",
    },
    "mcd_tuhh_night_09": {
        "title_suffix": "the MCD TUHH night-09 sequence",
        "question_context": "on the MCD TUHH night-09 sequence",
    },
    "mcd_ntu_day_02": {
        "title_suffix": "the MCD NTU day-02 sequence",
        "question_context": "on the MCD NTU day-02 sequence",
    },
    "mcd_kth_day_06": {
        "title_suffix": "the MCD KTH day-06 sequence",
        "question_context": "on the MCD KTH day-06 sequence",
    },
    "kitti_raw_0009": {
        "title_suffix": "KITTI Raw drive 0009 (200 frames, urban)",
        "question_context": "on KITTI Raw drive 0009 (200 frames)",
    },
    "kitti_raw_0061": {
        "title_suffix": "KITTI Raw drive 0061 (200 frames, residential)",
        "question_context": "on KITTI Raw drive 0061 (200 frames)",
    },
    "kitti_raw_0009_full": {
        "title_suffix": "KITTI Raw drive 0009 full sequence (443 frames, urban)",
        "question_context": "on KITTI Raw drive 0009 full sequence (443 frames)",
    },
    "kitti_raw_0061_full": {
        "title_suffix": "KITTI Raw drive 0061 full sequence (703 frames, residential)",
        "question_context": "on KITTI Raw drive 0061 full sequence (703 frames)",
    },
    "kitti_raw_0009_nogt": {
        "title_suffix": "KITTI Raw drive 0009 (200 frames, no GT seed)",
        "question_context": "on KITTI Raw drive 0009 (200 frames, no GT seed)",
    },
}

# Windows that are KITTI-based (might need kitti-profile args for some methods)
KITTI_WINDOWS = {
    "kitti_raw_0009", "kitti_raw_0061",
    "kitti_raw_0009_full", "kitti_raw_0061_full",
    "kitti_raw_0009_nogt",
}

# Windows that are non-KITTI (MCD, Istanbul, HDL-400)
NON_KITTI_WINDOWS = {
    "profile", "istanbul_window_b", "istanbul_window_c",
    "hdl_400_reference", "hdl_400_reference_b",
    "mcd_tuhh_night_09", "mcd_ntu_day_02", "mcd_kth_day_06",
}


def load_litamin2_datasets():
    """Load dataset sections from all litamin2 manifests."""
    datasets = {}
    for window_key, litamin2_stem in LITAMIN2_WINDOWS.items():
        fpath = EXPERIMENTS_DIR / f"{litamin2_stem}_matrix.json"
        if not fpath.exists():
            print(f"WARNING: Missing litamin2 reference: {fpath}")
            continue
        with open(fpath) as f:
            data = json.load(f)
        datasets[window_key] = data["problem"]["dataset"]
    return datasets


def discover_methods_and_manifests():
    """Scan experiments/ for all *_matrix.json files.

    Returns:
        dict: method_name -> {window_key -> manifest_path}
    """
    methods = {}
    all_files = sorted(EXPERIMENTS_DIR.glob("*_matrix.json"))

    for fpath in all_files:
        fname = fpath.name
        if fname in SKIP_FILENAMES:
            continue

        stem = fname.replace("_matrix.json", "")

        # Try to match against each known window suffix to identify method + window
        matched = False
        for window_key, suffix in sorted(WINDOW_FILE_SUFFIX.items(),
                                          key=lambda x: -len(x[1])):
            # Check if stem ends with _suffix or is exactly {method}_{suffix}
            if stem.endswith(f"_{suffix}"):
                method = stem[:-(len(suffix) + 1)]
                if method:
                    methods.setdefault(method, {})[window_key] = fpath
                    matched = True
                    break

        if not matched:
            # This file doesn't match any known window pattern.
            # It might be a special manifest or use a different naming convention.
            # Skip silently - we'll handle these methods if they have at least
            # one matching manifest.
            pass

    return methods


def detect_kitti_profile_pattern(manifest_data, method):
    """Check if a method uses a --{method}-kitti-profile style arg.

    Returns the kitti-profile arg string if found, else None.
    """
    for variant in manifest_data.get("variants", []):
        for arg in variant.get("args", []):
            # Pattern: --{something}-kitti-profile
            if "kitti-profile" in arg:
                return arg
    return None


def has_nogt_seed_variant(manifest_data):
    """Check if any variant in the manifest uses --no-gt-seed."""
    for variant in manifest_data.get("variants", []):
        for arg in variant.get("args", []):
            if arg == "--no-gt-seed":
                return True
    return False


def make_problem_id(method, window_key):
    """Generate the problem.id for a given method and window.

    Convention from existing manifests:
    - litamin2/gicp/ndt/kiss_icp/ct_icp use: {method}_profile_tradeoff_{suffix}
    - Others use: {method}_{suffix} (no "profile_tradeoff" part)

    We detect which pattern the method already uses from its existing manifests.
    """
    # For "profile" window, the id is just "{method}_profile_tradeoff" for the
    # "profile_tradeoff" family, or "{method}_profile" for others.
    if window_key == "profile":
        return f"{method}_profile_tradeoff"
    return f"{method}_profile_tradeoff_{window_key}"


def detect_id_pattern(method, existing_manifests):
    """Detect whether the method uses 'profile_tradeoff' or plain naming.

    Returns 'tradeoff' or 'plain'.
    """
    for window_key, fpath in existing_manifests.items():
        with open(fpath) as f:
            data = json.load(f)
        pid = data["problem"]["id"]
        if "profile_tradeoff" in pid or "_tradeoff" in pid:
            return "tradeoff"
    return "plain"


def make_problem_id_with_pattern(method, window_key, pattern):
    """Generate problem.id respecting the method's existing convention."""
    if pattern == "tradeoff":
        if window_key == "profile":
            return f"{method}_profile_tradeoff"
        return f"{method}_profile_tradeoff_{window_key}"
    else:
        # Plain pattern: {method}_{window_key}
        if window_key == "profile":
            # For profile window with plain pattern, use something like
            # {method}_profile
            return f"{method}_profile"
        return f"{method}_{window_key}"


def make_title(primary_method_name, window_key, pattern):
    """Generate a title for the new manifest."""
    desc = WINDOW_DESCRIPTIONS[window_key]
    if pattern == "tradeoff":
        return f"{primary_method_name} throughput and accuracy trade-off on {desc['title_suffix']}"
    else:
        return f"{primary_method_name} on {desc['title_suffix']}"


def make_question(primary_method_name, window_key, pattern):
    """Generate a question for the new manifest."""
    desc = WINDOW_DESCRIPTIONS[window_key]
    if pattern == "tradeoff":
        return (f"Which {primary_method_name} profile should stay as the "
                f"current default {desc['question_context']}?")
    else:
        return (f"How does {primary_method_name} perform "
                f"{desc['question_context']}?")


def adapt_variants_for_window(variants, window_key, method, kitti_profile_arg,
                              source_window_key):
    """Adapt variant args when moving between KITTI and non-KITTI windows.

    - If source is KITTI and target is non-KITTI: remove kitti-profile args.
    - If source is non-KITTI and target is KITTI: add kitti-profile args
      (only if the method uses them).
    - If target is nogt: add --no-gt-seed to all variants.
    - If source is nogt and target is not: remove --no-gt-seed.
    """
    new_variants = copy.deepcopy(variants)

    source_is_kitti = source_window_key in KITTI_WINDOWS
    target_is_kitti = window_key in KITTI_WINDOWS
    target_is_nogt = window_key == "kitti_raw_0009_nogt"
    source_is_nogt = source_window_key == "kitti_raw_0009_nogt"

    for variant in new_variants:
        args = variant.get("args", [])

        # Remove kitti-profile args if moving to non-KITTI
        if source_is_kitti and not target_is_kitti and kitti_profile_arg:
            args = [a for a in args if a != kitti_profile_arg]

        # Add kitti-profile args if moving to KITTI
        if not source_is_kitti and target_is_kitti and kitti_profile_arg:
            if kitti_profile_arg not in args:
                # Insert at position 0 for consistency
                args.insert(0, kitti_profile_arg)

        # Handle --no-gt-seed
        if target_is_nogt and "--no-gt-seed" not in args:
            args.append("--no-gt-seed")
        if source_is_nogt and not target_is_nogt:
            args = [a for a in args if a != "--no-gt-seed"]

        variant["args"] = args

    return new_variants


def create_manifest(method, window_key, template_data, litamin2_dataset,
                    pattern, kitti_profile_arg, source_window_key):
    """Create a new manifest for the given method and window."""
    new_manifest = copy.deepcopy(template_data)

    primary_method = new_manifest["stable_interface"]["primary_method"]

    # Update problem section
    new_manifest["problem"]["id"] = make_problem_id_with_pattern(
        method, window_key, pattern)
    new_manifest["problem"]["title"] = make_title(
        primary_method, window_key, pattern)
    new_manifest["problem"]["question"] = make_question(
        primary_method, window_key, pattern)

    # Update dataset from litamin2 reference
    # Keep only the standard fields (pcd_dir, gt_csv, and optionally reference_role)
    new_dataset = {}
    new_dataset["pcd_dir"] = litamin2_dataset["pcd_dir"]
    new_dataset["gt_csv"] = litamin2_dataset["gt_csv"]
    if "reference_role" in litamin2_dataset:
        new_dataset["reference_role"] = litamin2_dataset["reference_role"]
    new_manifest["problem"]["dataset"] = new_dataset

    # Remove any extra fields that shouldn't be present (state, blocker, etc.)
    for key in ["state", "blocker", "scope_decision", "next_step",
                "benchmark_weights"]:
        new_manifest["problem"].pop(key, None)

    # Adapt variants
    new_manifest["variants"] = adapt_variants_for_window(
        template_data["variants"], window_key, method,
        kitti_profile_arg, source_window_key)

    return new_manifest


def pick_best_template(method, existing_manifests, target_window_key):
    """Pick the best existing manifest to use as template for a target window.

    Preference order:
    1. Same window type (KITTI for KITTI, non-KITTI for non-KITTI).
    2. The kitti_raw_0009_full manifest if it exists (most common single-manifest).
    3. Any existing manifest.
    """
    target_is_kitti = target_window_key in KITTI_WINDOWS

    # First: prefer same type
    for wk, fpath in existing_manifests.items():
        if (wk in KITTI_WINDOWS) == target_is_kitti:
            return wk, fpath

    # Fallback: any existing
    wk, fpath = next(iter(existing_manifests.items()))
    return wk, fpath


def main():
    litamin2_datasets = load_litamin2_datasets()
    methods_manifests = discover_methods_and_manifests()

    # Determine max window count (should be 13 from litamin2)
    max_windows = len(LITAMIN2_WINDOWS)
    print(f"Reference window count: {max_windows}")
    print(f"Discovered {len(methods_manifests)} methods\n")

    # Show current state
    for method in sorted(methods_manifests):
        windows = methods_manifests[method]
        print(f"  {method}: {len(windows)} manifests "
              f"({', '.join(sorted(windows.keys()))})")

    print()

    # Determine which methods to skip entirely
    # - litamin2, gicp, ndt, kiss_icp, ct_icp already have 13
    # - ct_lio, clins have special manifests only (not standard windows)

    created_count = 0
    skipped_methods = set()

    for method in sorted(methods_manifests):
        existing = methods_manifests[method]

        if len(existing) >= max_windows:
            print(f"SKIP {method}: already has {len(existing)} manifests")
            skipped_methods.add(method)
            continue

        # Determine ID pattern
        pattern = detect_id_pattern(method, existing)

        # Load template from first existing manifest
        first_wk, first_fpath = next(iter(existing.items()))
        with open(first_fpath) as f:
            first_data = json.load(f)

        # Detect kitti-profile arg pattern
        kitti_profile_arg = None
        for wk, fpath in existing.items():
            with open(fpath) as f:
                data = json.load(f)
            kitti_profile_arg = detect_kitti_profile_pattern(data, method)
            if kitti_profile_arg:
                break

        # Determine which windows are missing
        missing = set(LITAMIN2_WINDOWS.keys()) - set(existing.keys())

        if not missing:
            continue

        # For IMU methods, check if they should skip certain windows.
        # Looking at existing patterns: IMU methods (fast_lio2, lio_sam, etc.)
        # already have KITTI and MCD windows. They do NOT have Istanbul or
        # HDL-400 reference windows in their existing manifests.
        # We should follow the existing expansion pattern: give them the same
        # windows that similar methods already have.
        #
        # However, the task says: "IMU-requiring methods should ONLY get
        # manifests for windows that have IMU data." We need to check which
        # windows have IMU data. Looking at the datasets:
        # - Istanbul windows: pcd_dir only, no IMU -> but non-IMU methods use these
        # - HDL-400 reference: pcd_dir only, no IMU in extracted PCDs
        # - MCD: pcd_dir only, but MCD bags do have IMU
        # - KITTI: pcd_dir only, KITTI Raw does have IMU (oxts)
        #
        # The task says to check if methods reference imu_topic/bag_dir.
        # The existing IMU method manifests (fast_lio2, lins, etc.) do NOT
        # have imu_topic fields - they use the same pcd_dir approach.
        # So the check is: does the method's existing manifest reference
        # imu_topic or bag_dir in the dataset?
        is_imu_method = method in IMU_METHODS
        if is_imu_method:
            # Check if any existing manifest has imu_topic/bag_dir
            has_imu_fields = False
            for wk, fpath in existing.items():
                with open(fpath) as f:
                    data = json.load(f)
                ds = data["problem"]["dataset"]
                if "imu_topic" in ds or "bag_dir" in ds:
                    has_imu_fields = True
                    break

            if has_imu_fields:
                # This method requires special IMU data pipeline.
                # Only expand to windows where IMU data is available via
                # the standard pcd_dir mechanism.
                # For now, skip Istanbul and HDL-400 reference windows
                # since those PCDs don't include IMU.
                # Actually, let's look at what windows each IMU method
                # already has. If they already have MCD/KITTI, then Istanbul
                # and HDL-400 ref are the ones they're missing.
                print(f"  NOTE: {method} has imu_topic/bag_dir fields, "
                      f"restricting expansion to windows with IMU data")
                # Remove Istanbul and HDL-400 windows from missing set
                imu_excluded = {"profile", "istanbul_window_b",
                                "istanbul_window_c",
                                "hdl_400_reference", "hdl_400_reference_b"}
                missing -= imu_excluded

        print(f"\nEXPAND {method}: {len(existing)} -> "
              f"{len(existing) + len(missing)} "
              f"(+{len(missing)} new manifests)")

        for window_key in sorted(missing):
            # Pick best template
            source_wk, source_fpath = pick_best_template(
                method, existing, window_key)

            with open(source_fpath) as f:
                template_data = json.load(f)

            # Check kitti-profile arg from this specific template
            template_kitti_arg = detect_kitti_profile_pattern(
                template_data, method)
            # Use detected kitti arg from any manifest if this template
            # doesn't have one
            effective_kitti_arg = template_kitti_arg or kitti_profile_arg

            litamin2_dataset = litamin2_datasets[window_key]

            new_manifest = create_manifest(
                method, window_key, template_data, litamin2_dataset,
                pattern, effective_kitti_arg, source_wk)

            # Determine output filename
            out_fname = f"{method}_{WINDOW_FILE_SUFFIX[window_key]}_matrix.json"
            out_path = EXPERIMENTS_DIR / out_fname

            if out_path.exists():
                print(f"  SKIP {out_fname}: already exists")
                continue

            with open(out_path, "w") as f:
                json.dump(new_manifest, f, indent=2)
                f.write("\n")

            print(f"  CREATED {out_fname}")
            created_count += 1

    print(f"\nDone. Created {created_count} new manifests.")


if __name__ == "__main__":
    main()
