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
#include <vector>
#include <numeric>
#include <exception>

#include "Line.h"
#include "ImagePreprocessing.h"
#include "A4CornersDetection.h"
#include "PointsComparation.h"
#include "PaperReconstruction.h"
#include "CoinsDetection.h"

#include "GetData.h"
#include "PointsComparation.h"
#include "CoinsSegmentation.h"
#include "DetectCoin.h"
#include "Histogram.h"

typedef std::vector<std::pair<float, cv::Point>> circleType;

using namespace cv;

int distance = 60;
int minLineDist = 30;
int minGradLinesOverlap = 10;
int bilaterialDiam = 30;
int cannyThresh = 80;
int houghThresh = 70;
int minGradLines = 0;
int imgIndex = 7; 


bool input(Mat& source, string number)
{
	destroyAllWindows();
	source = imread("../A4/" + number + "_cropped_.jpg");
	return !source.empty();
}

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
	std::vector<Point> corners;
	
	if (families.size() < A4CornersCount)
	{
		throw std::exception("Couldn't regonize A4 corners");
	}
	return accumulatePointFamilies(std::vector<std::set<Point2f, PointComparatorX> >(families.begin(), families.begin() + A4CornersCount));

}

void inputImg(string name, void* img)
{
	std::string a4 = "../A4/";
	std::string ext = ".jpg";
	std::stringstream str;
	//std::string name;
	/*str << imgIndex;
	str >> name;*/
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


	vector<Vec3i> circles{ Vec3i(20,20,5), Vec3i(80,90, 15) };
	//circleProjection(circles, sourceCopy, sourceCorners, transfromedPoints);

	cropInterestRegion(dst, transfromedPoints);
	
	//mainCoinsDetection(dst);

	namedWindow("Result", WINDOW_AUTOSIZE);
	imshow("Result", dst);
	*((Mat*)img) = dst;

}



int main()
{
	try
	{


	std::string name;
	cout << "Enter input image\n>> ";
	cin >> name;
	Mat source;
	
	inputImg(name, &source);
		

	CoinsData coinsData;
	coinsData.readData();
	Mat outputImg;

	Mat segm = source.clone();
	source = bilaterialBlurCoins(source);
	Mat m = getMask(source);
	outputImg = source.clone();
	auto circles = findCircleContours(source, outputImg);

	segmentCoins(circles, source);

	find_sum(source, circles, coinsData);
	}
	catch (const std::exception& error)
	{
		cout << "Error:\t" << error.what() << endl;
		system("pause");
	}
	return 0;
}
