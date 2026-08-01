#!/usr/bin/env python3
"""Fetch selected KITTI Odometry Velodyne sequences without downloading the 79 GiB ZIP."""

from __future__ import annotations

import argparse
import csv
import os
import struct
import urllib.request
import zlib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import BinaryIO, Iterable, Sequence
from zipfile import ZIP_STORED, ZipInfo

try:
    from remotezip import RemoteZip
except ImportError:  # pragma: no cover - exercised through the CLI error
    RemoteZip = None


DEFAULT_URL = (
    "https://s3.eu-central-1.amazonaws.com/avg-kitti/"
    "data_odometry_velodyne.zip"
)
LOCAL_FILE_HEADER = struct.Struct("<IHHHHHIIIHH")
LOCAL_FILE_SIGNATURE = 0x04034B50
COPY_CHUNK_BYTES = 8 * 1024 * 1024


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-root",
        required=True,
        help="Extraction root; archive paths such as dataset/sequences/00 are retained.",
    )
    parser.add_argument(
        "--sequence",
        action="append",
        required=True,
        help="Two-digit KITTI sequence ID. Repeatable.",
    )
    parser.add_argument("--url", default=DEFAULT_URL, help="Official KITTI ZIP URL.")
    parser.add_argument(
        "--workers",
        type=int,
        default=4,
        help="Parallel contiguous HTTP Range streams per sequence (default: 4).",
    )
    parser.add_argument(
        "--overwrite",
        action="store_true",
        help="Re-download frames even when their size matches the ZIP metadata.",
    )
    return parser.parse_args()


def normalize_sequences(values: Iterable[str]) -> list[str]:
    sequences: list[str] = []
    for raw in values:
        sequence = raw.zfill(2)
        if len(sequence) != 2 or not sequence.isdigit():
            raise ValueError(f"Invalid KITTI sequence: {raw!r}")
        if sequence not in sequences:
            sequences.append(sequence)
    return sequences


def select_sequence_infos(infos: Iterable[ZipInfo], sequence: str) -> list[ZipInfo]:
    prefix = f"dataset/sequences/{sequence}/velodyne/"
    selected = sorted(
        (
            info
            for info in infos
            if info.filename.startswith(prefix) and info.filename.endswith(".bin")
        ),
        key=lambda info: info.header_offset,
    )
    if not selected:
        raise RuntimeError(f"No Velodyne frames found for KITTI sequence {sequence}")
    if any(info.compress_type != ZIP_STORED for info in selected):
        raise RuntimeError(
            f"KITTI sequence {sequence} is no longer stored uncompressed; "
            "the selective range extractor must be updated."
        )
    return selected


