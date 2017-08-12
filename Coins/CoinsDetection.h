#pragma once
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <vector>
#include <set>
#include <map>

#include"Line.h"
#include "PointsComparation.h"
using namespace cv;

const Size A4(297, 210);//297 - width, 210 - height




bool isQuadHor(Point2f arr[]);

const int PointsQuantity = 4;

std::vector<cv::Point> accumulatePointFamilies(std::vector<std::set<cv::Point2f, PointComparatorX> > families);

Mat paperToRectangle(Mat& pict, std::vector<cv::Point> points, Mat& a4Corners);

bool isMatSorted(Mat& arr);

//Mat cutPaper(Mat& data, std::vector<Point2f> points);

Mat cropInterestRegion(Mat& source, Mat& a4Corners, std::vector<Point> originalPoints, Mat& transMat, Size procSize);