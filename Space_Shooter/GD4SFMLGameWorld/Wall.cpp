/*
Charlie Duff
D00183790
added full file
*/
#include "Wall.hpp"
#include "ResourceHolder.hpp"
#include "CategoryID.hpp"
#include "DataTables.hpp"


//added island vector to store island sizes, textureIDs, and categoryIDs
namespace
{
	const std::vector<WallData> Table = initializeWallData();
}


Wall::Wall(WallID type, const TextureHolder& textures) :
	Entity(1000)
	, mType(type)
	, mSprite(textures.get(Table[static_cast<int>(type)].texture), Table[static_cast<int>(type)].textureRect)
{
	sf::FloatRect bounds = mSprite.getLocalBounds();
	mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

sf::FloatRect Wall::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

void Wall::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}

unsigned int Wall::getCategory() const
{
	if (mType == WallID::Wall || mType == WallID::Wall2 || mType == WallID::Wall3 || mType == WallID::Wall4 || mType == WallID::Wall5 || mType == WallID::Wall6)
		return static_cast<int>(CategoryID::Wall1);
	else
		return static_cast<int>(CategoryID::Wall2);
}


