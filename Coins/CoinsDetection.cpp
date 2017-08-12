#include "CoinsDetection.h"
#include <iostream>
#include <numeric>
#include <iterator>

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

	if (res.at<Point2f>(0, 0).y > res.at<Point2f>(1, 0).y)
	{
		swap(res.at<Point2f>(0, 0), res.at<Point2f>(1, 0));
	}

	if (res.at<Point2f>(0, 1).y > res.at<Point2f>(1, 1).y)
	{
		swap(res.at<Point2f>(0, 1), res.at<Point2f>(1, 1));
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

bool isQuadHor(Point2f arr[])
{
	return ((norm(arr[0] - arr[1]) + norm(arr[2] - arr[3]))  >	(norm(arr[0] - arr[3]) + norm(arr[1] - arr[2])));
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

void calculateOutputPoints(Point2f* input, Point2f* output, double k = 1)
{
	//210 x 297
	int firstPointPlace = 10*k;
	if (isQuadHor(input))
	{

		output[0] = Point2f(firstPointPlace, firstPointPlace);//a - upper left point

		output[1].x = output[0].x+A4.width*k;//b - upper right point
		output[1].y = output[0].y;

		output[2].x = output[1].x;//c - down right point
		output[2].y = output[1].y + A4.height*k;

		output[3].x = output[0].x;// down left point
		output[3].y = output[2].y;
	}
	else
	{
		output[1] = Point2f(firstPointPlace, firstPointPlace);//b

		output[2].x = output[1].x + A4.width*k;
		output[2].y = output[1].y;

		output[3].x = output[2].x;
		output[3].y = output[2].y + A4.height*k;

		output[0].x = output[1].x;
		output[0].y = output[3].y;
	}
}

Mat paperToRectangle(Mat & pict, std::vector<cv::Point2f> points, Mat& a4corners)
{
	std::vector<Point2f> orderedPoints;
	Mat showPoints = transformVectorToMatrix(points);

	Mat res = sortMatrix(showPoints);
	a4corners = res;
	Point2f inputPoints[4];
	matrixBackToArray(res, inputPoints);
		
	Point2f outputPoints[4];
	calculateOutputPoints(inputPoints, outputPoints); 
	for (size_t i = 0; i < 4; i++)
	{
		circle(pict, outputPoints[i], 3, Scalar(0, 255, 0), -1);
	}

	imshow("Points", pict);
	Mat transMat = getPerspectiveTransform(inputPoints, outputPoints);
	std::cout << "Is horizontal = " << std::boolalpha << isHorizontal(res, pict, transMat) << std::endl;

	Mat transformedMatrix(pict.size(), pict.type());
	isMatSorted(res);
	warpPerspective(pict, transformedMatrix, transMat, pict.size());

	("Perspective", transformedMatrix);

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

bool isHorizontal(Mat & points, Mat & pict, Mat& transMat)
{
	if (!isMatSorted(points))
	{
		throw std::exception("Points are not sorted");
	}

	Point l0 = (points.at<Point2f>(0, 0) + points.at<Point2f>(0, 1))/2;
	Point l1 = (points.at<Point2f>(1, 0) + points.at<Point2f>(1, 1)) / 2;

	Point s0 = (points.at<Point2f>(0, 0) + points.at<Point2f>(1, 0)) / 2;
	Point s1 = (points.at<Point2f>(0, 1) + points.at<Point2f>(1, 1)) / 2;

	Line line0(l0, l1);
	Line line1(s0, s1);

	Point2f dst0;
	Point2f dst1;
	
	int angle;

	std::cout << "Angle of bigger line = ";
	
	if (norm(l0 - l1) > norm(s0 - s1))
	{
		angle = ((int)line0.angle() % 180);
		//std::cout << ((int)line0.angle() % 180);
		line(pict, l0, l1, Scalar::all(255));
		dst0 = transMat*l0;
		dst1 = transMat*l1;

	}
	else
	{ 
		angle  = ((int)line1.angle() % 180);
		//std::cout << ((int)line1.angle()%180);
		line(pict, s0, s1, Scalar::all(0));

		dst0 = transMat*s0;
		dst1 = transMat*s1;
	}
	std::cout << std::endl;

	angle = abs((int)Line(dst0, dst1).angle()%180);
	std::cout << "Long side is at " << angle << " angle grad" << std::endl;
	//imshow("Sides", pict);

	return angle < 45 || angle > 180 - 45;
}

Mat cutPaper(Mat & data, std::vector<Point2f> points)// to do!!!!
{
	Mat res;
	Mat mask(data.size(), CV_8UC1);
	rectangle(mask,Rect(points[0], points[2]), Scalar(255), -1);
	data.copyTo(res, mask);
	return res;
}

Mat cropInterestRegion(Mat & source, Mat & a4Corners, std::vector<Point2f> originalPoints, Mat & transMat, Size procSize)
{
	std::vector<Point2f> outputPoints;
	Point2f originalDiscretePoints[4];

	a4Corners = sortMatrix(a4Corners);
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

