#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <iostream>
#include "DetectCoin.h"

using namespace std;
using namespace cv;



void input(Mat& source)
{
	destroyAllWindows();
	cout << "enter number of picture:" << endl;
	string name;
	cin >> name;
	source = imread("../A4/" + name + "_cropped_.jpg");
}

int main()
{
	CoinsData coinsData;
	coinsData.readData();
	Mat source;	
	Mat outputImg;

	char ch = 0;
	do
	{
		cout << "to exit enter 'e' to input picture enter 'p'" << endl;
		cin >> ch;
		if (ch == 'p')
		{
			input(source);
			outputImg = source.clone();
			find_sum(source, findCircleContours(source, outputImg), coinsData);
		}
	} 
	while (ch != 'e');

	system("pause");
}
