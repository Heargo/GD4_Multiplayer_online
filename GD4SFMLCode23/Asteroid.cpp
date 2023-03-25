//HUGO REY D00262075 : Create the asteroid class.

#include "Asteroid.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include "ResourceHolder.hpp"
#include <iostream>
#include <SFML/Graphics/RectangleShape.hpp>

Asteroid::Asteroid(int size, const TextureHolder& textures)
	:Entity(size)
	, m_size(size)
	, m_sprite(textures.Get(Texture::kAsteroid)) //sf::IntRect(228, 0, 60, 59) for testing
{
	//set origin at the center of the sprite
	sf::FloatRect bounds = m_sprite.getLocalBounds();
	m_sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	
	//scale the sprite to the size of the asteroid
	m_sprite.setScale(m_size / bounds.width, m_size / bounds.height);

	//get random rotation direction (-1 or 1)
	m_rotationDirection = (rand() % 2) ==0 ? 1 : -1;	
	
	
	
}

unsigned int Asteroid::GetCategory() const
{
	return static_cast<unsigned int>(ReceiverCategories::kAsteroid);
}

int Asteroid::GetRadius()
{
	//get the radius of the sprite taking scale into account
	return static_cast<int>(std::max(m_sprite.getScale().x, m_sprite.getScale().y) * m_sprite.getTexture()->getSize().x / 2);
}

void Asteroid::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	/*sf::FloatRect bounds = GetBoundingRect();
	sf::RectangleShape rectangle(sf::Vector2f(bounds.width, bounds.height));
	rectangle.setFillColor(sf::Color::Red);
	rectangle.setPosition(bounds.left, bounds.top);
	target.draw(rectangle);*/
	target.draw(m_sprite, states);
}

void Asteroid::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	//Entity::UpdateCurrent(dt, commands);
	m_sprite.rotate(0.05 * m_rotationDirection);
}

sf::FloatRect Asteroid::GetBoundingRect() const
{
	sf::FloatRect bounds = GetWorldTransform().transformRect(m_sprite.getGlobalBounds());

	//limit heigh and width of the bounding box to avoid rotation issues
	bounds.width = std::min(bounds.width, m_size);
	bounds.height = std::min(bounds.height, m_size);
	//std::cout << "bounds" << bounds.top << " " << bounds.left << " " << bounds.width << " " << bounds.height << std::endl;
	//center the bounding box around the sprite
	bounds.left = GetWorldPosition().x - bounds.width / 2.f;
	bounds.top = GetWorldPosition().y - bounds.height / 2.f;

	return bounds;
}
