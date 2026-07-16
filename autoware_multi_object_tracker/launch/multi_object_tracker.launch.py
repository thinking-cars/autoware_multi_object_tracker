#!/usr/bin/env python3

# Copyright Institute for Automotive Engineering (ika), RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

import os

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch_xml.launch_description_sources import XMLLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, SetParameter
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    """Generate the launch description for the multi-object tracker node."""

    package_share = get_package_share_directory("autoware_multi_object_tracker")

    remappable_topics = [
        DeclareLaunchArgument("input_detection01_objects", default_value="~/input_detection01_objects"),
        DeclareLaunchArgument("input_detection02_objects", default_value="~/input_detection02_objects"),
        DeclareLaunchArgument("input_detection03_objects", default_value="~/input_detection03_objects"),
        DeclareLaunchArgument("input_detection04_objects", default_value="~/input_detection04_objects"),
        DeclareLaunchArgument("input_detection05_objects", default_value="~/input_detection05_objects"),
        DeclareLaunchArgument("input_detection06_objects", default_value="~/input_detection06_objects"),
        DeclareLaunchArgument("input_detection07_objects", default_value="~/input_detection07_objects"),
        DeclareLaunchArgument("input_detection08_objects", default_value="~/input_detection08_objects"),
        DeclareLaunchArgument("input_detection09_objects", default_value="~/input_detection09_objects"),
        DeclareLaunchArgument("input_detection10_objects", default_value="~/input_detection10_objects"),
        DeclareLaunchArgument("input_detection11_objects", default_value="~/input_detection11_objects"),
        DeclareLaunchArgument("input_detection12_objects", default_value="~/input_detection12_objects"),
        DeclareLaunchArgument("input_odometry", default_value="~/input_odometry"),
        DeclareLaunchArgument("output_objects", default_value="~/output_objects"),
        DeclareLaunchArgument("output_merged_objects", default_value="~/output_merged_objects"),
    ]

    channel_arguments = [
        DeclareLaunchArgument("input_detection01_channel", default_value="none"),
        DeclareLaunchArgument("input_detection02_channel", default_value="none"),
        DeclareLaunchArgument("input_detection03_channel", default_value="none"),
        DeclareLaunchArgument("input_detection04_channel", default_value="none"),
        DeclareLaunchArgument("input_detection05_channel", default_value="none"),
        DeclareLaunchArgument("input_detection06_channel", default_value="none"),
        DeclareLaunchArgument("input_detection07_channel", default_value="none"),
        DeclareLaunchArgument("input_detection08_channel", default_value="none"),
        DeclareLaunchArgument("input_detection09_channel", default_value="none"),
        DeclareLaunchArgument("input_detection10_channel", default_value="none"),
        DeclareLaunchArgument("input_detection11_channel", default_value="none"),
        DeclareLaunchArgument("input_detection12_channel", default_value="none"),
    ]

    args = [
        DeclareLaunchArgument("name", default_value="autoware_multi_object_tracker", description="node name"),
        DeclareLaunchArgument("namespace", default_value="", description="node namespace"),
        DeclareLaunchArgument(
            "log_level",
            default_value="info",
            description="ROS logging level (debug, info, warn, error, fatal)",
        ),
        *remappable_topics,
        *channel_arguments,
        DeclareLaunchArgument(
            "tracker_setting_path",
            default_value=os.path.join(package_share, "config", "multi_object_tracker_node.param.yaml"),
        ),
        DeclareLaunchArgument(
            "data_association_matrix_path",
            default_value=os.path.join(package_share, "config", "data_association_matrix.param.yaml"),
        ),
        DeclareLaunchArgument(
            "input_channels_path",
            default_value=os.path.join(package_share, "config", "input_channels.param.yaml"),
        ),
        DeclareLaunchArgument("publish_merged_objects", default_value="false"),
        DeclareLaunchArgument(
            "ego_source",
            default_value="tf",
            description="ego pose source: tf (look up from TF tree) or odometry (interpolate from input/odometry)",
        ),
        DeclareLaunchArgument("use_sim_time", default_value="false", description="use simulation clock"),
    ]

    agnocast_environment = IncludeLaunchDescription(
        XMLLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("autoware_agnocast_wrapper"),
                "launch",
                "agnocast_env.launch.xml",
            )
        )
    )

    node = Node(
        package="autoware_multi_object_tracker",
        executable="multi_object_tracker_node",
        namespace=LaunchConfiguration("namespace"),
        name=LaunchConfiguration("name"),
        parameters=[
            LaunchConfiguration("tracker_setting_path"),
            LaunchConfiguration("data_association_matrix_path"),
            LaunchConfiguration("input_channels_path"),
            {
                "publish_merged_objects": ParameterValue(
                    LaunchConfiguration("publish_merged_objects"), value_type=bool
                ),
                "ego_source": LaunchConfiguration("ego_source"),
            },
            {argument.name.replace("_", "/"): LaunchConfiguration(argument.name) for argument in channel_arguments},
        ],
        arguments=["--ros-args", "--log-level", LaunchConfiguration("log_level")],
        remappings=[(f"~/{argument.name.replace('_', '/')}", LaunchConfiguration(argument.name)) for argument in remappable_topics],
        additional_env={"LD_PRELOAD": LaunchConfiguration("ld_preload_value")},
        output="both",
        emulate_tty=True,
    )

    return LaunchDescription(
        [
            *args,
            agnocast_environment,
            SetParameter("use_sim_time", LaunchConfiguration("use_sim_time")),
            node,
        ]
    )
