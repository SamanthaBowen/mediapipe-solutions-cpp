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

#ifndef MEDIAPIPE_SOLUTIONS_SOLUTION_BASE_H_
#define MEDIAPIPE_SOLUTIONS_SOLUTION_BASE_H_

#include <any>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "mediapipe/framework/calculator.pb.h"
#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/packet.h"

#include "any.h"

// TODO: Document

namespace mediapipe_solutions {

class SolutionBase {
	public:
		SolutionBase(
			mediapipe::CalculatorGraphConfig graph_config,
			std::unordered_map<std::string, Any> &&side_inputs,
			std::vector<std::string> outputs
		);

		SolutionBase(
			std::string_view graph_config,
			std::unordered_map<std::string, Any> &&side_inputs,
			std::vector<std::string> outputs
		);

		void Close();
	protected:
		std::unordered_map<std::string, Any> Process(std::string_view input_stream, Any input);
		std::unordered_map<std::string, Any> Process(std::unordered_map<std::string_view, Any> &&inputs);
	private:
		mediapipe::CalculatorGraph graph_;
		std::chrono::steady_clock::time_point start_timestamp_;
		std::unordered_map<std::string, mediapipe::Packet> graph_outputs_;

		void Init(
			mediapipe::CalculatorGraphConfig graph_config,
			std::unordered_map<std::string, Any> side_inputs,
			std::vector<std::string> outputs
		);
};

}	// namespace mediapipe_solutions

#endif	// MEDIAPIPE_SOLUTIONS_SOLUTION_BASE_H_
