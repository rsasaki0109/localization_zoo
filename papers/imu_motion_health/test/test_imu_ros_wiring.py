#!/usr/bin/env python3
"""Static regression checks for the optional ROS 2 IMU wrapper.

This test intentionally uses only the Python standard library so the
dependency-light standalone SDK can verify its ROS integration surface even
on machines without ROS installed. The repository's ROS CI remains the
authoritative compile test.
"""

from __future__ import annotations

import ast
import pathlib
import sys
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[3]
ROS_PACKAGE = ROOT / "ros2" / "localization_zoo_ros"


class RosImuMotionHealthWiringTest(unittest.TestCase):
    def test_build_and_package_contract(self) -> None:
        cmake = (ROS_PACKAGE / "CMakeLists.txt").read_text(encoding="utf-8")
        self.assertIn("add_library(imu_motion_health_core STATIC", cmake)
        self.assertIn("add_executable(imu_motion_health_node", cmake)
        self.assertIn("target_link_libraries(imu_motion_health_node imu_motion_health_core)", cmake)
        self.assertIn("imu_motion_health_node\n", cmake)
        self.assertIn("add_executable(imu_calibration_node", cmake)
        self.assertIn("imu_calibration_node\n", cmake)

        package = ET.parse(ROS_PACKAGE / "package.xml").getroot()
        dependencies = {node.text for node in package.findall("depend")}
        self.assertTrue(
            {"rclcpp", "sensor_msgs", "nav_msgs", "std_msgs", "tf2_ros"}
            <= dependencies
        )

    def test_node_topics_events_and_profiles(self) -> None:
        source = (ROS_PACKAGE / "src" / "imu_motion_health_node.cpp").read_text(
            encoding="utf-8"
        )
        for evidence in (
            "rclcpp::SensorDataQoS()",
            "create_publisher<nav_msgs::msg::Odometry>",
            "create_publisher<std_msgs::msg::String>",
            "pipeline_->drainEvents()",
            "imu_motion_health::applyProfile",
            "imu_motion_health::loadProfileFile",
            "gyro_bias_jump_threshold",
            "impact_requires_accel_and_gyro",
            "zero_stamp_policy",
        ):
            self.assertIn(evidence, source)

    def test_launch_and_parameter_files_are_present_and_parseable(self) -> None:
        launch_path = ROS_PACKAGE / "launch" / "imu_motion_health.launch.py"
        ast.parse(launch_path.read_text(encoding="utf-8"), filename=str(launch_path))
        launch_text = launch_path.read_text(encoding="utf-8")
        self.assertIn('executable="imu_motion_health_node"', launch_text)
        self.assertIn('default_value="cargo"', launch_text)

        config = (ROS_PACKAGE / "config" / "imu_motion_health.yaml").read_text(
            encoding="utf-8"
        )
        self.assertIn("imu_motion_health_node:", config)
        self.assertIn("ros__parameters:", config)
        self.assertIn("profile: cargo", config)
        self.assertIn("imu_topic: /imu_raw", config)

    def test_calibration_node_and_combined_launch(self) -> None:
        source = (ROS_PACKAGE / "src" / "imu_calibration_node.cpp").read_text(encoding="utf-8")
        for evidence in ("sensor_msgs::msg::Temperature", "gyro_bias_at_reference",
                         "gyro_temperature_coefficient", "max_temperature_age_s",
                         "accel_bias", "publisher_->publish(output)"):
            self.assertIn(evidence, source)
        launch_path = ROS_PACKAGE / "launch" / "imu_calibrated_motion_health.launch.py"
        ast.parse(launch_path.read_text(encoding="utf-8"), filename=str(launch_path))
        launch_text = launch_path.read_text(encoding="utf-8")
        self.assertIn('executable="imu_calibration_node"', launch_text)
        self.assertIn('"imu_topic": "/imu/calibrated"', launch_text)
        config = (ROS_PACKAGE / "config" / "imu_calibration.yaml").read_text(encoding="utf-8")
        self.assertIn("reference_temperature_c:", config)
        self.assertIn("gyro_temperature_coefficient:", config)


if __name__ == "__main__":
    result = unittest.main(verbosity=2, exit=False)
    sys.exit(0 if result.result.wasSuccessful() else 1)
