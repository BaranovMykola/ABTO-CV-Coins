#include "GetData.h"

CoinRadius::CoinRadius()
	:
	value(0),
	averageRad(0),
	quantity(0)
{
}

void CoinRadius::output()
{
	cout << "value: " << value << " average radius: " << averageRad << " of " << quantity << " coins" << endl;
}

istream & operator >> (istream & stream, CoinRadius & coin)
{
	stream >> coin.value;
	stream >> coin.averageRad;
	stream >> coin.quantity;
	return stream;
}

ostream & operator<<(ostream & stream, CoinRadius & coin)
{
	stream << coin.value << " " << coin.averageRad << " " << coin.quantity << endl;
	return stream;
}

CoinsData::CoinsData()
{
}

vector<CoinRadius> CoinsData::get_coins()
{
	return coins;
}

void CoinsData::readData()
{
	fstream file(fileName);
	CoinRadius current;
	while (!file.eof())
	{
		file >> current;
		coins.push_back(current);
	}
}

void CoinsData::writeData()
{
	fstream file(fileName);
	for (size_t i = 0; i < coins.size(); i++)
	{
		file << coins[i];
	}
	file.close();
}

void CoinsData::addCoin(int coin, float radius)
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

int CoinsData::detect_coin_value(float rad, bool silver)
{
	float min_dist = rad;
	int coin_to_return = 0;
	int value = 0;
	int begin;
	int end;
	if (silver == true)
	{
		begin = 0;
		end = 3;
	}
	else
	{
		begin = 3;
		end = coins.size();
	}
	for (int i = begin; i < end; ++i)
	{
		if (abs(rad - coins[i].averageRad) < min_dist)
		{
			min_dist = abs(rad - coins[i].averageRad);
			coin_to_return = coins[i].value;
		}
	}
	return coin_to_return;
}
