#include <opencv2\core.hpp>

#include <set>

#include "PointsComparation.h"
#include "Line.h"
//#include ""

using namespace cv;

const Size maxPreprocessingImage(1200, 1200);

/*@brief

Cut down image size by 2 unless it less than $maxPreprocessingImage$
*/
void reduceSize(Mat& img);

/*@brief

Draw points on $img$ from $points$
@param mult For drawwing all points or approximate in one single point
*/
void drawPoint(std::set<Point2f,  PointComparatorX> points, Mat & img, Scalar color, bool mult = false);

/*@brief

Drawing lines from $lines$ on $img$
*/
void drawLines(Mat & img, std::vector<Line> lines);
