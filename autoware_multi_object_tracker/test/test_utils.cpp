// Copyright 2025 TIER IV, inc.
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
#include "test_utils.hpp"

#include <array>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>
namespace
{

constexpr size_t uuid_size = 16;
constexpr size_t hash_size = sizeof(size_t);

}  // namespace
// Convert string ID to 16-byte UUID
std::array<uint8_t, uuid_size> stringToUUID(const std::string & id)
{
  std::array<uint8_t, uuid_size> uuid{};
  std::size_t hash = std::hash<std::string>{}(id);
  // Distribute hash across UUID bytes
  std::memcpy(uuid.data(), &hash, hash_size);
  std::memcpy(uuid.data() + hash_size, &hash, hash_size);  // Repeat hash
  return uuid;
}

void printFrameStatsHeader()
{
  std::cout << std::left << std::setw(6) << "Frame" << std::setw(10) << "Total(ms)" << std::setw(10)
            << "Pred(ms)" << std::setw(10) << "Asc(ms)" << std::setw(10) << "Upd(ms)"
            << std::setw(10) << "Prn(ms)" << std::setw(10) << "Spw(ms)" << std::setw(6) << "Trk"
            << std::setw(6) << "Det" << std::setw(6) << "Prn" << std::setw(6) << "Spw"
            << std::setw(6) << "Fin" << std::endl;
}

void printFrameStats(
  int frame, int detection_size, int num_trackers0, int num_trackers2, int num_pruned,
  int num_spawned, const FunctionTimings & timings)
{
  std::cout << std::left << std::fixed << std::setprecision(3) << std::setw(6) << frame
            << std::setw(10) << timings.total.times.back() << std::setw(10)
            << timings.predict.times.back() << std::setw(10) << timings.associate.times.back()
            << std::setw(10) << timings.update.times.back() << std::setw(10)
            << timings.prune.times.back() << std::setw(10) << timings.spawn.times.back()
            << std::setw(6) << num_trackers0 << std::setw(6) << detection_size << std::setw(6)
            << num_pruned << std::setw(6) << num_spawned << std::setw(6) << num_trackers2
            << std::endl;
}

perception_msgs::msg::ObjectList toDetectedObjectsMsg(
  const autoware::multi_object_tracker::types::DynamicObjectList & dyn_objects)
{
  perception_msgs::msg::ObjectList object_list;
  object_list.header = dyn_objects.header;
  object_list.objects.reserve(dyn_objects.objects.size());
  for (const auto & dyn_object : dyn_objects.objects) {
    object_list.objects.emplace_back(
      autoware::multi_object_tracker::types::toObjectMsg(dyn_object));
  }
  return object_list;
}

// ==========================
// RosbagWriterHelper
// ==========================

RosbagWriterHelper::RosbagWriterHelper(bool enabled, const std::string & storage_format)
: enabled_(enabled)
{
  if (!enabled_) return;

  writer_ = std::make_unique<rosbag2_cpp::Writer>();

  // Generate timestamped bag name
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << "tracking_results_" << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
  bag_name_ = ss.str();
  std::cout << "Writing results to rosbag (" << storage_format << "): " << bag_name_ << std::endl;
  // Set up rosbag2 writer with selected format
  rosbag2_storage::StorageOptions storage_options;
  storage_options.uri = bag_name_;
  const std::set<std::string> supported_formats = {"sqlite3", "mcap"};
  if (supported_formats.count(storage_format) == 0) {
    std::cerr << "Warning: unsupported storage format '" << storage_format
              << "', falling back to 'sqlite3'\n";
    storage_options.storage_id = "sqlite3";
  } else {
    storage_options.storage_id = storage_format;
  }
  rosbag2_cpp::ConverterOptions converter_options;
  converter_options.input_serialization_format = rmw_get_serialization_format();
  converter_options.output_serialization_format = rmw_get_serialization_format();

  writer_->open(storage_options, converter_options);

  std::cout << "Rosbag opened successfully." << std::endl;
  // Register topics
  const auto serialization_format = rmw_get_serialization_format();

  {
    rosbag2_storage::TopicMetadata topic_metadata_det;
    topic_metadata_det.name = "/perception/object_recognition/detection/objects";
    topic_metadata_det.type = "perception_msgs/msg/ObjectList";
    topic_metadata_det.serialization_format = serialization_format;
    writer_->create_topic(topic_metadata_det);
  }

  {
    rosbag2_storage::TopicMetadata topic_metadata_rec;
    topic_metadata_rec.name = "/perception/object_recognition/tracking/objects";
    topic_metadata_rec.type = "perception_msgs/msg/ObjectList";
    topic_metadata_rec.serialization_format = serialization_format;
    writer_->create_topic(topic_metadata_rec);
  }
  std::cout << "Topics registered successfully." << std::endl;
}

RosbagWriterHelper::~RosbagWriterHelper()
{
  if (enabled_ && writer_) {
    writer_->close();
    const auto absolute_path = std::filesystem::absolute(bag_name_);
    std::cout << "Run rosbag by:\nros2 bag play " << absolute_path << std::endl;
  }
}

RosbagReaderHelper::RosbagReaderHelper(const std::string & path)
{
  namespace fs = std::filesystem;
  std::string bag_file;

  if (fs::is_directory(path)) {
    for (const auto & entry : fs::directory_iterator(path)) {
      const auto & ext = entry.path().extension();
      if (ext == ".db3" || ext == ".mcap") {
        bag_file = entry.path().string();
        break;
      }
    }
    if (bag_file.empty()) {
      throw std::runtime_error("No .db3 or .mcap file found in directory: " + path);
    }
  } else if (fs::is_regular_file(path)) {
    bag_file = path;
  } else {
    throw std::runtime_error("Invalid bag path: " + path);
  }

  reader_.open(bag_file);

  if (!reader_.has_next()) {
    throw std::runtime_error("No messages found in the bag file: " + bag_file);
  }

  std::cout << "Opened bag file: " << bag_file << std::endl;
}

bool RosbagReaderHelper::hasNext()
{
  return reader_.has_next();
}

std::shared_ptr<rosbag2_storage::SerializedBagMessage> RosbagReaderHelper::readNext()
{
  return reader_.read_next();
}
