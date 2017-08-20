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

void find_sum(Mat& mat, vector<pair<float, Point2f>>& circles)
{
	Mat valuesMat = Mat::zeros(mat.size(), CV_8UC1);
	for (int i = 0; i < circles.size(); ++i)
	{
		cout<<coins_data.detect_coin_value(circles[i].first)<<endl;
		circle(valuesMat, circles[i].second, circles[i].first, 255, -1);
		imshow("value", valuesMat);
		imshow("original", mat);
		waitKey();
	}
}

void input(Mat& source)
{
	destroyAllWindows();
	cout << "enter number of picture:" << endl;
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
	
	char ch = 0;
	do
	{
		cout << "to exit enter 'e' to input picture enter 'p'" << endl;
		cin >> ch;
		if (ch == 'p')
		{
			input(source);
			find_sum(source, find_circle_contours(source));
		}
	} while (ch != 'e');

	system("pause");
}
