// HUGO REY D00262075 : fix the maximum size of aircraft to 100px*100px
// Add Fire() to shoot projectiles.
// Add BulletPosition() to know where to spawn the projectile. It's adding an offset to the bullet to avoid collision at spawn.

#include "Aircraft.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "ResourceHolder.hpp"
#include "ReceiverCategories.hpp"
#include "Texture.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include <iostream>
#include "ProjectileCustom.hpp"
#include "Layers.hpp"
#include <SFML/Window/Mouse.hpp>

namespace
{
	const std::vector<AircraftData> Table = InitializeAircraftData();
}

Texture ToTextureID(AircraftType type)
{
	switch (type)
	{
	case AircraftType::kPlayer1:
		return Texture::kPlayer1;
		break;
	case AircraftType::kPlayer2:
		return Texture::kPlayer2;
		break;
	}
	return Texture::kPlayer1;
}

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts, SceneNode* m_air_layer)
	: Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type) 
	, m_sprite(textures.Get(ToTextureID(type)))
	, m_fire_rate(1)
	, m_spread_level(1)
	, m_missile_ammo(2)
	, m_health_display(nullptr)
	, m_missile_display(nullptr)
	, m_travelled_distance(0.f)
	, m_directions_index(0)
	, m_textures(textures)
	, m_air_layer(m_air_layer)
{
	sceneNodeName = "aircraft";
	
	sf::FloatRect bounds = m_sprite.getLocalBounds();
	m_sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	std::string empty_string = "";

	//fix max size of sprite to 100x100
	m_sprite.setScale(100.f / bounds.width, 100.f / bounds.height);

	/*std::unique_ptr<TextNode> health_display(new TextNode(fonts, empty_string));
	m_health_display = health_display.get();
	health_display->sceneNodeName = "health dislayer";
	AttachChild(std::move(health_display));*/

	/*if (GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft))
	{
		std::unique_ptr<TextNode> missile_display(new TextNode(fonts, empty_string));
		missile_display->setPosition(0, 70);
		m_missile_display = missile_display.get();
		AttachChild(std::move(missile_display));
	}*/
	UpdateTexts();

}

unsigned int Aircraft::GetCategory() const
{
	switch (m_type)
	{
	case AircraftType::kPlayer1:
		return static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
		
	case AircraftType::kPlayer2:
		return static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
		
	default:
		return static_cast<unsigned int>(ReceiverCategories::kEnemyAircraft);

	}
	//return static_cast<unsigned int>(ReceiverCategories::kDamagable);
}

void Aircraft::IncreaseFireRate()
{
	if (m_fire_rate < 5)
	{
		++m_fire_rate;
	}
}

void Aircraft::IncreaseFireSpread()
{
	if (m_spread_level < 3)
	{
		++m_spread_level;
	}
}

void Aircraft::CollectMissiles(unsigned int count)
{
	m_missile_ammo += count;
}

void Aircraft::UpdateTexts()
{
	/*m_health_display->SetString(std::to_string(GetHitPoints()) + "HP");
	m_health_display->setPosition(0.f, 50.f);
	m_health_display->setRotation(-getRotation());*/

}

void Aircraft::UpdateMovementPattern(sf::Time dt)
{
	//Enemy AI
	const std::vector<Direction>& directions = Table[static_cast<int>(m_type)].m_directions;
	if (!directions.empty())
	{
		//Move along the current direction, then change direction
		if (m_travelled_distance > directions[m_directions_index].m_distance)
		{
			m_directions_index = (m_directions_index + 1) % directions.size();
			m_travelled_distance = 0;
		}

		//Compute velocity
		double radians = Utility::ToRadians(directions[m_directions_index].m_angle + 90.f);
		float vx = GetMaxSpeed() * std::cos(radians);
		float vy = GetMaxSpeed() * std::sin(radians);

		SetVelocity(vx, vy);
		m_travelled_distance += GetMaxSpeed() * dt.asSeconds();



	}
}

float Aircraft::GetMaxSpeed() const
{
	return Table[static_cast<int>(m_type)].m_speed;
}

void Aircraft::Fire()
{
	//print to console to check if firing
	std::cout << "Firing" << std::endl;

	//get the direction of the aircraft
	float rotation = getRotation();
	
	//create a bullet
	ProjectileCustom::Type bulletType;
	switch (m_type)
	{
	case AircraftType::kPlayer1:
		bulletType = ProjectileCustom::Type::kPlayer1Bullet;
		break;
	case AircraftType::kPlayer2:
		bulletType = ProjectileCustom::Type::kPlayer2Bullet;
		break;

	default:
		bulletType = ProjectileCustom::Type::kPlayer1Bullet;
		break;
	}

	std::unique_ptr<ProjectileCustom> bullet(new ProjectileCustom(bulletType, m_textures));
	bullet->setPosition(BulletPosition());
	bullet->setRotation(rotation);
	//set the velocity of the bullet depending on the rotation of the aircraft
	sf::Vector2f velocity = sf::Vector2f(std::sin(rotation * 3.14159265 / 180), -std::cos(rotation * 3.14159265 / 180));
	bullet->SetVelocity(800.f * velocity);
	//add bullet to air layout
	m_air_layer->AttachChild(std::move(bullet));
	bullet = nullptr;
	
}

sf::Vector2f Aircraft::BulletPosition()
{
	//put the bullet 10px in front of the aircraft
	float rotation = getRotation();
	float offset = 50.f;
	sf::Vector2f pos = getPosition();
	
	//get the position of the bullet depending on the rotation of the aircraft
	//get direction vector
	sf::Vector2f direction = sf::Vector2f(std::sin(rotation * 3.14159265 / 180), -std::cos(rotation * 3.14159265 / 180));
	//normalize direction vector
	direction = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
	//multiply direction vector by offset
	direction = direction * offset;
	//add direction vector to position
	pos = pos + direction;	

	return pos;
}

void Aircraft::ApplyDamage(float damage)
{
	std::cout << "I took damage" << std::endl;	
	Damage((int)damage);

}


AircraftType Aircraft::GetType()
{
	return m_type;
}

void Aircraft::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

void Aircraft::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	UpdateTexts();
	Entity::UpdateCurrent(dt, commands);
}

void Aircraft::RotateInMouseDirection(sf::Vector2i mousePos, sf::RenderWindow& window)
{
	
	//log to console the mouse pos
	//std::cout << "mouse pos: " << mousePos.x << "," << mousePos.y << std::endl;

	//get current position in the screen (between 1920x1080)
	sf::Vector2i curPos = window.mapCoordsToPixel(getPosition());
		
	//std::cout << "curPos pos: " << curPos.x << "," << curPos.y << std::endl;
	const float PI = 3.14159265;

	float dx = curPos.x - mousePos.x;
	float dy = curPos.y - mousePos.y;
	float rotation = atan2f(dx, dy) * 180 / PI;


	//log to console
	//std::cout << "Rotation: " << rotation << std::endl;
	setRotation(-rotation);
}
