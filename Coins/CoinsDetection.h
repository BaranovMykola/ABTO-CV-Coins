#pragma once
#include <opencv2\core.hpp>


namespace HoughCircleConst
{
	static int cannyThresh = 137;
	static int bilaterialDiam = 20;
}

using namespace cv;

void mainCoinsDetection(Mat & cropped);

void houghCallBack(int, void * userData);
