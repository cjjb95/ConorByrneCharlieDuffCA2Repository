#include "World.hpp"
#include "ParticleID.hpp"
#include "ParticleNode.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include "EmitterNode.hpp"


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
	: mTarget(outputTarget)
	, mSceneTexture()
	, mCamera(outputTarget.getDefaultView())
	, mFonts(fonts)
	, mSounds(sounds)
	, mTextures()
	, mSceneGraph()
	, mSceneLayers()
	, mWorldBounds(0.f, 0.f, 2000.f, mCamera.getSize().y)
	, mSpawnPosition(mCamera.getSize().x / 2.f, mWorldBounds.height - mCamera.getSize().y / 2.f)
	, mScrollSpeed(-50.f)
	, mPlayerAircraft(nullptr)
	, mPlayer2Aircraft(nullptr)
	, mEnemySpawnPoints()
	, mActiveEnemies()
	, mSpawnPositions()

{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	loadTextures();
	buildScene();

	// Prepare the view
	mCamera.setCenter(mSpawnPosition);
}

void World::update(sf::Time dt)
{
	// Scroll the world, reset player velocity
	mCamera.move(-mScrollSpeed * dt.asSeconds(),0.f);
	mPlayerAircraft->setVelocity(0.f, 0.f);
	mPlayer2Aircraft->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide missiles
	destroyEntitiesOutsideView();
	guideMissiles();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnEnemies();

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();
}

void World::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
		mSceneTexture.setView(mCamera);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
	}
	else
	{
		mTarget.setView(mCamera);
		mTarget.draw(mSceneGraph);
	}
}

CommandQueue& World::getCommandQueue()
{
	return mCommandQueue;
}




bool World::hasAlivePlayer() const
{
	return !mPlayerAircraft->isMarkedForRemoval();
}

bool World::hasAlivePlayer2() const
{
	return !mPlayer2Aircraft->isMarkedForRemoval();
}

bool World::hasPlayerReachedEnd() const
{
	return !mWorldBounds.contains(mPlayerAircraft->getPosition());
}

bool World::hasPlayer2ReachedEnd() const
{
	return !mWorldBounds.contains(mPlayer2Aircraft->getPosition());
}

void World::updateSounds()
{
	//Set the listener to the player position
	mSounds.setListenPosition(mPlayerAircraft->getWorldPosition());
	//Remove unused sounds
	mSounds.removeStoppedSounds();

}

void World::loadTextures()
{
	mTextures.load(TextureID::Entities, "Media/Textures/Entities.png");
	mTextures.load(TextureID::Jungle, "Media/Textures/Jungle.png");
	mTextures.load(TextureID::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(TextureID::Particle, "Media/Textures/Particle.png");
	mTextures.load(TextureID::FinishLine, "Media/Textures/FinishLine.png");
	mTextures.load(TextureID::Space, "Media/Textures/space.png");
}

bool matchesCategories(SceneNode::Pair& colliders, CategoryID type1, CategoryID type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (((static_cast<int>(type1))& category1) && ((static_cast<int>(type2))& category2))
	{
		return true;
	}
	else if (((static_cast<int>(type1))& category2) && ((static_cast<int>(type2))& category1))
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	for (SceneNode::Pair pair : collisionPairs)
	{
		if (matchesCategories(pair, CategoryID::PlayerAircraft, CategoryID::EnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);

			// Collision: Player damage = enemy's remaining HP
			player.damage(enemy.getHitpoints());
			enemy.destroy();
		}

		else if (matchesCategories(pair, CategoryID::PlayerAircraft, CategoryID::Pickup))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			pickup.apply(player);
			player.playerLocalSound(mCommandQueue, SoundEffectID::CollectPickup);
			pickup.destroy();
		}

		else if (matchesCategories(pair, CategoryID::EnemyAircraft, CategoryID::AlliedProjectile)
			|| matchesCategories(pair, CategoryID::PlayerAircraft, CategoryID::EnemyProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to aircraft, destroy projectile
			aircraft.damage(projectile.getDamage());
			projectile.destroy();
		}
	}
}

void World::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(LayerID::LayerCount); ++i)
	{
		CategoryID category = (i == (static_cast<int>(LayerID::LowerAir))) ? CategoryID::SceneAirLayer : CategoryID::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background

	sf::Texture& texture = mTextures.get(TextureID::Space);
	sf::IntRect textureRect(mWorldBounds);
	texture.setRepeated(true);
	

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> backgroundSprite(new SpriteNode(texture, textureRect));
	backgroundSprite->setPosition(mWorldBounds.left, mWorldBounds.top);

	mSceneLayers[static_cast<int>(LayerID::Background)]->attachChild(std::move(backgroundSprite));

	//Add the finish line to the scene
	sf::Texture& finishTexture = mTextures.get(TextureID::FinishLine);
	std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
	finishSprite->setPosition(2010.f, 0.f);
	finishSprite->setRotation(90);
	mSceneLayers[static_cast<int>(LayerID::Background)]->attachChild(std::move(finishSprite));

	//Add particle nodes for smoke and propellant
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleID::Smoke, mTextures));
	mSceneLayers[static_cast<int>(LayerID::LowerAir)]->attachChild(std::move(smokeNode));

	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleID::Propellant, mTextures));
	mSceneLayers[static_cast<int>(LayerID::LowerAir)]->attachChild(std::move(propellantNode));

	//Add the sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));


	sf::Vector2f spawnPoint1(mCamera.getSize().x / 2.f, mWorldBounds.height - mCamera.getSize().y / 2.f);
	sf::Vector2f spawnPoint2((mCamera.getSize().x / 2.f)-60, (mWorldBounds.height - mCamera.getSize().y / 2.f)-60);
	mSpawnPositions[0] = spawnPoint1;
	mSpawnPositions[1] = spawnPoint2;

	// Add player's aircraft
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftID::Eagle, mTextures, mFonts));
	mPlayerAircraft = player.get();
	mPlayerAircraft->setPosition(mSpawnPositions[0]);
	mPlayerAircraft->setRotation(90);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(player));


	// Add player 2's aircraft
	std::unique_ptr<Aircraft> player2(new Aircraft(AircraftID::Eagle2, mTextures, mFonts));
	mPlayer2Aircraft = player2.get();
	mPlayer2Aircraft->setPosition(mSpawnPositions[1]);
	mPlayer2Aircraft->setRotation(90);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(player2));

	addEnemies();
}

