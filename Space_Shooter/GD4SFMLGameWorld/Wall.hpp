/*
Conor Byrne
D00185566
Charlie Duff
D00183790
added full file
*/
#pragma once
#include "Entity.hpp"
#include "WallID.hpp"
#include "ResourceIdentifiers.hpp"
#include "ResourceHolder.hpp"

class Wall : public Entity
{
public:
	Wall(WallID type, const TextureHolder& textures);
	virtual sf::FloatRect getBoundingRect() const;
	virtual unsigned int	getCategory() const;
private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;


private:
	//Different Walls can have different textures
	WallID mType;
	sf::Sprite mSprite;

};