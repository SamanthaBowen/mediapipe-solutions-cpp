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
using namespace google::protobuf;
using namespace mediapipe;

namespace {
	template <typename Rep, typename Period>
	inline mediapipe::Timestamp ToTimestamp(std::chrono::duration<Rep, Period> value) {
		return mediapipe::Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(value).count());
	}

	// TODO: Enable calculator options modification for more calculators.
	/*
	void CreateOptions(CalculatorGraphConfig::Node &node)
	{
		std::string_view calculator = node.calculator();

		if (calculator == "ConstantSidePacketCalculator")
			node.set_allocated_options(new ConstantSidePacketCalculatorOptions());
		else if (calculator == "ImageTransformationCalculator")
			node.set_allocated_options(new ImageTransformationCalculatorOptions());
		else if (calculator == "LandmarksSmoothingCalculator")
			node.set_allocated_options(new LandmarksSmoothingCalculatorOptions());
		else if (calculator == "LogicCalculator")
			node.set_allocated_options(new LogicCalculatorOptions());
		else if (calculator == "ThresholdingCalculator")
			node.set_allocated_options(new ThresholdingCalculatorOptions());
		else if (calculator == "TensorsToDetectionsCalculator")
			node.set_allocated_options(new TensorsToDetectionsCalculatorOptions());
		else if (calculator == "Lift2DFrameAnnotationTo3DCalculator")
			node.set_allocated_options(new Lift2DFrameAnnotationTo3DCalculatorOptions());
		else
			throw logic_error("Modifying the calculator options of "s + string(calculator) + " is not supported.");
	}
	*/

	void SetField(Message *message, const FieldDescriptor *field, any &&value) {
		const auto &reflection = message->GetReflection();

		if (field->is_repeated()) {
			switch (field->cpp_type()) {
				case FieldDescriptor::CppType::CPPTYPE_INT32:
					reflection->AddInt32(message, field, any_cast<int32_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_INT64:
					reflection->AddInt64(message, field, any_cast<int64_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_UINT32:
					reflection->AddUInt32(message, field, any_cast<uint32_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_UINT64:
					reflection->AddUInt64(message, field, any_cast<uint64_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_DOUBLE:
					reflection->AddDouble(message, field, any_cast<double>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_FLOAT:
					reflection->AddFloat(message, field, any_cast<float>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_BOOL:
					reflection->AddBool(message, field, any_cast<bool>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_ENUM:
					// TODO: Also try AddEnum EnumValueDescriptor
					reflection->AddEnumValue(message, field, any_cast<int>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_STRING:
					reflection->AddString(message, field, any_cast<string>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_MESSAGE:
					reflection->AddAllocatedMessage(message, field, any_cast<Message *>(move(value)));
					break;
			}
		}
		else {
			switch (field->cpp_type()) {
				case FieldDescriptor::CppType::CPPTYPE_INT32:
					reflection->SetInt32(message, field, any_cast<int32_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_INT64:
					reflection->SetInt64(message, field, any_cast<int64_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_UINT32:
					reflection->SetUInt32(message, field, any_cast<uint32_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_UINT64:
					reflection->SetUInt64(message, field, any_cast<uint64_t>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_DOUBLE:
					reflection->SetDouble(message, field, any_cast<double>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_FLOAT:
					reflection->SetFloat(message, field, any_cast<float>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_BOOL:
					reflection->SetBool(message, field, any_cast<bool>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_ENUM:
					// TODO: Also try SetEnum EnumValueDescriptor
					reflection->SetEnumValue(message, field, any_cast<int>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_STRING:
					reflection->SetString(message, field, any_cast<string>(value));
					break;
				case FieldDescriptor::CppType::CPPTYPE_MESSAGE:
					reflection->SetAllocatedMessage(message, any_cast<Message *>(move(value)), field);
					break;
			}
		}
	}

	void SetField(Message *message, const string &field, any &&value) {
		const auto &reflection = message->GetReflection();

		unordered_map<string, const FieldDescriptor *> fields;

		{
			vector<const FieldDescriptor *> fieldsList;
			reflection->ListFields(*message, &fieldsList);

			for (const auto fieldDescriptor : fieldsList)
				fields.emplace(fieldDescriptor->name(), fieldDescriptor);
		}

		if (fields.count(field))
			SetField(message, fields.at(field), move(value));
		else if (fields.count("ext"))
			SetField(reflection->MutableMessage(message, fields.at("ext")), field, move(value));
		else
			throw out_of_range("Field \'" + field + "\' not found.");
	}
}

namespace mediapipe_solutions {

SolutionBase::SolutionBase(
	CalculatorGraphConfig graph_config,
	unordered_map<string, Any> &&side_inputs,
	vector<string> outputs,
	unordered_map<string, any> options
) {
	Init(move(graph_config), move(side_inputs), move(outputs), move(options));
}

SolutionBase::SolutionBase(
	string_view graph_config,
	unordered_map<string, Any> &&side_inputs,
	vector<string> outputs,
	unordered_map<string, any> options
) :
	SolutionBase(
		mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(string(graph_config)),
		move(side_inputs),
		move(outputs),
		move(options)
	) {
}

void SolutionBase::Init(
	CalculatorGraphConfig graph_config,
	unordered_map<string, Any> side_inputs,
	vector<string> outputs,
	unordered_map<string, any> options
) {
	ValidatedGraphConfig validated_graph_config;
	validated_graph_config.Initialize(graph_config);
	graph_config = validated_graph_config.Config();

	unordered_map<string, unordered_map<string, any>> optionsUnflattened;

	for (auto &option : options) {
		vector<string> splitName = absl::StrSplit(option.first, '.');
		optionsUnflattened[move(splitName.at(0))].emplace(move(splitName.at(1)), move(option.second));
	}

	for (auto &node : *(graph_config.mutable_node())) {
		const auto &nodeName = node.name();

		if (optionsUnflattened.count(nodeName)) {
			if (!node.has_options())
				throw out_of_range("Node is missing options.");

			auto *nodeOptions = node.mutable_options();

			for (auto &option : optionsUnflattened.at(nodeName))
				SetField(nodeOptions, option.first, move(option.second));

			optionsUnflattened.erase(nodeName);
		}
	}

	if (!optionsUnflattened.empty())
		throw out_of_range("No such node(s) exists.");

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
