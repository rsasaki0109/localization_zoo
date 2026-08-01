import importlib.util
import io
import sys
import tempfile
import unittest
import zlib
import csv
from pathlib import Path
from unittest import mock
from zipfile import ZIP_STORED, ZipInfo


REPO_ROOT = Path(__file__).resolve().parents[1]


def load_module(name: str, relative_path: str):
    spec = importlib.util.spec_from_file_location(name, REPO_ROOT / relative_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Cannot load {relative_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


class FakeRangeResponse(io.BytesIO):
    status = 206

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        self.close()


class KittiSelectiveFetchTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module(
            "test_fetch_kitti_odometry_sequences",
            "evaluation/scripts/fetch_kitti_odometry_sequences.py",
        )

    def make_entry(self, filename: str, payload: bytes, offset: int):
        name = filename.encode()
        crc = zlib.crc32(payload) & 0xFFFFFFFF
        header = self.module.LOCAL_FILE_HEADER.pack(
            self.module.LOCAL_FILE_SIGNATURE,
            20,
            0,
            ZIP_STORED,
            0,
            0,
            crc,
            len(payload),
            len(payload),
            len(name),
            0,
        )
        info = ZipInfo(filename)
        info.header_offset = offset
        info.compress_type = ZIP_STORED
        info.file_size = len(payload)
        info.compress_size = len(payload)
        info.CRC = crc
        return info, header + name + payload

    def test_range_extractor_writes_only_selected_frames_and_timestamps(self) -> None:
        filename0 = "dataset/sequences/07/velodyne/000000.bin"
        filename1 = "dataset/sequences/07/velodyne/000001.bin"
        info0, encoded0 = self.make_entry(filename0, b"frame-zero", 0)
        info1, encoded1 = self.make_entry(filename1, b"frame-one!", len(encoded0))

        with tempfile.TemporaryDirectory() as tmpdir:
            output_root = Path(tmpdir)
            with mock.patch.object(
                self.module.urllib.request,
                "urlopen",
                return_value=FakeRangeResponse(encoded0 + encoded1),
            ):
                extracted, reused = self.module.extract_sequence_range(
                    "https://example.invalid/kitti.zip",
                    [info0, info1],
                    output_root,
                )
            velodyne = output_root / "dataset" / "sequences" / "07" / "velodyne"
            self.module.write_index_timestamps(velodyne, 2)

            self.assertEqual((extracted, reused), (2, 0))
            self.assertEqual((velodyne / "000000.bin").read_bytes(), b"frame-zero")
            self.assertEqual((velodyne / "000001.bin").read_bytes(), b"frame-one!")
            self.assertEqual(
                (velodyne / "frame_timestamps.csv").read_text(),
                "frame,timestamp\n0,0\n1,1\n",
            )

    def test_select_sequence_infos_filters_and_orders(self) -> None:
        a = ZipInfo("dataset/sequences/00/velodyne/000001.bin")
        b = ZipInfo("dataset/sequences/00/velodyne/000000.bin")
        other = ZipInfo("dataset/sequences/05/velodyne/000000.bin")
        for index, info in enumerate((a, b, other)):
            info.compress_type = ZIP_STORED
            info.header_offset = 100 - index

        selected = self.module.select_sequence_infos([a, other, b], "00")
        self.assertEqual([info.filename for info in selected], [b.filename, a.filename])

    def test_partition_infos_is_contiguous_and_complete(self) -> None:
        infos = [ZipInfo(f"{index}.bin") for index in range(10)]
        chunks = self.module.partition_infos(infos, 4)
        self.assertEqual([len(chunk) for chunk in chunks], [3, 3, 3, 1])
        self.assertEqual([item for chunk in chunks for item in chunk], infos)


class LidarSotaVerifierTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module(
            "test_verify_lidar_sota_data",
            "evaluation/scripts/verify_lidar_sota_data.py",
        )

    def test_count_frames_supports_native_kitti_bin(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "000000.bin").write_bytes(b"")
            (root / "000001.bin").write_bytes(b"")
            (root / "notes.txt").write_text("ignored")
            self.assertEqual(self.module.count_frames(root, "kitti_bin"), 2)

    def test_count_frames_supports_nested_pcd_alias(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            for index in range(2):
                frame = root / f"{index:08d}"
                frame.mkdir()
                (frame / "cloud.pcd").write_text("pcd")
            (root / "00000002").mkdir()
            self.assertEqual(self.module.count_frames(root, "pcd"), 2)

    def test_count_frames_supports_flat_pcd_directory(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            (root / "00000000.pcd").write_text("pcd")
            (root / "00000001.PCD").write_text("pcd")
            self.assertEqual(self.module.count_frames(root, "pcd_directory"), 2)


class IdentityReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.module = load_module(
            "test_make_identity_reference_csv",
            "evaluation/scripts/make_identity_reference_csv.py",
        )

    def test_frame_count_source_writes_matching_strict_timestamps(self) -> None:
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            output = root / "identity.csv"
            timestamps = root / "frame_timestamps.csv"
            with mock.patch.object(
                sys,
                "argv",
                [
                    str(self.module.__file__),
                    "--frames",
                    "3",
                    "--output",
                    str(output),
                    "--frame-timestamps-output",
                    str(timestamps),
                ],
            ):
                self.assertEqual(self.module.main(), 0)
            with timestamps.open(newline="") as handle:
                self.assertEqual(
                    list(csv.reader(handle)),
                    [["frame", "timestamp"], ["0", "0"], ["1", "1"], ["2", "2"]],
                )
            with output.open(newline="") as handle:
                rows = list(csv.reader(handle))
            self.assertEqual(len(rows), 4)
            self.assertEqual(rows[-1], ["2", "0.0", "0.0", "0.0", "0.0", "0.0", "0.0"])


if __name__ == "__main__":
    unittest.main()
