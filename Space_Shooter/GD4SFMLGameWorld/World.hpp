#pragma once

#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Aircraft.hpp"
#include "LayerID.hpp"
#include "CommandQueue.hpp"
#include "AircraftID.hpp"
#include "Pickup.hpp"
#include "PostEffect.hpp"
#include "BloomEffect.hpp"
#include "SoundNode.hpp"
#include "SoundPlayer.hpp"
#include "NetworkProtocol.hpp"
#include "GameActionID.hpp"

#include "SFML/System/NonCopyable.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Graphics/Texture.hpp"

#include <array>


//Forward declaration
namespace sf
{
	class RenderTarget;
}


class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds);
	void update(sf::Time dt);
	void draw();
	CommandQueue& getCommandQueue();
	bool hasAlivePlayer() const;
	bool hasAlivePlayer2() const;
	bool hasPlayerReachedEnd() const;
	bool hasPlayer2ReachedEnd() const;
	void updateSounds();

private:
	void loadTextures();
	void buildScene();
	void adaptPlayerPosition();
	void adaptPlayerVelocity();
	void handleCollisions();

	void spawnEnemies();
	void addEnemies();
	void addEnemy(AircraftID type, float relX, float relY);

	sf::FloatRect getBattlefieldBounds() const;
	sf::FloatRect getViewBounds() const;

	void destroyEntitiesOutsideView();

	void guideMissiles();

	struct SpawnPoint
	{
		SpawnPoint(AircraftID type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
		{
		}

		AircraftID type;
		float x;
		float y;
	};

private:
	sf::RenderTarget& mTarget;
	sf::RenderTexture mSceneTexture;
	sf::View mCamera;
	TextureHolder mTextures;
	FontHolder& mFonts;
	SoundPlayer& mSounds;

	SceneNode mSceneGraph;
	std::array<SceneNode*, static_cast<int>(LayerID::LayerCount)> mSceneLayers;
	CommandQueue mCommandQueue;

	sf::FloatRect mWorldBounds;
	sf::Vector2f mSpawnPosition;
	std::array<sf::Vector2f, 2>mSpawnPositions;
	float mScrollSpeed;
	float mScrollSpeedCompensation;
	std::vector<Aircraft*> mPlayerAircraft;
	Aircraft* mPlayer2Aircraft;

	std::vector<SpawnPoint>	mEnemySpawnPoints;
	std::vector<Aircraft*> mActiveEnemies;

	BloomEffect	mBloomEffect;
};