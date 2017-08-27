#include "PaperReconstruction.h"

#include <opencv2\imgproc.hpp>
#include <numeric>
#include <iterator>

void sortMatrix(Mat& mat)
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

		for (int i = 0; i < mat.rows - 1; i++)
		{
			for (int j = 0; j < mat.cols; j++)
			{
				Point2f& up = mat.at<Point2f>(i, j);
				Point2f& down = mat.at<Point2f>(i + 1, j);
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
	Mat pointsMat(2, 2, CV_32FC2);
	int count = 0;
	for (int i = 0; i < pointsMat.rows; i++)
	{
		for (int j = 0; j < pointsMat.cols; j++)
		{
			pointsMat.at<Point2f>(i, j) = points[count++];
		}
	}
	return pointsMat;
}

bool isQuadHor(Point2f arr[])
{
	return ((norm(arr[0] - arr[1]) + norm(arr[2] - arr[3]))  >	(norm(arr[0] - arr[3]) + norm(arr[1] - arr[2])));
}

std::vector<cv::Point> accumulatePointFamilies(std::vector<std::set<cv::Point2f, PointComparatorX> > families)
{
	std::vector<cv::Point> points;
	for (size_t i = 0; i < A4CornersCount; i++)
	{
		cv::Point2f sum = std::accumulate(families[i].begin(), families[i].end(), cv::Point2f(0, 0));
		sum = sum / (double)families[i].size();
		points.push_back(sum);
	}
	return points;
}

void matrixBackToArray(Mat data, Point2f* res)
{
	res[0] = data.at<Point2f>(0, 0);
	res[1] = data.at<Point2f>(0, 1);
	res[2] = data.at<Point2f>(1, 1);
	res[3] = data.at<Point2f>(1, 0);
}

void calculateOutputPoints(Point2f* input, Point2f* output, float k)
{
	float firstPointPlace = 10 * k;
	short shift = isQuadHor(input) ? 0 : 1;

	output[shift] = Point2f(firstPointPlace, firstPointPlace);//a - upper left point

	output[1 + shift].x = output[0 + shift].x + A4.width*k;//b - upper right point
	output[1 + shift].y = output[0 + shift].y;

	output[2 + shift].x = output[1 + shift].x;//c - down right point
	output[2 + shift].y = output[1 + shift].y + A4.height*k;

	output[(3 + shift) % 4].x = output[shift].x;// down left point
	output[(3 + shift) % 4].y = output[2 + shift].y;
}

bool isMatSorted(Mat & matrix)
{
	bool sorted = true;
	for (int i = 0; i < matrix.cols; i++)
	{
		for (int j = 0; j < matrix.rows; j++)
		{
			float x0;
			float x1;
			float y0;
			float y1;

			if (j + 1 < matrix.rows)
			{
				y0 = matrix.at<Point2f>(j, i).y;
				y1 = matrix.at<Point2f>(j + 1, i).y;
				if (y0 > y1)
				{
					sorted = false;
				}
			}
			if (i + 1 < matrix.cols)
			{
				x0 = matrix.at<Point2f>(j, i).x;
				x1 = matrix.at<Point2f>(j, i + 1).x;
				if (x0 > x1)
				{
					sorted = false;
				}
			}

		}
	}
	return sorted;
}

inline cv::Point2f operator*(cv::Mat& M, const cv::Point2f& p)
{
	cv::Mat_<float> src(3/*rows*/, 1 /* cols */);

	src(0, 0) = p.x;
	src(1, 0) = p.y;
	src(2, 0) = 1.0;

	cv::Mat_<float> dst = M*src; //USE MATRIX ALGEBRA 
	return cv::Point2f(dst(0, 0), dst(1, 0));
}

void cropInterestRegion(Mat & source, std::vector<Point> pointsRect)
{
	auto region = Rect(pointsRect[0], pointsRect[2]);
	source = source(region);
}

void circleProjection(std::vector<std::pair<float, Point2f>> circles, Mat & src, std::vector<Point> sourceCorners, std::vector<Point> transformedCorners)
{
	Mat draw = Mat::zeros(src.size(), CV_8UC3);

	for (auto i : circles)
	{
		circle(draw, i.second, i.first, Scalar(1, 1, 255), 1);
	}

	Point2f transP2f[4];
	Point2f sourceP2f[4];

	Mat sc = transformVectorToMatrix(sourceCorners);
	sortMatrix(sc);
	matrixBackToArray(sc, sourceP2f);

	/*Mat tc = transformVectorToMatrix(transformedCorners);
	matrixBackToArray(tc, transP2f);*/

	transP2f[0] = transformedCorners[0]+Point(-10,-10);
	transP2f[1] = transformedCorners[1]+Point(-10,-10);
	transP2f[2] = transformedCorners[2]+Point(-10,-10);
	transP2f[3] = transformedCorners[3]+Point(-10,-10);

	Mat transMat = getPerspectiveTransform(sourceP2f, transP2f);

	Mat drawT;

	warpPerspective(draw, drawT, transMat, draw.size(), WARP_INVERSE_MAP);
	//src += drawT;
	drawT.copyTo(src, drawT);
}
