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

#ifndef MEDIAPIPE_SOLUTIONS_SOLUTIONS_HANDS_H_
#define MEDIAPIPE_SOLUTIONS_SOLUTIONS_HANDS_H_

#include "../solution_base.h"

#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/landmark.pb.h"

namespace mediapipe_solutions {

enum class HandLandmark {
	WRIST = 0,
	THUMB_CMC = 1,
	THUMB_MCP = 2,
	THUMB_IP = 3,
	THUMB_TIP = 4,
	INDEX_FINGER_MCP = 5,
	INDEX_FINGER_PIP = 6,
	INDEX_FINGER_DIP = 7,
	INDEX_FINGER_TIP = 8,
	MIDDLE_FINGER_MCP = 9,
	MIDDLE_FINGER_PIP = 10,
	MIDDLE_FINGER_DIP = 11,
	MIDDLE_FINGER_TIP = 12,
	RING_FINGER_MCP = 13,
	RING_FINGER_PIP = 14,
	RING_FINGER_DIP = 15,
	RING_FINGER_TIP = 16,
	PINKY_MCP = 17,
	PINKY_PIP = 18,
	PINKY_DIP = 19,
	PINKY_TIP = 20,
};

enum class Handedness {
	LEFT = 0,
	RIGHT = 1,
};

class HandNormalizedLandmarkList : public mediapipe::NormalizedLandmarkList {
	public:
		explicit HandNormalizedLandmarkList(const mediapipe::NormalizedLandmarkList &other);
		explicit HandNormalizedLandmarkList(mediapipe::NormalizedLandmarkList &&other);

		const mediapipe::NormalizedLandmark &landmark(HandLandmark handLandmark) const;

		using mediapipe::NormalizedLandmarkList::landmark;
};

class Hands : public SolutionBase {
	public:
		/*Hands(
			bool static_image_mode = false, int max_num_hands = 2,
			double min_detection_confidence = 0.5, double min_tracking_confidence = 0.5
		);*/
		
		Hands(int max_num_hands = 2);

		std::unordered_map<Handedness, HandNormalizedLandmarkList> Process(std::unique_ptr<mediapipe::ImageFrame> image);
};

inline HandNormalizedLandmarkList::HandNormalizedLandmarkList(const mediapipe::NormalizedLandmarkList &other) : NormalizedLandmarkList(other) {
}

inline HandNormalizedLandmarkList::HandNormalizedLandmarkList(mediapipe::NormalizedLandmarkList &&other) : NormalizedLandmarkList(std::move(other)) {
}

inline const mediapipe::NormalizedLandmark &HandNormalizedLandmarkList::landmark(HandLandmark handLandmark) const {
	return landmark(int(handLandmark));
}

}

#endif // MEDIAPIPE_SOLUTIONS_SOLUTIONS_HANDS_H_
