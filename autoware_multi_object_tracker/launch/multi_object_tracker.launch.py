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
        DeclareLaunchArgument("input/detection01/objects", default_value="~/input/detection01/objects"),
        DeclareLaunchArgument("input/detection02/objects", default_value="~/input/detection02/objects"),
        DeclareLaunchArgument("input/detection03/objects", default_value="~/input/detection03/objects"),
        DeclareLaunchArgument("input/detection04/objects", default_value="~/input/detection04/objects"),
        DeclareLaunchArgument("input/detection05/objects", default_value="~/input/detection05/objects"),
        DeclareLaunchArgument("input/detection06/objects", default_value="~/input/detection06/objects"),
        DeclareLaunchArgument("input/detection07/objects", default_value="~/input/detection07/objects"),
        DeclareLaunchArgument("input/detection08/objects", default_value="~/input/detection08/objects"),
        DeclareLaunchArgument("input/detection09/objects", default_value="~/input/detection09/objects"),
        DeclareLaunchArgument("input/detection10/objects", default_value="~/input/detection10/objects"),
        DeclareLaunchArgument("input/detection11/objects", default_value="~/input/detection11/objects"),
        DeclareLaunchArgument("input/detection12/objects", default_value="~/input/detection12/objects"),
        DeclareLaunchArgument("input/odometry", default_value="~/input/odometry"),
        DeclareLaunchArgument("output/objects", default_value="~/output/objects"),
        DeclareLaunchArgument("output/merged_objects", default_value="~/output/merged_objects"),
    ]

    channel_arguments = [
        DeclareLaunchArgument("input/detection01/channel", default_value="none"),
        DeclareLaunchArgument("input/detection02/channel", default_value="none"),
        DeclareLaunchArgument("input/detection03/channel", default_value="none"),
        DeclareLaunchArgument("input/detection04/channel", default_value="none"),
        DeclareLaunchArgument("input/detection05/channel", default_value="none"),
        DeclareLaunchArgument("input/detection06/channel", default_value="none"),
        DeclareLaunchArgument("input/detection07/channel", default_value="none"),
        DeclareLaunchArgument("input/detection08/channel", default_value="none"),
        DeclareLaunchArgument("input/detection09/channel", default_value="none"),
        DeclareLaunchArgument("input/detection10/channel", default_value="none"),
        DeclareLaunchArgument("input/detection11/channel", default_value="none"),
        DeclareLaunchArgument("input/detection12/channel", default_value="none"),
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
            {argument.name: LaunchConfiguration(argument.name) for argument in channel_arguments},
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
