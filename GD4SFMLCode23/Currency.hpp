//HUGO REY D00262075 : C

#pragma once
#include "Entity.hpp"
#include "AircraftType.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include "TextNode.hpp"
#include "ResourceHolder.hpp"


class Currency : public Entity
{
public:
	Currency(float radius, const TextureHolder& textures);
	unsigned int GetCategory() const override;
	int value;
private:
	sf::Sprite m_sprite;
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	int m_rotationDirection;
};

