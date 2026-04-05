#include "Include.h"
#include "Client.h"

std::string GetIp(); // Client
inline std::string GetName(); // Both
int GetPort();

Client::Client()
	: winText(font, ""), loseText(font, "")
{
	socket.setBlocking(true);
	
	font.openFromFile("comic.ttf");

}

Client::~Client()
{

	for (int i = 0; i < playedCards.size(); i++)
	{
		delete playedCards[i];
		playedCards[i] = nullptr;
	}

	delete firstCard;
	firstCard = nullptr;
}

void Client::TryConnect(std::string ip, int port)
{
	socket.connect(sf::IpAddress::resolve(ip).value(), port);
		
}

void Client::SendName()
{
	sf::Packet packet;

	packet << (int)NAME;
	packet << name;

	socket.send(packet);
}


void Client::Run(sf::RenderWindow* w, ImageManager* im)
{
	window = w;
	manager = im;

	ConnectAndSendName();
	GetCards();
	GetNames();

	window = new sf::RenderWindow(sf::VideoMode({ 1000, 900 }), "Hearts", sf::Style::Titlebar | sf::Style::Close);
	window->setTitle(name);

	playerDot.setRadius(10);
	playerDot.setFillColor(sf::Color::Red);

	for (int i = 0; i < playerAmount; i++)
	{
		playedCards.push_back(nullptr);

		totalPoints.push_back(sf::Text(font, playerNames[i].getString() + ": 0"));
		roundPoints.push_back(sf::Text(font, playerNames[i].getString() + ": 0"));

		totalPoints[i].setPosition({ 10.f, 300.f + i * 50.f });
		roundPoints[i].setPosition({ 500.f, 300.f + i * 50.f });
	}

	for (int i = 0; i < 3; i++)
	{
		chosenSendCards.push_back(sf::RectangleShape());

		chosenSendCards[i].setOutlineColor(sf::Color::Red);
		chosenSendCards[i].setOutlineThickness(3.f);
		chosenSendCards[i].setSize({ 80.f, 116.f });
		chosenSendCards[i].setPosition({ 1000.f, 900.f });
		chosenSendCards[i].setFillColor(sf::Color::Transparent);

	}

	ShowHand();

	PlayGame();

	system("pause");
}

void Client::ConnectAndSendName()
{
	std::string ip = "";

	ip = GetIp();
	int port = GetPort();
	name = GetName();
	

	TryConnect(ip, port);
	SendName();

	while (true)
	{
		sf::Packet packet;
		int type = 0;

		socket.receive(packet);

		packet >> type;


		if (type == NEXT)
			break;
		else if (type == RESEND_NAME)
		{
			name = GetName();
			SendName();
		}

	}

}

void Client::GetCards()
{

	sf::Packet packet;
	int type = 0;
	int cardAmount = 0;

	socket.receive(packet);


	packet >> type;

	if (type == START_CARDS)
	{
		packet >> cardAmount;

		int suit = 'N';
		int rank = 'N';

		for (int i = 0; i < cardAmount; i++)
		{
			packet >> suit;
			packet >> rank;

			cards.push_back(Card((char)suit, (char)rank));
		}

	}

}

void Client::GetNames()
{
	sf::Packet packet;
	int type = 0;

	socket.receive(packet);

	packet >> type;
	
	if (type == PEOPLE_NAMES)
	{
		packet >> playerAmount;

		for (int i = 0; i < playerAmount; i++)
		{
			std::string n = "";

			packet >> n;
			playerNames.push_back(sf::Text(font, n, 30));
			playerNames[i].setPosition({ (float)(800 / (playerAmount + 1) * (i + 1)), 50 });
		}

	}

}

void Client::ShowHand()
{
	window->clear();

	cardSprites.clear();

	for (int i = 0; i < cards.size(); i++)
	{
		std::string path = "";
		path += cards[i].suit;
		path += cards[i].rank;

		cardSprites.push_back(
			sf::Sprite(manager->cardTexture.find(path)->second)
		);

		int x = (i % 9) * 90;
		int y = 0;

		if (i < 9)
			y = 774;
		else
			y = 648;


		cardSprites[i].setPosition({ (float)x, (float)y });


	}

	for (int i = 0; i < cardSprites.size(); i++)
		window->draw(cardSprites[i]);

	for (int i = 0; i < playerNames.size(); i++)
		window->draw(playerNames[i]);

	for (int i = 0; i < playedCards.size(); i++)
	{
		if(playedCards[i] != nullptr)
			window->draw(*playedCards[i]);
	}

	for (int i = 0; i < totalPoints.size(); i++)
		window->draw(totalPoints[i]);

	for (int i = 0; i < roundPoints.size(); i++)
		window->draw(roundPoints[i]);

	for (int i = 0; i < 3; i++)
		window->draw(chosenSendCards[i]);

	window->draw(playerDot);

	window->display();

}

