/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
#pragma once
#include <SFML/System/Vector2.hpp>
enum class GameActionID
{
	EnemyExplode,
};

struct Action
{
	Action()
	{ // leave uninitialized
	}

	Action(GameActionID type, sf::Vector2f position)
		: type(type)
		, position(position)
	{
	}

	GameActionID type;
	sf::Vector2f position;
};