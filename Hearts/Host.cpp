#include "Include.h"
#include "Host.h"

inline std::string GetName(); // Both
int GetPlayerAmount(); // Host

Host::Host()
	: winText(font, ""), loseText(font, ""), lowestCard('N', 'N')
{
	listener.setBlocking(false);

	listener.listen(5000);

	std::cout << "Local ip: " << sf::IpAddress::getLocalAddress().value() << std::endl;


	std::cout << "Public ip: ";

	sf::Http http("ifconfig.me");
	sf::Http::Request request("/ip");

	sf::Http::Response response = http.sendRequest(request, sf::seconds(2));

	if (response.getStatus() == sf::Http::Response::Status::Ok)
	{
		std::string ipStr = response.getBody();

		ipStr.erase(ipStr.find_last_not_of(" \n\r\t") + 1);

		std::cout << sf::IpAddress::resolve(ipStr).value() << std::endl;

	}
	else
	{
		http.setHost("ident.me");
		request.setUri("/");

		sf::Http::Response response = http.sendRequest(request, sf::seconds(2));

		if (response.getStatus() == sf::Http::Response::Status::Ok)
		{
			std::string ipStr = response.getBody();

			ipStr.erase(ipStr.find_last_not_of(" \n\r\t") + 1);

			std::cout << sf::IpAddress::resolve(ipStr).value() << std::endl;

		}

		

	}

	font.openFromFile("comic.ttf");

}

Host::~Host()
{
	for (int i = 0; i < clients.size(); i++)
		delete clients[i];


	for (int i = 0; i < playedCards.size(); i++)
	{
		delete playedCards[i];
		playedCards[i] = nullptr;
	}


	for (int i = 0; i < playedCardsSprites.size(); i++)
	{
		delete playedCards[i];
		playedCards[i] = nullptr;
	}


	delete firstCard;
	firstCard = nullptr;

}

void Host::TryAccept()
{
	Client* newClient = new Client();

	if (listener.accept(newClient->socket) == sf::Socket::Status::Done)
	{
		clients.push_back(newClient);
		clients[clients.size() - 1]->socket.setBlocking(false);
		//std::cout << "Accepted client" << std::endl;
	}
	else
		delete newClient;
	

}

void Host::TryReceive(int clientIndex)
{


	sf::Packet packet;
	int type = 0;

	clients[clientIndex]->socket.receive(packet);

	packet >> type;

	if (type == NAME)
	{
		std::string namn = "";
		packet >> namn;

		sf::Packet sendPacket;

		if (names.find(namn) == names.end())
		{
			if(namn != "")
			{
				names.insert(namn);
				clients[clientIndex]->name = namn;

				//std::cout << "Sending next" << std::endl;
				sendPacket << NEXT;

				clients[clientIndex]->socket.send(sendPacket);
			}

		}
		else
		{
			sendPacket << RESEND_NAME;

			clients[clientIndex]->socket.send(sendPacket);
		}

	}

}

void Host::GetNames()
{

	while (names.size() < (playerAmount - 1))
	{
		//std::cout << names.size() << std::endl;

		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i]->name != "")
				continue;

			TryReceive(i);
			
		}
	}


}