void Client::PlayGame()
{
	socket.setBlocking(false);

	while (window->isOpen())
	{
		if(!lost)
		{
			Events();

			TryReceive();

			if (chooseSendCards && sendIndex.size() == 3)
				SendCards();
		}
		else
		{
			while (const std::optional event = window->pollEvent())
			{
				if (event->is<sf::Event::Closed>())
					window->close();
			}
		}


	}
}

void Client::Events()
{

	while (const std::optional event = window->pollEvent())
	{
		if (event->is<sf::Event::Closed>())
			window->close();
		else if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouseEvent->button == sf::Mouse::Button::Left)
			{
				for (int i = 0; i < cardSprites.size(); i++)
				{
					if (cardSprites[i].getGlobalBounds().contains(sf::Vector2f(mouseEvent->position)))
					{
						if (chooseSendCards)
						{
							if (sendIndex.find(i) == sendIndex.end() && sendIndex.size() != 3)
							{
								sendIndex.insert(i);
								chosenSendCards[sendIndex.size() - 1].setPosition(cardSprites[i].getPosition());
								ShowHand();
								//std::cout << "Added: " << cards[i].suit << cards[i].rank << std::endl;
							}
						}
						else if(myTurn)
						{
							if (firstCard != nullptr) // You have lowest clubs
							{
								if (cards[i] == *firstCard)
								{
									PlayCard(cards[i]);
									delete firstCard;
									firstCard = nullptr;
								}
							}
							else
							{
								if (followSuit == 'N')
								{
									if (cards[i].suit == 'H' || (cards[i].suit == 'S' && cards[i].rank == 'Q'))
									{
										if (pointsPlayed)
											PlayCard(cards[i]);
										else if (HaveSuit('S') == false
											&& HaveSuit('C') == false
											&& HaveSuit('D') == false)
											PlayCard(cards[i]);
									}
									else
										PlayCard(cards[i]);
								}
								else
								{
									if (cards[i].suit == followSuit)
										PlayCard(cards[i]);
									else if (HaveSuit() == false)
										PlayCard(cards[i]);

								}
							}
						}
						break;
					}
				}
			}
		}

	}
}

void Client::TryReceive()
{
	sf::Packet packet;
	int type = 0;

	if (socket.receive(packet) == sf::Socket::Status::Done)
	{
		packet >> type;

		// Debug
		//std::cout << "Type " << TypeStrings.find(type)->second << std::endl;

		if (type == SWITCH_CARDS)
			chooseSendCards = true;
		else if (type == THREE_CARDS_GET)
			GetSentCards(packet);
		else if (type == WHO_HAS_CARD)
			CheckLowest(packet);
		else if (type == PLAYER_TURN)
			GetTurn(packet);
		else if (type == X_PLAYED_CARD)
			GetPlayed(packet);
		else if (type == FOLLOW_SUIT)
			GetSuit(packet);
		else if (type == POINTS_BOOL)
			GetPointsBool(packet);
		else if (type == ROUND_POINTS)
			GetRoundPoints(packet);
		else if (type == NEW_ROUND)
			NewRound(packet);
		else if (type == TOTAL_POINTS)
			GetTotalPoints(packet);
		else if (type == LOSE)
			EndHearts(packet);
	}

}

void Client::SendCards()
{
	//std::cout << "Send cards you want to switch" << std::endl;

	sf::Packet packet;
	
	packet << THREE_CARDS_GIVE;

	for (auto& itr : sendIndex)
	{
		packet << (int)cards[itr].suit;
		packet << (int)cards[itr].rank;
	}

	/* Also need to get cards for clients */
	int counterIndex = 0;

	cards.erase(
		std::remove_if(cards.begin(), cards.end(),
			[&](Card c)
			{
				return sendIndex.find(counterIndex++) != sendIndex.end();
			}), cards.end()
	);


	socket.send(packet);

	chooseSendCards = false;

}

void Client::GetSentCards(sf::Packet packet)
{
	int suit = 0;
	int rank = 0;

	for (int i = 0; i < 3; i++)
	{
		packet >> suit;
		packet >> rank;

		cards.push_back(Card(suit, rank));

	}

	for (int i = 0; i < 3; i++)
		chosenSendCards[i].setPosition({ 1000.f, 900.f });

	ShowHand();

}

void Client::CheckLowest(sf::Packet packet)
{
	int suit = 0;
	int rank = 0;

	packet >> suit;
	packet >> rank;

	Card lowest(suit, rank);

	for (int i = 0; i < cards.size(); i++)
	{
		if (cards[i] == lowest)
		{
			SendLowest();
			firstCard = new Card(lowest);
		}
	}

}

void Client::SendLowest()
{
	sf::Packet packet;

	packet << I_HAVE_CARD;

	socket.send(packet);
}

void Client::GetTurn(sf::Packet packet)
{
	packet >> turn;

	playerDot.setPosition({ playerNames[turn].getPosition().x, 10.f });

	if (playerNames[turn].getString() == name)
		myTurn = true;

	ShowHand();
}

