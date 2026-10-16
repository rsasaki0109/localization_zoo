#!/usr/bin/env python3
"""Pace the committed scans through real ROS nodes and compare odometry/TF to C++."""
import argparse
import copy
import io
import json
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest

import numpy as np
import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node
from sensor_msgs.msg import PointCloud2, PointField
from tf2_msgs.msg import TFMessage

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'python'))
from example_paper_odometry import METHODS, load_scans


def cloud(points, stamp, frame, *, double=False, big_endian=False):
    msg = PointCloud2()
    msg.header.stamp.sec = stamp
    msg.header.frame_id = frame
    msg.height, msg.width = 1, len(points)
    size = 8 if double else 4
    msg.fields = [PointField(name=name, offset=size * i,
                            datatype=PointField.FLOAT64 if double else PointField.FLOAT32, count=1)
                  for i, name in enumerate(('x', 'y', 'z'))]
    msg.is_bigendian, msg.is_dense = big_endian, True
    msg.point_step, msg.row_step = 3 * size, len(points) * 3 * size
    msg.data = points.astype(('>' if big_endian else '<') + ('f8' if double else 'f4')).tobytes()
    return msg


def pose_matrix(position, orientation):
    x, y, z, w = orientation.x, orientation.y, orientation.z, orientation.w
    norm = x*x + y*y + z*z + w*w
    if not np.isfinite(norm) or abs(norm - 1) > 1e-8:
        raise AssertionError(f'invalid quaternion norm {norm}')
    matrix = np.eye(4)
    matrix[:3, :3] = [
        [1 - 2*(y*y + z*z), 2*(x*y - z*w), 2*(x*z + y*w)],
        [2*(x*y + z*w), 1 - 2*(x*x + z*z), 2*(y*z - x*w)],
        [2*(x*z - y*w), 2*(y*z + x*w), 1 - 2*(x*x + y*y)],
    ]
    matrix[:3, 3] = position.x, position.y, position.z
    return matrix


