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

#include "hands.h"

#include <string_view>

#include "mediapipe/calculators/core/constant_side_packet_calculator.pb.h"

#include "mediapipe/framework/deps/file_helpers.h"
#include "mediapipe/framework/formats/classification.pb.h"

using namespace std;
using namespace mediapipe;

namespace mediapipe_solutions {

namespace {
	unordered_map<string, Any> CreateSideInputs(int max_num_hands) {
		unordered_map<string, Any> side_inputs;
		
		side_inputs.emplace("num_hands", max_num_hands);
		return side_inputs;
	}
	
	/*
	google::protobuf::Message *CreateConstantSidePacket(bool value) {
		auto result = new ConstantSidePacketCalculatorOptions::ConstantSidePacket();
		
		result->set_bool_value(value);
		return result;
	}
	*/
}

/*
Hands::Hands(
	bool static_image_mode = false, int max_num_hands = 2,
	double min_detection_confidence = 0.5, double min_tracking_confidence = 0.5
) : SolutionBase(
	BINARYPB_FILE_PATH,					// binary_graph_path
	// calculator_params
	{
		{
			"ConstantSidePacketCalculator.packet", 
			{
				ConstantSidePacketCalculatorOptions.ConstantSidePacket(
					!static_image_mode
				)
			}
		},
		{
			"palmdetectioncpu__TensorsToDetectionsCalculator.min_score_thresh",
			min_detection_confidence
		},
		{
			"handlandmarkcpu__ThresholdingCalculator.threshold",
			min_tracking_confidence
		}
	},
	{ { "num_hands", max_num_hands } },					// side_inputs
	{ { "landmarks", "handedness" } }	// outputs
	) {
}
*/

Hands::Hands(
		//bool static_image_mode,
		int max_num_hands,
		float min_detection_confidence, double min_tracking_confidence
	)
	: SolutionBase(
		string(
		"input_stream: \"input_video\""
		"output_stream: \"landmarks\""
		"node {"
		"calculator: \"HandLandmarkTrackingCpu\""
		"input_stream: \"IMAGE:input_video\""
		"input_side_packet: \"NUM_HANDS:num_hands\""
		"output_stream: \"LANDMARKS:landmarks\""
		"output_stream: \"HANDEDNESS:handedness\""
		"output_stream: \"PALM_DETECTIONS:multi_palm_detections\""
		"output_stream: \"HAND_ROIS_FROM_LANDMARKS:multi_hand_rects\""
		"output_stream: \"HAND_ROIS_FROM_PALM_DETECTIONS:multi_palm_rects\""
		"}"),
		CreateSideInputs(max_num_hands),					// side_inputs
		{ { string("landmarks"), string("handedness") } },	// outputs
		{
			//{
			//	"handlandmarktrackingcpu__ConstantSidePacketCalculator.packet",
			//	CreateConstantSidePacket(!static_image_mode)
			//},
			{
				"handlandmarktrackingcpu__palmdetectioncpu__TensorsToDetectionsCalculator.min_score_thresh",
				min_detection_confidence
			},
			{
				"handlandmarktrackingcpu__handlandmarkcpu__ThresholdingCalculator.threshold",
				min_tracking_confidence
			}
		}
	) {
}

unordered_map<Handedness, HandNormalizedLandmarkList> Hands::Process(unique_ptr<ImageFrame> image) {
	unordered_map<Handedness, HandNormalizedLandmarkList> processed;
	
	auto output = SolutionBase::Process("input_video", Any::Adopt(move(image)));
	
	if (output.count("landmarks") and output.count("handedness")) {
		auto landmarkLists = move(output.at("landmarks")).Get<vector<NormalizedLandmarkList>>();
		auto handednessLists = move(output.at("handedness")).Get<vector<ClassificationList>>();
		
		if (landmarkLists.size() != handednessLists.size())
			throw logic_error("Failed to match landmarks with hand.");
		
		for (size_t i = 0; i < handednessLists.size(); ++i) {
			processed.emplace(
				Handedness(handednessLists.at(i).classification().at(0).index()),
				move(landmarkLists.at(i))
			);
		}
	}
	
	return processed;
}

}