void Client::PlayCard(Card card)
{
	sf::Packet packet;

	packet << CARD_PLAYED;

	packet << (int)card.suit;
	packet << (int)card.rank;

	socket.send(packet);

	cards.erase(
		std::remove_if(cards.begin(), cards.end(),
			[card](Card c)
			{
				return card.suit == c.suit && card.rank == c.rank;
			}), cards.end()
	);

	ShowHand();

	myTurn = false;
}

void Client::GetPlayed(sf::Packet packet)
{
	int index = 0;
	int suit = 0;
	int rank = 0;

	packet >> index;

	packet >> suit;
	packet >> rank;

	std::string first = "";
	first += (char)suit;
	first += (char)rank;

	if (playedCards[0] != nullptr &&
		playedCards[1] != nullptr &&
		playedCards[2] != nullptr)
	{
		for (int i = 0; i < 3; i++)
		{
			delete playedCards[i];

			playedCards[i] = nullptr;
		}
	}

	playedCards[index] = new sf::Sprite(manager->cardTexture.find(first)->second);
	playedCards[index]->setPosition({ playerNames[index].getPosition().x, 100.f });

	ShowHand();

}

bool Client::HaveSuit() // Check if player has follow suit
{
	for (int i = 0; i < cards.size(); i++)
		if (cards[i].suit == followSuit)
			return true;

	return false;
}

void Client::GetSuit(sf::Packet packet)
{
	int suit = 0;

	packet >> suit;

	followSuit = (char)suit;
}

void Client::GetPointsBool(sf::Packet packet)
{
	packet >> pointsPlayed;
}

void Client::UpdateRoundPoints(int index, int points)
{
	roundPoints[index].setString(
	playerNames[index].getString() + ": " + std::to_string(points)
	);

	ShowHand();
}

void Client::GetRoundPoints(sf::Packet packet)
{
	int index = 0;
	int newPoints = 0;

	packet >> index;
	packet >> newPoints;

	UpdateRoundPoints(index, newPoints);
	
}


void Client::UpdateTotalPoints(int index, int points)
{
	totalPoints[index].setString(
		playerNames[index].getString() + ": " + std::to_string(points)
	);
}

void Client::GetTotalPoints(sf::Packet packet)
{
	for (int i = 0; i < totalPoints.size(); i++)
	{
		int points = 0;
		packet >> points;
		UpdateTotalPoints(i, points);
	}

}

void Client::NewRound(sf::Packet packet)
{
	for (int i = 0; i < playerAmount; i++)
	{
		delete playedCards[i];
		playedCards[i] = nullptr;
	}

	playerDot.setPosition({ 0.f, 0.f });
	turn = 0;
	myTurn = false;
	pointsPlayed = false;
	followSuit = 'N';
	chooseSendCards = false;
	delete firstCard;
	firstCard = nullptr;
	sendIndex.clear();

	sf::Packet cardPacket;

	while (cards.size() == 0)
	{
		cardPacket.clear();

		while (const std::optional event = window->pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window->close();
		}

		if (socket.receive(cardPacket) == sf::Socket::Status::Done)
		{
			int type = 0;
			int cardAmount = 0;

			cardPacket >> type;

			if (type == START_CARDS)
			{
				cardPacket >> cardAmount;

				int suit = 'N';
				int rank = 'N';

				for (int i = 0; i < cardAmount; i++)
				{
					cardPacket >> suit;
					cardPacket >> rank;

					cards.push_back(Card((char)suit, (char)rank));
				}
			}

		}

	}

	ShowHand();
	

}

void Client::EndHearts(sf::Packet packet)
{
	lost = true;

	sf::Text losers(font, "");
	std::string losersStr = "";

	std::vector<sf::Text> rankings;

	packet >> losersStr;

	losers.setString(losersStr);
	losers.setPosition({ 10.f, 50.f });

	for (int i = 0; i < playerNames.size(); i++)
	{
		std::string str = "";
		int points = 0;

		packet >> str;
		packet >> points;

		rankings.push_back(sf::Text(font, str + ": " + std::to_string(points)));
		rankings[i].setPosition({ 50.f, 100.f + i * 30.f });

	}



	window->clear();

	window->draw(losers);

	for (int i = 0; i < rankings.size(); i++)
		window->draw(rankings[i]);

	window->display();
}

bool Client::HaveSuit(char suit)
{
	for (int i = 0; i < cards.size(); i++)
		if (cards[i].suit == suit)
			return true;

	return false;
}

std::string GetIp()
{
	std::string ret = "";

	std::cout << "Skriv ip address: ";
	std::cin >> ret;

	return ret;
}

int GetPort()
{
	int port = 0;

	std::cout << "Skriv port: ";
	std::cin >> port;

	return port;
}

std::string GetName()
{
	std::string ret = "";

	std::cout << "Skriv ditt namn: ";
	std::cin >> ret;

	return ret;
}