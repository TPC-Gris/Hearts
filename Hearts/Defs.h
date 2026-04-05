#pragma once
#include "Include.h"

enum PacketType
{
	NAME = 0,			// Client -> Host
	RESEND_NAME,		// Host   -> Client
	NEXT,				// Host   -> Client
	START_CARDS,		// Host   -> Clients
	PEOPLE_NAMES,		// Host   -> Clients
	/* Nytt */
	SWITCH_CARDS,		// Host   -> Clients
	THREE_CARDS_GIVE,   // Client -> Host
	THREE_CARDS_GET,    // Host   -> Client
	WHO_HAS_CARD,		// Host   -> Clients
	I_HAVE_CARD,		// Client -> Host
	PLAYER_TURN,		// Host   -> Clients
	CARD_PLAYED,		// Client -> Host
	FOLLOW_SUIT,		// Host   -> Clients
	X_PLAYED_CARD,		// Host   -> Clients
	POINTS_BOOL,		// Host   -> Clients
	ROUND_POINTS,		// Host	  -> Clients
	TOTAL_POINTS,		// Host   -> Clients
	NEW_ROUND,			// Host   -> Cients
	LOSE				// Host   -> Clients

	/* Gammalt */
	//CARD,
	//TURN,
	//HMM_PUT_CARD,
	//UPDATE_TOTAL,
	//UPDATE_ROUND,
	//OVER
};

// DEBUG
const std::map<int, std::string> TypeStrings =
{
	{NAME, "[ Name ]"},
	{RESEND_NAME, "[ Resend Name ]"},
	{NEXT, "[ Next ]"},
	{START_CARDS, "[ Start Cards ]"},
	{PEOPLE_NAMES, "[ People Names ]"},
	{SWITCH_CARDS, "[ Switch Cards ]"},
	{THREE_CARDS_GIVE, "[ Three Cards Give ]"},
	{THREE_CARDS_GET, "[ Three Cards Get ]"},
	{WHO_HAS_CARD, "[ Who Has Card ]"},
	{I_HAVE_CARD, "[ I Have Card ]"},
	{PLAYER_TURN, "[ Player Turn ]"},
	{CARD_PLAYED, "[ Card Played ]"},
	{FOLLOW_SUIT, "[ Follow Suit ]"},
	{X_PLAYED_CARD, "[ X Played Card ]"},
	{POINTS_BOOL, "[ Points Bool ]"},
	{ROUND_POINTS, "[ Round Points ]"},
	{TOTAL_POINTS, "[ Total Points ]"},
	{NEW_ROUND, "[ New Round ]"},
	{LOSE, "[ Lose ]"}
};

const std::map<char, int> CharValues =
{
	{'A', 14},
	{'K', 13},
	{'Q', 12},
	{'J', 11},
	{'T', 10},
	{'9', 9},
	{'8', 8},
	{'7', 7},
	{'6', 6},
	{'5', 5},
	{'4', 4},
	{'3', 3},
	{'2', 2}
};