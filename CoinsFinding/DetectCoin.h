#pragma once
#ifndef DETECT_COIN_H
#define DETECT_COIN_H

#include <opencv2\core.hpp>
#include <vector>
#include "GetData.h"

using namespace std;
using namespace cv;


void printRadius(vector<pair<float, Point2f>>& circles, Mat& outputImg);

void printContours(Mat & tresh, vector<vector<Point>>& contours, Mat& outputImg);

void printCircles(vector<pair<float, Point2f>>& circles, Mat & mat);

void printValue(Mat & mat, vector<pair<float, Point2f>>& circles, vector<int>& values);

Mat getMask1(Mat & photo);

Mat getMask2(Mat & photo);

Mat getMask3(Mat& photo);

Mat remove_shades(Mat & photo);

bool canCircleBeCoin(Point2f curr_center, float curr_rad, Size matSize);

void checkContours(vector<pair<float, Point2f>>& circles, Point2f curr_center, float curr_rad, Size sourceImgSize, Mat& outputImg);

Mat getMask(Mat & source);

vector<pair<float, Point2f>> processContours(vector<vector<Point>>& contours, Size sourceImgSize, Mat& outputImg);

vector<pair<float, Point2f>> findCircleContours(Mat & source, Mat& outputImg);

void find_sum(Mat & mat, vector<pair<float, Point2f>>& circles, CoinsData& coinsData);

bool is_silver(Mat & orig_pict, Point2f center, float radius);

#endif DETECT_COIN_H
