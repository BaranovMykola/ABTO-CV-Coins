#pragma once
#include <iostream>
#include <opencv2\core.hpp>
#include <string>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

struct CoinRadius
{
	int value;
	double averageRad;
	int quantity;
	CoinRadius():
		value(0),
		averageRad(0),
		quantity(0)
	{}
	friend istream& operator >> (istream& stream, CoinRadius& coin)
	{
		stream >> coin.value;
		stream >> coin.averageRad;
		stream >> coin.quantity;
		return stream;
	}

	void output()
	{
		cout << "value: " << value << " average radius: " << averageRad << " of " << quantity << " coins" << endl;
	}
	friend ostream& operator<< (ostream& stream, CoinRadius& coin)
	{
		stream <<  coin.value <<" "<<  coin.averageRad<<" " <<  coin.quantity <<endl;
		return stream;
	}
};

/*
Good average values:
5 13.539 6
10 9.65167 3
25 11.9619 10
50 13.1324 4*/

class CoinsData
{
	vector<CoinRadius> coins;
	string fileName = "AverageValue.txt";
public:
	CoinsData()
	{
	}
	vector<CoinRadius> get_coins()
	{
		return coins;
	}
	void readData()
	{
		fstream file(fileName);
		CoinRadius current;
		while (!file.eof())
		{
			file >> current;
			coins.push_back(current);
		}
	}
	void writeData()
	{
		fstream file(fileName);
		for (size_t i = 0; i < coins.size(); i++)
		{
			file << coins[i];
		}
		file.close();
	}

	void addCoin(int coin, float radius)
	{
		int index;
		for (size_t i = 0; i < coins.size(); i++)
		{
			if (coin == coins[i].value)
			{
				index = i;
				break;
			}
		}
		double curr_rad = coins[index].averageRad*coins[index].quantity;
		curr_rad += radius;
		++coins[index].quantity;
		coins[index].averageRad = curr_rad / coins[index].quantity;		
	}

	int detect_coin(float rad)
	{
		float min_dist = rad;
		int coin_to_return = 0;
		int value = 0;
		for (int i = 0; i < coins.size(); ++i)
		{
			if (abs(rad - coins[i].averageRad) < min_dist)
			{
				min_dist = abs(rad - coins[i].averageRad);
				coin_to_return = coins[i].value;
			}
		}
		return coin_to_return;
	}
};