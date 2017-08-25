#include "DetectCoin.h"
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

Mat RS(Mat& photo);

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
	//	Mat mask = filteredImg - backgroundPlanes[0];
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
	int blockSize = 1;
	int C = 1;

	namedWindow("n");
	createTrackbar("SIZE", "n", &blockSize, 500);
	createTrackbar("C", "n", &C, 100);
	//threshold(originalPlanes[0], grayB, 0, 255, THRESH_BINARY | THRESH_OTSU);
	//threshold(originalPlanes[1], grayG, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat otsuthreshold(photo.size(), grayscale.type());
	while (waitKey(100) != 27)
	{
		adaptiveThreshold(grayscale, bin, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, blockSize * 2 + 1, C);
		morphologyEx(bin, bin, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		threshold(grayscale, otsuthreshold, 0, 255, THRESH_BINARY | THRESH_OTSU);
		imshow("orig", originalPlanes[0]);
		imshow("threshold", bin);
		imshow("otsu", otsuthreshold);
	}
	return grayscale;
}
Mat getMask2(Mat& photo)
{
	return Mat();
}
Mat remove_shades(Mat& photo)//method that tries to deal with bad background and divide foreground from background
{ 
	Mat res(photo.size(), CV_8UC1);
	Mat hsvPhoto;
	Mat filteredImg(photo.size(), photo.type());
	cvtColor(photo, hsvPhoto, CV_BGR2HSV);
	bilateralFilter(hsvPhoto, filteredImg, 8, 140, 140);

	
	Mat background = imread("../A4/14_cropped_.jpg");// get background
	Mat hsvBackground;
	cvtColor(background, background, CV_BGR2HSV);// converts background to hsv
	bilateralFilter(background, hsvBackground, 4, 100, 100);

	Mat grayscale;
	

	vector<Mat> backgroundPlanes(3);
	split(background, backgroundPlanes);//splits background to hsv planes

	vector<Mat> originalPlanes(3);
	split(filteredImg, originalPlanes);
//	Mat mask = filteredImg - backgroundPlanes[0];
	vector<Mat> oneChannelDiff(3);//difference vector between background and picture 

	for (int i = 0; i < 3; ++i)
	{
		absdiff(backgroundPlanes[i], originalPlanes[i], oneChannelDiff[i]);
	}

	bitwise_not(oneChannelDiff[2], oneChannelDiff[2]);
	Mat colGrayscale;
	merge(oneChannelDiff, colGrayscale);

	Mat gold;
	absdiff(originalPlanes[2], oneChannelDiff[0], gold);
	cvtColor(colGrayscale, grayscale, CV_BGR2GRAY);

	threshold(gold, gold, 0, 255, THRESH_BINARY | THRESH_OTSU);
	vector<Mat> sharpenChannels(3);

	Mat silver;

	absdiff(originalPlanes[2], gold, silver);

	absdiff(silver, backgroundPlanes[0], silver);


	Mat empSilver;
	absdiff(silver, gold, empSilver);

	bitwise_not(empSilver, empSilver);
	
	Mat all(photo.size(), CV_8UC1);
	all = 255;

	empSilver.copyTo(all, gold);

	bitwise_and(gold, all, all);


	threshold(all, all, 0, 255, THRESH_BINARY | THRESH_OTSU);
	Mat grayB(photo.size(), CV_8UC1);
	int blockSize = 1;
	int C = 1;

	namedWindow("n");
	createTrackbar("SIZE", "n", &blockSize, 500);
	createTrackbar("C", "n", &C, 100);
	Mat otsuthreshold(photo.size(), grayscale.type());
	while (waitKey(100) != 27)
	{
		adaptiveThreshold(grayscale, grayB, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, blockSize*2+1, C);
		morphologyEx(grayB, grayB, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		threshold(grayscale, otsuthreshold, 0, 255, THRESH_BINARY | THRESH_OTSU);
		imshow("orig", originalPlanes[0]);
		imshow("threshold", grayB);
		imshow("otsu", otsuthreshold);
	}
	/*namedWindow("window");
	int tr1 = 1;
	int tr2 = 10;
	createTrackbar("tr1", "window", &tr1, 500);
	createTrackbar("tr2", "window", &tr2, 500);
	Mat cannied(photo.size(), CV_8UC1);
	while (waitKey(100) != 27)
	{

	Canny(oneChannelDiff[2], cannied, tr1, tr2, 7);
	imshow("win", grayB);
	imshow("w", cannied);
	}*/

	Mat hsvPict(photo.size(), CV_8UC1);

	

	vector<Mat> hsvVec(3);// splits hsv to three channels
	split(hsvPict, hsvVec);
	Mat binary(photo.size(), CV_8UC1);
	Mat goldCoinsMask;

	threshold(originalPlanes[0], goldCoinsMask, 0, 255, THRESH_BINARY | THRESH_OTSU);


	bitwise_not(goldCoinsMask, goldCoinsMask);
	double meanVal = mean(grayscale, goldCoinsMask)[0];

	Mat pictWithoutGoldCoins; 

	grayscale.copyTo(pictWithoutGoldCoins, goldCoinsMask);
	
	Mat averageValMat(filteredImg.size(), CV_8UC1);
	averageValMat = meanVal;
	bitwise_not(goldCoinsMask, goldCoinsMask);

	//dilate(goldCoinsMask, goldCoinsMask, Mat());
	Mat IHopeRes;
	absdiff(oneChannelDiff[0], oneChannelDiff[2], IHopeRes);

	threshold(IHopeRes, IHopeRes, 0, 255, THRESH_BINARY | THRESH_OTSU);
	averageValMat.copyTo(grayscale, goldCoinsMask);
	bitwise_not(IHopeRes, IHopeRes);

	bitwise_or(IHopeRes, goldCoinsMask, res);
	//threshold(grayscale, grayscale, 0, 255, THRESH_BINARY | THRESH_OTSU);
		return res;
}

Mat RS(Mat& photo)//method that tries to deal with bad background and divide foreground from background
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
	//	absdiff(backgroundPlanes[0], hsvVec[0], oneChannelDiff[0]);
	//	absdiff(backgroundPlanes[0], hsvVec[1], oneChannelDiff[1]);
	absdiff(backgroundPlanes[0], hsvVec[2], oneChannelDiff[2]);

	//	Mat tryGetSilverCoins;
	//	absdiff(oneChannelDiff[2], backgroundPlanes[2], tryGetSilverCoins);
	//tryGetSilverCoins = oneChannelDiff[2] - backgroundPlanes[2];

	//		Mat firstMask;
	//		threshold(hsvVec[0], firstMask, 0, 255, THRESH_BINARY | THRESH_OTSU);

	Mat thirdMask = oneChannelDiff[2].clone();
	double max;
	minMaxLoc(oneChannelDiff[2], 0, &max);
	oneChannelDiff[2] *= (255 / (max));
	threshold(oneChannelDiff[2], oneChannelDiff[2], 0, 255, THRESH_BINARY | THRESH_OTSU);



	//	bitwise_and(firstMask, thirdMask, res);

	return oneChannelDiff[2];
}

	/*vector<Mat> vecBin(3);
	Mat matBin(photo.size(), CV_8UC1);
	threshold(oneChannelDiff[0], vecBin[0], 0, 255, THRESH_OTSU | THRESH_BINARY);
	threshold(oneChannelDiff[1], vecBin[1], 0, 255, THRESH_OTSU | THRESH_BINARY);
	threshold(oneChannelDiff[2], vecBin[2], 0, 255, THRESH_OTSU | THRESH_BINARY);

	merge(vecBin, matBin);


	Mat grayscaleDiffMerged(photo.size(), CV_8UC3);
	merge(oneChannelDiff, grayscaleDiffMerged);

	Mat reallyGrayscale(photo.size(), CV_8UC1);
	cvtColor(grayscaleDiffMerged, reallyGrayscale, CV_BGR2GRAY);

	Mat threshedDiffMat(photo.size(), CV_8UC1);

	threshold(reallyGrayscale, threshedDiffMat, 0, 255, THRESH_OTSU | THRESH_BINARY);
*/

	//cvtColor(background, background, CV_BGR2GRAY);
	//Mat firstDifference(photo.size(), CV_8UC1);
	//firstDifference = (grayscale - background);

	//Mat secondDifference(photo.size(), CV_8UC1);
	//secondDifference = (background - grayscale);

	//Mat binaryFirst(photo.size(), CV_8UC1);
	//Mat binarySecond(photo.size(), CV_8UC1);

	//threshold(firstDifference, binaryFirst, 0, 255, THRESH_BINARY|THRESH_OTSU);
	//threshold(secondDifference, binarySecond, 0, 255, THRESH_BINARY|THRESH_OTSU);

	//Mat sumGrayscale = firstDifference + secondDifference;
	//Mat sumBin = binaryFirst + binarySecond;

	/*namedWindow("trakbars");
	int size = 11;
	createTrackbar("size", "trakbars", &size, 300);
	int C = 30;
	createTrackbar("C", "trakbars", &C, 100);

	while (waitKey(100) != 27)
	{
		adaptiveThreshold(grayscale, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 2 * size + 1, C);
		imshow("output", res);
	}
	*/


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