#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>

#include <vector>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <numeric>

#include "GetData.h"
#include "DetectSilverColor.h"
#include "PointsComparation.h"

#include "DetectCoin.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace std;
using namespace cv;

bool input(Mat& source, string number)
{
	destroyAllWindows();
	source = imread("../A4/" + number + "_cropped_.jpg");
	return !source.empty();
}

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
		threshold(dst, dst, 8, 0, THRESH_TOZERO);
		normalize(dst, dstVis, 0, 1, NORM_MINMAX);
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

void showHist(cv::Mat & frame)
{
	Mat planes[3];
	Mat hist[3];
	split(frame, planes);
	const int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	for (size_t i = 0; i < 3; i++)
	{
		calcHist(&planes[i], 1, 0, Mat(), hist[i], 1, &histSize, &histRange, range);
	}

	int hist_w = 1024; int hist_h = 1000;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	for (size_t i = 0; i < 3; i++)
	{
		normalize(hist[i], hist[i], 0, histImage.rows, NORM_MINMAX, -1, Mat());
	}
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[0].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[0].at<float>(i))),
			 Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[1].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[1].at<float>(i))),
			 Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[2].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[2].at<float>(i))),
			 Scalar(0, 0, 255), 2, 8, 0);
	}
	imshow("Histogram", histImage);
}


int main()
{
	Mat source;

	string ch;
	CoinsData coinsData;
	Mat outputImg;

	while (ch != "e")
	{
		destroyAllWindows();
		cout << ">> ";
		cin >> ch;
		if (ch != "e")
		{
			if (input(source, ch))
			{
				outputImg = source.clone();
				find_sum(source, findCircleContours(source, outputImg), coinsData);
			}
		}
	} 
	return 0;
}