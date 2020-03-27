/*
Charlie Duff
D00183790
*/
#include "DataTables.hpp"
#include "Aircraft.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "AircraftID.hpp"
#include "ProjectileID.hpp"
#include "PickupID.hpp"
#include "PlayerID.hpp"
#include "CategoryID.hpp"
#include "ParticleID.hpp"
#include "Wall.hpp"



std::vector<PlayerData> initializePlayerData()
{
	std::vector<PlayerData> data(static_cast<int>(PlayerID::TypeCount));
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::A] = ActionID::MoveLeft;
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::D] = ActionID::MoveRight;
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::W] = ActionID::MoveUp;
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::S] = ActionID::MoveDown;
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::Space] = ActionID::Fire;
	data[static_cast<int>(PlayerID::Player1)].mKeyBinding[sf::Keyboard::N] = ActionID::LaunchMissile;
	data[static_cast<int>(PlayerID::Player1)].categoryID = CategoryID::PlayerAircraft;

	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::Left] = ActionID::MoveLeft;
	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::Right] = ActionID::MoveRight;
	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::Up] = ActionID::MoveUp;
	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::Down] = ActionID::MoveDown;
	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::Enter] = ActionID::Fire;
	data[static_cast<int>(PlayerID::Player2)].mKeyBinding[sf::Keyboard::M] = ActionID::LaunchMissile;
	data[static_cast<int>(PlayerID::Player2)].categoryID = CategoryID::Player2Aircraft;


	return data;
}