void Host::Run(sf::RenderWindow* w, ImageManager* im)
{
	window = w;
	manager = im;

	AcceptPlayersAndNames();
	SetUpGame();
	SendCards();
	SendNames();


	window = new sf::RenderWindow(sf::VideoMode({ 1000, 900 }), "Hearts", sf::Style::Titlebar | sf::Style::Close);

	window->setTitle(name);

	playerDot.setRadius(10);
	playerDot.setFillColor(sf::Color::Red);

	for (int i = 0; i < playerAmount; i++)
	{
		totalPoints.push_back(0);
		roundPoints.push_back(0);

		playedCards.push_back(nullptr);
		playedCardsSprites.push_back(nullptr);

		totalPointsT.push_back(sf::Text(font, playerNames[i].getString() + ": 0"));
		roundPointsT.push_back(sf::Text(font, playerNames[i].getString() + ": 0"));

		totalPointsT[i].setPosition({ 10.f, 300.f + i * 50.f });
		roundPointsT[i].setPosition({ 500.f, 300.f + i * 50.f });
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

	/* Behöver inte ändra någonting ovanför detta */

	ShowHand(); // Behöver inte ändra showHand


	PlayGame();

	system("pause");
}

void Host::AcceptPlayersAndNames()
{
	playerAmount = GetPlayerAmount();

	while (clients.size() < (playerAmount - 1))
		TryAccept();

	GetNames();

	name = GetName();

	while (names.find(name) != names.end())
		name = GetName();

}

void Host::SetUpGame()
{
	std::random_device random;
	std::mt19937 generation(random());
	std::uniform_int_distribution<> distribution(0, 51);

	std::vector<Card> deck;
	std::vector<Card> shuffledDeck;

	std::unordered_set<int> rNumbers;

	for (int s = 0; s < 4; s++)
	{
		char suit = 'N';

		switch (s)
		{
		case 0: suit = 'C'; break; // Clubs
		case 1: suit = 'S'; break; // Spades
		case 2: suit = 'D'; break; // Diamonds
		case 3: suit = 'H'; break; // Hearts
		}

		for (int r = 1; r <= 13; r++)
		{
			char rank = 'N';

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

			deck.push_back(Card(suit, rank));

		}
	}

	int randomNumber = 0;

	for (int i = 0; i < 52; i++)
	{
		randomNumber = distribution(generation);

		if (rNumbers.find(randomNumber) == rNumbers.end())
		{
			rNumbers.insert(randomNumber);
			shuffledDeck.push_back(deck[randomNumber]);
		}
		else
			i--;
	}

	if (playerAmount == 3)
	{
		shuffledDeck.erase(
			std::remove_if(shuffledDeck.begin(), shuffledDeck.end(),
				[](Card card)
				{
					return (card.suit == 'C' && card.rank == '2');
				}), shuffledDeck.end()
					);
		
		lowestCard = Card('C', '3');
	
	}
	else if (playerAmount == 5)
	{
		shuffledDeck.erase(
			std::remove_if(shuffledDeck.begin(), shuffledDeck.end(),
				[](Card card)
				{
					return (card.suit == 'C' && card.rank == '2')
						|| (card.suit == 'D' && card.rank == '2');
				}), shuffledDeck.end()
					);

		lowestCard = Card('C', '3');

	}
	else if (playerAmount == 6)
	{
		shuffledDeck.erase(
			std::remove_if(shuffledDeck.begin(), shuffledDeck.end(),
				[](Card card)
				{
					return (card.suit == 'C' && card.rank == '2')
						|| (card.suit == 'C' && card.rank == '3')
						|| (card.suit == 'D' && card.rank == '2')
						|| (card.suit == 'D' && card.rank == '3');
				}), shuffledDeck.end()
					);

		lowestCard = Card('C', '4');

	}

	int cardsPerPerson = shuffledDeck.size() / playerAmount;


	for (int i = 0; i < playerAmount; i++)
	{
		playersCards.push_back(
			std::vector<Card>(
				shuffledDeck.begin() + cardsPerPerson * i,
				shuffledDeck.begin() + cardsPerPerson * (i + 1))
		);
	}

	

}

void Host::SendCards()
{
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Packet packet;

		packet << START_CARDS;
		packet << (int)playersCards[i].size();
		//std::cout << "PC: " << playersCards[i].size() << std::endl;

		for (int k = 0; k < playersCards[i].size(); k++)
		{
			packet << (int)playersCards[i][k].suit;
			packet << (int)playersCards[i][k].rank;
		}

		clients[i]->socket.send(packet);
	}

	cards = playersCards[playersCards.size() - 1];

}

void Host::SendNames()
{
	for (int i = 0; i < clients.size(); i++)
	{
		sf::Packet packet;

		packet << PEOPLE_NAMES;
		packet << playerAmount;

		for (int k = 0; k < clients.size(); k++)
			packet << clients[k]->name;
		

		packet << name;

		clients[i]->socket.send(packet);

	}

	for (int i = 0; i < clients.size(); i++)
	{
		playerNames.push_back(sf::Text(font, clients[i]->name, 30));
		playerNames[i].setPosition({ (float)(800 / (playerAmount + 1) * (i + 1)), 50 });
	}

	playerNames.push_back(sf::Text(font, name, 30));
	playerNames[playerAmount - 1].setPosition({ (float)(800 / (playerAmount + 1) * playerAmount), 50 });

}

