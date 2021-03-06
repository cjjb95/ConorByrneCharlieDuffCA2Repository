/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
#include "World.hpp"
#include "ParticleID.hpp"
#include "ParticleNode.hpp"
#include "NetworkNode.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include "EmitterNode.hpp"


World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked)
	: mTarget(outputTarget)
	, mSceneTexture()
	, mCamera(outputTarget.getDefaultView())
	, mFonts(fonts)
	, mSounds(sounds)
	, mTextures()
	, mSceneGraph()
	, mSceneLayers()
	, mWorldBounds(0.f, 0.f, 5000.f, mCamera.getSize().y) //CD - worldbounds x value is now 5000 long rather than the starting 2000, this will carry over to both local on online, bounds were flipped from original to swap from top down to sidescroller 
	, mSpawnPosition(mCamera.getSize().x / 2.f, mWorldBounds.height - mCamera.getSize().y / 2.f)
	, mScrollSpeed(-100.f) //CD -Changed to increase speed of game and make it more fast paced, worldbounds was also adjusted for this reason
	,mScrollSpeedCompensation(1.f)
	, mPlayerAircraft()
	, mEnemySpawnPoints()
	, mActiveEnemies()
	, mSpawnPositions()
	,mWall() //CD - walls added to world
	,mNetworkedWorld(networked)
	,mNetworkNode(nullptr) //CD - Netword node added
	,mFinishSprite(nullptr)

{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);
	loadTextures();
	buildScene();

	// Prepare the view
	mCamera.setCenter(mSpawnPosition);
}

void World::setWorldScrollCompensation(float compensation)
{
	mScrollSpeedCompensation = compensation;
}

void World::update(sf::Time dt)
{
	// Scroll the world, reset player velocity
	mCamera.move(-mScrollSpeed * dt.asSeconds()*mScrollSpeedCompensation,0.f);

	for (Aircraft* player : mPlayerAircraft)
	{
		player->setVelocity(0.f, 0.f);
		//player->setRotation(90);  CD - was used to reposition texture when using older textures
	}

	// Setup commands to destroy entities, and guide missiles
	destroyEntitiesOutsideView();
	guideMissiles();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove aircrafts that were destroyed (World::removeWrecks() only destroys the entities, not the pointers in mPlayerAircraft)
	auto firstToRemove = std::remove_if(mPlayerAircraft.begin(), mPlayerAircraft.end(), std::mem_fn(&Aircraft::isMarkedForRemoval));
	mPlayerAircraft.erase(firstToRemove, mPlayerAircraft.end());

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

void World::createPickup(sf::Vector2f position, PickupID type)
{
	std::unique_ptr<Pickup> pickup(new Pickup(type, mTextures));
	pickup->setPosition(position);
	pickup->setVelocity(0.f, 1.f);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(pickup));
}

Aircraft* World::getAircraft(int identifier) const
{
	for (Aircraft* a : mPlayerAircraft)
	{
		if (a->getIdentifier() == identifier)
			return a;
	}

	return nullptr;
}


void World::removeAircraft(int identifier)
{
	Aircraft* aircraft = getAircraft(identifier);
	if (aircraft)
	{
		aircraft->destroy();
		mPlayerAircraft.erase(std::find(mPlayerAircraft.begin(), mPlayerAircraft.end(), aircraft));
	}
}


Aircraft* World::addAircraft(int identifier)
{
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftID::Eagle, mTextures, mFonts));
	player->setPosition(mCamera.getCenter());
	player->setIdentifier(identifier);

	mPlayerAircraft.push_back(player.get());
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(player));
	return mPlayerAircraft.back();
}

bool World::hasAlivePlayer() const
{
	return mPlayerAircraft.size() > 0;
}


void World::setCurrentBattleFieldPosition(float lineY)
{
	mCamera.setCenter(mCamera.getCenter().x, lineY - mCamera.getSize().y / 2);
	mSpawnPosition.y = mWorldBounds.height;
}


bool World::pollGameAction(Action& out)
{
	return mNetworkNode->pollGameAction(out);
}

void World::setWorldHeight(float height)
{
	mWorldBounds.height = height;
}

bool World::hasPlayerReachedEnd() const
{
	if (Aircraft* aircraft = getAircraft(1))
	{
		return !mWorldBounds.contains(aircraft->getPosition());
	}
	else
	{
		return false;
	}
}

