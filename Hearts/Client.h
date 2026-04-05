#pragma once

class Client
{
public:
	Client();
	~Client();

	void TryConnect(std::string ip);

	void SendName();


	// Big functions
	void Run(sf::RenderWindow* w, ImageManager* im);
	
	void ConnectAndSendName();
	void GetCards();
	void GetNames();
	void ShowHand();
	void PlayGame();

	void Events();

	void TryReceive();
	void SendCards();
	void GetSentCards(sf::Packet packet);
	void CheckLowest(sf::Packet packet);
	void SendLowest();
	void GetTurn(sf::Packet packet);
	void PlayCard(Card card);
	void GetPlayed(sf::Packet packet);
	bool HaveSuit();
	void GetSuit(sf::Packet packet);
	void GetPointsBool(sf::Packet packet);
	void UpdateRoundPoints(int index, int points);
	void GetRoundPoints(sf::Packet packet);
	void UpdateTotalPoints(int index, int points);
	void GetTotalPoints(sf::Packet packet);
	void NewRound(sf::Packet packet);
	void EndHearts(sf::Packet packet);
	bool HaveSuit(char suit);

	sf::RenderWindow* window = nullptr;
	ImageManager* manager = nullptr;

	sf::TcpSocket socket;
	std::string name = "";

	std::vector<Card> cards;

	std::vector<sf::Sprite> cardSprites;

	sf::Font font;

	std::vector<sf::Text> playerNames;
	std::vector<sf::Sprite*> playedCards;


	sf::CircleShape playerDot;

	std::vector<sf::Text> totalPoints;
	std::vector<sf::Text> roundPoints;

	sf::Text winText;
	sf::Text loseText;

	bool myTurn = false;
	int turn = 0;
	bool pointsPlayed = false;
	char followSuit = 'N';
	bool chooseSendCards = false;
	int playerAmount = 0;

	Card* firstCard = nullptr;

	std::unordered_set<int> sendIndex;

	std::vector<sf::RectangleShape> chosenSendCards;

	bool lost = false;

};

