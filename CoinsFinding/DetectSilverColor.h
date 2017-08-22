#pragma once
#include "GetData.h"
#include "opencv2\imgproc.hpp"
using namespace std;
using namespace cv;




void splitToHls(Mat& pict, vector<Mat>& hls)
{
	Mat hlsPict;
	cvtColor(pict, hlsPict, CV_BGR2HLS);
	split(hlsPict, hls);
}

void splitToYuv(Mat& pict)
{
	vector<Mat> yuv;
	Mat yuvPict;
	cvtColor(pict, yuvPict, CV_BGR2YUV);
	split(yuvPict, yuv);
}

void splitToLab(Mat& pict)
{
	vector<Mat> lab;
	Mat labPict;
	cvtColor(pict, labPict, CV_BGR2Lab);
	split(labPict, lab);


}

bool coin_hist(Mat& mat, Point center, float radius)
{ 
 	Mat mask =  Mat::zeros(mat.size(), CV_8UC1);
	circle(mask, center, radius, Scalar(255), -1);

	vector<Mat> hlsPlanes;
	splitToHls(mat, hlsPlanes);

	float range[] = { 0,360 };
	const float* histRange = { range };

	bool uniform = true;
	bool accumulate = false;

	Mat hHlsHist;

	int histBins = 180;
	const int histSize[] = { histBins };

	int planeIndex = 2;
	calcHist(&hlsPlanes[planeIndex], 1, 0, mask, hHlsHist,1, histSize, &histRange, uniform, accumulate);


	int histRows = 180;
	int histCols = histBins;
	Mat pict(180, 180, CV_8UC1);
	normalize(hHlsHist, hHlsHist, 0, 180, NORM_MINMAX, -1, Mat());


	for (int i = 1; i < histBins; i++)
	{
		line(pict, Point((i - 1), 180 - cvRound(hHlsHist.at<float>(i - 1))),
			Point((i), 180 - cvRound(hHlsHist.at<float>(i))),
			Scalar(255), 2, 8, 0);
	}

	Mat hlsPict;
	merge(hlsPlanes, hlsPict);
	Scalar mean_val = mean(hlsPict, mask);
	return false;
}

bool is_silver(Mat& orig_pict, Point2f center, float radius)
{
	Mat mask = Mat::zeros(orig_pict.size(), CV_8UC1);
	circle(mask, center, radius, Scalar(255), -1);
	Mat hls_pict;
	cvtColor(orig_pict, hls_pict, CV_BGR2HLS);
	Scalar mean_val = mean(hls_pict, mask);
	return (mean_val[0] > 60);
}