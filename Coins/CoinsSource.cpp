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

std::vector<Line> getLines(std::vector<Vec2f> lines)
{
	std::vector<Line> points;
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point2f pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		points.emplace_back(pt1, pt2);
	}
	return points;
}

std::vector<Point2f> findRectangleCorners(std::vector<Line> lines, Mat& img)
{
	std::vector<Point2f> points;
	int count = 0;
	for each (auto i in lines)
	{
		for each (auto j in lines)
		{
			double x;
			double y;
			if (i.vertical != j.vertical)
			{
				if (i.vertical)
				{
					x = i.a;
					y = j.a*x + j.b;
				}
				else if (j.vertical)
				{
					x = j.a;
					y = i.a*x + i.b;
				}
			}
			else if (i.a != j.a && !i.vertical)
			{
				x = -(i.b - j.b) / (i.a - j.a);
				y = i.a*x + i.b;
			}
			else
			{
				continue;
			}
			if (x > 0 && y > 0 && x < img.cols && y < img.rows &&
				((int)abs(i.angle() - j.angle())) % 180 > minGradCustom && ((int)abs(i.angle() - j.angle())) % 180 < 180 - minGradCustom)
			{
				points.push_back(Point2f(x, y));
			}
		}
	}
	return points;
}

std::vector<std::set<Point2f, Comp> > partitionPoints2Families(std::vector<Point2f> points)
{
	std::vector<std::set<Point2f, Comp> > families;
	if (points.size() > 3500) { std::cout << "Point threshold reached" << std::endl;return families; }
	if (points.size() > 0)
	{
		int count = 0;
		auto a = std::set<Point2f, Comp>({ points.front() });
		families.push_back(a);
		for each (auto var in points)
		{
			++count;
			if (count % 100 == 0)
			{
				std::cout << "Processing points..." << count << " \ " << points.size() << std::endl;
			}
			bool inserted = false;
			for (auto i = families.begin(); i < families.end(); ++i)
			{
				double averageDist = std::accumulate(i->begin(), i->end(), 0.0, [&](double sum, Point2f p) { return static_cast<double>(cv::norm(var - p)) + sum; });
				/*if (norm(*(i->begin()) - var) < distance)
				{
					i->insert(var);
					inserted = true;
					break;
				}*/
				//averageDist+=
				averageDist /= i->size();
				if (averageDist < distance)
				{
					i->insert(var);
					inserted = true;
				}
			}
			if (!inserted)
			{
				families.push_back(std::set<Point2f, Comp>({ var }));
			}
		}
	}
	return families;
}

void drawPoint(std::set<Point2f, Comp> points, Mat& img, Scalar color, bool mult = false)
{
	if (mult)
	{
		for each (auto var in points)
		{
			circle(img, var, 5, Scalar(0, 255, 0), -1);
		}
	}
	else
	{
		Point2f sum = std::accumulate(points.begin(), points.end(), Point2f(0, 0));
		sum = sum / (double)points.size();
		circle(img, sum, 5, color, 1);
	}
	imshow("Points", img);
}

void drawLines(Mat& img, std::vector<Line> lines)
{
	std::cout << "Drawing lines..." << std::endl;
	for (size_t i = 0; i < lines.size(); i++)
	{
		Point pt1 = lines[i].pt0;
		Point pt2 = lines[i].pt1;
		line(img, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
	}
	imshow("Lines", img);
}

void reduceSize(Mat& img)
{
	Mat dst;
	while (img.cols >= 1200 || img.rows >= 1200)
	{
		pyrDown(img, dst);
		img = dst;
	}
}

std::vector<Point2f> getA4Corners(Mat& input)
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
	imshow("Edges", edges);

	std::vector<Vec2f> Houghlines;
	std::cout << "Hough transfroming..." << std::endl;
	HoughLines(edges, Houghlines, hough_rho + 1, CV_PI / 180.0, hough_thresh, 0, 0, minGrad*(CV_PI / 180));
	auto lines = getLines(Houghlines);

	Mat result = input;
	drawLines(result, lines);
	auto points = findRectangleCorners(lines, result);

	auto families = partitionPoints2Families(points);

	for each (auto var in families)
	{
		drawPoint(var, result, Scalar::all(255));
	}

	std::sort(families.begin(), families.end(), [](std::set<Point2f, Comp> l, std::set<Point2f, Comp> r) { return l.size() > r.size(); });
	return accumulatePointFamilies(result, std::vector<std::set<Point2f, Comp> >(families.begin(), families.begin() + PointsQuantity));
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
	reduceSize(*imgMat);

	paperToRectangle(*imgMat, getA4Corners(*imgMat));


}

int main()
{
	try
	{
		std::string action;
		std::cin >> action;
		Mat source;
		const char* panel = "Preprocessing";
		namedWindow(panel, CV_WINDOW_NORMAL);
		createTrackbar("Img", panel, &imgIndex, 13, changeInput, &source);
		createTrackbar("B diameter", panel, &bil_d, 50, changeInput, &source);
		createTrackbar("C low", panel, &canny_low, 900, changeInput, &source);
		createTrackbar("H rho", panel, &hough_rho, 300, changeInput, &source);
		createTrackbar("H thresh", panel, &hough_thresh, 900, changeInput, &source);
		createTrackbar("H min grad", panel, &minGrad, 90, changeInput, &source);
		createTrackbar("P dist", panel, &distance, 900, changeInput, &source);
		createTrackbar("L minGradCust", panel, &minGradCustom, 90, changeInput, &source);
		createTrackbar("L marginK", panel, &marginK, 2300, changeInput, &source);

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
			while (action != "-1")
			{
				std::stringstream str;
				str << action;
				str >> imgIndex;
				changeInput(0, &source);
				
				waitKey();

				std::cout << "enter action: \t";
				std::cin >> action;

			}
		}
	}
	catch (std::exception exc)
	{
		std::cout << exc.what() << std::endl;
	}
	return 0;
}