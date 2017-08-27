#pragma once
#include <opencv2\core.hpp>

#include <vector>
#include <set>

#include "PointsComparation.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace cv;
using namespace std;

/*@brief

Finds local maximum
*/
void localMax(const cv::Mat & src, cv::Mat & mask, const bool remove_plateaus);

/*@brief

Find the most closed point
*/
bool isNearest(std::set<Point2f, PointComparatorX> points, Point2f item, int minDist);

/*@brief

Merges nearest circles
*/
circleType mergeNearest(circleType circles, int minDist, cv::Mat & dst);

/*@brief 

Finds coins by segmentation algorithm
*/
void segmentCoins(std::vector<std::pair<float, cv::Point2f>>& circles, cv::Mat source);

/*@brief

Blurs coins without bluring edges
*/
cv::Mat bilaterialBlurCoins(Mat & source);