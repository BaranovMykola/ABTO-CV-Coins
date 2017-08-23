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

void bBlur(Mat& source)
{
	imshow("source", source);
	Mat b;
	int d = 30;
	bilateralFilter(source, b, d, d * 2, d / 2);
	source = b.clone();
	namedWindow("b", CV_WINDOW_NORMAL);
	imshow("b", source);
	imshow("mask", getMask(source));
	waitKey();
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
				bBlur(source);
				outputImg = source.clone();
				find_sum(source, findCircleContours(source, outputImg), coinsData);
			}
		}
	} 
	return 0;
}