// Copyright 2024 TIER IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "autoware/multi_object_tracker/types.hpp"

#include "autoware/multi_object_tracker/object_model/uuid.hpp"

#include <tf2/utils.h>

#include <perception_msgs/msg/iscactr.hpp>
#include <perception_msgs/msg/object_classification.hpp>

#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace autoware::multi_object_tracker
{

namespace types
{

namespace
{

// Map perception_msgs ObjectClassification type → internal classes::Label
classes::Label toInternalLabel(const uint8_t perception_type)
{
  using Cls = perception_msgs::msg::ObjectClassification;
  switch (perception_type) {
    case Cls::PEDESTRIAN:
    case Cls::VRU:
      return classes::Label::PEDESTRIAN;
    case Cls::BICYCLE:
    case Cls::MICRO:
      return classes::Label::BICYCLE;
    case Cls::MOTORCYCLE:
      return classes::Label::MOTORCYCLE;
    case Cls::CAR:
      return classes::Label::CAR;
    case Cls::UTILITY:
      return classes::Label::TRUCK;
    case Cls::BUS:
      return classes::Label::BUS;
    case Cls::ANIMAL:
      return classes::Label::ANIMAL;
    case Cls::UNCLASSIFIED:
    case Cls::UNKNOWN:
    default:
      return classes::Label::UNKNOWN;
  }
}

// Map internal classes::Label → perception_msgs ObjectClassification type
uint8_t toPerceptionType(const classes::Label label)
{
  using Cls = perception_msgs::msg::ObjectClassification;
  switch (label) {
    case classes::Label::PEDESTRIAN:
      return Cls::PEDESTRIAN;
    case classes::Label::BICYCLE:
      return Cls::BICYCLE;
    case classes::Label::MOTORCYCLE:
      return Cls::MOTORCYCLE;
    case classes::Label::CAR:
      return Cls::CAR;
    case classes::Label::TRUCK:
    case classes::Label::TRAILER:
      return Cls::UTILITY;
    case classes::Label::BUS:
      return Cls::BUS;
    case classes::Label::ANIMAL:
      return Cls::ANIMAL;
    case classes::Label::UNKNOWN:
    case classes::Label::HAZARD:
    case classes::Label::OVER_DRIVABLE:
    case classes::Label::UNDER_DRIVABLE:
    default:
      return Cls::UNKNOWN;
  }
}

std::vector<classes::Classification> toClassificationsFromPerceptionMsgs(
  const std::vector<perception_msgs::msg::ObjectClassification> & classifications)
{
  std::vector<classes::Classification> result;
  result.reserve(classifications.size());
  for (const auto & c : classifications) {
    result.push_back({toInternalLabel(c.type), static_cast<float>(c.probability)});
  }
  return result;
}

std::vector<perception_msgs::msg::ObjectClassification> toPerceptionClassificationMsgs(
  const std::vector<classes::Classification> & classifications)
{
  std::vector<perception_msgs::msg::ObjectClassification> result;
  result.reserve(classifications.size());
  for (const auto & c : classifications) {
    perception_msgs::msg::ObjectClassification msg;
    msg.type = toPerceptionType(c.label);
    msg.probability = static_cast<double>(c.probability);
    result.push_back(msg);
  }
  return result;
}

bool hasValidCovariance2d(const std::array<double, 36> & covariance)
{
  const double xx = covariance[0];
  const double xy = covariance[1];
  const double yx = covariance[6];
  const double yy = covariance[7];

  // perception_msgs uses -1 for invalid variance and DBL_MAX for unknown covariance.
  constexpr double unknown_covariance = std::numeric_limits<double>::max();
  const auto is_valid_variance = [unknown_covariance](const double value) {
    return std::isfinite(value) && value >= 0.0 && value != unknown_covariance;
  };
  const auto is_valid_cross_covariance = [unknown_covariance](const double value) {
    return std::isfinite(value) && std::abs(value) != unknown_covariance;
  };

  return is_valid_variance(xx) && is_valid_variance(yy) && is_valid_cross_covariance(xy) &&
         is_valid_cross_covariance(yx);
}

}  // namespace

DynamicObject toDynamicObject(
  const perception_msgs::msg::Object & obj, const uint channel_index)
{
  using namespace perception_msgs::object_access;

  DynamicObject dynamic_object;

  // Always generate UUID for consistency (shared generator across the package).
  dynamic_object.uuid = object_model::generate_uuid();

  // Existence probability
  dynamic_object.channel_index = channel_index;
  if (obj.existence_probability < 1e-6) {
    dynamic_object.existence_probability = default_existence_probability;
  } else if (obj.existence_probability > 0.999) {
    dynamic_object.existence_probability = 0.999;
  } else {
    dynamic_object.existence_probability = obj.existence_probability;
  }
  dynamic_object.existence_probabilities.push_back(
    {channel_index, dynamic_object.existence_probability});

  // Classification
  dynamic_object.classification =
    toClassificationsFromPerceptionMsgs(obj.state.classifications);

  // Pose (getPoseCovariance returns a zero-initialized vector, avoiding uninitialized warnings)
  dynamic_object.pose = getPose(obj.state);
  const auto pose_cov_vec = getPoseCovariance(obj.state);
  std::copy(pose_cov_vec.begin(), pose_cov_vec.end(), dynamic_object.pose_covariance.begin());

  // Velocity: XYZ in header frame (rotated from object-frame lon/lat using yaw)
  const auto vel_xyz = getVelocityXYZ(obj.state);
  dynamic_object.twist.linear = vel_xyz;

  // Twist covariance — extract the 2×2 XY block from the velocity covariance
  const auto vel_with_cov = getVelocityXYZWithCovariance(obj.state);
  dynamic_object.twist_covariance.fill(0.0);
  dynamic_object.twist_covariance[0] = vel_with_cov.covariance[0];   // var(vx,vx)
  dynamic_object.twist_covariance[1] = vel_with_cov.covariance[1];   // cov(vx,vy)
  dynamic_object.twist_covariance[6] = vel_with_cov.covariance[6];   // cov(vy,vx)
  dynamic_object.twist_covariance[7] = vel_with_cov.covariance[7];   // var(vy,vy)

  // Kinematics flags
  dynamic_object.kinematics.has_position_covariance =
    hasValidCovariance2d(dynamic_object.pose_covariance);
  dynamic_object.kinematics.orientation_availability = OrientationAvailability::AVAILABLE;
  dynamic_object.kinematics.has_twist = true;
  dynamic_object.kinematics.has_twist_covariance =
    hasValidCovariance2d(dynamic_object.twist_covariance);
  dynamic_object.kinematics.is_stationary = false;

  // Shape — ISCACTR stores WIDTH(y), LENGTH(x), HEIGHT(z)
  dynamic_object.shape.type = autoware_perception_msgs::msg::Shape::BOUNDING_BOX;
  dynamic_object.shape.dimensions.x = getLength(obj.state);
  dynamic_object.shape.dimensions.y = getWidth(obj.state);
  dynamic_object.shape.dimensions.z = getHeight(obj.state);
  dynamic_object.area = getArea(dynamic_object.shape);

  return dynamic_object;
}

DynamicObjectList toDynamicObjectList(
  const perception_msgs::msg::ObjectList & obj_list, const uint channel_index)
{
  DynamicObjectList dynamic_objects;
  dynamic_objects.header = obj_list.header;
  dynamic_objects.channel_index = channel_index;
  dynamic_objects.objects.reserve(obj_list.objects.size());
  for (const auto & obj : obj_list.objects) {
    dynamic_objects.objects.emplace_back(toDynamicObject(obj, channel_index));
  }
  dynamic_objects.buildUuidIndex();
  return dynamic_objects;
}

void DynamicObjectList::buildUuidIndex() const
{
  uuid_to_index_.clear();
  uuid_to_index_.reserve(objects.size());
  for (size_t i = 0; i < objects.size(); ++i) {
    uuid_to_index_.emplace(objects[i].uuid, i);
  }
}

std::optional<size_t> DynamicObjectList::getObjectIndexByUuid(
  const unique_identifier_msgs::msg::UUID & uuid) const
{
  if (uuid_to_index_.size() != objects.size()) {
    buildUuidIndex();
  }
  const auto it = uuid_to_index_.find(uuid);
  if (it != uuid_to_index_.end()) {
    return it->second;
  }
  return std::nullopt;
}

perception_msgs::msg::Object toObjectMsg(const DynamicObject & dyn_object)
{
  using namespace perception_msgs::object_access;
  using perception_msgs::msg::ISCACTR;

  perception_msgs::msg::Object obj;

  // Encode UUID as uint64 using the first 8 bytes
  uint64_t id = 0;
  for (int i = 0; i < 8; ++i) {
    id |= static_cast<uint64_t>(dyn_object.uuid.uuid[i]) << (56 - 8 * i);
  }
  obj.id = id;

  obj.existence_probability = dyn_object.existence_probability;

  // Initialize ISCACTR state (zeros all fields, sets covariance to INVALID)
  initializeState(obj.state, ISCACTR::MODEL_ID);

  // Position
  setPosition(obj.state, dyn_object.pose.position, false);

  // Pose covariance (position + orientation)
  setPoseCovariance(obj.state, dyn_object.pose_covariance);

  // Yaw and velocity (XYZ → lon/lat using yaw)
  const double yaw = tf2::getYaw(dyn_object.pose.orientation);
  const double var_vx = dyn_object.twist_covariance[0];
  const double cov_vxy = dyn_object.twist_covariance[1];
  const double var_vy = dyn_object.twist_covariance[7];
  setVelocityXYZYawWithCovariance(
    obj.state, dyn_object.twist.linear, yaw, var_vx, var_vy, cov_vxy);

  // Dimensions — shape.dimensions.x=length, .y=width, .z=height for BOUNDING_BOX
  setLength(obj.state, dyn_object.shape.dimensions.x, false);
  setWidth(obj.state, dyn_object.shape.dimensions.y, false);
  setHeight(obj.state, dyn_object.shape.dimensions.z, false);

  // Classification
  obj.state.classifications = toPerceptionClassificationMsgs(dyn_object.classification);

  return obj;
}

double getArea(const autoware_perception_msgs::msg::Shape & shape)
{
  switch (shape.type) {
    case autoware_perception_msgs::msg::Shape::BOUNDING_BOX:
      return shape.dimensions.x * shape.dimensions.y;
    case autoware_perception_msgs::msg::Shape::CYLINDER:
      return shape.dimensions.x * shape.dimensions.x * M_PI * 0.25;
    case autoware_perception_msgs::msg::Shape::POLYGON: {
      double area = 0.0;
      for (size_t i = 0; i < shape.footprint.points.size(); ++i) {
        size_t j = (i + 1) % shape.footprint.points.size();
        area += 0.5 * (shape.footprint.points.at(i).x * shape.footprint.points.at(j).y -
                       shape.footprint.points.at(j).x * shape.footprint.points.at(i).y);
      }
      return area;
    }
    default:
      return 0.0;
  }
}

}  // namespace types

}  // namespace autoware::multi_object_tracker
