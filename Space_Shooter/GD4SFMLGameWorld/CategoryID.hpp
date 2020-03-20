#pragma once

//Entity/SceneNode category, used to dispatch commands
enum class CategoryID
{
	None = 0,
	SceneAirLayer = 1 << 0,
	PlayerAircraft = 1 << 1,
	Player2Aircraft = 1 << 2,
	AlliedAircraft = 1 << 3,
	EnemyAircraft = 1 << 4,
	Pickup = 1 << 5,
	AlliedProjectile = 1 << 6,
	EnemyProjectile = 1 << 7,
	ParticleSystem = 1 << 8,
	SoundEffect = 1 << 9,
	Network = 1 << 10,

	Aircraft = PlayerAircraft | Player2Aircraft | AlliedAircraft | EnemyAircraft,
	Projectile = AlliedProjectile | EnemyProjectile,
};