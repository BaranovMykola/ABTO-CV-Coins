#include <opencv2\core.hpp>
#include <set>
#include <iterator>

#include "PointsComparation.h"

using namespace cv;
using namespace std;

const Size A4(297, 210);//297 - width, 210 - height
const int A4CornersCount = 4;

/*@brief

Sorts matrix by rows and cols
*/
void sortMatrix(Mat& mat);

/*@brief

Construct 2x2 Mat<Point2f> from 4 points in $points$
(0,0) = 1st
(0,1) = 2nd
(0,1) = 3th
(1,1) = 4th
*/
Mat transformVectorToMatrix(std::vector<Point> points);

/*@brief

Checks if rectangle of 4th points are horizontal
*/
bool isQuadHor(Point2f arr[]);

/*@brief

Generate averge point from each family of points and construct vector from it
*/
std::vector<cv::Point> accumulatePointFamilies(std::vector<std::set<cv::Point2f, PointComparatorX> > families);

/*@brief

Construct array from Mat_<Point2f> 2x2
Pt0 = data[0,0]
Pt1 = data[0,1]
Pt2 = data[1,1]
Pt3 = data[1,0]
*/
void matrixBackToArray(Mat data, Point2f * res);

/*@brief

Calculate new corners point to transform rectangle of $input$ points to quadrangle
of $output$ points with ration $A4$ constant

@param k Size multiplier
*/
void calculateOutputPoints(Point2f * input, Point2f * output, double k = 1);

/*@brief

Checks if matrix is sorted by rows and cols
*/
bool isMatSorted(Mat& matrix);

/*@brief

Multiplies matirx $M$ by point $p$ using matrix algebra
*/
inline cv::Point2f operator*(cv::Mat& M, const cv::Point2f& p);

/*@brief

Reconstructs paper by two sets of four points
*/
void cropInterestRegion(Mat& source, std::vector<Point> pointsRect);

/*@brief

Reconstructs $pict$ by points $points$ so rectangle of points $points$ will be
quadrangle
*/
template<typename _Ty, typename _alloc = std::allocator<_Ty>, template<typename, typename> class _Cont>
inline Mat paperToRectangle(Mat& pict, Mat& dst, std::vector<cv::Point> points, std::insert_iterator<_Cont<_Ty, _alloc>> insIt, float k = 1.0)
{
	dst = Mat(pict.size(), pict.type());
	
	Mat pointsMat = transformVectorToMatrix(points);
	sortMatrix(pointsMat);
	
	Point2f inputPoints[A4CornersCount];
	Point2f outputPoints[A4CornersCount];
	
	matrixBackToArray(pointsMat, inputPoints);
	calculateOutputPoints(inputPoints, outputPoints, 1);
	
	for (size_t i = 0; i < A4CornersCount; i++)
	{
		insIt++ = outputPoints[i];
	}

	Mat transMat = getPerspectiveTransform(inputPoints, outputPoints);
	warpPerspective(pict, dst, transMat, pict.size());
	return transMat;
}