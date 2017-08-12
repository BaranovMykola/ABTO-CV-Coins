#include "CoinsDetection.h"
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

#include <vector>

using namespace HoughCircleConst;

void mainCoinsDetection(Mat & cropped)
{
	/*namedWindow("Original", CV_WINDOW_NORMAL);
	imshow("Original", cropped);*/
	
	Mat croppedGray;
	cvtColor(cropped, croppedGray, CV_BGR2GRAY);

	const char* hw = "HoughPanel";
	namedWindow(hw);
	
	Mat bilateral;
	bilateralFilter(croppedGray, bilateral, bilaterialDiam, bilaterialDiam/2, bilaterialDiam*2);

	createTrackbar("CannyThreshold", hw, &cannyThresh, 500, houghCallBack, &bilateral);
	houghCallBack(0, &bilateral);
	waitKey();

}

void houghCallBack(int, void* userData)
{
	Mat* img = static_cast<Mat*>(userData);



	Mat edges;
	Canny(*img, edges, cannyThresh / 2, cannyThresh);
	namedWindow("Edges", CV_WINDOW_KEEPRATIO);
	Mat morph;
	morphologyEx(edges, morph, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	//imshow("Edges", morph);

	std::vector<Vec3f> circles;
	
	HoughCircles(morph, circles, HOUGH_GRADIENT, 4, 16*(img->rows/210.0), 254, 60, img->rows/28, img->rows/18);
	for (auto i : circles)
	{
		circle(*img, Point(i[0], i[1]), i[2], Scalar(0, 0, 255), 3);
		imshow("Edges", *img);
		waitKey(100);
	}
	imshow("Edges", *img);

}