#pragma once


class Host
{
public:
	Host();
	~Host();

	void TryAccept();
	void TryReceive(int clientIndex);
	void GetNames();

	

	// Bug functions
	void Run(sf::RenderWindow* w, ImageManager* im);
	void AcceptPlayersAndNames();
	void SetUpGame();
	void SendCards();
	void SendNames();
	void ShowHand();
	void PlayGame();

	//void SendTurn();
	//void TryGetPlayedCard();
	//void SendCardPlayed();
	//void PlayCard(Card card);
	//void UpdateTotal();
	//void UpdateRound();
	//void SomeoneLost(int lI, int wI, int lP, int wP);
	//void CalculateRound();
	//void CalculateGame();

	void Events();

	void SendSwitchFlag();
	void GetSwitch(sf::Packet packet, int index);
	void SendSwitchCards();
	void TryReceiveGame();
	void SendLowest();
	void GetLowestIndex(sf::Packet packet, int index);
	void SendTurn();
	void GetPlayed(sf::Packet packet, int index);
	void SendSuit();
	void SendPlayed(Card card, int index);
	void PlayCard(Card card);
	bool HaveSuit();
	void EndStick();
	void SendPointsBool();
	void UpdateRoundPoints(int index);
	void UpdateTotalPoints();
	void SendRoundPoints(int index);
	void SendTotalPoints();
	void EndRound();
	void EndHearts();
	bool HaveSuit(char suit);
	

	sf::RenderWindow* window = nullptr;
	ImageManager* manager = nullptr;

	sf::TcpListener listener;
	std::vector<Client*> clients;
	std::string name = "";
	int playerAmount = 0;
	std::unordered_set<std::string> names;
	/* Ändra helst ingen över detta */


	std::vector<std::vector<Card>> playersCards; // Alla spelares kort

	std::vector<Card> cards; // Host cards
	std::vector<sf::Sprite> cardSprites; // Host card sprites

	sf::Font font;

	std::vector<sf::Text> playerNames;
	std::vector<Card*> playedCards; // Lagda kort
	std::vector<sf::Sprite*> playedCardsSprites; // Lagda kort sprites

	int turn = -1; // Vems tur
	int cardsPlayed = 0; // Hur många kort som lagts

	std::vector<int> totalPoints;
	std::vector<int> roundPoints;

	std::vector<sf::Text> totalPointsT;
	std::vector<sf::Text> roundPointsT;

	sf::CircleShape playerDot;

	sf::Text winText;
	sf::Text loseText;

	bool pointsPlayed = false;
	char followSuit = 'N';
	int stickNumber = 1;
	int switchReceived = 0;
	bool switchSent = false;
	bool chooseSendCards = false;
	bool cardSwitched = false;
	std::unordered_set<int> hostSendIndex;

	Card* sendCards[3][3]; // [Index] [Cards]

	std::vector<sf::RectangleShape> chosenSendCards;

	Card lowestCard;

	Card* firstCard = nullptr;

	bool lost = false;

	int round = 1;

};

