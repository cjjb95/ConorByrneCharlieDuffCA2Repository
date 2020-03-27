/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
#pragma once

#include "ResourceIdentifiers.hpp"
#include "TextureID.hpp"
#include "ActionID.hpp"
#include "CategoryID.hpp"


#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>
#include <functional>
#include <map>

class Aircraft;


struct PlayerData
{
	std::map<sf::Keyboard::Key, ActionID> mKeyBinding;
	CategoryID categoryID;
};


struct Direction
{
	Direction(float angle, float distance)
		:angle(angle), distance(distance)
	{}

	float angle;
	float distance;
};

struct AircraftData
{
	int hitpoints;
	float speed;
	TextureID texture;
	sf::IntRect textureRect;
	sf::Time fireInterval;
	std::vector<Direction> directions;
	bool hasRollAnimation;
};

struct ProjectileData
{
	int damage;
	float speed;
	TextureID texture;
	sf::IntRect textureRect;
};

struct PickupData
{
	std::function<void(Aircraft&)> action;
	TextureID texture;
	sf::IntRect textureRect;
};

struct ParticleData
{
	sf::Color color;
	sf::Time lifetime;
};

struct WallData
{
	TextureID texture;
	sf::IntRect textureRect;
	CategoryID categoryID;
};

std::vector<PlayerData> initializePlayerData();
std::vector<AircraftData> initializeAircraftData();
std::vector<ProjectileData> initializeProjectileData();
std::vector<PickupData> initializePickupData();
std::vector<ParticleData> initializeParticleData();
std::vector<WallData> initializeWallData();