//CD - Edited speed, textures and animation for player aircraft(eagle), edited texture and movement pattern for enemy aircraft(raptor, avenger)
std::vector<AircraftData> initializeAircraftData()
{
	std::vector<AircraftData> data(static_cast<int>(AircraftID::TypeCount));
	data[static_cast<int>(AircraftID::Eagle)].hitpoints = 100;
	data[static_cast<int>(AircraftID::Eagle)].speed = 300.f;
	data[static_cast<int>(AircraftID::Eagle)].fireInterval = sf::seconds(1);
	data[static_cast<int>(AircraftID::Eagle)].textureRect = sf::IntRect(0, 0, 75, 88);
	data[static_cast<int>(AircraftID::Eagle)].texture = TextureID::Robot;
	data[static_cast<int>(AircraftID::Eagle)].hasRollAnimation = true;

	data[static_cast<int>(AircraftID::Eagle2)].hitpoints = 100;
	data[static_cast<int>(AircraftID::Eagle2)].speed = 200.f;
	data[static_cast<int>(AircraftID::Eagle2)].fireInterval = sf::seconds(1);
	data[static_cast<int>(AircraftID::Eagle2)].textureRect = sf::IntRect(0, 0, 48, 64);
	data[static_cast<int>(AircraftID::Eagle2)].texture = TextureID::Entities;
	data[static_cast<int>(AircraftID::Eagle2)].hasRollAnimation = true;

	data[static_cast<int>(AircraftID::Raptor)].hitpoints = 20;
	data[static_cast<int>(AircraftID::Raptor)].speed = 80.f;
	data[static_cast<int>(AircraftID::Raptor)].fireInterval = sf::Time::Zero;
	data[static_cast<int>(AircraftID::Raptor)].texture = TextureID::Enemy1;
	data[static_cast<int>(AircraftID::Raptor)].textureRect = sf::IntRect(0, 0, 123, 66);

	data[static_cast<int>(AircraftID::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(AircraftID::Raptor)].directions.push_back(Direction(-45.f, 160.f));
	data[static_cast<int>(AircraftID::Raptor)].directions.push_back(Direction(+45.f, 80.f));
	data[static_cast<int>(AircraftID::Raptor)].hasRollAnimation = false;

	data[static_cast<int>(AircraftID::Avenger)].hitpoints = 40;
	data[static_cast<int>(AircraftID::Avenger)].speed = 50.f;
	data[static_cast<int>(AircraftID::Avenger)].fireInterval = sf::seconds(2);
	data[static_cast<int>(AircraftID::Avenger)].texture = TextureID::Enemy2;
	data[static_cast<int>(AircraftID::Avenger)].textureRect = sf::IntRect(0, 0, 123, 66);
	data[static_cast<int>(AircraftID::Avenger)].directions.push_back(Direction(+45.f, 50.f));
	data[static_cast<int>(AircraftID::Avenger)].directions.push_back(Direction(0.f, 50.f));
	data[static_cast<int>(AircraftID::Avenger)].directions.push_back(Direction(-45.f, 100.f));
	data[static_cast<int>(AircraftID::Avenger)].directions.push_back(Direction(0.f, 50.f));
	data[static_cast<int>(AircraftID::Avenger)].directions.push_back(Direction(+45.f, 50.f));
	data[static_cast<int>(AircraftID::Avenger)].hasRollAnimation = false;

	return data;
}

std::vector<ProjectileData> initializeProjectileData()
{
	std::vector<ProjectileData> data(static_cast<int>(ProjectileID::TypeCount));

	data[static_cast<int>(ProjectileID::AlliedBullet)].damage = 10;
	data[static_cast<int>(ProjectileID::AlliedBullet)].speed = 400.f;
	data[static_cast<int>(ProjectileID::AlliedBullet)].texture = TextureID::Entities;
	data[static_cast<int>(ProjectileID::AlliedBullet)].textureRect = sf::IntRect(175, 64, 3, 14);

	data[static_cast<int>(ProjectileID::EnemyBullet)].damage = 10;
	data[static_cast<int>(ProjectileID::EnemyBullet)].speed = 300.f;
	data[static_cast<int>(ProjectileID::EnemyBullet)].texture = TextureID::Entities;
	data[static_cast<int>(ProjectileID::EnemyBullet)].textureRect = sf::IntRect(175, 64, 3, 14);


	data[static_cast<int>(ProjectileID::Missile)].damage = 200;
	data[static_cast<int>(ProjectileID::Missile)].speed = 250.f;
	data[static_cast<int>(ProjectileID::Missile)].texture = TextureID::Entities;
	data[static_cast<int>(ProjectileID::Missile)].textureRect = sf::IntRect(160, 64, 15, 32);

	return data;
}

std::vector<PickupData> initializePickupData()
{
	std::vector<PickupData> data(static_cast<int>(PickupID::TypeCount));
	data[static_cast<int>(PickupID::HealthRefill)].texture = TextureID::Entities;
	data[static_cast<int>(PickupID::HealthRefill)].textureRect = sf::IntRect(0, 64, 40, 40);
	data[static_cast<int>(PickupID::HealthRefill)].action = [](Aircraft& a) {a.repair(25); };

	data[static_cast<int>(PickupID::MissileRefill)].texture = TextureID::Entities;
	data[static_cast<int>(PickupID::MissileRefill)].textureRect = sf::IntRect(40, 64, 40, 40);
	data[static_cast<int>(PickupID::MissileRefill)].action = std::bind(&Aircraft::collectMissiles, std::placeholders::_1, 3);

	data[static_cast<int>(PickupID::FireSpread)].texture = TextureID::Entities;
	data[static_cast<int>(PickupID::FireSpread)].textureRect = sf::IntRect(80, 64, 40, 40);
	data[static_cast<int>(PickupID::FireSpread)].action = std::bind(&Aircraft::increaseSpread, std::placeholders::_1);

	data[static_cast<int>(PickupID::FireRate)].texture = TextureID::Entities;
	data[static_cast<int>(PickupID::FireRate)].textureRect = sf::IntRect(120, 64, 40, 40);
	data[static_cast<int>(PickupID::FireRate)].action = std::bind(&Aircraft::increaseFireRate, std::placeholders::_1);

	return data;
}

std::vector<ParticleData> initializeParticleData()
{
	std::vector<ParticleData> data(static_cast<int>(ParticleID::ParticleCount));

	data[static_cast<int>(ParticleID::Propellant)].color = sf::Color(255, 255, 50);
	data[static_cast<int>(ParticleID::Propellant)].lifetime = sf::seconds(0.6f);

	data[static_cast<int>(ParticleID::Smoke)].color = sf::Color(50, 50, 50);
	data[static_cast<int>(ParticleID::Smoke)].lifetime = sf::seconds(4.f);

	return data;
}

//CD - Added wall data table, Several types of walls with different textures for say long or short wall or horizontal or vertical walls
std::vector<WallData> initializeWallData()
{
	std::vector<WallData> data(static_cast<int>(WallID::WallCount));
	//horizontal top
	data[static_cast<int>(WallID::Wall)].texture = TextureID::Wall;
	data[static_cast<int>(WallID ::Wall)].textureRect = sf::IntRect(0, 0, 480, 31);
	data[static_cast<int>(WallID::Wall)].categoryID = CategoryID::Wall1;
	//horrizontal bottom
	data[static_cast<int>(WallID::Wall2)].texture = TextureID::Wall2;
	data[static_cast<int>(WallID::Wall2)].textureRect = sf::IntRect(0, 0, 480, 31);
	data[static_cast<int>(WallID::Wall2)].categoryID = CategoryID::Wall1;
	//vertical top
	data[static_cast<int>(WallID::Wall3)].texture = TextureID::Wall3;
	data[static_cast<int>(WallID::Wall3)].textureRect = sf::IntRect(0, 0, 32, 206);
	data[static_cast<int>(WallID::Wall3)].categoryID = CategoryID::Wall1;
	//vertical bottom
	data[static_cast<int>(WallID::Wall4)].texture = TextureID::Wall4;
	data[static_cast<int>(WallID::Wall4)].textureRect = sf::IntRect(0, 0, 32, 206);
	data[static_cast<int>(WallID::Wall4)].categoryID = CategoryID::Wall1;
	//vertical long top
	data[static_cast<int>(WallID::Wall5)].texture = TextureID::Wall5;
	data[static_cast<int>(WallID::Wall5)].textureRect = sf::IntRect(0, 0, 160, 206);
	data[static_cast<int>(WallID::Wall5)].categoryID = CategoryID::Wall1;
	//vertical long bottom
	data[static_cast<int>(WallID::Wall6)].texture = TextureID::Wall6;
	data[static_cast<int>(WallID::Wall6)].textureRect = sf::IntRect(0, 0, 160, 206);
	data[static_cast<int>(WallID::Wall6)].categoryID = CategoryID::Wall1;

	return data;
}