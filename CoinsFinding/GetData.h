#pragma once
#ifndef GET_DATA_H
#define GET_DATA_H
#include <iostream>
#include <opencv2\core.hpp>
#include <string>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

/*
Good average values:
5 13.539 6
10 9.65167 3
25 11.9619 10
50 13.1324 4
*/

struct CoinRadius
{
	int value;
	double averageRad;
	int quantity;
	CoinRadius();
	friend istream& operator >> (istream& stream, CoinRadius& coin);
	void output();
	friend ostream& operator<< (ostream& stream, CoinRadius& coin);
};


class CoinsData
{
	vector<CoinRadius> coins;
	string fileName = "AverageValue.txt";
public:
	CoinsData();
	vector<CoinRadius> get_coins();
	void readData();
	void writeData();
	

	void addCoin(int coin, float radius);
	
	int detect_coin_value(float rad);
};
#endif GET_DATA_H