void Host::ShowHand()
{
	window->clear();

	cardSprites.clear();

	for (int i = 0; i < cards.size(); i++)
	{
		std::string first = "";
		first += cards[i].suit;
		first += cards[i].rank;

		cardSprites.push_back(
			sf::Sprite(manager->cardTexture.find(first)->second)
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

	for (int i = 0; i < playedCardsSprites.size(); i++)
	{
		if(playedCardsSprites[i] != nullptr)
			window->draw(*playedCardsSprites[i]);
	}

	for (int i = 0; i < totalPointsT.size(); i++)
		window->draw(totalPointsT[i]);

	for (int i = 0; i < roundPointsT.size(); i++)
		window->draw(roundPointsT[i]);

	for (int i = 0; i < 3; i++)
		window->draw(chosenSendCards[i]);

	window->draw(playerDot);

	window->display();

}

void Host::PlayGame()
{

	while (window->isOpen())
	{
		if(!lost)
		{
			Events();

			if (stickNumber == 1 && cardsPlayed == 0 && !switchSent)
				SendSwitchFlag();

			if (switchReceived == clients.size() && hostSendIndex.size() == 3)
				SendSwitchCards();

			if (cardSwitched && turn == -1)
				SendLowest();


			TryReceiveGame(); // Try to receive things from clients
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

void Host::Events()
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
							if (hostSendIndex.find(i) == hostSendIndex.end() && hostSendIndex.size() != 3)
							{
								hostSendIndex.insert(i);
								chosenSendCards[hostSendIndex.size() - 1].setPosition(cardSprites[i].getPosition());
								ShowHand();
								//std::cout << "Added: " << cards[i].suit << cards[i].rank << std::endl;
							}
						}
						else if(turn == clients.size())
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

void Host::SendSwitchFlag()
{
	//std::cout << "Send switch flag" << std::endl;

	switchSent = true;  // Spelar ingen roll att jag sätter dne först
						// Båda vägarna kommer sätta den till true ändå

	if (round % playerAmount == 0) // Om man inte ska ge till någon
	{
		switchReceived = clients.size();
		chooseSendCards = false;
		cardSwitched = true;

		for (int i = 0; i < roundPoints.size(); i++)
		{
			roundPoints[i] = 0;
			UpdateRoundPoints(i);
		}

		return;
	}

	chooseSendCards = true;

	sf::Packet packet;
	packet << SWITCH_CARDS;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

}

void Host::GetSwitch(sf::Packet packet, int index)
{
	int type = 0;

	int suit1 = 'N';
	int suit2 = 'N';
	int suit3 = 'N';

	int rank1 = 'N';
	int rank2 = 'N';
	int rank3 = 'N';

	packet >> suit1;
	packet >> rank1;

	packet >> suit2;
	packet >> rank2;

	packet >> suit3;
	packet >> rank3;

	int moveAmount = 0;
	int sendIndex = 0;

	// <-- Vänster ____ Höger -->
	// Kolla om det är höger eller vänster man ska ge
	// Höger är jämn, vänster är ojämn

	if (round % 2 == 1) // Vänster
		moveAmount = -((round + 1) % playerAmount) / 2;
	else // Höger
		moveAmount = (round % playerAmount) / 2;
	
	sendIndex = index + moveAmount;

	if (sendIndex >= playerAmount) // För mycket år höger
		sendIndex = 0 + (sendIndex - playerAmount);
	else if (sendIndex < 0) // För mycket åt vänster
		sendIndex = (playerAmount + sendIndex);

	sendCards[sendIndex][0] = new Card(suit1, rank1);
	sendCards[sendIndex][1] = new Card(suit2, rank2);
	sendCards[sendIndex][2] = new Card(suit3, rank3);

	switchReceived++;
	
}

void Host::SendSwitchCards()
{
	//std::cout << "Send switched cards: " << std::endl;

	/* Set up host send cards */
	int moveAmount = 0;
	int sendIndex = 0;

	// <-- Vänster ____ Höger -->
	// Kolla om det är höger eller vänster man ska ge
	// Höger är jämn, vänster är ojämn

	if (round % 2 == 1) // Vänster
		moveAmount = -((round + 1) % playerAmount) / 2;
	else // Höger
		moveAmount = (round % playerAmount) / 2;

	sendIndex = clients.size() + moveAmount;

	if (sendIndex >= playerAmount) // För mycket år höger
		sendIndex = 0 + (sendIndex - playerAmount);
	else if (sendIndex < 0) // För mycket åt vänster
		sendIndex = (playerAmount + sendIndex);

	int cardIndex = 0;

	for(auto& itr : hostSendIndex)
	{
		sendCards[sendIndex][cardIndex] = new Card(cards[itr]);
		cardIndex++;
	}

	for (int i = 0; i < clients.size(); i++)
	{
		sf::Packet packet;
		packet << THREE_CARDS_GET;

		for (int k = 0; k < 3; k++)
		{
			packet << (int)sendCards[i][k]->suit;
			packet << (int)sendCards[i][k]->rank;
		}

		clients[i]->socket.send(packet);

	}

	// Remove sent cards from host
	int counterIndex = 0;

	cards.erase(
		std::remove_if(cards.begin(), cards.end(),
			[&](Card c)
			{
				return hostSendIndex.find(counterIndex++) != hostSendIndex.end();
			}), cards.end()
	);

	// Insert gotten cards
	for (int i = 0; i < 3; i++)
		cards.push_back(*sendCards[clients.size()][i]);

	// Reset variables
	hostSendIndex.clear();
	
	for (int i = 0; i < playerAmount; i++)
		for (int k = 0; k < 3; k++)
		{
			delete sendCards[i][k];
			sendCards[i][k] = nullptr;
		}


	for (int i = 0; i < 3; i++)
		chosenSendCards[i].setPosition({ 1000.f, 900.f });

	for (int i = 0; i < roundPoints.size(); i++)
	{
		roundPoints[i] = 0;
		UpdateRoundPoints(i);
	}
	

	ShowHand();

	cardSwitched = true;
	chooseSendCards = false;


}

void Host::TryReceiveGame()
{
	sf::Packet packet;
	int type = 0;
	int index = -1;

	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i]->socket.receive(packet) == sf::Socket::Status::Done)
		{
			index = i;
			break;
		}
	}
	
	if (index == -1)
		return;


	packet >> type;

	// Debug
	//std::cout << "Type " << TypeStrings.find(type)->second << std::endl;

	if (type == THREE_CARDS_GIVE)
		GetSwitch(packet, index);
	else if (type == I_HAVE_CARD)
		GetLowestIndex(packet, index);
	else if (type == CARD_PLAYED)
		GetPlayed(packet, index);


}

void Host::SendLowest()
{
	for (int i = 0; i < cards.size(); i++)
	{
		if (cards[i] == lowestCard)
		{
			turn = clients.size();
			//std::cout << "Host has lowest card" << std::endl;

			SendTurn();

			firstCard = new Card(lowestCard);

			return;
		}
	}

	//std::cout << "Sending lowest flag and card" << std::endl;

	sf::Packet packet;

	packet << WHO_HAS_CARD;
	packet << (int)lowestCard.suit;
	packet << (int)lowestCard.rank;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

	turn = -2;
}

void Host::GetLowestIndex(sf::Packet packet, int index)
{
	turn = index;

	SendTurn();

	//std::cout << clients[index]->name << " has lowest card" << std::endl;

}

void Host::SendTurn()
{

	sf::Packet packet;

	packet << PLAYER_TURN;
	packet << turn;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

	/* Update stuff */



	playerDot.setPosition({ playerNames[turn].getPosition().x, 10.f });

	ShowHand();

}

void Host::GetPlayed(sf::Packet packet, int index)
{
	int suit = 0;
	int rank = 0;

	packet >> suit;
	packet >> rank;

	std::string first = "";
	first += (char)suit;
	first += (char)rank;

	if (cardsPlayed == 0)
	{
		followSuit = (char)suit;
		SendSuit();
	}

	if (playedCards[0] != nullptr &&
		playedCards[1] != nullptr &&
		playedCards[2] != nullptr)
	{
		for (int i = 0; i < 3; i++)
		{
			delete playedCards[i];
			delete playedCardsSprites[i];

			playedCards[i] = nullptr;
			playedCardsSprites[i] = nullptr;
		}
	}

	playedCards[index] = new Card(suit, rank);
	playedCardsSprites[index] = new sf::Sprite(manager->cardTexture.find(first)->second);
	playedCardsSprites[index]->setPosition({ playerNames[index].getPosition().x, 100.f });

	ShowHand();

	SendPlayed(Card(suit, rank), index);

	turn++;
	cardsPlayed++;

	// Check if everyone put down card
	if (cardsPlayed == playerAmount)
		EndStick();
	else
	{
		if (turn > playerAmount)
			turn = 0;

		SendTurn();
	}
}

void Host::SendSuit()
{
	sf::Packet packet;

	packet << FOLLOW_SUIT;

	packet << (int)followSuit;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);
}

