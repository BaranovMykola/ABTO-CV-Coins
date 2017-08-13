#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

const int accuracy = 1;
const int minr = 8-accuracy;
const int maxr = 13+accuracy;
const int minDist = minr * 2;
const int dp = 1;
int highThresh = 200;
int HoughThresh = 7;

void input(Mat& source)
{
	destroyAllWindows();
	string name;
	cin >> name;
	source = imread("../A4/" + name + "_cropped_.jpg");
	imshow("source", source);
	namedWindow("draw", CV_WINDOW_NORMAL);
	createTrackbar("highThresh", "draw", &highThresh, 700);
	createTrackbar("HoughThresh", "draw", &HoughThresh, 300);
	Scalar averge = mean(source);
	cout << "Mean = " << averge << std::endl;
	Mat channels[3];
	split(source, channels);
	for (size_t i = 0; i < 3; i++)
	{
		threshold(channels[i], channels[i], averge[i], 255, CV_THRESH_TOZERO);
	}
	merge(channels, 3, source);

}

int main()
{
	Mat source;
	input(source);

	
	int ch = 0;
	while (ch != 27)
	{
		ch = waitKey(1);
		if (ch == 32)
		{
			input(source);
		}
		vector<Vec3f> circles;
		Mat sourceGray;
		cvtColor(source, sourceGray, CV_BGR2GRAY);
		HoughCircles(sourceGray, circles, CV_HOUGH_GRADIENT, dp, minDist, highThresh, HoughThresh, minr, maxr);
		Mat draw = source.clone();
		for (auto i : circles)
		{
			circle(draw, Point(i[0], i[1]), i[2], Scalar(0, 0, 255), 1);
		}
		imshow("draw", draw);
	}
}