class PaperNodes(unittest.TestCase):
    reports = {}

    @classmethod
    def tearDownClass(cls):
        report = {'methods': cls.reports}
        if ARGS.report:
            ARGS.report.write_text(json.dumps(report, indent=2) + '\n')
        print('PAPER_ROS_FIXTURE_REPORT ' + json.dumps(report))

    def spin_until(self, node, predicate, timeout=30):
        deadline = time.monotonic() + timeout
        while not predicate() and time.monotonic() < deadline:
            rclpy.spin_once(node, timeout_sec=0.1)
        self.assertTrue(predicate(), 'timed out waiting for ROS discovery/output')

    def test_native_parity_frames_stamps_and_tf(self):
        scans = load_scans(ROOT / 'evaluation/fixtures/mcd_kth_smoke')
        with tempfile.TemporaryDirectory() as directory:
            paths = []
            for i, (_, scan) in enumerate(scans):
                path = Path(directory) / f'{i}.f64'
                scan.astype('<f8').tofile(path)
                paths.append(str(path))
            for method in METHODS:
                with self.subTest(method=method):
                    expected = subprocess.run([str(ARGS.reference), method, *paths],
                                              check=True, capture_output=True, text=True)
                    reference = np.loadtxt(io.StringIO(expected.stdout), delimiter=',', ndmin=2)
                    namespace = f'/paper_fixture/{method}'
                    sensor = f'{method}_lidar'
                    received, transforms = {}, {}
                    odom_poses, tf_poses = [], []
                    observer = Node(f'{method}_fixture_observer')
                    publisher = observer.create_publisher(PointCloud2, namespace + '/points', 10)
                    def on_odom(msg):
                        received[msg.header.stamp.sec] = msg
                    def on_tf(msg):
                        for transform in msg.transforms:
                            if transform.child_frame_id == sensor:
                                transforms[transform.header.stamp.sec] = transform
                    odom_subscription = observer.create_subscription(
                        Odometry, namespace + '/odom', on_odom, 10)
                    tf_subscription = observer.create_subscription(TFMessage, '/tf', on_tf, 10)
                    command = [str(ARGS.node_dir / f'{method}_node'), '--ros-args',
                               '-r', f'__ns:={namespace}', '-r', f'points_raw:={namespace}/points',
                               '-r', f'odom:={namespace}/odom', '-p', 'map_frame_id:=fixture_world',
                               '-p', 'input_reliability:=reliable']
                    log_path = Path(directory) / f'{method}.log'
                    with log_path.open('w') as log:
                        child = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
                    try:
                        def discovered():
                            self.assertIsNone(child.poll(), log_path.read_text())
                            return (publisher.get_subscription_count() > 0 and
                                    odom_subscription.get_publisher_count() > 0 and
                                    tf_subscription.get_publisher_count() > 0)
                        self.spin_until(observer, discovered)
                        # Invalid headers must not advance either time or the core stream.
                        publisher.publish(cloud(scans[0][1][:4], 0, sensor))
                        publisher.publish(cloud(scans[0][1][:4], -1, sensor))
                        publisher.publish(cloud(scans[0][1][:4], 999, ''))
                        malformed = cloud(scans[0][1][:4], 999, sensor)
                        malformed.data = malformed.data[:2]
                        publisher.publish(malformed)
                        for i, (_, points) in enumerate(scans):
                            stamp = 100 + i
                            msg = cloud(points, stamp, sensor, double=i == 0, big_endian=i == 1)
                            publisher.publish(msg)
                            # Repeating the same stamp is safe: the node rejects
                            # duplicates. Retry large DDS samples after discovery.
                            deadline = time.monotonic() + 30
                            retry = time.monotonic() + 2
                            while stamp not in received and time.monotonic() < deadline:
                                self.assertIsNone(child.poll(), log_path.read_text())
                                rclpy.spin_once(observer, timeout_sec=0.1)
                                if time.monotonic() >= retry:
                                    publisher.publish(msg)
                                    retry = time.monotonic() + 2
                            self.assertIn(stamp, received, log_path.read_text())
                            self.spin_until(observer, lambda: stamp in transforms)
                            result = received[stamp]
                            self.assertEqual(result.header.frame_id, 'fixture_world')
                            self.assertEqual(result.child_frame_id, sensor)
                            odom_poses.append(pose_matrix(result.pose.pose.position, result.pose.pose.orientation))
                            np.testing.assert_allclose(
                                pose_matrix(result.pose.pose.position, result.pose.pose.orientation),
                                reference[i, :16].reshape(4, 4), rtol=0, atol=1e-10)
                            tf = transforms[stamp]
                            self.assertEqual(tf.header.frame_id, 'fixture_world')
                            tf_poses.append(pose_matrix(tf.transform.translation, tf.transform.rotation))
                            np.testing.assert_allclose(
                                pose_matrix(tf.transform.translation, tf.transform.rotation),
                                reference[i, :16].reshape(4, 4), rtol=0, atol=1e-10)
                            if i == 0:
                                publisher.publish(cloud(points[:4], 1000, 'different_frame'))
                                publisher.publish(copy.deepcopy(msg))
                        self.assertEqual(set(received), {100 + i for i in range(len(scans))})
                        self.reports[method] = {
                            'frames': len(scans),
                            'max_abs_odom_delta': float(np.max(np.abs(np.asarray(odom_poses).reshape(-1, 16) - reference[:, :16]))),
                            'max_abs_tf_delta': float(np.max(np.abs(np.asarray(tf_poses).reshape(-1, 16) - reference[:, :16]))),
                            'stamps': sorted(received),
                            'frame_id': 'fixture_world', 'child_frame_id': sensor,
                            'odom_poses': [p.reshape(-1).tolist() for p in odom_poses],
                            'tf_poses': [p.reshape(-1).tolist() for p in tf_poses],
                            'native_poses': reference[:, :16].tolist(),
                        }
                    finally:
                        child.send_signal(signal.SIGINT)
                        try:
                            child.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            child.kill()
                            child.wait()
                        observer.destroy_node()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--node-dir', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--report', type=Path)
    ARGS, remaining = parser.parse_known_args()
    rclpy.init()
    try:
        unittest.main(argv=[sys.argv[0], *remaining])
    finally:
        rclpy.shutdown()