void Host::SendPlayed(Card card, int index)
{
	sf::Packet packet;

	packet << X_PLAYED_CARD;

	packet << index;

	packet << (int)card.suit;
	packet << (int)card.rank;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

}

void Host::PlayCard(Card card)
{
	std::string first = "";
	first += (char)card.suit;
	first += (char)card.rank;

	if (cardsPlayed == 0)
	{
		followSuit = card.suit;
		SendSuit();
	}

	if (playedCards[0] != nullptr &&
		playedCards[1] != nullptr &&
		playedCards[2] != nullptr)
	{
		for (int i = 0; i < 3; i++)
		{
			delete playedCards[i];
			delete playedCardsSprites[i];

			playedCards[i] = nullptr;
			playedCardsSprites[i] = nullptr;
		}
	}

	playedCards[clients.size()] = new Card(card);
	playedCardsSprites[clients.size()] = new sf::Sprite(manager->cardTexture.find(first)->second);
	playedCardsSprites[clients.size()]->setPosition({playerNames[clients.size()].getPosition().x, 100.f});

	cards.erase(
		std::remove_if(cards.begin(), cards.end(),
			[card](Card c)
			{
				return card.suit == c.suit && card.rank == c.rank;
			}), cards.end()
				);

	ShowHand();

	SendPlayed(card, clients.size());

	turn++;
	cardsPlayed++;

	// Check if everyone put down card
	if (cardsPlayed == playerAmount)
		EndStick();
	else
	{
		if (turn >= playerAmount)
			turn = 0;

		SendTurn();
	}

}

