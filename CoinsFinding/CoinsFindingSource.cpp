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

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace std;
using namespace cv;

bool input(Mat& source, string number)
{
	destroyAllWindows();
	source = imread("../A4/" + number + "_cropped_.jpg");
	return !source.empty();
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
				Mat segm = source.clone();
				source = bilaterialBlurCoins(source);
				outputImg = source.clone();
				auto circles = findCircleContours(source, outputImg);

				segmentCoins(circles, source);

				find_sum(source, circles, coinsData);
			}
		}
	} 
	return 0;
}