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
#include "CoinsSegmentation.h"
#include "DetectCoin.h"
#include "Histogram.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace std;
using namespace cv;

bool input(Mat& source, string number)
{
	destroyAllWindows();
	source = imread("../A4/" + number + "_cropped_.jpg");
	return !source.empty();
}

void detectSilver(Mat& source)
{
	source = bilaterialBlurCoins(source);
	Mat hsl;
	Mat lab;
	cvtColor(source, hsl, CV_BGR2HLS);
	cvtColor(source, lab, CV_BGR2HLS);
	Mat pl[3];
	Mat plLab[3];
	Mat plS[3];
	split(hsl, pl);
	split(lab, plLab);
	split(source, plS);
	threshold(pl[0], pl[0], 70, 255, THRESH_BINARY);
	erode(pl[0], pl[0], getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	morphologyEx(pl[0], pl[0], MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));
	Mat backg(source.size(), CV_8UC3);
	backg = mean(source, pl[0]);
	Mat sourceclone = source.clone();
	backg.copyTo(sourceclone, pl[0]);
	Mat diff;
	absdiff(source, sourceclone, diff);
	Mat dpl[3];
	split(diff, dpl);
	Mat mix = dpl[0];
	mix += dpl[1];
	mix += dpl[2];
	//mix = bilaterialBlurCoins(mix);

	namedWindow("Panel");
	int b = 0;
	int c = 0;
	int m = 0;
	createTrackbar("b", "Panel", &b, 150);
	createTrackbar("c", "Panel", &c, 150);
	createTrackbar("m", "Panel", &m, 150);

	while (true)
	{
		Mat t = Mat::zeros(mix.size(), CV_8UC1);
		threshold(mix, t, b, 255, THRESH_BINARY);
		imshow("t", t);
		waitKey(1);
	}
	//threshold(diff, diff, 40, 255, THRESH_BINARY);
	/*morphologyEx(diff, diff, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
	morphologyEx(diff, diff, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(7, 7)));*/
}


int main()
{
	Mat source;

	string ch;
	CoinsData coinsData;
	coinsData.readData();
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
				cout << "Overexposed: " << boolalpha << isOverexposed(source) << endl;
				/*autoContrast(source);
				truncInv(source);*/
				//detectSilver(source);
				Mat segm = source.clone();
				source = bilaterialBlurCoins(source);
				Mat m = getMask(source);
				outputImg = source.clone();
				auto circles = findCircleContours(source, outputImg);

				segmentCoins(circles, source);

				find_sum(source, circles, coinsData);
			}
		}
	} 
	
	return 0;
}