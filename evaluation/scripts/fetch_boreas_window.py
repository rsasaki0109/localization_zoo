#!/usr/bin/env python3
"""Fetch a deterministic LiDAR window or its GT from the public Boreas S3 bucket."""

from __future__ import annotations

import argparse
import concurrent.futures
import hashlib
import json
import os
import shutil
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Any


S3_BASE = "https://boreas.s3.amazonaws.com"
XML_NAMESPACE = {"s3": "http://s3.amazonaws.com/doc/2006-03-01/"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sequence", required=True)
    parser.add_argument("--output-root", required=True)
    parser.add_argument("--phase", choices=("scans", "ground-truth"), default="scans")
    parser.add_argument("--start", type=int, default=0)
    parser.add_argument("--frames", type=int, default=600)
    parser.add_argument("--workers", type=int, default=4)
    return parser.parse_args()


def list_objects(prefix: str) -> list[dict[str, Any]]:
    objects: list[dict[str, Any]] = []
    token = ""
    while True:
        query = {"list-type": "2", "prefix": prefix}
        if token:
            query["continuation-token"] = token
        with urllib.request.urlopen(f"{S3_BASE}/?{urllib.parse.urlencode(query)}") as response:
            root = ET.fromstring(response.read())
        for item in root.findall("s3:Contents", XML_NAMESPACE):
            key = item.findtext("s3:Key", default="", namespaces=XML_NAMESPACE)
            objects.append(
                {
                    "key": key,
                    "size": int(item.findtext("s3:Size", default="0", namespaces=XML_NAMESPACE)),
                    "etag": item.findtext("s3:ETag", default="", namespaces=XML_NAMESPACE).strip('"'),
                }
            )
        if root.findtext("s3:IsTruncated", default="false", namespaces=XML_NAMESPACE) != "true":
            break
        token = root.findtext("s3:NextContinuationToken", default="", namespaces=XML_NAMESPACE)
    return objects


def download_one(item: dict[str, Any], destination: Path) -> dict[str, Any]:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.is_file() and destination.stat().st_size == item["size"]:
        return {**item, "path": str(destination), "reused": True}
    temporary = destination.with_suffix(destination.suffix + ".part")
    url = f"{S3_BASE}/{urllib.parse.quote(item['key'], safe='/')}"
    with urllib.request.urlopen(url) as source, temporary.open("wb") as sink:
        shutil.copyfileobj(source, sink, length=1024 * 1024)
    if temporary.stat().st_size != item["size"]:
        raise RuntimeError(f"Size mismatch for {item['key']}")
    os.replace(temporary, destination)
    return {**item, "path": str(destination), "reused": False}


def selection_sha256(items: list[dict[str, Any]]) -> str:
    payload = "".join(f"{item['key']}\t{item['size']}\t{item['etag']}\n" for item in items)
    return hashlib.sha256(payload.encode()).hexdigest()


def main() -> int:
    args = parse_args()
    sequence_root = Path(args.output_root).resolve() / args.sequence
    if args.phase == "scans":
        all_items = sorted(
            (
                item
                for item in list_objects(f"{args.sequence}/lidar/")
                if item["key"].endswith(".bin")
            ),
            key=lambda item: item["key"],
        )
        selected = all_items[args.start : args.start + args.frames]
        if len(selected) != args.frames:
            raise RuntimeError(f"Requested {args.frames} scans, found {len(selected)}")
        destinations = [sequence_root / "lidar" / Path(item["key"]).name for item in selected]
    else:
        key = f"{args.sequence}/applanix/lidar_poses.csv"
        matches = [item for item in list_objects(key) if item["key"] == key]
        if len(matches) != 1:
            raise RuntimeError(f"Unable to locate {key}")
        selected = matches
        destinations = [sequence_root / "applanix/lidar_poses.csv"]

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.workers) as executor:
        completed = list(executor.map(download_one, selected, destinations))
    manifest = {
        "schema_version": 1,
        "source": "s3://boreas",
        "sequence": args.sequence,
        "phase": args.phase,
        "selection": {"start": args.start, "frames": args.frames},
        "selection_sha256": selection_sha256(selected),
        "bytes": sum(item["size"] for item in selected),
        "objects": completed,
    }
    manifest_path = sequence_root / f"fetch_{args.phase.replace('-', '_')}_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"manifest": str(manifest_path), "objects": len(selected), "bytes": manifest["bytes"]}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
