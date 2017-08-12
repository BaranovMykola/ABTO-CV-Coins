#include "ImagePreprocessing.h"

#include <opencv2\imgproc.hpp>
#include <set>
#include <vector>
#include <numeric>
#include <iostream>

	/*888888888888888888888888888*/ #include "CoinsDetection.h"

void reduceSize(Mat& img)
{
	Mat dst;
	while (img.cols >=  maxPreprocessingImage.width || img.rows >= maxPreprocessingImage.height)
	{
		pyrDown(img, dst);
		img = dst;
	}
}

void drawPoint(std::set<Point2f, PointComparatorX> points, Mat& img, Scalar color, bool mult)
{
	if (mult)
	{
		std::cout << "Drawing " << points.size() << " points..." << std::endl;
		for each (auto var in points)
		{
			circle(img, var, 5, Scalar(0, 255, 0), -1);
		}
	}
	else
	{
		std::cout << "Drawing approximation " << points.size() << " points" << std::endl;
		Point2f sum = std::accumulate(points.begin(), points.end(), Point2f(0, 0));
		sum = sum / (double)points.size();
		circle(img, sum, 5, color, 1);
	}
	imshow("Points", img);
}

void drawLines(Mat& img, std::vector<Line> lines)
{
	std::cout << "Drawing " << lines.size() << " lines..." << std::endl;
	for (size_t i = 0; i < lines.size(); i++)
	{
		Point pt1 = lines[i].pt0;
		Point pt2 = lines[i].pt1;
		line(img, pt1, pt2, Scalar(0, 0, 255), 1, LINE_AA);
	}
	imshow("Lines", img);
}