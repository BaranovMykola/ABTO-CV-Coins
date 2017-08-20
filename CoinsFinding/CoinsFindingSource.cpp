#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <vector>
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

bool check_paper_edges(Point2f curr_center, float curr_rad, Mat& mat)
{
	int eps = 5;
	return !(curr_center.x - curr_rad+eps<0 || curr_center.y - curr_rad+eps<0 || curr_center.x + curr_rad-eps>mat.cols || curr_center.y + curr_rad-eps>mat.rows);
}

void check_contours(vector<Point2f>& centers, vector<float>& rad, Point2f curr_center, float curr_rad, Mat& mat, Mat& outputImg)
{
	if (curr_rad * 2 > 15 && curr_rad * 2 < 30 && check_paper_edges(curr_center, curr_rad, mat))
	{
		centers.push_back(curr_center);
		rad.push_back(curr_rad);
		circle(outputImg, curr_center, curr_rad, Scalar(255, 0, 0), -1);
		splitToHls(mat);
		/*
		imshow("found contours", outputImg);
		if (coin_hist(mat, curr_center, curr_rad) != true)
		{
			cout<<"value: "<< coins_data.detect_coin(curr_rad)<<endl;
		}
		waitKey();*/
	}
}


void find_circle_contours(Mat source)
{

	Mat gray;
	cvtColor(source, gray, COLOR_BGR2GRAY);

	Mat tresh(source.size(), CV_8UC1);
	threshold(gray, tresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

	erode(tresh, tresh, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

	bitwise_not(tresh, tresh);

	vector<vector<Point>> contours;
	findContours(tresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Mat outputImg(tresh.size(), CV_8UC3);//only to print

	cvtColor(tresh, outputImg, CV_GRAY2BGR);

	drawContours(outputImg, contours, -1, Scalar(0, 0, 255));

	vector<Point2f> centers;
	vector<float> rad;

	for (int i = 0; i < contours.size(); ++i)
	{
		Point2f curr_cent;
		float curr_rad;
		minEnclosingCircle(contours[i], curr_cent, curr_rad);
		check_contours(centers, rad, curr_cent, curr_rad, source, outputImg);
	}

	cout << "quantity of found circles: " << rad.size() << endl;
	waitKey();

}

void input(Mat& source)
{
	destroyAllWindows();
	string name;
	cin >> name;
	source = imread("../A4/" + name + "_cropped_.jpg");
	imshow("source", source);
	waitKey();
}

int main()
{
	coins_data.readData();
	Mat source;
	input(source);

	
	int ch = 0;
	while (ch != 27)
	{
		ch = waitKey(1);
		if (ch == 32)
		{
			input(source);
			//find_circle_contours(source);
			//splitToLab(source);
			splitToHls(source);
		}
	//coins_data.writeData();
	}

	for each (CoinRadius i in coins_data.get_coins())
	{
		cout << i;
	}

	system("pause");
}
