#pragma once
#include <opencv2\core.hpp>

/*@brief

Compares Point2f via x coordinate
*/
class PointComparatorX
{
public:
	bool operator()(const cv::Point2f& l, const  cv::Point2f& r);
};

