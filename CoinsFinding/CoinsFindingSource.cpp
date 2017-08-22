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

#pragma region detect_circles

Mat remove_shades(Mat& photo)
{	
	Mat res(photo.size(), CV_8UC1);
	/*Mat hsvMat(photo.size(), photo.type());
	cvtColor(photo, hsvMat, CV_BGR2HSV);

	vector<Mat> planes;
	split(hsvMat, planes);
	
	threshold(planes[2], planes[2], 128, 128, THRESH_TRUNC);

	threshold(planes[1], planes[1], 90, 90, THRESH_TRUNC);
	Mat hsvChanged;
	merge(planes, hsvChanged);
	cvtColor(hsvChanged, res, CV_HSV2BGR);
	Mat inverted = Scalar(255, 255, 255) - res;*/
	Mat grayscale (photo.size(), CV_8UC1);
	cvtColor(photo, grayscale, CV_BGR2GRAY);
	adaptiveThreshold(grayscale, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 3);
	return res;
}

bool can_circle_be_coin(Point2f curr_center, float curr_rad, Mat& mat)
{
	int eps = 5;
	return !(curr_center.x - curr_rad+eps<0 || curr_center.y - curr_rad+eps<0 || curr_center.x + curr_rad-eps>mat.cols || curr_center.y + curr_rad-eps>mat.rows);
}

void check_contours(vector<pair<float, Point2f>>& circles, Point2f curr_center, float curr_rad, Mat& mat, Mat& outputImg)//
{
	float diam = curr_rad * 2;
	if (diam > 15 && diam < 30 && can_circle_be_coin(curr_center, curr_rad, mat))
	{
		circles.push_back(make_pair(curr_rad, curr_center));
		circle(outputImg, curr_center, curr_rad, Scalar(255, 0, 0), -1);//draws circles on outputImg
	}
}

void printContours(Mat&tresh, Mat& outputImg,vector<vector<Point>>& contours)//prints found contours
{
	cvtColor(tresh, outputImg, CV_GRAY2BGR);

	drawContours(outputImg, contours, -1, Scalar(0, 0, 255));

	cout << "drew initial contours" << endl;
}

Mat getMask(Mat& source)//returns binary mask of coins
{
	Mat thresh;
	Mat gray;
	cvtColor(source, gray, COLOR_BGR2GRAY);

	threshold(gray, thresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

	erode(thresh, thresh, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

	bitwise_not(thresh, thresh);

	return thresh;
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
}

vector<pair<float, Point2f>> find_circle_contours(Mat& source)
{
	Mat thresh_img(source.size(), CV_8UC1);

	thresh_img = getMask(source);
	
	vector<vector<Point>> contours;
	findContours(thresh_img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Mat outputImg(thresh_img.size(), CV_8UC3);
	printContours(thresh_img, outputImg, contours);

	

	vector<pair<float, Point2f>> circles;

	process_contours(contours, circles, source, outputImg);


	for (size_t i = 0; i < circles.size(); i++)
	{
		int print = circles[i].first * 100;
		putText(outputImg, to_string(print), Point(circles[i].second.x - circles[i].first, circles[i].second.y - circles[i].first), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 255, 0));
	}

	for (size_t i = 0; i < circles.size(); i++)
	{
		circle(source, circles[i].second, circles[i].first, Scalar(0, 0, 255));
	}
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
			cout << "value: " << value << endl;
			col = Scalar (255, 255, 0);
			sum += 10;
		}
		else
		{
			if (value == 25)
			{
				cout << "value: " << value << endl;
				col = Scalar(0, 0, 255);
				sum += 25;
			}
			else
			{
				bool silver = is_silver(mat, circles[i].second, circles[i].first);
				if (silver)
				{
					cout << "value: 5" << endl;
					sum += 5;
					col = Scalar(0, 255, 0);
				}
				else
				{
					cout << "value: 50" << endl;
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

void input(Mat& source)
{
	destroyAllWindows();
	cout << "enter number of picture:" << endl;
	string name;
	cin >> name;
	source = imread("../A4/" + name + "_cropped_.jpg");
	//imshow("source", source);
	// waitKey();
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
