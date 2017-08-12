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

/*@brief

Return vector of point of overlaps lines in image area
@param minAngle Points detects if angle of lines > minAngle
*/
std::vector<Point2f> findRectangleCorners(std::vector<Line> lines, Size imgSize, int minAngle);

/*@brief

Transfrom vector of points to differents families of point which are closing for each other
@param maxDistance Point will no belong to fammilies if distance to center of family > minDistance
*/
std::vector<std::set<Point2f, PointComparatorX>> partitionPoints2Families(std::vector<Point2f> points, int minDistance);
