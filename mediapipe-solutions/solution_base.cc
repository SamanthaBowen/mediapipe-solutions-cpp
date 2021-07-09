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

#include "mediapipe-solutions/solution_base.h"

#include <chrono>
#include <filesystem>
#include "absl/strings/str_split.h"

#include "mediapipe/framework/formats/classification.pb.h"
#include "mediapipe/framework/formats/detection.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/matrix.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/port/parse_text_proto.h"

#include "mediapipe-solutions/util/util.h"

using namespace std;
using namespace std::chrono;
using namespace mediapipe;

namespace {
	template <typename Rep, typename Period>
	inline mediapipe::Timestamp ToTimestamp(std::chrono::duration<Rep, Period> value) {
		return mediapipe::Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(value).count());
	}
}

namespace mediapipe_solutions {

SolutionBase::SolutionBase(
	CalculatorGraphConfig graph_config,
	unordered_map<string, Any> &&side_inputs,
	vector<string> outputs
) {
	//filesystem::path root_path = filesystem::current_path() /* TODO: [:-3] */;
	//SetResourceDir(root_path);

	Init(graph_config, move(side_inputs), outputs);
}

SolutionBase::SolutionBase(
	string_view graph_config,
	unordered_map<string, Any> &&side_inputs,
	vector<string> outputs
) :
	SolutionBase(
		mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(string(graph_config)),
		move(side_inputs),
		move(outputs)
	) {
}

void SolutionBase::Init(
	CalculatorGraphConfig graph_config,
	unordered_map<string, Any> side_inputs,
	vector<string> outputs
) {
	graph_.Initialize(graph_config);
	start_timestamp_ = steady_clock::now();

	for (const auto &output : outputs) {
		graph_outputs_.emplace(output, Packet());
		graph_.ObserveOutputStream(
			output,
			[this, output](auto &&output_packet) { graph_outputs_.at(output) = output_packet; return absl::OkStatus(); }
		);
	}

	map<string, Packet> input_side_packets;

	const auto timestamp = ToTimestamp(steady_clock::now() - start_timestamp_);
	
	for (auto &nameDataPair : side_inputs) {
		const auto &name = nameDataPair.first;

		input_side_packets.emplace(name, move(nameDataPair.second).At(timestamp));
	}

	ThrowIfNotOk(graph_.StartRun(input_side_packets));
}

void SolutionBase::Close() {
	graph_.CloseAllPacketSources();
	graph_.WaitUntilDone();
	graph_outputs_.clear();
}

unordered_map<string, Any> SolutionBase::Process(unordered_map<string_view, Any> &&inputs) {
	const auto timestamp = ToTimestamp(steady_clock::now() - start_timestamp_);

	for (auto &&input : inputs) {
		ThrowIfNotOk(graph_.AddPacketToInputStream(string(input.first), move(input.second).At(timestamp)));
	}

	unordered_map<string, Any> outputs;

	ThrowIfNotOk(graph_.WaitUntilIdle());

	for (auto &&output : graph_outputs_) {
		if (!output.second.IsEmpty())
			outputs.emplace(output.first, move(output.second));
	}

	return outputs;
}

unordered_map<string, Any> SolutionBase::Process(string_view input_stream, Any input) {
	unordered_map<string_view, Any> inputs;
	
	inputs.emplace(input_stream, move(input));
	return Process(move(inputs));
}

}
