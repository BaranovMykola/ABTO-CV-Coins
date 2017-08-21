#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include "GetData.h"
#include "DetectSilverColor.h"

using namespace std;
using namespace cv;

const int accuracy = 1;
const int minr = 8-accuracy;
const int maxr = 13+accuracy;
const int minDist = minr * 2;
const int dp = 1;
int highThresh = 200;
int HoughThresh = 7;

CoinsData coins_data;

#pragma region detect_circles


bool can_circle_be_coin(Point2f curr_center, float curr_rad, Mat& mat)
{
	int eps = 5;
	return !(curr_center.x - curr_rad+eps<0 || curr_center.y - curr_rad+eps<0 || curr_center.x + curr_rad-eps>mat.cols || curr_center.y + curr_rad-eps>mat.rows);
}

void check_contours(vector<pair<float, Point2f>>& circles, Point2f curr_center, float curr_rad, Mat& mat, Mat& outputImg)
{
	float diam = curr_rad * 2;
	if (diam > 15 && diam < 30 && can_circle_be_coin(curr_center, curr_rad, mat))
	{
		circles.push_back(make_pair(curr_rad, curr_center));
		circle(outputImg, curr_center, curr_rad, Scalar(255, 0, 0), -1);//draws circles on outputImg
	}
}

void printContours(Mat&tresh, Mat& outputImg,vector<vector<Point>>& contours)//only for printing during debugging
{
	cvtColor(tresh, outputImg, CV_GRAY2BGR);

	drawContours(outputImg, contours, -1, Scalar(0, 0, 255));

	cout << "drew initial contours" << endl;
}

void prepare_bin_img(Mat& source, Mat& tresh)
{
	Mat gray;
	cvtColor(source, gray, COLOR_BGR2GRAY);

	threshold(gray, tresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

	erode(tresh, tresh, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

	bitwise_not(tresh, tresh);

}

void process_contours(vector<vector<Point>>& contours, vector<pair<float, Point2f>>& circles, Mat& source, Mat& outputImg)
{
	for (int i = 0; i < contours.size(); ++i)
	{
		Point2f curr_cent;
		float curr_rad;
		minEnclosingCircle(contours[i], curr_cent, curr_rad);
		check_contours(circles, curr_cent, curr_rad, source, outputImg);
	}
	//cout << "quantity of found circles: " << circles.size() << endl;
	//waitKey();
}

vector<pair<float, Point2f>> find_circle_contours(Mat& source)
{
	Mat thresh_img(source.size(), CV_8UC1);

	prepare_bin_img(source, thresh_img);
	
	vector<vector<Point>> contours;
	findContours(thresh_img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Mat outputImg(thresh_img.size(), CV_8UC3);
	printContours(thresh_img, outputImg, contours);

	vector<pair<float, Point2f>> circles;

	process_contours(contours, circles, source, outputImg);

	//imshow("contours ", outputImg);
	//waitKey();
	return circles;
}
#pragma endregion

# pragma region detect_coin_value
void find_sum(Mat& mat, vector<pair<float, Point2f>>& circles)
{
	int sum = 0;
	Mat valuesMat = Mat::zeros(mat.size(), CV_8UC3);
	for (int i = 0; i < circles.size(); ++i)
	{
		Scalar col(255, 0, 0);
		int value = coins_data.detect_coin_value(circles[i].first);
		if (value == 10)
		{
			//cout << "value: " << value << endl;
			col = Scalar (255, 255, 0);
			sum += 10;
		}
		else
		{
			if (value == 25)
			{
				//cout << "value: " << value << endl;
				col = Scalar(0, 0, 255);
				sum += 25;
			}
			else
			{
				bool silver = is_silver(mat, circles[i].second, circles[i].first);
				if (silver)
				{
					//cout << "value: 5" << endl;
					sum += 5;
					col = Scalar(0, 255, 0);
				}
				else
				{
					//cout << "value: 50" << endl;
					sum += 50;
					col = Scalar(255, 0, 0);
				}
			}
		}
		circle(valuesMat, circles[i].second, circles[i].first, col, -1);
	}

	imshow("value", valuesMat);
	imshow("original", mat);
	waitKey();
}
#pragma endregion 

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

void segmentCoins(std::vector<std::pair<float, cv::Point2f>>& circles, cv::Mat source)
{
	Mat mask(source.size(), CV_8UC1);

	prepare_bin_img(source, mask);

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

	while (waitKey(30) != 27)
	{
		Mat dst;
		imshow("mask", mask);
		distanceTransform(mask, dst, DIST_L2, cv::DistanceTransformMasks::DIST_MASK_3, CV_32F);

		Mat dstVis;
		threshold(dst, dst, 8, 0, THRESH_TOZERO);
		normalize(dst, dstVis, 0, 1, NORM_MINMAX);
		Mat t = dst;
		//threshold(dstVis, t, 0.7, 1, THRESH_TOZERO);

		/*Mat dil;
		dilate(dstVis, dil, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
		Mat max = dil-dstVis;*/

		Mat mask;
		non_maxima_suppression(t, mask, true);

		vector<Point> lmax;
		findNonZero(mask, lmax);
		//vector<Vec3f> circles;
		Mat draw = Mat::zeros(dst.size(), CV_8UC3);
		for (auto i : lmax)
		{
			circles.push_back(make_pair(dst.at<float>(i)+1, i));
			circle(source, i, circles.back().first, Scalar(0, 0, 255), 1);
		}
		imshow("draw", draw);
		float eps = FLT_EPSILON;
		
	}

	return;
}



int main()
{
	coins_data.readData();
	Mat source;

	string ch;
	while (ch != "e")
	{
		destroyAllWindows();
		cout << ">> ";
		cin >> ch;
		if (ch != "e")
		{
			if (input(source, ch))
			{
				auto circles = find_circle_contours(source);
				segmentCoins(circles, source);
				find_sum(source, circles);
			}
			else
			{
				cout << "Error while loading file [" << ch << "]" << endl;
			}
		}
	}
	return 0;
}
