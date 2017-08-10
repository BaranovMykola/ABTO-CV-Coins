#include "CoinsDetection.h"
#include <iostream>
#include <numeric>

//void removeValue(std::vector<Point2f> data, std::vector<Point2f>::iterator it)
//{
//	
//}
//
//Point2f min_x_coordinate(std::vector<Point_<float>>& points)
//{
//	
//	auto it = (std::min_element(points.begin(),points.end(), [](cv::Point_<float>pl, cv::Point_<float> pr)
//	{
//		return pl.x < pr.x;
//	}));
//	Point2f toReturn = *it;
//	points.erase(it);
//	return toReturn;
//}
//
//Point2f min_y_coordinate(std::vector<Point_<float>>& points)
//{
//	auto it = std::min_element(points.begin(), points.end(), [](cv::Point_<float>pl, cv::Point_<float> pr)
//	{
//		return pl.y < pr.y;
//	});	
//	Point2f toReturn = *it;
//	points.erase(it);
//	return toReturn;
//}
//
//Point2f max_x_coordinate(std::vector<Point2f>& points)
//{
//	auto it = max_element(points.begin(), points.end(), [](cv::Point2f pl, cv::Point2f pr)
//	{
//		return pl.x < pr.x;
//	});
//	Point2f toReturn = *it;
//	points.erase(it);
//	return toReturn;
//}
//
//Point2f max_y_coordinate(std::vector<Point2f>& points)
//{
//	auto it = max_element(points.begin(), points.end(), [](cv::Point2f pl, cv::Point2f pr)
//	{
//		return pl.y < pr.y;
//	});
//	Point2f toReturn = *it;
//	points.erase(it);
//	return toReturn;
//}

Mat sortMatrix(Mat mat)
{
	Mat res = mat.clone();
	if (res.at<Point2f>(0, 0).x > res.at<Point2f>(0, 1).x)
	{
		swap(res.at<Point2f>(0, 0), res.at<Point2f>(0, 1));
	}

	if (res.at<Point2f>(1, 0).x > res.at<Point2f>(1, 1).x)
	{
		swap(res.at<Point2f>(1, 0), res.at<Point2f>(1, 1));
	}

	if (res.at<Point2f>(0, 0).y > res.at<Point2f>(1,0).y)
	{
		swap(res.at<Point2f>(0, 0), res.at<Point2f>(1,0));
	}

	if (res.at<Point2f>(0,1).y > res.at<Point2f>(1, 1).y)
	{
		swap(res.at<Point2f>(0, 1), res.at<Point2f>(1, 1));
	}

	if (res.at<Point2f>(0, 0).x > res.at<Point2f>(0, 1).x)
	{
		swap(res.at<Point2f>(0, 0), res.at<Point2f>(0, 1));
	}

	if (res.at<Point2f>(1, 0).x > res.at<Point2f>(1, 1).x)
	{
		swap(res.at<Point2f>(1, 0), res.at<Point2f>(1, 1));
	}
	return res;
}

Mat transformVectorToMatrix(std::vector<Point2f> points)
{
	Mat res(2, 2, CV_32FC2);
	int count = 0;
	for (size_t i = 0; i < res.rows; i++)
	{
		for (size_t j = 0; j < res.cols; j++)
		{
			res.at<Point2f>(i, j) = points[count];
			++count;
		}
	}
	return res;
}
void orderbyPoints(std::vector<cv::Point2f>& points)
{
	std::vector<Point2f> orderedPoints;

}

void print()
{
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!";
}

std::vector<cv::Point2f> accumulatePointFamilies( Mat& pict, std::vector<std::set<cv::Point2f, Comp> > families)//what if vectorsize < 4
{
	std::vector<cv::Point2f> res;
	for (size_t i = 0; i < PointsQuantity; i++)
	{
		cv::Point2f sum = std::accumulate(families[i].begin(), families[i].end(), cv::Point2f(0, 0));
		sum = sum / (double)families[i].size();
		res.push_back(sum);
	}
	return res;
}

void matrixBackToArray( Mat data, Point2f* res)
{
	res[0] = data.at<Point2f>(0, 0);
	res[1] = data.at<Point2f>(0, 1);
	res[2] = data.at<Point2f>(1, 1);
	res[3] = data.at<Point2f>(1, 0);
}

void calculateOutputPoints(Point2f* input, Point2f* output)
{
	output[0] = input[0];

	output[1].x = input[1].x;
	output[1].y = input[0].y;

	output[2].x = output[1].x;
	output[2].y = input[2].y;

	output[3].x = output[0].x;
	output[3].y = output[2].y;
}

void paperToRectangle(Mat & pict, std::vector<cv::Point2f> points)
{ 
	std::vector<Point2f> orderedPoints;
	Mat showPoints = transformVectorToMatrix(points);

	Mat res = sortMatrix(showPoints);
	Point2f inputPoints[4];
	matrixBackToArray(res, inputPoints);
	
	Point2f outputPoints[4];
	calculateOutputPoints(inputPoints, outputPoints); 
	Mat transMat = getPerspectiveTransform(inputPoints, outputPoints);

	Mat transformedMatrix(pict.size(), pict.type());
	warpPerspective(pict, transformedMatrix, transMat, pict.size());
}
 