bool Host::HaveSuit() // Check if player has follow suit
{
	for (int i = 0; i < cards.size(); i++)
		if (cards[i].suit == followSuit)
			return true;

	return false;
}

void Host::EndStick()
{
	int points = 0;
	int whoBegun = turn;

	// Kolla så att allting fungerar
	if (whoBegun == 3)
		whoBegun = 0;


	int whoLost = whoBegun;
	char highestRank = playedCards[whoBegun]->rank;

	for (int i = 0; i < playedCards.size(); i++)
	{
		if (playedCards[i]->suit == 'H')
			points++;
		else if (playedCards[i]->suit == 'S' && playedCards[i]->rank == 'Q')
			points += 130;

		if (playedCards[i]->suit == followSuit
			&& CharValues.find(playedCards[i]->rank)->second > CharValues.find(highestRank)->second)
		{
			highestRank = playedCards[i]->rank;
			whoLost = i;
		}
	}

	if (points > 0 && pointsPlayed == false)
	{
		pointsPlayed = true;
		SendPointsBool();
	}

	roundPoints[whoLost] += points;
	
	UpdateRoundPoints(whoLost);

	cardsPlayed = 0;
	turn = whoLost;
	followSuit = 'N';
	stickNumber++;

	// Check if no cards are left in hand, then round has ended
	if (cards.size() == 0)
		EndRound();
	else
	{
		SendSuit();
		SendTurn();
	}

	if(!lost)
		ShowHand();

}

void Host::SendPointsBool()
{
	sf::Packet packet;

	packet << POINTS_BOOL;

	packet << pointsPlayed;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);
}

