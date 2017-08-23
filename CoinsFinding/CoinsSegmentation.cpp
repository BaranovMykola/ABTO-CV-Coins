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

void non_maxima_suppression(const cv::Mat& src, cv::Mat& mask, const bool remove_plateaus)
{
	// find pixels that are equal to the local neighborhood not maximum (including 'plateaus')
	cv::dilate(src, mask, cv::Mat());
	cv::compare(src, mask, mask, cv::CMP_GE);

	// optionally filter out pixels that are equal to the local minimum ('plateaus')
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
		for (auto j : families)
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
		average.push_back(make_pair(r+1, sum));
	}
	return average;
}


void segmentCoins(std::vector<std::pair<float, cv::Point2f>>& circles, cv::Mat source)
{
	Mat mask(source.size(), CV_8UC1);

	/*threshold(source, mask, 170, 255, THRESH_BINARY | CV_ADAPTIVE_THRESH_MEAN_C);
	Mat bm;
	cvtColor(mask, bm, CV_BGR2GRAY);
	threshold(bm, mask, 255 / 3, 255, THRESH_BINARY);
	threshold(bm, mask, 255 / 3, 255, THRESH_BINARY_INV);*/
	mask = getMask(source);

	for (auto i : circles)
	{
		circle(mask, i.second, i.first, Scalar::all(0), -1);
	}

	int morphSize = 8;
	int cTresh = 200;
	int hTresh = 9;
	int dp = 1;
	int bh = 1;

	namedWindow("Panel");
	createTrackbar("Morph", "Panel", &morphSize, 40);
	createTrackbar("bh", "Panel", &bh, 40);
	createTrackbar("dp", "Panel", &dp, 10);
	createTrackbar("cThresh", "Panel", &cTresh, 256);
	createTrackbar("hTresh", "Panel", &hTresh, 90);

	//namedWindow("hough", CV_WINDOW_NORMAL);

	circleType additional;
	while (waitKey(30) != 27)
	{
		Mat dst;
		imshow("maskLocal", mask);
		distanceTransform(mask, dst, DIST_L2, cv::DistanceTransformMasks::DIST_MASK_3, CV_32F);

		Mat dstVis;
		normalize(dst, dstVis, 0, 1, NORM_MINMAX);
		threshold(dst, dst, 8, 0, THRESH_TOZERO);
		Mat t = dst;
		//threshold(dstVis, t, 0.7, 1, THRESH_TOZERO);

		/*Mat dil;
		dilate(dstVis, dil, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
		Mat max = dil-dstVis;*/

		Mat maskLocal;
		non_maxima_suppression(t, maskLocal, true);

		vector<Point> lmax;
		findNonZero(maskLocal, lmax);
		//vector<Vec3f> circles;
		Mat draw = source.clone();
		additional.clear();
		for (auto i : lmax)
		{
			additional.push_back(make_pair(dst.at<float>(i), i));
		}

		auto filtered = mergeNearest(additional, 16, dst);
		additional.clear();
		additional = filtered;

		for (auto i : filtered)
		{
			//additional.push_back(make_pair(dst.at<float>(i), i));
			circle(draw, i.second, i.first, Scalar(0, 0, 255), 1);
		}

		imshow("draw", draw);

	}
	circles.insert(circles.end(), additional.begin(), additional.end());

	return;
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

void overexposedThresh(Mat& source)
{
	int thr = 100;
	int thr2 = 100;
	namedWindow("panel");
	createTrackbar("thr", "panel", &thr, 255);
	createTrackbar("thr2", "panel", &thr2, 255);
	while (waitKey(30) != 27)
	{
		Mat t;
		threshold(source, t, thr, 255, THRESH_TRUNC);
		Mat diff;
		absdiff(t, source, diff);
		threshold(diff, diff, thr2, 255, THRESH_BINARY);
		imshow("t", diff);
	}
}