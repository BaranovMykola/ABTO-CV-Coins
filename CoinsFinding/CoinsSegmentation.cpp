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
	cv::dilate(src, mask, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
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
		average.push_back(make_pair(r+2, sum));
	}
	return average;
}

void customLocalMax(cv::Mat& dist, Mat& mask)
{
	Mat k = (Mat_<float>(Size(5, 5)) << -0.5, -0.5, -0.5, -0.5,-0.5,
			 -0.5, -1, -1, -1, -0.5,
			 -0.5, -1, 16, -1, -0.5,
			 -0.5, -1, -1, -1, -0.5,
			 -0.5, -0.5, -0.5, -0.5, -0.5);
	mask = Mat::zeros(dist.size(), CV_32F);
	filter2D(dist, mask, mask.depth(), k);
}

circleType mergeRemote(circleType& circles, int minDist, cv::Mat& dst)
{
	vector<int> votes(circles.size());
	for (int i = 0; i < circles.size(); i++)
	{
		for (int j = 0; j < circles.size(); j++)
		{
			float perc = 0.1;
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

	/*threshold(source, mask, 170, 255, THRESH_BINARY | CV_ADAPTIVE_THRESH_MEAN_C);
	Mat bm;
	cvtColor(mask, bm, CV_BGR2GRAY);
	threshold(bm, mask, 255 / 3, 255, THRESH_BINARY);
	threshold(bm, mask, 255 / 3, 255, THRESH_BINARY_INV);*/
	//mask = getMask(source);
	Mat clone = source.clone();
	autoContrast(clone);
	mask = truncInv(clone);

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
		Mat customMask;
		customLocalMax(t, customMask);

		vector<Point> lmax;
		findNonZero(maskLocal, lmax);
		//vector<Vec3f> circles;
		Mat draw = source.clone();
		additional.clear();
		for (auto i : lmax)
		{
			additional.push_back(make_pair(dst.at<float>(i), i));
		}

		auto filtered = mergeNearest(additional, 8, dst);
		Mat full = draw.clone();
		for (auto i : additional)
		{
			circle(full, i.second, i.first, Scalar(0, 0, 255), 1);
		}

		Mat fullFiltered = draw.clone();
		for (auto i : filtered)
		{
			circle(fullFiltered, i.second, i.first, Scalar(0, 0, 255), 1);
		}
		mergeRemote(filtered, 3, full);
		additional.clear();
		additional = filtered;
		

		for (auto i : filtered)
		{
			//additional.push_back(make_pair(dst.at<float>(i), i));
			circle(draw, i.second, i.first, Scalar(0, 0, 255), 1);
		}
		namedWindow("draw", CV_WINDOW_NORMAL);
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


void autoContrast(Mat& source)
{
	Mat pl[3];
	split(source, pl);
	for (int i = 0; i < 3; i++)
	{
		double max;
		minMaxLoc(pl[i], 0, &max, 0, 0);
		if (max > 200)
		{
			pl[i] -= 80;
			max = 200;
		}
		pl[i] *= 255 / max;
	}
	merge(pl, 3, source);
}

Mat truncInv(Mat& source)
{
	double max;
	Scalar av = mean(source);
	minMaxLoc(source, 0, &max, 0, 0);
	for (int i = 0; i < source.rows; i++)
	{
		uchar* row = source.ptr<uchar>(i);
		for (int j = 0; j < source.cols; j++)
		{
			for (int c = 0; c < source.channels(); c++)
			{
				//row[j+c]
				if (row[j * 3 + c] > av[c] * 2 && i > 0 && j > 0 && i + 2<source.rows && j + 2 < source.cols)
				{
					cv::Point point(j, i);
					Mat onePixelSourceROI = source(cv::Rect(point - Point(1, 1), cv::Size(3, 3)));

					Mat dest;
					Mat k = Mat::zeros(Size(3, 3), CV_8UC1);
					cv::filter2D(onePixelSourceROI,
								 dest,
								 CV_8UC3,
								 k,
								 cv::Point(-1, -1),
								 0,
								 cv::BORDER_CONSTANT);
					dest.copyTo(onePixelSourceROI);
				}
			}
		}
	}
	Mat res = source;
	morphologyEx(source, res, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	Mat tre;
	Mat pl[3];
	split(res, pl);
	Mat adt[3];
	for (int i = 0; i < 3; i++)
	{
		adaptiveThreshold(pl[i], adt[i], 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 155, 0);
	}
	tre = adt[0] + adt[1] + adt[2];
	return tre;
}