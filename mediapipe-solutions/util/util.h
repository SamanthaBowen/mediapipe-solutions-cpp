// Copyright 2020 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MEDIAPIPE_SOLUTIONS_UTIL_UTIL_H_
#define MEDIAPIPE_SOLUTIONS_UTIL_UTIL_H_

#include "mediapipe/framework/calculator.pb.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/timestamp.h"

namespace mediapipe_solutions {

inline void ThrowError(const ::absl::Status& status)
{
  switch (status.code()) {
    case absl::StatusCode::kInvalidArgument:
      throw std::invalid_argument(status.message().data());
    case absl::StatusCode::kNotFound:
      throw std::system_error(make_error_code(std::errc::no_such_file_or_directory), status.message().data());
    case absl::StatusCode::kAlreadyExists:
      throw std::system_error(make_error_code(std::errc::file_exists), status.message().data());
    case absl::StatusCode::kUnimplemented:
      throw std::logic_error(status.message().data());
    default:
      throw std::runtime_error(status.message().data());
  }
}

inline void ThrowIfNotOk(const ::absl::Status& status) {
  if (!status.ok()) {
    ThrowError(status);
  }
}

inline std::string TimestampValueString(const mediapipe::Timestamp& timestamp) {
  if (timestamp == mediapipe::Timestamp::Unset()) {
    return "UNSET";
  } else if (timestamp == mediapipe::Timestamp::Unstarted()) {
    return "UNSTARTED";
  } else if (timestamp == mediapipe::Timestamp::PreStream()) {
    return "PRESTREAM";
  } else if (timestamp == mediapipe::Timestamp::Min()) {
    return "MIN";
  } else if (timestamp == mediapipe::Timestamp::Max()) {
    return "MAX";
  } else if (timestamp == mediapipe::Timestamp::PostStream()) {
    return "POSTSTREAM";
  } else if (timestamp == mediapipe::Timestamp::OneOverPostStream()) {
    return "ONEOVERPOSTSTREAM";
  } else if (timestamp == mediapipe::Timestamp::Done()) {
    return "DONE";
  } else {
    return timestamp.DebugString();
  }
}

// Reads a CalculatorGraphConfig from a file. If failed, raises a PyError.
inline ::mediapipe::CalculatorGraphConfig ReadCalculatorGraphConfigFromFile(
    const std::string& file_name
) {
  ::mediapipe::CalculatorGraphConfig graph_config_proto;
  ThrowIfNotOk(mediapipe::file::Exists(file_name));
  
  std::string graph_config_string;
  ThrowIfNotOk(mediapipe::file::GetContents(file_name, &graph_config_string,
                                        /*read_as_binary=*/true));
  if (!graph_config_proto.ParseFromArray(graph_config_string.c_str(),
                                         graph_config_string.length())) {
    throw std::runtime_error(
        absl::StrCat("Failed to parse the binary graph: ", file_name).c_str());
  }
  return graph_config_proto;
}

}  // namespace mediapipe_solutions

#endif  // MEDIAPIPE_SOLUTIONS_UTIL_UTIL_H_
