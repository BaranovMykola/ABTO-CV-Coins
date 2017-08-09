#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <numeric>

#include "Line.h"

using namespace cv;

int cannyLow = 255;
int cannyUp = 0;
int rho = 1500;
int eps = 0;
int shiftConst = 40;
int minTreshold = 150;
int maxTreshold = 255;
int distance = 29;
int morph_size = 7;

Mat foto()
{
	VideoCapture cap;
	cap.open(1);
	Mat frame;
	cap >> frame;
	cap >> frame;
	return frame;
}

void showVideo(String name = "VideoStream")
{
	VideoCapture cap;
	cap.open(1);
	Mat frame;
	cap >> frame;
	while (waitKey(1) != 27)
	{
		cap >> frame;
		imshow(name, frame);
	}
}

Mat customCanny(Mat& img)
{
	Mat canny;
	Canny(img, canny, cannyLow, cannyUp);
	return canny;
}

Scalar rndScalar(RNG rnd)
{
	return Scalar(rnd.uniform(0, 255), rnd.uniform(0, 255), rnd.uniform(0, 255));
}

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

void comparePoints(std::vector<Line>& points)
{
	for (auto i = points.begin(); i < points.end(); ++i)
	{
		Line l0 = *i;
		for (auto j = i + 1; j < points.end(); ++j)
		{
			Line l1 = *j;
			double diffAngle = l0.angle() - l1.angle();
			if (abs(diffAngle) < eps / 9.0)
			{
				Line diff((l0.a + l1.a) / 2, (l0.b + l1.b) / 2);
				//points.erase(j);
				points.erase(i);
				//points.push_back(diff);
				i = points.begin();
				//if (points.size() == 4)
				//{
				//	return;
				//}
				break;
			}
		}
	}
}

std::vector<Point2f> findRectangle(std::vector<Line> lines, Mat& img)
{
	std::vector<Point2f> points;
	int count = 0;
	for each (auto i in lines)
	{
		for each (auto j in lines)
		{
			if (i.a != j.a)
			{
				double x = -(i.b - j.b) / (i.a - j.a);
				double y = i.a*x + i.b;
				if (x > 0 && y > 0 && x < img.cols && y < img.rows)
				{
					points.push_back(Point2f(x, y));
				}
			}
		}
	}
	return points;
}

class Comp
{
public:
	bool operator()(const Point2f& l, const  Point2f& r) { return l.x < r.x; };
};

std::vector<std::set<Point2f, Comp> > generateRelative(std::vector<Point2f> points)
{
	std::vector<std::set<Point2f, Comp> > families;
	auto a = std::set<Point2f, Comp>({ points.front() });
	families.push_back(a);
	for each (auto var in points)
	{
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
	return families;
}

void drawPoint(std::set<Point2f, Comp> points, Mat& img, bool mult = false)
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
		circle(img, sum, 5, Scalar(0, 255, 0), -1);
	}
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
}

VideoCapture cap;
int bil_d = 30;
int canny_low = 200;
int canny_hight = 900;
int hough_rho = 1500;
int hough_theta = 12;
int hough_thresh = 58;

Mat input;

void on_trackbar(int, void*)
{
	Mat img = input;
	//cap >> img;

	
		
	Mat imgGray;
	cvtColor(img, imgGray, COLOR_BGR2GRAY);

	Mat edges;
	std::cout << "Canny edge detection..." << std::endl;
	Canny(imgGray, edges, canny_low, canny_low * 3, 3, true);
	imshow("Edges", edges);

	std::vector<Vec2f> Houghlines;
	std::cout << "Hough transfroming..." << std::endl;
	HoughLines(edges, Houghlines, hough_rho / 1000.0, (hough_theta/3.0)*(CV_PI/180)+0.000001, hough_thresh);
	auto customLines = getLines(Houghlines);

	Mat result = input.clone();
	drawLines(result, customLines);

	auto points = findRectangle(customLines, result);

		//std::set<Point2f, Comp> pointsSet(points.begin(), points.end());
		auto families = generateRelative(points);
		std::sort(families.begin(), families.end(), [](std::set<Point2f, Comp> l, std::set<Point2f, Comp> r) { return l.size() > r.size(); });

		for (size_t i = 0; i < 4 && i < families.size(); i++)
		{
			drawPoint(families[i], result);
		}
	imshow("Lines", result);
	std::cout << "\t***\tIteration ended\t***" << std::endl;
}

int main()
{

	const char* panel = "Preprocessing";
	namedWindow(panel, CV_WINDOW_NORMAL);
	createTrackbar("B diameter", panel, &bil_d, 50, on_trackbar);
	createTrackbar("C low", panel, &canny_low, 900, on_trackbar);
	createTrackbar("H rho/1000", panel, &hough_rho, 2000, on_trackbar);
	createTrackbar("H theta/3", panel, &hough_theta, 90, on_trackbar);
	createTrackbar("H thresh", panel, &hough_thresh, 900, on_trackbar);
	createTrackbar("P dist", panel, &distance, 900, on_trackbar);

	cap.open(1);
	Mat temp;
	cap >> temp;
	while (waitKey(1) != 27)
	{
		cap >> input;

		Mat bilateral;

		std::cout << "BilaterialFiltering..." << std::endl;
		bilateralFilter(input, bilateral, bil_d, bil_d * 2, bil_d / 2);
		imshow("bilaterial", bilateral);
		input = bilateral;


		on_trackbar(0, 0);
	}
	//while (waitKey(1) != 27)
	//{


	//	//Mat sharp;
	//	//Mat kernel = (Mat_<char>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	//	//filter2D(img, sharp, img.depth(), kernel);
	//	//Mat imgcopy = img;
	//	//Mat img_gray;
	//	//cvtColor(img, img_gray, CV_BGR2GRAY);
	//	//Mat hist;
	//	//equalizeHist(img_gray, hist);
	//	//img_gray = hist;
	//	//threshold(img_gray, img_gray, minTreshold, 255, THRESH_BINARY);
	//	//
	//	//Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	//	//morphologyEx(img_gray, img_gray, MORPH_OPEN, element);

	//	//imshow("Treshold", img_gray);
	//	//img = img_gray;
	//	//cvtColor(img, img, CV_GRAY2BGR);

	//	//Mat edges = customCanny(img);
	//	//imshow("Edges", edges);
	//	//
	//	//std::vector<Vec2f> lines;
	//	//HoughLines(edges, lines, rho/1000.0, CV_PI / 180, 100, 0, 0);
	//	//
	//	//
	//	//Mat img_clone = imgcopy.clone();
	//	//auto customLines = getLines(lines);
	//	////comparePoints(customLines);
	//	//std::cout << "Found " << customLines.size() << " lines\t";

	//	//for (size_t i = 0; i < customLines.size(); i++)
	//	//{
	//	//	Point pt1 = customLines[i].pt0;
	//	//	Point pt2 = customLines[i].pt1;
	//	//	line(img_clone, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
	//	//}

	//	//auto points = findRectangle(customLines, img);

	//	////std::set<Point2f, Comp> pointsSet(points.begin(), points.end());
	//	//auto families = generateRelative(points);
	//	//std::sort(families.begin(), families.end(), [](std::set<Point2f, Comp> l, std::set<Point2f, Comp> r) { return l.size() > r.size(); });

	//	//for (size_t i = 0; i < 4 && i < families.size(); i++)
	//	//{
	//	//	drawPoint(families[i], img_clone);
	//	//}

	//	//std::cout << "/ " << points.size() << " points in " << families.size() << " families" << std::endl;

	//	//imshow("Lines", img_clone);

	//}

	waitKey();
	return 0;
}