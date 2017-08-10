#pragma once
#include <opencv2\core.hpp>

struct Line
{
	Line(double _a = 0, double _b = 0, bool _vertival = false);
	Line(cv::Point2f pt0, cv::Point2f pt1);
	~Line();

	double angle()const;
	double dist(const Line& other)const;

	double a;
	double b;
	cv::Point2f pt0;
	cv::Point2f pt1;
	bool vertical;
};