def partition_infos(infos: Sequence[ZipInfo], workers: int) -> list[list[ZipInfo]]:
    if workers < 1:
        raise ValueError("workers must be at least 1")
    chunk_size = max(1, (len(infos) + workers - 1) // workers)
    return [
        list(infos[start : start + chunk_size])
        for start in range(0, len(infos), chunk_size)
    ]


def read_exact(stream: BinaryIO, byte_count: int) -> bytes:
    chunks: list[bytes] = []
    remaining = byte_count
    while remaining:
        chunk = stream.read(min(remaining, COPY_CHUNK_BYTES))
        if not chunk:
            raise EOFError(f"Range response ended with {remaining} bytes still expected")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def discard_exact(stream: BinaryIO, byte_count: int) -> None:
    remaining = byte_count
    while remaining:
        chunk = stream.read(min(remaining, COPY_CHUNK_BYTES))
        if not chunk:
            raise EOFError(f"Range response ended with {remaining} bytes still expected")
        remaining -= len(chunk)


def copy_exact_with_crc(stream: BinaryIO, output: BinaryIO, byte_count: int) -> int:
    remaining = byte_count
    checksum = 0
    while remaining:
        chunk = stream.read(min(remaining, COPY_CHUNK_BYTES))
        if not chunk:
            raise EOFError(f"Range response ended with {remaining} bytes still expected")
        output.write(chunk)
        checksum = zlib.crc32(chunk, checksum)
        remaining -= len(chunk)
    return checksum & 0xFFFFFFFF


def write_index_timestamps(velodyne_dir: Path, frame_count: int) -> None:
    path = velodyne_dir / "frame_timestamps.csv"
    temporary = path.with_suffix(".csv.part")
    with temporary.open("w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["frame", "timestamp"])
        writer.writerows((index, index) for index in range(frame_count))
    os.replace(temporary, path)


def extract_sequence_range(
    url: str,
    infos: Sequence[ZipInfo],
    output_root: Path,
    overwrite: bool = False,
) -> tuple[int, int]:
    needed = [
        info
        for info in infos
        if overwrite
        or not (output_root / info.filename).is_file()
        or (output_root / info.filename).stat().st_size != info.file_size
    ]
    if not needed:
        return 0, len(infos)

    first_offset = needed[0].header_offset
    last = needed[-1]
    estimated_end = last.header_offset + 30 + len(last.filename.encode()) + len(last.extra)
    estimated_end += last.compress_size + 4096
    request = urllib.request.Request(
        url,
        headers={"Range": f"bytes={first_offset}-{estimated_end - 1}"},
    )

    extracted = 0
    skipped = 0
    with urllib.request.urlopen(request, timeout=120) as response:
        status = getattr(response, "status", None)
        if status != 206:
            raise RuntimeError(f"Server ignored HTTP Range request (status={status})")
        absolute_offset = first_offset
        for info in (item for item in infos if item.header_offset >= first_offset):
            gap = info.header_offset - absolute_offset
            if gap < 0:
                raise RuntimeError("Unexpected overlapping ZIP entries")
            discard_exact(response, gap)
            absolute_offset += gap

            header = read_exact(response, LOCAL_FILE_HEADER.size)
            absolute_offset += len(header)
            (
                signature,
                _version,
                _flags,
                compression,
                _time,
                _date,
                _crc,
                _compressed_size,
                _file_size,
                filename_length,
                extra_length,
            ) = LOCAL_FILE_HEADER.unpack(header)
            if signature != LOCAL_FILE_SIGNATURE or compression != ZIP_STORED:
                raise RuntimeError(f"Unexpected local ZIP header for {info.filename}")
            filename = read_exact(response, filename_length)
            discard_exact(response, extra_length)
            absolute_offset += filename_length + extra_length
            if filename.decode("utf-8") != info.filename:
                raise RuntimeError(
                    f"ZIP index mismatch: expected {info.filename}, got {filename!r}"
                )

            destination = output_root / info.filename
            destination.parent.mkdir(parents=True, exist_ok=True)
            should_write = (
                overwrite
                or not destination.is_file()
                or destination.stat().st_size != info.file_size
            )
            if should_write:
                temporary = destination.with_suffix(destination.suffix + ".part")
                try:
                    with temporary.open("wb") as output:
                        checksum = copy_exact_with_crc(
                            response, output, info.compress_size
                        )
                    if checksum != info.CRC:
                        raise RuntimeError(
                            f"CRC mismatch for {info.filename}: "
                            f"{checksum:08x} != {info.CRC:08x}"
                        )
                    os.replace(temporary, destination)
                except BaseException:
                    temporary.unlink(missing_ok=True)
                    raise
                extracted += 1
            else:
                discard_exact(response, info.compress_size)
                skipped += 1
            absolute_offset += info.compress_size

            if info is last:
                break
    return extracted, len(infos) - extracted


def main() -> int:
    args = parse_args()
    if RemoteZip is None:
        raise SystemExit(
            "This script needs remotezip for the ZIP directory. Install it with "
            "`python -m pip install remotezip`."
        )
    try:
        sequences = normalize_sequences(args.sequence)
    except ValueError as error:
        raise SystemExit(str(error)) from error
    if args.workers < 1:
        raise SystemExit("--workers must be at least 1")

    output_root = Path(os.path.expandvars(os.path.expanduser(args.output_root))).resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    print(f"Reading remote ZIP directory: {args.url}", flush=True)
    with RemoteZip(args.url) as archive:
        all_infos = archive.infolist()

    for sequence in sequences:
        infos = select_sequence_infos(all_infos, sequence)
        total_gib = sum(info.file_size for info in infos) / (1024**3)
        print(
            f"KITTI {sequence}: {len(infos)} frames, {total_gib:.2f} GiB",
            flush=True,
        )
        chunks = partition_infos(infos, args.workers)
        with ThreadPoolExecutor(max_workers=len(chunks)) as executor:
            counts = list(
                executor.map(
                    lambda chunk: extract_sequence_range(
                        args.url, chunk, output_root, args.overwrite
                    ),
                    chunks,
                )
            )
        extracted = sum(count[0] for count in counts)
        skipped = sum(count[1] for count in counts)
        velodyne_dir = (
            output_root / "dataset" / "sequences" / sequence / "velodyne"
        )
        write_index_timestamps(velodyne_dir, len(infos))
        print(
            f"KITTI {sequence}: extracted={extracted}, reused={skipped}, "
            f"path={velodyne_dir}",
            flush=True,
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
