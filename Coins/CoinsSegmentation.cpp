#include "CoinsSegmentation.h"
#include <opencv2\imgproc.hpp>
#include <set>
#include <numeric>
#include <opencv2\highgui.hpp>

#include "PointsComparation.h"
#include "DetectCoin.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace cv;
using namespace std;

void localMax(const cv::Mat& src, cv::Mat& mask, const bool remove_plateaus)
{
	cv::dilate(src, mask, getStructuringElement(MORPH_ELLIPSE, Size(3,3)));
	cv::compare(src, mask, mask, cv::CMP_GE);

	if (remove_plateaus)
	{
		cv::Mat non_plateau_mask;
		cv::erode(src, non_plateau_mask, cv::Mat());
		cv::compare(src, non_plateau_mask, non_plateau_mask, cv::CMP_GT);
		cv::bitwise_and(mask, non_plateau_mask, mask);
	}
}

bool isNearest(std::set<Point2f, PointComparatorX> points, Point2f item, int minDist)
{
	bool all = std::all_of(points.begin(), points.end(), [&](Point2f i) { return cv::norm(i - item) < minDist; });
	return all;
}

circleType mergeNearest(circleType circles, int minDist, cv::Mat& dst)
{
	std::vector<std::set<Point2f, PointComparatorX>> families;
	for (auto i : circles)
	{
		bool inserted = false;
		for (auto& j : families)
		{
			if (isNearest(j, i.second, minDist))
			{
				j.insert(i.second);
				inserted = true;
				break;
			}
		}
		if (!inserted)
		{
			families.push_back(std::set<Point2f, PointComparatorX>{i.second});
		}
	}

	circleType average;

	for (auto i : families)
	{
		Point2f sum;
		sum = std::accumulate(i.begin(), i.end(), Point2f());
		sum /= (float)i.size();
		float r;
		auto rIt = std::max_element(i.begin(), i.end(), [&](Point2f l, Point2f r) { return dst.at<float>(l) < dst.at<float>(r); });
		r = dst.at<float>(*rIt);
		average.push_back(make_pair(r+0.5, sum));
	}
	return average;
}

circleType mergeRemote(circleType& circles, int minDist, cv::Mat& dst)
{
	vector<int> votes(circles.size());
	for (int i = 0; i < circles.size(); i++)
	{
		for (int j = 0; j < circles.size(); j++)
		{
			float perc = 0.2;
			float dist = norm(circles[i].second - circles[j].second);
			if (dist > circles[i].first*(1-perc) && 
				dist < circles[i].first*(1 + perc)
				|| 
				dist > circles[j].first*(1 - perc) &&
				dist < circles[j].first*(1 + perc))
			{
				votes[i]++;
				votes[j]++;
			}
		}
	}

	for (int i = 0;i < votes.size();++i)
	{
		if (votes[i] >= 4)
		{
			circles.erase(circles.begin() + i);
			votes.erase(votes.begin() + i);
			i = 0;
		}
	}
	return circleType();
}

void segmentCoins(std::vector<std::pair<float, cv::Point2f>>& circles, cv::Mat source)
{
	Mat mask(source.size(), CV_8UC1);
	mask = getMask(source);

	for (auto i : circles)
	{	//remove explored circles from mask
		circle(mask, i.second, i.first, Scalar::all(0), -1);
	}

	
	circleType additional;
	Mat dst;
	distanceTransform(mask, dst, DIST_L2, cv::DistanceTransformMasks::DIST_MASK_3, CV_32F);

	threshold(dst, dst, 8, 0, THRESH_TOZERO);

	Mat maskLocal;
	localMax(dst, maskLocal, true);
	Mat customMask;
	customLocalMax(dst, customMask);

	vector<Point> lmax;
	findNonZero(maskLocal, lmax);
	
	for (auto i : lmax)
	{
		additional.push_back(make_pair(dst.at<float>(i), i));
	}

	auto filtered = mergeNearest(additional, 8, dst); // merge neares circles
	mergeRemote(filtered, 3, Mat()); // merge remoted circle (vote algorithm)
	additional.clear();

	for (auto i : filtered)
	{
		if (canCircleBeCoin(i.second, i.first, mask.size()))
		{
			additional.push_back(i);
		}
	}

	circles.insert(circles.end(), additional.begin(), additional.end());
}

cv::Mat bilaterialBlurCoins(Mat& source)
{
	Mat b;
	Mat blured = source.clone();
	int d = 10;
	for (int i = 0; i < 3; i++)
	{

		bilateralFilter(blured, b, d, d * 2, d / 2);
		blured = b.clone();
	}
	return blured;
}