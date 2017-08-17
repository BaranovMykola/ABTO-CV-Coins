#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

const int accuracy = 1;
const int minr = 8-accuracy;
const int maxr = 13+accuracy;
const int minDist = minr * 2;
const int dp = 1;
int highThresh = 200;
int HoughThresh = 7;


bool check_paper_edges(Point2f curr_center, float curr_rad, Mat& mat)
{
	int eps = 5;
	return !(curr_center.x - curr_rad+eps<0 || curr_center.y - curr_rad+eps<0 || curr_center.x + curr_rad-eps>mat.cols || curr_center.y + curr_rad-eps>mat.rows);
}

void check_contours(vector<Point2f>& centers, vector<float>& rad, Point2f curr_center, float curr_rad, Mat& mat)
{
	if (curr_rad * 2 > 15 && curr_rad * 2 < 30 && check_paper_edges(curr_center, curr_rad, mat))
	{
		centers.push_back(curr_center);
		rad.push_back(curr_rad);
	}
}

void find_circle_contours(Mat source)
{

	Mat gray;
	cvtColor(source, gray, COLOR_BGR2GRAY);

	Mat tresh(source.size(), CV_8UC1);
	threshold(gray, tresh, 0, 255, THRESH_BINARY | THRESH_OTSU);

	erode(tresh, tresh, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

	bitwise_not(tresh, tresh);

	vector<vector<Point>> contours;
	findContours(tresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Mat outputImg(tresh.size(), CV_8UC3);

	cvtColor(tresh, outputImg, CV_GRAY2BGR);

	drawContours(outputImg, contours, -1, Scalar(0, 0, 255));

	vector<Point2f> centers;
	vector<float> rad;

	for (int i = 0; i < contours.size(); ++i)
	{
		Point2f curr_cent;
		float curr_rad;
		minEnclosingCircle(contours[i], curr_cent, curr_rad);
		check_contours(centers, rad, curr_cent, curr_rad, source);

	}

	for (size_t i = 0; i < rad.size(); ++i)
	{
		circle(outputImg, centers[i], rad[i], Scalar(255, 0, 0));
	}

	imshow("found contours", outputImg);
	cout << "quantity of found circles: " << rad.size() << endl;
	waitKey();

}
void input(Mat& source)
{
	destroyAllWindows();
	string name;
	cin >> name;
	source = imread("../A4/" + name + "_cropped_.jpg");
	imshow("source", source);
}

int main()
{
	Mat source;
	input(source);

	
	int ch = 0;
	while (ch != 27)
	{
		ch = waitKey(1);
		if (ch == 32)
		{
			input(source);
			find_circle_contours(source);
		}
	}
}


/*
//HoughCircles
/*
vector<Vec3f> circles;
//HoughCircles(tresh, circles, CV_HOUGH_GRADIENT, 1, 5, 200, 15);

//for (size_t i = 0; i < circles.size(); i++)
//{
//	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
//	int radius = cvRound(circles[i][2]);
//	// draw the circle center
//	circle(source, center, 3, Scalar(0, 255, 0),1, 8, 0);
//	// draw the circle outline
//	circle(source, center, radius, Scalar(0, 0, 255), 1, 8, 0);
//}
*/

//watershed
/*
//bitwise_not(tresh, tresh);

//Mat morphol;
//morphologyEx(tresh, morphol, MORPH_OPEN, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));
//Mat sure_background;
//dilate(morphol, sure_background, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

//Mat distance(tresh.size(), CV_32F);
//distanceTransform(morphol, distance, CV_DIST_L2, 5);//result the same as morphol matrix, but i won`t delete this, because algorithm says to do like this

//normalize(distance, distance, 0, 1., NORM_MINMAX);
//threshold(distance, distance, .4, 1., CV_THRESH_BINARY);

//dilate(distance, distance, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

//   Mat dist_8u;
//   distance.convertTo(dist_8u, CV_8U);
//   vector<vector<Point> > contours;
//   findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
//   Mat markers = Mat::zeros(distance.size(), CV_32SC1);
//   for (size_t i = 0; i < contours.size(); i++)
//       drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
//
//
//   circle(markers, Point(5,5), 3, CV_RGB(255,255,255), -1);

// //  imshow("Markers", markers*10000);
////	imshow("treshed value", tresh);
//watershed(source, markers);*/


void try_code(Mat m)
{

	destroyAllWindows();
	string name;
	cin >> name;
	m = imread("../A4/" + name + "_cropped_.jpg");
	// Load the image
	Mat src = m;
	// Show source image
	imshow("Source Image", src);
	// Change the background from white to black, since that will help later to extract
	// better results during the use of Distance Transform
	for (int x = 0; x < src.rows; x++) {
		for (int y = 0; y < src.cols; y++) {
			if (src.at<Vec3b>(x, y) == Vec3b(255, 255, 255)) {
				src.at<Vec3b>(x, y)[0] = 0;
				src.at<Vec3b>(x, y)[1] = 0;
				src.at<Vec3b>(x, y)[2] = 0;
			}
		}
	}
	// Show output image
	//imshow("Black Background Image", src);
	// Create a kernel that we will use for accuting/sharpening our image
	Mat kernel = (Mat_<float>(3, 3) <<
		1, 1, 1,
		1, -8, 1,
		1, 1, 1); // an approximation of second derivative, a quite strong kernel
				  // do the laplacian filtering as it is
				  // well, we need to convert everything in something more deeper then CV_8U
				  // because the kernel has some negative values,
				  // and we can expect in general to have a Laplacian image with negative values
				  // BUT a 8bits unsigned int (the one we are working with) can contain values from 0 to 255
				  // so the possible negative number will be truncated
	Mat imgLaplacian;
	Mat sharp = src; // copy source image to another temporary one
	filter2D(sharp, imgLaplacian, CV_32F, kernel);
	src.convertTo(sharp, CV_32F);
	Mat imgResult = sharp - imgLaplacian;
	// convert back to 8bits gray scale
	imgResult.convertTo(imgResult, CV_8UC3);
	imgLaplacian.convertTo(imgLaplacian, CV_8UC3);
	// imshow( "Laplace Filtered Image", imgLaplacian );
	imshow("New Sharped Image", imgResult);
	src = imgResult; // copy back
					 // Create binary image from source image
	Mat bw;
	cvtColor(src, bw, CV_BGR2GRAY);
	threshold(bw, bw, 40, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	imshow("Binary Image", bw);
	// Perform the distance transform algorithm
	Mat dist;
	distanceTransform(bw, dist, CV_DIST_L2, 3);
	// Normalize the distance image for range = {0.0, 1.0}
	// so we can visualize and threshold it
	normalize(dist, dist, 0, 1., NORM_MINMAX);
	imshow("Distance Transform Image", dist);
	// Threshold to obtain the peaks
	// This will be the markers for the foreground objects
	threshold(dist, dist, .4, 1., CV_THRESH_BINARY);
	// Dilate a bit the dist image
	Mat kernel1 = Mat::ones(3, 3, CV_8UC1);
	dilate(dist, dist, kernel1);
	imshow("Peaks", dist);
	// Create the CV_8U version of the distance image
	// It is needed for findContours()
	Mat dist_8u;
	dist.convertTo(dist_8u, CV_8U);
	// Find total markers
	vector<vector<Point> > contours;
	findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// Create the marker image for the watershed algorithm
	Mat markers = Mat::zeros(dist.size(), CV_32SC1);
	// Draw the foreground markers
	for (size_t i = 0; i < contours.size(); i++)
		drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i) + 1), -1);
	// Draw the background marker
	circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
	imshow("Markers", markers * 10000);
	// Perform the watershed algorithm
	watershed(src, markers);
	Mat mark = Mat::zeros(markers.size(), CV_8UC1);
	markers.convertTo(mark, CV_8UC1);
	bitwise_not(mark, mark);
	//    imshow("Markers_v2", mark); // uncomment this if you want to see how the mark
	// image looks like at that point
	// Generate random colors
	vector<Vec3b> colors;
	for (size_t i = 0; i < contours.size(); i++)
	{
		int b = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int r = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}
	// Create the result image
	Mat dst = Mat::zeros(markers.size(), CV_8UC3);
	// Fill labeled objects with random colors
	for (int i = 0; i < markers.rows; i++)
	{
		for (int j = 0; j < markers.cols; j++)
		{
			int index = markers.at<int>(i, j);
			if (index > 0 && index <= static_cast<int>(contours.size()))
				dst.at<Vec3b>(i, j) = colors[index - 1];
			else
				dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
		}
	}
	// Visualize the final image
	imshow("Final Result", dst);
	waitKey(0);
}
