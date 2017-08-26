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
#include <iterator>

#include "Line.h"
#include "ImagePreprocessing.h"
#include "A4CornersDetection.h"
#include "PointsComparation.h"
#include "PaperReconstruction.h"
#include "CoinsDetection.h"

using namespace cv;

int distance = 60;
int minLineDist = 30;
int minGradLinesOverlap = 10;
int bilaterialDiam = 30;
int cannyThresh = 80;
int houghThresh = 70;
int minGradLines = 0;
int imgIndex = 7; // 8, 6 crashes





std::vector<Point> getA4Corners(Mat& input,
								int bilateralDiam, 
								int cannyThresh, 
								int houghThresh, 
								int minGradLines, 
								int minGradLinesOverlap, 
								int minPointDistance)
{
	reduceSize(input);
	Mat bilateral;
	bilateralFilter(input, bilateral, bilateralDiam, bilateralDiam * 2, bilateralDiam / 2);
	bilateral;

	Mat imgGray;
	cvtColor(bilateral, imgGray, COLOR_BGR2GRAY);

	Mat edges;
	std::cout << "Canny edge detection..." << std::endl;
	Canny(imgGray, edges, cannyThresh, cannyThresh * 3, 3, true);
	//namedWindow("Edges", CV_WINDOW_NORMAL);
	//imshow("Edges", edges);

	std::vector<Vec2f> Houghlines;
	std::cout << "Hough transfroming..." << std::endl;
	HoughLines(edges, Houghlines, 1, CV_PI / 180.0, houghThresh, 0, 0, minGradLines*(CV_PI / 180));
	auto lines = getLines(Houghlines);

	//Mat result = input;
	drawLines(input, lines);
	auto points = findRectangleCorners(lines, input.size(), minGradLinesOverlap);

	auto families = partitionPoints2Families(points, minPointDistance);

	for each (auto var in families)
	{
		drawPoint(var, input, Scalar::all(255));
	}

	std::sort(families.begin(), families.end(), [](std::set<Point2f,  PointComparatorX> l, std::set<Point2f,  PointComparatorX> r) { return l.size() > r.size(); });
	return accumulatePointFamilies(std::vector<std::set<Point2f,  PointComparatorX> >(families.begin(), families.begin() + A4CornersCount));
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

	auto corners = getA4Corners(sourceCopy, bilaterialDiam, cannyThresh, houghThresh, minGradLines, minGradLinesOverlap, ::distance);
	std::vector<Point> sourceCorners;
	float k = imgMat->size().width / sourceCopy.cols;
	std::transform(corners.begin(), corners.end(), std::inserter(sourceCorners, sourceCorners.begin()), [=](auto i) { return i*k; });

	Mat dst;
	std::vector<Point> transfromedPoints;
	paperToRectangle(*imgMat, dst, sourceCorners, std::inserter(transfromedPoints, transfromedPoints.begin()), k);

	Mat draw = Mat::zeros(dst.size(), CV_8UC3);
	circle(dst, Point(100, 100), 40, Scalar(0, 0, 255), -1);

	Point2f transP2f[4];
	transP2f[0] = transfromedPoints[0];
	transP2f[1] = transfromedPoints[1];
	transP2f[2] = transfromedPoints[2];
	transP2f[3] = transfromedPoints[3];

	Point2f sourceP2f[4];
	sourceP2f[0] = sourceCorners[0];
	sourceP2f[1] = sourceCorners[2];
	sourceP2f[2] = sourceCorners[3];
	sourceP2f[3] = sourceCorners[1];
	for (size_t i = 0; i < 4; i++)
	{
		circle(dst, transP2f[i], 10, Scalar(0, 0, 255), -1);
		circle(dst, sourceP2f[i], 150, Scalar(0, 255, 0), -1);
	}

	Mat sc = transformVectorToMatrix(sourceCorners);
	sortMatrix(sc);
	matrixBackToArray(sc, sourceP2f);

	Mat transMat = getPerspectiveTransform(sourceP2f, transP2f);

	Mat drawT;

	vector<Point2f> s;
	vector<Point2f> t;

	std::transform(sourceCorners.begin(), sourceCorners.end(), back_inserter(s), [](Point p) { return p; });
	std::transform(transfromedPoints.begin(), transfromedPoints.end(), back_inserter(t), [](Point p) { return p; });
	warpPerspective(dst, drawT, transMat,draw.size(), WARP_INVERSE_MAP);
	namedWindow("drawT", CV_WINDOW_NORMAL);
	imshow("drawT", drawT);
	Mat ms(s);
	Mat mt(t);
	//warpPerspective(ms,mt,drawT, Size(1, 4), WARP_INVERSE_MAP);

	cropInterestRegion(dst, transfromedPoints);
	imwrite(a4 + name + "_cropped_" + ext, dst);

	//mainCoinsDetection(dst);

	namedWindow("Result", WINDOW_AUTOSIZE);
	imshow("Result", dst);

}



int main()
{
	std::string action;
	Mat source;
	const char* panel = "Preprocessing";
	/*namedWindow(panel, CV_WINDOW_NORMAL);
	createTrackbar("Img", panel, &imgIndex, 13, changeInput, &source);
	createTrackbar("B diameter", panel, &bilaterialDiam, 50, changeInput, &source);
	createTrackbar("C low", panel, &cannyThresh, 900, changeInput, &source);
	createTrackbar("H rho", panel, &hough_rho, 300, changeInput, &source);
	createTrackbar("H thresh", panel, &houghThresh, 900, changeInput, &source);
	createTrackbar("H min grad", panel, &minGradLines, 90, changeInput, &source);
	createTrackbar("P dst", panel, &distance, 900, changeInput, &source);
	createTrackbar("L minGradCust", panel, &minGradLinesOverlap, 90, changeInput, &source);
	createTrackbar("L marginK", panel, &marginK, 2300, changeInput, &source);
	*/
	std::cout << ">> ";
	std::cin >> action;
	if (action == "all")
	{
		for (int i = 0; i < 13; i++)
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

			std::cout << ">> ";
			std::cin >> action;
		}
	}
	return 0;
}
