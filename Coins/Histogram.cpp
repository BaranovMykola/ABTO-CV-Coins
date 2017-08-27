#include "Histogram.h"

#include <opencv2\imgproc.hpp>

#include <numeric>

using namespace cv;

cv::Mat showHist(cv::Mat & frame, int chanel)
{
	Mat planes[3];
	Mat hist[3];
	split(frame, planes);
	const int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	for (size_t i = 0; i < 3; i++)
	{
		calcHist(&planes[i], 1, 0, Mat(), hist[i], 1, &histSize, &histRange, range);
	}

	int hist_w = 1024; int hist_h = 1000;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	for (size_t i = 0; i < 3; i++)
	{
		normalize(hist[i], hist[i], 0, histImage.rows, NORM_MINMAX, -1, Mat());
	}
	for (int i = 1; i < histSize; i++)
	{
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[0].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[0].at<float>(i))),
			 Scalar(255, 0, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[1].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[1].at<float>(i))),
			 Scalar(0, 255, 0), 2, 8, 0);
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist[2].at<float>(i - 1))),
			 Point(bin_w*(i), hist_h - cvRound(hist[2].at<float>(i))),
			 Scalar(0, 0, 255), 2, 8, 0);
	}
	return hist[chanel];
}

bool isOverexposed(cv::Mat & frame)
{
	Mat lab;
	cvtColor(frame, lab, CV_BGR2Lab);
	Mat histL = showHist(lab, 0);
	float sum = 0;
	for (int i =histL.rows*0.9; i < histL.rows; i++)
	{
		sum += histL.at<float>(Point(0, i));
	}
	float thresh = frame.cols*frame.rows*0.001;
	return sum > thresh;
}