void Host::UpdateRoundPoints(int index)
{
	if (roundPoints[index] == 26)
	{
		for (int i = 0; i < roundPoints.size(); i++)
			if (i == index)
				roundPoints[i] = 0;
			else
				roundPoints[i] = 26;

		for (int i = 0; i < roundPoints.size(); i++)
			roundPointsT[i].setString(
				playerNames[i].getString() + ": " + std::to_string(roundPoints[i])
			);

		for (int k = 0; k < roundPoints.size(); k++)
			SendRoundPoints(k);
		
		
	}
	else
	{
		roundPointsT[index].setString(
			playerNames[index].getString() + ": " + std::to_string(roundPoints[index])
		);

		SendRoundPoints(index);
	}

}

void Host::UpdateTotalPoints()
{
	for (int i = 0; i < roundPoints.size(); i++)
	{
		totalPoints[i] += roundPoints[i];
		totalPointsT[i].setString(
			playerNames[i].getString() + ": " + std::to_string(totalPoints[i])
		);
	}
	
}

void Host::SendRoundPoints(int index)
{
	sf::Packet packet;

	packet << ROUND_POINTS;

	packet << index;

	packet << roundPoints[index];

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

}

void Host::SendTotalPoints()
{
	sf::Packet packet;

	packet << TOTAL_POINTS;

	for (int i = 0; i < totalPoints.size(); i++)
		packet << totalPoints[i];
	

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);
}

void Host::EndRound()
{
	round++;

	UpdateTotalPoints();
	SendTotalPoints();

	sf::Packet packet;
	packet << NEW_ROUND;

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);

	for (int i = 0; i < playerAmount; i++)
	{
		delete playedCards[i];
		delete playedCardsSprites[i];

		playedCards[i] = nullptr;
		playedCardsSprites[i] = nullptr;
	}

	turn = -1;
	cardsPlayed = 0;
	playerDot.setPosition({ 0.f, 0.f });
	pointsPlayed = false;
	followSuit = 'N';
	stickNumber = 1;
	switchReceived = 0;
	switchSent = 0;
	chooseSendCards = true;
	cardSwitched = false;
	hostSendIndex.clear();

	for (int i = 0; i < playerAmount; i++)
	{
		for(int k = 0; k < 3; k++)
		{
			delete sendCards[i][k];
			sendCards[i][k] = nullptr;
		}
	}

	delete firstCard;
	firstCard = nullptr;

	playersCards.clear();

	SetUpGame();
	SendCards();

	for (int i = 0; i < totalPoints.size(); i++)
	{
		if (totalPoints[i] >= 50)
			EndHearts();
	}

}

void Host::EndHearts()
{
	lost = true;

	sf::Text losers(font);
	std::string loserStr = "";

	for (int i = 0; i < totalPoints.size(); i++)
	{
		if (totalPoints[i] >= 50)
			loserStr += ", " + playerNames[i].getString();
	}

	loserStr = loserStr.substr(2, loserStr.size() - 1);
	loserStr += " Förlorade";

	losers.setString(loserStr);
	losers.setPosition({ 10.f, 50.f });

	std::map<int, std::string> ranking;

	for (int i = 0; i < totalPoints.size(); i++)
		ranking.insert({ totalPoints[i], playerNames[i].getString() });

	sf::Packet packet;
	packet << LOSE;

	packet << loserStr;

	std::vector<sf::Text> rankingsT;

	for (auto& itr : ranking)
	{
		packet << itr.second;
		packet << itr.first;

		rankingsT.push_back(sf::Text(font, itr.second + ": " + std::to_string(itr.first)));
	}

	for (int i = 0; i < clients.size(); i++)
		clients[i]->socket.send(packet);



	window->clear();
	
	window->draw(losers);

	for (int i = 0; i < playerAmount; i++)
	{
		rankingsT[i].setPosition({ 50.f, 100.f + i * 30.f });
		window->draw(rankingsT[i]);
	}
	
	window->display();
}

bool Host::HaveSuit(char suit)
{
	for (int i = 0; i < cards.size(); i++)
		if (cards[i].suit == suit)
			return true;

	return false;
}

std::string GetName()
{
    std::string ret = "";

    std::cout << "Skriv ditt namn: ";
    std::cin >> ret;

    return ret;
}

int GetPlayerAmount()
{
    int ret;
    std::cout << "Hur många är ni totalt: ";
    std::cin >> ret;

    return ret;
}

