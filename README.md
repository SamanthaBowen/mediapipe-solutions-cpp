# Unofficial MediaPipe Solutions for C++

This repository is for unofficial C++ APIs for MediaPipe solutions loosely based on MediaPipe's official Python APIs. Currently, this only contains a hand tracking API.

## Getting started

You can compile the hand tracking solution and test program with `bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe-solutions:hands mediapipe-solutions:hands-test`. Depending on the platform, you may need to have the necessary MediaPipe data files located in the directory specified by the `resource_root_dir` flag.
