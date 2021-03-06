/*
Conor Byrne
D00185566
Charlie Duff
D00183790
*/
#pragma once
#include "SceneNode.hpp"
#include "SFML/Graphics/Sprite.hpp"

class SpriteNode : public SceneNode
{
public:
	explicit SpriteNode(const sf::Texture& texture);
	SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect);

private:
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Sprite mSprite;
};