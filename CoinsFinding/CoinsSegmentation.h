#pragma once
#include <opencv2\core.hpp>

#include <vector>
#include <set>

#include "PointsComparation.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace cv;
using namespace std;

void non_maxima_suppression(const cv::Mat & src, cv::Mat & mask, const bool remove_plateaus);

bool isNearest(std::set<Point2f, PointComparatorX> points, Point2f item, int minDist);

circleType mergeNearest(circleType circles, int minDist, cv::Mat & dst);

void segmentCoins(std::vector<std::pair<float, cv::Point2f>>& circles, cv::Mat source);

cv::Mat bilaterialBlurCoins(Mat & source);
