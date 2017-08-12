#include "A4CornersDetection.h"

#include <opencv2\core.hpp>
#include <vector>
#include <set>
#include <numeric>
#include <iostream>

#include "Line.h"
#include "PointsComparation.h"

using namespace cv;

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

std::vector<Point2f> findRectangleCorners(std::vector<Line> lines, Size imgSize, int minAngle)
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
			float diffAngle = static_cast<int>(abs(i.angle() - j.angle())) % 180;
			if (x > 0 && y > 0 && x < imgSize.width && y < imgSize.height &&
				diffAngle > minAngle && diffAngle < 180 - minAngle)
			{
				points.push_back(Point2f(x, y));
			}
		}
	}
	return points;
}

std::vector<std::set<Point2f, PointComparatorX> > partitionPoints2Families(std::vector<Point2f> points, int minDistance)
{
	std::vector<std::set<Point2f,  PointComparatorX> > families;
	if (points.size() > 3500) { std::cout << "Point threshold reached" << std::endl;return families; }
	if (points.size() > 0)
	{
		int count = 0;
		auto a = std::set<Point2f,  PointComparatorX>({ points.front() });
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
				averageDist /= i->size();
				if (averageDist < minDistance)
				{
					i->insert(var);
					inserted = true;
				}
			}
			if (!inserted)
			{
				families.push_back(std::set<Point2f,  PointComparatorX>({ var })); //creating new family
			}
		}
	}
	return families;
} // can be improved