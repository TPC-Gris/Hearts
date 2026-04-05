#pragma once

class Card
{
public:
	Card(char s, char r)
	{
		suit = s;
		rank = r;
	}

	bool operator==(const Card& right) const
	{
		if (suit == right.suit
			&& rank == right.rank)
			return true;

		return false;
	}

	char suit = 'N';
	char rank = 'N';

};