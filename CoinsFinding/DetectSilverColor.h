#pragma once
#include "GetData.h"
#include "opencv2\imgproc.hpp"
using namespace std;
using namespace cv;

/*

/// Separate the image in 3 places ( B, G and R )
vector<Mat> bgr_planes;
split( src, bgr_planes );

/// Establish the number of bins
int histSize = 256;

/// Set the ranges ( for B,G,R) )
float range[] = { 0, 256 } ;
const float* histRange = { range };

bool uniform = true; bool accumulate = false;

Mat b_hist, g_hist, r_hist;

/// Compute the histograms:
calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );

// Draw the histograms for B, G and R
int hist_w = 512; int hist_h = 400;
int bin_w = cvRound( (double) hist_w/histSize );

Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

/// Normalize the result to [ 0, histImage.rows ]
normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

/// Draw for each channel
for( int i = 1; i < histSize; i++ )
{
line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
Scalar( 255, 0, 0), 2, 8, 0  );
*/
/*
C++: void calcHist(const Mat* images, int nimages, const int* channels, InputArray mask, OutputArray hist, int dims, const int* histSize, 
const float** ranges, bool uniform=true, bool accumulate=false )*/
bool is_silver(Mat& hist);

Scalar findMean(Mat& mat, Mat& mask)
{
	Scalar res = mean(mat, mask);
	return res;
}


void splitToHls(Mat& pict)
{
	vector<Mat> hls;
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
	Mat hsvMat;
	cvtColor(mat, hsvMat, CV_BGR2HSV);
	vector<Mat> hsvPlanes;
	split(hsvMat, hsvPlanes);

	float range[] = { 0,360 };
	const float* histRange = { range };

	bool uniform = true;
	bool accumulate = false;

	Mat hHsvHist;

	int histBins = 180;
	const int histSize[] = { histBins };

	calcHist(&hsvPlanes[0], 1, 0, mask, hHsvHist,1, histSize, &histRange, uniform, accumulate);


	int histRows = 180;
	int histCols = histBins;
	Mat pict(180, 180, CV_8UC1);
	normalize(hHsvHist, hHsvHist, 0, 180, NORM_MINMAX, -1, Mat());
	for (int i = 1; i < histBins; i++)
	{
		line(pict, Point((i - 1), 180 - cvRound(hHsvHist.at<float>(i - 1))),
			Point((i), 180 - cvRound(hHsvHist.at<float>(i))),
			Scalar(255), 2, 8, 0);
	}

	findMean(hsvPlanes[0], mask);
 /*	bool res = is_silver(hHsvHist);
	if (res)
	{
		cout << "value: 5" << " (silver)"<<endl;
	}
	cout<<"is silver - " << boolalpha << res << endl;*/
	return false;
}


bool is_silver(Mat& hist)//6!!! 9!!!
{
	bool res=true;

	bool meetNonZero = false;
	bool meetZeroSecondTime = false;
	for (int i = 0; i < hist.rows; ++i)
	{
		if (hist.at<float>(i,0) > 5)
		{
			meetNonZero = true;
			if (meetNonZero == true && meetZeroSecondTime == true)
			{
				res = false;
				break;
			}
		}
		else
		{
			if (meetNonZero == true)
			{
				meetZeroSecondTime = true;
			}
		}
	}
	return res;
} 