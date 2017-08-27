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

void printValue(Mat& mat, vector<pair<float, Point2f>>& circles, vector<int>& values)//prints value of coins onsource image
{
	for (size_t i = 0; i < values.size(); ++i)
	{
		putText(mat, to_string(values[i]), Point(circles[i].second.x- circles[i].first/2, circles[i].second.y + circles[i].first/2), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0, 255, 0));
	}
}



Mat goldContours(Mat& photo)
{
	Mat res(photo.size(), CV_8UC1);
	Mat hsv;
	vector<Mat> hsvPlanes(3);
	cvtColor(photo, hsv, CV_BGR2HSV);
	Mat hsvFiltered;
	bilateralFilter(hsv, hsvFiltered, 8, 100, 100);
	split(hsv, hsvPlanes);

	Scalar lower(10, 30, 60);
	Scalar upper(50, 130, 165);

	inRange(hsv, lower, upper, res);
	Scalar meanVal;
	meanVal = mean(hsv);
	
	return res;
}
Mat getMask1(Mat& photo)
{

	Mat filteredImg(photo.size(), photo.type());

	Mat res(photo.size(), CV_8UC1);
	bilateralFilter(photo, filteredImg, 8, 140, 140);


	Mat background = imread("../A4/14_cropped_.jpg");// get background

	cvtColor(background, background, CV_BGR2HSV);// converts background to hsv

	Mat grayscale;
	cvtColor(filteredImg, grayscale, CV_BGR2GRAY);

	vector<Mat> backgroundPlanes(3);
	split(background, backgroundPlanes);//splits background to hsv planes

	vector<Mat> originalPlanes(3);
	split(filteredImg, originalPlanes);
	vector<Mat> oneChannelDiff(3);//difference vector between background and picture 

	for (int i = 0; i < 3; ++i)
	{
		absdiff(backgroundPlanes[i], originalPlanes[i], oneChannelDiff[i]);
	}

	bitwise_not(oneChannelDiff[2], oneChannelDiff[2]);
	Mat colGrayscale;
	merge(oneChannelDiff, colGrayscale);

	cvtColor(colGrayscale, grayscale, CV_BGR2GRAY);


	Mat bin(photo.size(), CV_8UC1);
	int blockSize = 103;
	int C = 8;

	namedWindow("n");
	createTrackbar("SIZE", "n", &blockSize, 500);
	createTrackbar("C", "n", &C, 100);
	Mat otsuthreshold(photo.size(), grayscale.type());
	
	while (waitKey(100) != 27)
	{
		adaptiveThreshold(grayscale, bin, 254, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, blockSize * 2 + 1, C);
		//
		imshow("", bin);
		//morphologyEx(bin, bin, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	}
	
	return bin;
}
Mat getMask2(Mat& photo)
{

	Mat hsvPhoto;
	Mat filteredImg(photo.size(), photo.type());
	cvtColor(photo, hsvPhoto, CV_BGR2HSV);
	bilateralFilter(hsvPhoto, filteredImg, 8, 150, 150);


	Mat background = imread("../A4/14_cropped_.jpg");// get background
	Mat hsvBackground;
	cvtColor(background, background, CV_BGR2HSV);// converts background to hsv

	Mat grayscale;


	vector<Mat> backgroundPlanes(3);
	split(background, backgroundPlanes);//splits background to hsv planes

	vector<Mat> originalPlanes(3);
	split(filteredImg, originalPlanes);
	vector<Mat> oneChannelDiff(3);//difference vector between background and picture 

	for (int i = 0; i < 3; ++i)
	{
		absdiff(backgroundPlanes[0], originalPlanes[i], oneChannelDiff[i]);
	}

	Mat zeroAndTwoChannelsDiff;
	absdiff(oneChannelDiff[0], oneChannelDiff[2], zeroAndTwoChannelsDiff);
	threshold(originalPlanes[0], originalPlanes[0], 50, 255, THRESH_TRUNC );


	bitwise_not(oneChannelDiff[2], oneChannelDiff[2]);
	Mat colGrayscale;
	merge(oneChannelDiff, colGrayscale);

	Mat gold;
	absdiff(originalPlanes[2], oneChannelDiff[0], gold);
	cvtColor(colGrayscale, grayscale, CV_BGR2GRAY);


	threshold(gold, gold, 0, 255, THRESH_BINARY | THRESH_OTSU);

	filteredImg.copyTo(backgroundPlanes[0], gold);

	namedWindow("window");
	int tr1 = 35;
	int tr2 = 42;

	Mat cannied(photo.size(), CV_8UC1);
	
	Canny(backgroundPlanes[0], cannied, tr1, tr2, 3, true);
	
	erode(gold, gold, getStructuringElement(MORPH_ELLIPSE, Size(3,3)));
	bitwise_and(gold, cannied, cannied);
	return cannied;
}
Mat getMask3(Mat& photo)//method that tries to divide foreground from background
{
	Mat res(photo.size(), CV_8UC1);
	Mat hsvPict(photo.size(), CV_8UC1);
	cvtColor(photo, hsvPict, CV_BGR2HSV);



	vector<Mat> hsvVec(3);
	split(hsvPict, hsvVec);


	Mat background = imread("../A4/14_cropped_.jpg");

	cvtColor(background, background, CV_BGR2HSV);

	vector<Mat> backgroundPlanes(3);
	split(background, backgroundPlanes);


	vector<Mat> oneChannelDiff(3);

	absdiff(backgroundPlanes[0], hsvVec[2], oneChannelDiff[2]);


	Mat thirdMask = oneChannelDiff[2].clone();
	double max;
	minMaxLoc(oneChannelDiff[2], 0, &max);
	oneChannelDiff[2] *= (255 / (max));
	threshold(oneChannelDiff[2], oneChannelDiff[2], 0, 255, THRESH_BINARY | THRESH_OTSU);

	return oneChannelDiff[2];
}
Mat remove_shades(Mat& photo)//method that tries to deal with bad background and divide foreground from background
{ 
	Mat hsv = photo.clone();
 	vector<Mat> hsvPlanes(3);
	split(hsv, hsvPlanes);

	threshold(hsvPlanes[2], hsvPlanes[2], 0, 255, THRESH_BINARY | THRESH_OTSU);
	for (int i = 0; i < 3; ++i)
	{
		equalizeHist(hsvPlanes[i], hsvPlanes[i]);
	}
	merge(hsvPlanes, hsv);


	return hsv;
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
	vector<int> values;
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
					value = 5;
					col = Scalar(0, 255, 0);
				}
				else
				{
					sum += 50;
					value = 50;
					col = Scalar(255, 0, 0);
				}
			}
		}
		circle(valuesMat, circles[i].second, circles[i].first, col, -1);
		values.push_back(value);
	}

	printValue(mat, circles, values);
//	imshow("value", valuesMat);
	cv::imshow("original", mat);
	cv::waitKey();
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