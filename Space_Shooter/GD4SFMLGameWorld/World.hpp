/*
Charlie Duff
D00183790
*/
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
#include "Wall.hpp"
#include "WallID.hpp"


#include "SFML/System/NonCopyable.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/Graphics/Texture.hpp"

#include <array>

class NetworkNode;

//Forward declaration
namespace sf
{
	class RenderTarget;
}


class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked = false);
	void update(sf::Time dt);
	void draw();
	CommandQueue& getCommandQueue();

	Aircraft* addAircraft(int identifier);
	void removeAircraft(int identifier);
	void setCurrentBattleFieldPosition(float lineY);
	void setWorldHeight(float height);

	sf::FloatRect getViewBounds() const;

	void addEnemy(AircraftID type, float relX, float relY);
	void sortEnemies();

	bool hasAlivePlayer() const;
	bool hasPlayerReachedEnd() const;
	void updateSounds();

	void createPickup(sf::Vector2f position, PickupID type);

	Aircraft* getAircraft(int identifier) const;
	bool pollGameAction(Action& out);
	void setWorldScrollCompensation(float compensation);


private:
	void loadTextures();
	void buildScene();
	void adaptPlayerPosition();
	void adaptPlayerVelocity();
	void handleCollisions();

	void spawnEnemies();
	void addEnemies();

	sf::FloatRect getBattlefieldBounds() const;


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

	std::vector<SpawnPoint>	mEnemySpawnPoints;
	std::vector<Aircraft*> mActiveEnemies;
	std::array<Wall*, 30>mWall;
	BloomEffect	mBloomEffect;
	bool mNetworkedWorld;
	NetworkNode* mNetworkNode;
	SpriteNode* mFinishSprite;
};