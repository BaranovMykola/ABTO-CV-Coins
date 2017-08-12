#pragma once
#include <opencv2\core.hpp>

class PointComparatorX
{
public:
	bool operator()(const cv::Point2f& l, const  cv::Point2f& r) { return l.x < r.x; };
};

