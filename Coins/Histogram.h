#pragma once
#include <opencv2\core.hpp>

cv::Mat showHist(cv::Mat & frame, int chanel);

bool isOverexposed(cv::Mat& frame);