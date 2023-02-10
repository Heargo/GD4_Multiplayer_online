#include "Currency.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "ResourceHolder.hpp"
#include <iostream>

Currency::Currency(float radius, const TextureHolder& textures)
	:Entity(radius)
	, m_sprite(textures.Get(Texture::kAsteroid))
	, m_rotationDirection(1)
{
	value = 1*(radius/10);
	//set origin at the center of the sprite
	sf::FloatRect bounds = m_sprite.getLocalBounds();
	m_sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	
	//random rotation direction (-1 or 1)
	m_rotationDirection = (rand() % 2) == 0 ? 1 : -1;
}

void Currency::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

void Currency::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Entity::UpdateCurrent(dt, commands);
	m_sprite.rotate(0.08 * m_rotationDirection);
}


unsigned int Currency::GetCategory() const
{
	return static_cast<unsigned int>(ReceiverCategories::kAsteroid);
}
