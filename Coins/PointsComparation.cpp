#include "PointsComparation.h"

bool PointComparatorX::operator()(const cv::Point2f & l, const cv::Point2f & r)
{
	return l.x < r.x;
}
