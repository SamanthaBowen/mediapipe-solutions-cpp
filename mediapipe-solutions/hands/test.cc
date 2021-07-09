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
				cout << "\t" "Hand" << endl;
				
				auto landmarkList = hand.second;
				for (const auto &landmark : landmarkList.landmark())
					cout << "\t\t (" << landmark.x() << ", " << landmark.y() << ", " << landmark.z() << ")" << endl;
			}
		}
	}

	hands.Close();

	return EXIT_SUCCESS;
}
