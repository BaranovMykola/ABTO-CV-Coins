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

	
	std::stringstream str;
	str << action;
	str >> imgIndex;
	changeInput(0, &source);
		

	string ch;
	CoinsData coinsData;
	coinsData.readData();
	Mat outputImg;


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


				//namedWindow("p");
				//createTrackbar("a", "p", &a, 100);
				//createTrackbar("b", "p", &b, 100);
				find_sum(source, circles, coinsData);
			
	return 0;
}