void World::updateSounds()
{
	sf::Vector2f listenerPosition;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (mPlayerAircraft.empty())
	{
		listenerPosition = mCamera.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for (Aircraft* aircraft : mPlayerAircraft)
			listenerPosition += aircraft->getWorldPosition();

		listenerPosition /= static_cast<float>(mPlayerAircraft.size());
	}
	//Set the listener to the player position
	mSounds.setListenPosition(listenerPosition);
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
	mTextures.load(TextureID::Wall, "Media/Textures/wall.png");
	mTextures.load(TextureID::Wall2, "Media/Textures/wall2.png");
	mTextures.load(TextureID::Wall3, "Media/Textures/wall_barricade.png");
	mTextures.load(TextureID::Wall4, "Media/Textures/wall_barricade2.png");
	mTextures.load(TextureID::Wall5, "Media/Textures/wall_barricade3.png");
	mTextures.load(TextureID::Wall6, "Media/Textures/wall_barricade4.png");
	mTextures.load(TextureID::Robot, "Media/Textures/Robot_sheet.png");
	mTextures.load(TextureID::Enemy1, "Media/Textures/EnemyShip.png");
	mTextures.load(TextureID::Enemy2, "Media/Textures/EnemyShip2.png");
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
			player.increaseScore();
			std::cout << player.getScore() << std::endl;
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

		else if (matchesCategories(pair, CategoryID::PlayerAircraft, CategoryID::Wall1)) //CD - if player collides with wall, they die
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to aircraft, destroy projectile
			aircraft.damage(200);
		}

		else if (matchesCategories(pair, CategoryID::AlliedProjectile, CategoryID::Wall1)) //CD - if player projectiles collide with a wall they are destroyed
		{
			auto& projectile = static_cast<Aircraft&>(*pair.first);
			// destroy projectile if it hits a wall
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
	finishSprite->setPosition(5000.f, -100.f);
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

	// Add network node, if necessary
	if (mNetworkedWorld)
	{
		std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
		mNetworkNode = networkNode.get();
		mSceneGraph.attachChild(std::move(networkNode));
	}

	//�dd walls - Both Obsticle and Border Walls
	//CD - Walls added for a border along with obsticles. both walls cause death on impact
	#pragma region ObsticleWalls

	//obsticle walls
	std::unique_ptr<Wall> Wall7(new Wall(WallID::Wall3, mTextures));
	mWall[6] = Wall7.get();
	mWall[6]->setPosition(1000, 565);
	mWall[6]->scale(1, 1);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall7));

	std::unique_ptr<Wall> Wall8(new Wall(WallID::Wall6, mTextures));
	mWall[7] = Wall8.get();
	mWall[7]->setPosition(700, 100);
	mWall[7]->scale(1, 1);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall8));

	std::unique_ptr<Wall> Wall9(new Wall(WallID::Wall5, mTextures));
	mWall[8] = Wall9.get();
	mWall[8]->setPosition(1500, 565);
	mWall[8]->scale(1, 1);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall9));

	std::unique_ptr<Wall> Wall10(new Wall(WallID::Wall4, mTextures));
	mWall[9] = Wall10.get();
	mWall[9]->setPosition(3500, 100);
	mWall[9]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall10));

	std::unique_ptr<Wall> Wall21(new Wall(WallID::Wall6, mTextures));
	mWall[20] = Wall21.get();
	mWall[20]->setPosition(2200, 100);
	mWall[20]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall21));

	std::unique_ptr<Wall> Wall22(new Wall(WallID::Wall4, mTextures));
	mWall[21] = Wall22.get();
	mWall[21]->setPosition(3000, 100);
	mWall[21]->scale(1, 1);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall22));

	std::unique_ptr<Wall> Wall23(new Wall(WallID::Wall3, mTextures));
	mWall[22] = Wall23.get();
	mWall[22]->setPosition(3000, 565);
	mWall[22]->scale(1, 1);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall23));

	std::unique_ptr<Wall> Wall24(new Wall(WallID::Wall3, mTextures));
	mWall[23] = Wall24.get();
	mWall[23]->setPosition(4000, 565);
	mWall[23]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall24));

	std::unique_ptr<Wall> Wall25(new Wall(WallID::Wall4, mTextures));
	mWall[24] = Wall25.get();
	mWall[24]->setPosition(4350, 100);
	mWall[24]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall25));

