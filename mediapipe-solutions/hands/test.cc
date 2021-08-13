#include <chrono>
#include <filesystem>
#include <string_view>

#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"

#include "mediapipe/framework/formats/image_frame_opencv.h"

#include "../hands/hands.h"

using namespace std;
using namespace std::chrono;
using namespace mediapipe;
using namespace mediapipe_solutions;

namespace
{
	cv::VideoCapture CreateCvCapture()
	{
		cv::VideoCapture capture;
		capture.open("/dev/video0");

		capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
		capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
		capture.set(cv::CAP_PROP_FPS, 30);
		
		if (!capture.isOpened())
			throw std::runtime_error("Video open failed.");
		
		return capture;
	}

	std::unique_ptr<mediapipe::ImageFrame> GetFrame(cv::VideoCapture &capture) {
		// Capture opencv camera or video frame.
		cv::Mat camera_frame_raw;
		capture >> camera_frame_raw;
		if (camera_frame_raw.empty()) {
			return nullptr;
		}
		cv::Mat camera_frame;
		cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
		cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);

		// Wrap Mat into an ImageFrame.
		auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
			mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
			mediapipe::ImageFrame::kDefaultAlignmentBoundary);
		cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
		camera_frame.copyTo(input_frame_mat);
		
		return input_frame;
	}

	std::string_view ToStringView(HandLandmark obj) {
		switch (obj) {
			case HandLandmark::WRIST:
				return "WRIST";
				break;
			case HandLandmark::THUMB_CMC:
				return "THUMB_CMC";
				break;
			case HandLandmark::THUMB_MCP:
				return "THUMB_MCP";
				break;
			case HandLandmark::THUMB_IP:
				return "THUMB_IP";
				break;
			case HandLandmark::THUMB_TIP:
				return "THUMB_TIP";
				break;
			case HandLandmark::INDEX_FINGER_MCP:
				return "INDEX_FINGER_MCP";
				break;
			case HandLandmark::INDEX_FINGER_PIP:
				return "INDEX_FINGER_PIP";
				break;
			case HandLandmark::INDEX_FINGER_DIP:
				return "INDEX_FINGER_DIP";
				break;
			case HandLandmark::INDEX_FINGER_TIP:
				return "INDEX_FINGER_TIP";
				break;
			case HandLandmark::MIDDLE_FINGER_MCP:
				return "MIDDLE_FINGER_MCP";
				break;
			case HandLandmark::MIDDLE_FINGER_PIP:
				return "MIDDLE_FINGER_PIP";
				break;
			case HandLandmark::MIDDLE_FINGER_DIP:
				return "MIDDLE_FINGER_DIP";
				break;
			case HandLandmark::MIDDLE_FINGER_TIP:
				return "MIDDLE_FINGER_TIP";
				break;
			case HandLandmark::RING_FINGER_MCP:
				return "RING_FINGER_MCP";
				break;
			case HandLandmark::RING_FINGER_PIP:
				return "RING_FINGER_PIP";
				break;
			case HandLandmark::RING_FINGER_DIP:
				return "RING_FINGER_DIP";
				break;
			case HandLandmark::RING_FINGER_TIP:
				return "RING_FINGER_TIP";
				break;
			case HandLandmark::PINKY_MCP:
				return "PINKY_MCP";
				break;
			case HandLandmark::PINKY_PIP:
				return "PINKY_PIP";
				break;
			case HandLandmark::PINKY_DIP:
				return "PINKY_DIP";
				break;
			case HandLandmark::PINKY_TIP:
				return "PINKY_TIP";
				break;
		}
	}
}

int main()
{
	//filesystem::path root_path = filesystem::current_path() /* TODO: [:-3] */;
	//SetResourceDir(root_path);
	
	auto capture = CreateCvCapture();
	
	Hands hands;
	
	while (true) {
		if (auto frame = GetFrame(capture)) {
			const auto results = hands.Process(move(frame));

			cout << "Frame" << endl;

			for (const auto &hand : results) {
				switch (hand.first) {
					case Handedness::LEFT:
						cout << "\t" "LEFT" << endl;
						break;
					case Handedness::RIGHT:
						cout << "\t" "RIGHT" << endl;
						break;
				}

				auto landmarkList = hand.second;

				for (int i = 0; i < landmarkList.landmark_size(); ++i) {
					const auto &landmark = landmarkList.landmark(i);

					cout << "\t\t"
						<< ToStringView(HandLandmark(i)) << " "
						<< "(" << landmark.x() << ", " << landmark.y() << ", " << landmark.z() << ")"
						<< endl;
				}
			}
		}
	}

	hands.Close();

	return EXIT_SUCCESS;
}
