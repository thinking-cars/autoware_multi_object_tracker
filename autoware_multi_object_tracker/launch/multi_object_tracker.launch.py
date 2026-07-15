#!/usr/bin/env python3

# Copyright Institute for Automotive Engineering (ika), RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

import os

from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import XMLLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, SetParameter
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    """Generate the launch description for the multi-object tracker node."""

    package_share = get_package_share_directory("autoware_multi_object_tracker")

    remappable_topics = [
        DeclareLaunchArgument("input_object_list_topic_01", default_value="~/input/detection01/objects"),
        DeclareLaunchArgument("input_object_list_topic_02", default_value="~/input/detection02/objects"),
        DeclareLaunchArgument("input_object_list_topic_03", default_value="~/input/detection03/objects"),
        DeclareLaunchArgument("input_object_list_topic_04", default_value="~/input/detection04/objects"),
        DeclareLaunchArgument("input_object_list_topic_05", default_value="~/input/detection05/objects"),
        DeclareLaunchArgument("input_object_list_topic_06", default_value="~/input/detection06/objects"),
        DeclareLaunchArgument("input_object_list_topic_07", default_value="~/input/detection07/objects"),
        DeclareLaunchArgument("input_object_list_topic_08", default_value="~/input/detection08/objects"),
        DeclareLaunchArgument("input_object_list_topic_09", default_value="~/input/detection09/objects"),
        DeclareLaunchArgument("input_object_list_topic_10", default_value="~/input/detection10/objects"),
        DeclareLaunchArgument("input_object_list_topic_11", default_value="~/input/detection11/objects"),
        DeclareLaunchArgument("input_object_list_topic_12", default_value="~/input/detection12/objects"),
        DeclareLaunchArgument("odometry_topic", default_value="~/input/odometry"),
        DeclareLaunchArgument("objects_topic", default_value="~/output/objects"),
        DeclareLaunchArgument("merged_objects_topic", default_value="~/output/merged_objects"),
    ]

    channel_arguments = [f"input/detection{index:02d}/channel" for index in range(1, 13)]

    args = [
        DeclareLaunchArgument("name", default_value="autoware_multi_object_tracker", description="node name"),
        DeclareLaunchArgument("namespace", default_value="", description="node namespace"),
        DeclareLaunchArgument(
            "log_level",
            default_value="info",
            description="ROS logging level (debug, info, warn, error, fatal)",
        ),
        *remappable_topics,
        *[DeclareLaunchArgument(name, default_value="none") for name in channel_arguments],
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
                **{name: LaunchConfiguration(name) for name in channel_arguments},
                "publish_merged_objects": ParameterValue(
                    LaunchConfiguration("publish_merged_objects"), value_type=bool
                ),
                "ego_source": LaunchConfiguration("ego_source"),
            },
        ],
        arguments=["--ros-args", "--log-level", LaunchConfiguration("log_level")],
        remappings=[(f"~/{argument.name}", LaunchConfiguration(argument.name)) for argument in remappable_topics],
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
