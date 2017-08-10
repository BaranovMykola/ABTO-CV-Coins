#pragma once
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <vector>
#include <set>
#include <map>

#include"Line.h"

using namespace cv;
class Comp
{
public:
	bool operator()(const Point2f& l, const  Point2f& r) { return l.x < r.x; };
};

const int PointsQuantity = 4;
void print();

std::vector<cv::Point2f> accumulatePointFamilies( Mat& pict,std::vector<std::set<cv::Point2f, Comp> > families);

void paperToRectangle(Mat& pict, std::vector<cv::Point2f> points);