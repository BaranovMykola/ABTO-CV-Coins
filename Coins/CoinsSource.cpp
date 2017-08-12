#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <numeric>
#include <string>
#include <future>
#include <sstream>

#include "Line.h"
#include "CoinsDetection.h"
#include "ImagePreprocessing.h"
#include "A4CornersDetection.h"
#include "PointsComparation.h"

using namespace cv;

int distance = 29;
int minLineDist = 30;
int minGradCustom = 10;
int marginK = 10;
int bil_d = 30;
int canny_low = 80;
int hough_rho = 0;
int hough_thresh = 70;
int minGrad = 0;
int imgIndex = 7; // 8, 6 crashes





std::vector<Point> getA4Corners(Mat& input)
{
	reduceSize(input);
	Mat bilateral;
	bilateralFilter(input, bilateral, bil_d, bil_d * 2, bil_d / 2);
	input = bilateral;

	Mat imgGray;
	cvtColor(input, imgGray, COLOR_BGR2GRAY);

	Mat edges;
	std::cout << "Canny edge detection..." << std::endl;
	Canny(imgGray, edges, canny_low, canny_low * 3, 3, true);
	namedWindow("Edges", CV_WINDOW_NORMAL);
	//imshow("Edges", edges);

	std::vector<Vec2f> Houghlines;
	std::cout << "Hough transfroming..." << std::endl;
	HoughLines(edges, Houghlines, hough_rho + 1, CV_PI / 180.0, hough_thresh, 0, 0, minGrad*(CV_PI / 180));
	auto lines = getLines(Houghlines);

	Mat result = input;
	drawLines(result, lines);
	auto points = findRectangleCorners(lines, result.size(), minGradCustom);

	auto families = partitionPoints2Families(points, distance);

	for each (auto var in families)
	{
		drawPoint(var, result, Scalar::all(255));
	}

	std::sort(families.begin(), families.end(), [](std::set<Point2f,  PointComparatorX> l, std::set<Point2f,  PointComparatorX> r) { return l.size() > r.size(); });
	return accumulatePointFamilies(std::vector<std::set<Point2f,  PointComparatorX> >(families.begin(), families.begin() + PointsQuantity));
}

void changeInput(int, void* img)
{
	std::string a4 = "../A4/";
	std::string ext = ".jpg";
	std::stringstream str;
	std::string name;
	str << imgIndex;
	str >> name;
	std::string path = a4 + name + ext;
	Mat* imgMat = static_cast<Mat*>(img);
	*imgMat = imread(path);
	Mat sourceCopy = imgMat->clone();

	auto corners = getA4Corners(sourceCopy);
	Mat a4corners;

	Mat transMat = paperToRectangle(sourceCopy, corners, a4corners);
	Mat dst = cropInterestRegion(*imgMat, a4corners, corners, transMat, sourceCopy.size());

	namedWindow("Result", WINDOW_AUTOSIZE);
	imshow("Result", dst);

}

int main()
{
	std::string action;
	std::cin >> action;
	Mat source;
	const char* panel = "Preprocessing";
	/*namedWindow(panel, CV_WINDOW_NORMAL);
	createTrackbar("Img", panel, &imgIndex, 13, changeInput, &source);
	createTrackbar("B diameter", panel, &bil_d, 50, changeInput, &source);
	createTrackbar("C low", panel, &canny_low, 900, changeInput, &source);
	createTrackbar("H rho", panel, &hough_rho, 300, changeInput, &source);
	createTrackbar("H thresh", panel, &hough_thresh, 900, changeInput, &source);
	createTrackbar("H min grad", panel, &minGrad, 90, changeInput, &source);
	createTrackbar("P dist", panel, &distance, 900, changeInput, &source);
	createTrackbar("L minGradCust", panel, &minGradCustom, 90, changeInput, &source);
	createTrackbar("L marginK", panel, &marginK, 2300, changeInput, &source);
*/
	if (action == "all")
	{
		for (size_t i = 0; i < 13; i++)
		{
			imgIndex = i;
			changeInput(0, &source);
			if (waitKey() == 27)
			{
				break;
			}
		}
	}
	else
	{
		while (action != "q")
		{
			std::stringstream str;
			str << action;
			str >> imgIndex;
			changeInput(0, &source);

			if (waitKey() == 27)
			{
				break;
			}

			std::cout << "enter action: \t";
			std::cin >> action;
		}
	}
	return 0;
}