#include "Line.h"

#include <cmath>

Line::Line(double _a, double _b, bool _vertical):
	a(_a),
	b(_b),
	vertical(_vertical)
{
	pt0.x = 0;
	pt0.y = b;
	pt1.x = 1;
	pt1.y = a + b;
}

Line::Line(cv::Point2f pt0, cv::Point2f pt1):
	pt0(pt0),
	pt1(pt1)
{
	if (pt0.x != pt1.x)
	{
		a = (pt0.y - pt1.y) / (pt0.x - pt1.x);
		b = pt0.y - a*pt0.x;
		vertical = false;
	}
	else
	{
		a = pt1.x;
		b = pt1.x;
		vertical = true;
	}
}

Line::~Line()
{
}

double Line::angle() const
{
	return atan(a) * 180 / 3.14;
}

double Line::dist(const Line & other) const
{
	if (other.a != a)
	{
		return 0;
	}
	return abs(other.b - b);
}
