#include <opencv2\core.hpp>
#include <vector>
#include <set>

#include "PointsComparation.h"
#include "Line.h"

using namespace cv;

/*@brief

Returns line from polar coordinate system after Hough Line Transfrom
*/
std::vector<Line> getLines(std::vector<Vec2f> lines);

std::vector<Point2f> findRectangleCorners(std::vector<Line> lines, Size imgSize, int minGradCustom);

std::vector<std::set<Point2f, PointComparatorX>> partitionPoints2Families(std::vector<Point2f> points, int distance);