#pragma endregion

	#pragma region BorderWalls
	//Border walls
	//top
	std::unique_ptr<Wall> Wall1(new Wall(WallID::Wall, mTextures));
	mWall[0] = Wall1.get();
	mWall[0]->setPosition(600, 10);
	mWall[0]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall1));

	std::unique_ptr<Wall> Wall2(new Wall(WallID::Wall, mTextures));
	mWall[1] = Wall2.get();
	mWall[1]->setPosition(1200, 10);
	mWall[1]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall2));

	std::unique_ptr<Wall> Wall11(new Wall(WallID::Wall, mTextures));
	mWall[10] = Wall11.get();
	mWall[10]->setPosition(1800, 10);
	mWall[10]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall11));

	std::unique_ptr<Wall> Wall3(new Wall(WallID::Wall, mTextures));
	mWall[2] = Wall3.get();
	mWall[2]->setPosition(2400, 10);
	mWall[2]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall3));

	std::unique_ptr<Wall> Wall12(new Wall(WallID::Wall, mTextures));
	mWall[11] = Wall12.get();
	mWall[11]->setPosition(3000, 10);
	mWall[11]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall12));

	std::unique_ptr<Wall> Wall13(new Wall(WallID::Wall, mTextures));
	mWall[12] = Wall13.get();
	mWall[12]->setPosition(3600, 10);
	mWall[12]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall13));

	std::unique_ptr<Wall> Wall14(new Wall(WallID::Wall, mTextures));
	mWall[13] = Wall14.get();
	mWall[13]->setPosition(4200, 10);
	mWall[13]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall14));

	std::unique_ptr<Wall> Wall15(new Wall(WallID::Wall, mTextures));
	mWall[14] = Wall15.get();
	mWall[14]->setPosition(4800, 10);
	mWall[14]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall15));

	//bottom
	std::unique_ptr<Wall> Wall4(new Wall(WallID::Wall2, mTextures));
	mWall[3] = Wall4.get();
	mWall[3]->setPosition(600, 710);
	mWall[3]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall4));

	std::unique_ptr<Wall> Wall5(new Wall(WallID::Wall2, mTextures));
	mWall[4] = Wall5.get();
	mWall[4]->setPosition(1200, 710);
	mWall[4]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall5));

	std::unique_ptr<Wall> Wall6(new Wall(WallID::Wall2, mTextures));
	mWall[5] = Wall6.get();
	mWall[5]->setPosition(1800, 710);
	mWall[5]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall6));

	std::unique_ptr<Wall> Wall16(new Wall(WallID::Wall2, mTextures));
	mWall[15] = Wall16.get();
	mWall[15]->setPosition(2400, 710);
	mWall[15]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall16));


	std::unique_ptr<Wall> Wall17(new Wall(WallID::Wall2, mTextures));
	mWall[16] = Wall17.get();
	mWall[16]->setPosition(3000, 710);
	mWall[16]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall17));

	std::unique_ptr<Wall> Wall18(new Wall(WallID::Wall2, mTextures));
	mWall[17] = Wall18.get();
	mWall[17]->setPosition(3600, 710);
	mWall[17]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall18));

	std::unique_ptr<Wall> Wall19(new Wall(WallID::Wall2, mTextures));
	mWall[18] = Wall19.get();
	mWall[18]->setPosition(4200, 710);
	mWall[18]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall19));

	std::unique_ptr<Wall> Wall20(new Wall(WallID::Wall2, mTextures));
	mWall[19] = Wall20.get();
	mWall[19]->setPosition(4800, 710);
	mWall[19]->scale(3, 3);
	mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(Wall20));
#pragma endregion

	//Add Enemies
	addEnemies();
}


void World::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;
	for (Aircraft* aircraft : mPlayerAircraft)
	{
		sf::Vector2f position = aircraft->getPosition();
		position.x = std::max(position.x, viewBounds.left + borderDistance);
		position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
		position.y = std::max(position.y, viewBounds.top + borderDistance);
		position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
		aircraft->setPosition(position);
	}
}

void World::adaptPlayerVelocity()
{
	for (Aircraft* aircraft : mPlayerAircraft)
	{
		sf::Vector2f velocity = aircraft->getVelocity();

		// If moving diagonally, reduce velocity (to have always same velocity)
		if (velocity.x != 0.f && velocity.y != 0.f)
			aircraft->setVelocity(velocity / std::sqrt(2.f));

		// Add scrolling velocity
		aircraft->accelerate(-mScrollSpeed, 0.f);
		//CD -Scrolling was flipped so player scrolls horizontally rather than vertically
	}
}

void World::addEnemies()
{
	if (mNetworkedWorld)
	{
		return;
	}

	// Add enemies to the spawn point container
	addEnemy(AircraftID::Avenger, 500.f, 0.f);
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
	sortEnemies();
}

void World::sortEnemies()
{
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
		

		if (mNetworkedWorld)
		{
			enemy->disablePickups();
		}

		mSceneLayers[static_cast<int>(LayerID::UpperAir)]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}


void World::destroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(CategoryID::Projectile);
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