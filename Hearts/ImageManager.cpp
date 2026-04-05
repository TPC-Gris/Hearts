#include "Include.h"
#include "ImageManager.h"

ImageManager::ImageManager()
{
	char suit = 'N';
	char rank = 'N';
	std::string path = "";
	std::string mapFirst = "";

	for (int s = 0; s < 4; s++)
	{
		if (s == 0)
			suit = 'C';
		else if (s == 1)
			suit = 'S';
		else if (s == 2)
			suit = 'D';
		else if (s == 3)
			suit = 'H';

		for (int r = 1; r <= 13; r++)
		{
			if (r == 1)
				rank = 'A';
			else if (r == 10)
				rank = 'T';
			else if (r == 11)
				rank = 'J';
			else if (r == 12)
				rank = 'Q';
			else if (r == 13)
				rank = 'K';
			else
				rank = '0' + r;

			path += "PlayingCards\\";
			path += suit;
			path += rank;
			path += ".png";

			mapFirst += suit;
			mapFirst += rank;

			cardTexture.insert({mapFirst, sf::Texture(path, false, sf::IntRect({0, 0}, {80, 116}))});
			cardTexture.find(mapFirst)->second.setSmooth(true);
			
			path = "";
			mapFirst = "";

		}
	}


}

ImageManager::~ImageManager()
{


}
