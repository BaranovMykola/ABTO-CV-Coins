#include "DetectCoin.h"
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void printRadius(vector<pair<float, Point2f>>& circles, Mat& outputImg)//prints circles and radius value on binary image
{
	for (size_t i = 0; i < circles.size(); i++)
	{
		int print = circles[i].first * 100;
		putText(outputImg, to_string(print), Point(circles[i].second.x - circles[i].first, circles[i].second.y - circles[i].first), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 255, 0));
	}
}

void printContours(Mat & tresh, vector<vector<Point>>& contours, Mat& outputImg)//prints found contours on binary image
{
	cvtColor(tresh, outputImg, CV_GRAY2BGR);

	drawContours(outputImg, contours, -1, Scalar(0, 0, 255));
}

void printCircles(vector<pair<float, Point2f>>& circles, Mat& mat)//prints circles on mat
{
	for (size_t i = 0; i < circles.size(); i++)
	{
		circle(mat, circles[i].second, circles[i].first, Scalar(0, 0, 255));
	}
}

Mat remove_shades(Mat& photo)//method that tries to deal with bad background
{
	Mat res(photo.size(), CV_8UC1);
	Mat grayscale(photo.size(), CV_8UC1);
	cvtColor(photo, grayscale, CV_BGR2GRAY);
	namedWindow("trakbars");
	int size = 11;
	createTrackbar("size", "trakbars", &size, 300);
	int C = 30;
	createTrackbar("C", "trakbars", &C, 100);

	while (waitKey(100) != 27)
	{
		adaptiveThreshold(grayscale, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 2 * size + 1, C);
		imshow("output", res);
	}

	return res;
}

bool canCircleBeCoin(Point2f curr_center, float curr_rad, Size matSize)// checks whether contours intersect edges of paper
{
	int eps = 5;
	return !(curr_center.x - curr_rad + eps<0 || curr_center.y - curr_rad + eps<0 || curr_center.x + curr_rad - eps>matSize.width || curr_center.y + curr_rad - eps>matSize.height);
}

void checkContours(vector<pair<float, Point2f>>& circles, Point2f curr_center, float curr_rad, Size sourceImgSize, Mat& outputImg)//checks sizes of contours
{
	float diam = curr_rad * 2;
	if (diam > 15 && diam < 30 && canCircleBeCoin(curr_center, curr_rad, sourceImgSize))
	{
		circles.push_back(make_pair(curr_rad, curr_center));
		circle(outputImg, curr_center, curr_rad, Scalar(255, 0, 0), -1);//draws circles on outputImg
	}
}

Mat getMask(Mat& source)//returns binary mask of coins
{
	Mat thresh;
	Mat gray;
	cvtColor(source, gray, COLOR_BGR2GRAY);

	threshold(gray, thresh, 166, 255, THRESH_BINARY | THRESH_OTSU);

	erode(thresh, thresh, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

	bitwise_not(thresh, thresh);

	return thresh;
}

vector<pair<float, Point2f>> processContours(vector<vector<Point>>& contours, Size sourceImgSize, Mat& outputImg)
{
	vector<pair<float, Point2f>> circles;
	for (int i = 0; i < contours.size(); ++i)
	{
		Point2f curr_cent;
		float curr_rad;
		minEnclosingCircle(contours[i], curr_cent, curr_rad);
		checkContours(circles, curr_cent, curr_rad, sourceImgSize, outputImg);
	}
	return circles;
}

vector<pair<float, Point2f>> findCircleContours(Mat& source, Mat& outputImg)
{
	Mat thresh_img(source.size(), CV_8UC1);
	thresh_img = getMask(source);

	vector<vector<Point>> contours;
	findContours(thresh_img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	printContours(thresh_img, contours, outputImg);

	vector<pair<float, Point2f>> circles = processContours(contours, Size(source.cols, source.rows), outputImg);

	printRadius(circles, outputImg);

	printCircles(circles, source);

	return circles;
}


# pragma region detect_coin_value
void find_sum(Mat& mat, vector<pair<float, Point2f>>& circles, CoinsData& coinsData)
{
	int sum = 0;
	Mat valuesMat = Mat::zeros(mat.size(), CV_8UC3);
	for (int i = 0; i < circles.size(); ++i)
	{
		Scalar col(255, 0, 0);
		int value = coinsData.detect_coin_value(circles[i].first);
		if (value == 10)
		{
			col = Scalar(255, 255, 0);
			sum += 10;
		}
		else
		{
			if (value == 25)
			{
				col = Scalar(0, 0, 255);
				sum += 25;
			}
			else
			{
				bool silver = is_silver(mat, circles[i].second, circles[i].first);
				if (silver)
				{
					sum += 5;
					col = Scalar(0, 255, 0);
				}
				else
				{
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
bool is_silver(Mat& orig_pict, Point2f center, float radius)
{
	Mat mask = Mat::zeros(orig_pict.size(), CV_8UC1);
	circle(mask, center, radius, Scalar(255), -1);
	Mat hls_pict;
	cvtColor(orig_pict, hls_pict, CV_BGR2HLS);
	Scalar mean_val = mean(hls_pict, mask);
	return (mean_val[0] > 60);
}
#pragma endregion 