#include "CoinsDetection.h"
#include <iostream>
#include <numeric>
#include <iterator>



void sortMatrix(Mat mat)
{
	for (size_t c = 0; c < 2; c++) // for 2x2
	{
		for (int i = 0; i < mat.rows; i++)
		{
			Point2f* row = mat.ptr<Point2f>(i);
			std::sort(row, row + mat.cols, [](auto l, auto r)
			{
				return l.x < r.x;
			});
		}

		for (int i = 0; i < mat.rows-1; i++)
		{
			for (int j = 0; j < mat.cols; j++)
			{
				Point2f& up = mat.at<Point2f>(i,j);
				Point2f& down = mat.at<Point2f>(i+1, j);
				if (up.y > down.y)
				{
					std::swap(up, down);
				}
			}
		}
	}
}

Mat transformVectorToMatrix(std::vector<Point> points)
{
	Mat res(2, 2, CV_32FC2);
	int count = 0;
	for (size_t i = 0; i < res.rows; i++)
	{
		for (size_t j = 0; j < res.cols; j++)
		{
			res.at<Point2f>(i, j) = points[count++];
		}
	}
	return res;
}

void orderbyPoints(std::vector<cv::Point2f>& points)
{
	std::vector<Point2f> orderedPoints;
}

bool isQuadHor(Point2f arr[])
{
	return ((norm(arr[0] - arr[1]) + norm(arr[2] - arr[3]))  >	(norm(arr[0] - arr[3]) + norm(arr[1] - arr[2])));
}

std::vector<cv::Point> accumulatePointFamilies(std::vector<std::set<cv::Point2f,  PointComparatorX> > families)//what if vectorsize < 4
{
	std::vector<cv::Point> points;
	for (size_t i = 0; i < PointsQuantity; i++)
	{
		cv::Point2f sum = std::accumulate(families[i].begin(), families[i].end(), cv::Point2f(0, 0));
		sum = sum / (double)families[i].size();
		points.push_back(sum);
	}	
	return points;
}

void matrixBackToArray( Mat data, Point2f* res)
{
	res[0] = data.at<Point2f>(0, 0);
	res[1] = data.at<Point2f>(0, 1);
	res[2] = data.at<Point2f>(1, 1);
	res[3] = data.at<Point2f>(1, 0);
}

void calculateOutputPoints(Point2f* input, Point2f* output, double k = 1)
{
	//210 x 297
	int firstPointPlace = 10*k;
	short shift = isQuadHor(input) ? 0 : 1;

		output[shift] = Point2f(firstPointPlace, firstPointPlace);//a - upper left point

		output[1+shift].x = output[0+shift].x+A4.width*k;//b - upper right point
		output[1+shift].y = output[0+shift].y;

		output[2+shift].x = output[1+shift].x;//c - down right point
		output[2+shift].y = output[1+shift].y + A4.height*k;

		output[(3+shift)%4].x = output[shift].x;// down left point
		output[(3+shift)%4].y = output[2+shift].y;
}

Mat paperToRectangle(Mat & pict, std::vector<cv::Point> points, Mat& a4corners)
{
	Mat pointsMat = transformVectorToMatrix(points);
	sortMatrix(pointsMat);
	a4corners = pointsMat;
	Point2f inputPoints[4];
	Point2f outputPoints[4];
	matrixBackToArray(pointsMat, inputPoints);
	calculateOutputPoints(inputPoints, outputPoints); 
	Mat transMat = getPerspectiveTransform(inputPoints, outputPoints);
	Mat transformedMatrix(pict.size(), pict.type());
	warpPerspective(pict, transformedMatrix, transMat, pict.size());
	return transMat;
}

bool isMatSorted(Mat & arr)
{
	bool sorted = true;
	for (size_t i = 0; i < arr.cols; i++)
	{
		for (size_t j = 0; j < arr.rows; j++)
		{
			float x0;
			float x1;
			float y0;
			float y1;
			
			if (j + 1 < arr.rows)
			{
				y0 = arr.at<Point2f>(j, i).y;
				y1 = arr.at<Point2f>(j + 1, i).y;
				if (y0 > y1)
				{
					sorted = false;
				}
			}
			if (i + 1 < arr.cols)
			{
				x0 = arr.at<Point2f>(j, i).x;
				x1 = arr.at<Point2f>(j, i + 1).x;
				if (x0 > x1)
				{
					sorted = false;
				}
			}

		}
	}
 	return sorted;
}

inline cv::Point2f operator*(cv::Mat M, const cv::Point2f& p)
{
	cv::Mat_<double> src(3/*rows*/, 1 /* cols */);

	src(0, 0) = p.x;
	src(1, 0) = p.y;
	src(2, 0) = 1.0;

	cv::Mat_<double> dst = M*src; //USE MATRIX ALGEBRA 
	return cv::Point2f(dst(0, 0), dst(1, 0));
}

Mat cropInterestRegion(Mat & source, Mat & a4Corners, std::vector<Point> originalPoints, Mat & transMat, Size procSize)
{
	std::vector<Point2f> outputPoints;
	Point2f originalDiscretePoints[4];

	sortMatrix(a4Corners);
	originalDiscretePoints[0] = (Point)a4Corners.at<Point2f>(0, 0);
	originalDiscretePoints[1] = (Point)a4Corners.at<Point2f>(0, 1);
	originalDiscretePoints[2] = (Point)a4Corners.at<Point2f>(1, 1);
	originalDiscretePoints[3] = (Point)a4Corners.at<Point2f>(1, 0);

	Point2f outputlDiscretePoints[4];


	float k = source.size().width / procSize.width;
	int j = 0;
	for (Point2f& i : originalDiscretePoints)
	{
		i = originalDiscretePoints[j++];
		i *= k;
		//circle(source, i, 36, Scalar(255, 0, 0), -1);
	}
	
	calculateOutputPoints(originalDiscretePoints, outputlDiscretePoints, k);
	
	for (auto i : outputlDiscretePoints)
	{
		//circle(source, i, 36, Scalar(0, 0, 255), -1);
	}

	Mat newTransMat = getPerspectiveTransform(originalDiscretePoints, outputlDiscretePoints);

	Mat dst;
	warpPerspective(source, dst, newTransMat, source.size());
	auto pt0 = outputlDiscretePoints[0];
	auto pt1 = outputlDiscretePoints[2];
	//pt0 *= k;
	//pt1 *= k;
	auto r = Rect(pt0, pt1);
	Mat res = dst(r);
	return res;
}