void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	sf::Vector2f position = mPlayerAircraft->getPosition();
	position.x = std::max(position.x, viewBounds.left + borderDistance);
	position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
	position.y = std::max(position.y, viewBounds.top + borderDistance);
	position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
	mPlayerAircraft->setPosition(position);

	sf::Vector2f position2 = mPlayer2Aircraft->getPosition();
	position2.x = std::max(position2.x, viewBounds.left + borderDistance);
	position2.x = std::min(position2.x, viewBounds.left + viewBounds.width - borderDistance);
	position2.y = std::max(position2.y, viewBounds.top + borderDistance);
	position2.y = std::min(position2.y, viewBounds.top + viewBounds.height - borderDistance);
	mPlayer2Aircraft->setPosition(position2);
}

void World::adaptPlayerVelocity()
{
	sf::Vector2f velocity = mPlayerAircraft->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity.x != 0.f && velocity.y != 0.f)
		mPlayerAircraft->setVelocity(velocity / std::sqrt(2.f));

	// Add scrolling velocity
	mPlayerAircraft->accelerate(-mScrollSpeed,0.f);


	sf::Vector2f velocity2 = mPlayer2Aircraft->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity2.x != 0.f && velocity2.y != 0.f)
		mPlayer2Aircraft->setVelocity(velocity2 / std::sqrt(2.f));

	// Add scrolling velocity
	mPlayer2Aircraft->accelerate(-mScrollSpeed, 0.f);
}

void World::addEnemies()
{
	// Add enemies to the spawn point container
	addEnemy(AircraftID::Raptor, 500.f, 0.f);
	addEnemy(AircraftID::Raptor, 1000.f, 500.f);
	addEnemy(AircraftID::Raptor, 1150.f, +100.f);
	addEnemy(AircraftID::Raptor, -100.f, 1150.f);
	addEnemy(AircraftID::Avenger, 70.f, 1500.f);
	addEnemy(AircraftID::Avenger, -70.f, 1500.f);

	addEnemy(AircraftID::Avenger, -70.f, 1710.f);
	addEnemy(AircraftID::Avenger, 70.f, 1700.f);
	addEnemy(AircraftID::Avenger, 30.f, 1850.f);
	addEnemy(AircraftID::Raptor, 300.f, 2200.f);
	addEnemy(AircraftID::Raptor, -300.f, 2200.f);
	addEnemy(AircraftID::Raptor, 0.f, 2200.f);
	addEnemy(AircraftID::Raptor, 0.f, 2500.f);
	addEnemy(AircraftID::Avenger, -300.f, 2700.f);
	addEnemy(AircraftID::Avenger, -300.f, 2700.f);
	addEnemy(AircraftID::Raptor, 0.f, 3000.f);
	addEnemy(AircraftID::Raptor, 250.f, 3250.f);
	addEnemy(AircraftID::Raptor, -250.f, 3250.f);
	addEnemy(AircraftID::Avenger, 0.f, 3500.f);
	addEnemy(AircraftID::Avenger, 0.f, 3700.f);
	addEnemy(AircraftID::Raptor, 0.f, 3800.f);
	addEnemy(AircraftID::Avenger, 0.f, 4000.f);
	addEnemy(AircraftID::Avenger, -200.f, 4200.f);
	addEnemy(AircraftID::Raptor, 200.f, 4200.f);
	addEnemy(AircraftID::Raptor, 0.f, 4400.f);

	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [](SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World::addEnemy(AircraftID type, float relX, float relY)
{
	SpawnPoint spawn(type, mSpawnPosition.x + relX, mSpawnPosition.y - relY);
	mEnemySpawnPoints.push_back(spawn);
}

void World::spawnEnemies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty()
		&& mEnemySpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();

		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(270.f);

		mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(CategoryID::Projectile) | static_cast<int>(CategoryID::EnemyAircraft);
	command.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.destroy();
	});

	mCommandQueue.push(command);
}

void World::guideMissiles()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(CategoryID::EnemyAircraft);
	enemyCollector.action = derivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that guides all missiles to the enemy which is currently closest to the player
	Command missileGuider;
	missileGuider.category = static_cast<int>(CategoryID::AlliedProjectile);
	missileGuider.action = derivedAction<Projectile>([this](Projectile& missile, sf::Time)
	{
		// Ignore unguided bullets
		if (!missile.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Aircraft* closestEnemy = nullptr;

		// Find closest enemy
		for (Aircraft* enemy : mActiveEnemies)
		{
			float enemyDistance = distance(missile, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			missile.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(missileGuider);
	mActiveEnemies.clear();
}

sf::FloatRect World::getViewBounds() const
{
	return sf::FloatRect(mCamera.getCenter() - mCamera.getSize() / 2.f, mCamera.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}