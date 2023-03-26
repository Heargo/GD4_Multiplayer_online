#include "Aircraft.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include "ResourceHolder.hpp"
#include "ReceiverCategories.hpp"
#include "Texture.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include "ProjectileType.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "PickupType.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"

#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
	const std::vector<AircraftData> Table = InitializeAircraftData();
}

Texture ToTextureID(bool isLocalPlayer)
{
	if(isLocalPlayer)
		return Texture::kLocalPlayer;
	else
		return Texture::kRemotePlayer;
}

Aircraft::Aircraft(AircraftType type, const TextureHolder& textures, const FontHolder& fonts, bool isLocalPlayer) 
	: Entity(Table[static_cast<int>(type)].m_hitpoints)
	, m_type(type) 
	, m_sprite(textures.Get(Table[static_cast<int>(type)].m_texture), Table[static_cast<int>(type)].m_texture_rect)
	, m_explosion(textures.Get(Texture::kExplosion))
	, m_is_firing(false)
	, m_is_launching_missile(false)
	, m_fire_countdown(sf::Time::Zero)
	, m_is_marked_for_removal(false)
	, m_fire_rate(1)
	, m_spread_level(1)
	, m_missile_ammo(2)
	, m_health_display(nullptr)
	, m_missile_display(nullptr)
	, m_travelled_distance(0.f)
	, m_directions_index(0)
	, m_identifier(0)
	, m_show_explosion(true)
	, m_explosion_began(false)
	, m_spawned_pickup(false)
	, m_pickups_enabled(true)
	, m_is_local_player(isLocalPlayer)
	
{

	m_sprite = sf::Sprite(textures.Get(ToTextureID(isLocalPlayer)));
	//set the max size of the sprite to 100x100
	sf::FloatRect bounds = m_sprite.getLocalBounds();
	m_sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	//std::string empty_string = "";

	//fix max size of sprite to 100x100
	m_sprite.setScale(100.f / bounds.width, 100.f / bounds.height);


	m_explosion.SetFrameSize(sf::Vector2i(256, 256));
	m_explosion.SetNumFrames(16);
	m_explosion.SetDuration(sf::seconds(1));

	Utility::CentreOrigin(m_sprite);
	Utility::CentreOrigin(m_explosion);

	m_fire_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_fire_command.action = [this, &textures](SceneNode& node, sf::Time dt)
	{
		CreateBullet(node, textures);
	};

	//m_missile_command.category = static_cast<int>(ReceiverCategories::kScene);
	//m_missile_command.action = [this, &textures](SceneNode& node, sf::Time dt)
	//{
	//	CreateProjectile(node, ProjectileType::kMissile, 0.f, 0.5f, textures);
	//};

	m_drop_pickup_command.category = static_cast<int>(ReceiverCategories::kScene);
	m_drop_pickup_command.action = [this, &textures](SceneNode& node, sf::Time)
	{
		CreatePickup(node, textures);
	};


	std::unique_ptr<TextNode> health_display(new TextNode(fonts, ""));
	m_health_display = health_display.get();
	AttachChild(std::move(health_display));

	if (Aircraft::GetCategory() == static_cast<int>(ReceiverCategories::kPlayerAircraft))
	{
		std::unique_ptr<TextNode> missile_display(new TextNode(fonts, ""));
		missile_display->setPosition(0, 70);
		m_missile_display = missile_display.get();
		AttachChild(std::move(missile_display));
	}
	UpdateTexts();

}

int Aircraft::GetMissileAmmo() const
{
	return m_missile_ammo;
}

bool Aircraft::IsLocalPlayer() const
{
	return m_is_local_player;
}

void Aircraft::SetMissileAmmo(int ammo)
{
	m_missile_ammo = ammo;
}

void Aircraft::DisablePickups()
{
	m_pickups_enabled = false;
}

int	Aircraft::GetIdentifier()
{
	return m_identifier;
}

void Aircraft::SetIdentifier(int identifier)
{
	m_identifier = identifier;
}

unsigned int Aircraft::GetCategory() const
{
	if (IsAllied())
	{
		return static_cast<int>(ReceiverCategories::kPlayerAircraft);
	}
	return static_cast<int>(ReceiverCategories::kEnemyAircraft);
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
	if (IsDestroyed())
	{
		m_health_display->SetString("");
	}
	else
	{
		std::string playerName = "Player " + std::to_string(m_identifier) +"\n";
		m_health_display->SetString(playerName+std::to_string(GetHitPoints()) + "HP");
	}
	m_health_display->setPosition(0.f, 50.f);
	m_health_display->setRotation(-getRotation());

	if (m_missile_display)
	{
		if (m_missile_ammo == 0)
		{
			m_missile_display->SetString("");
		}
		else
		{
			std::string str = "pos " + std::to_string(GetWorldPosition().x) + " " + std::to_string(GetWorldPosition().y) + "\n";
			m_missile_display->SetString(str);
			m_health_display->setPosition(0.f, 100.f);
			m_health_display->setRotation(-getRotation());
		}
	}
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
			m_travelled_distance = 0.f;
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
	if (Table[static_cast<int>(m_type)].m_fire_interval != sf::Time::Zero)
	{
		m_is_firing = true;
	}
}

void Aircraft::LaunchMissile()
{
	if (m_missile_ammo > 0)
	{
		m_is_launching_missile = true;
		--m_missile_ammo;
	}
}

void Aircraft::CreateBullet(SceneNode& node, const TextureHolder& textures) const
{
	ProjectileType type = IsAllied() ? ProjectileType::kAlliedBullet : ProjectileType::kEnemyBullet;	
	//get the position of the bullet depending on the rotation of the aircraft
	float rotation = getRotation();
	float offset = 50.f;
	sf::Vector2f pos = getPosition();
	sf::Vector2f direction = sf::Vector2f(std::sin(rotation * 3.14159265 / 180), -std::cos(rotation * 3.14159265 / 180));
	direction = direction / std::sqrt(direction.x * direction.x + direction.y * direction.y);
	direction = direction * offset;
	pos = pos + direction;
	
	CreateProjectile(node, type, pos, textures);
	/*switch (m_spread_level)
	{
	case 1:
		CreateProjectile(node, type, 0.0f, 0.5f, textures);
		break;
	case 2:
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	case 3:
		CreateProjectile(node, type, 0.0f, 0.5f, textures);
		CreateProjectile(node, type, -0.5f, 0.5f, textures);
		CreateProjectile(node, type, 0.5f, 0.5f, textures);
		break;
	}*/
}


void Aircraft::CreateProjectile(SceneNode& node, ProjectileType type, sf::Vector2f bulletPosition, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures,m_identifier));
	//set the velocity of the bullet depending on the rotation of the aircraft
	float rotation = getRotation();
	sf::Vector2f velocity = sf::Vector2f(std::sin(rotation * 3.14159265 / 180), -std::cos(rotation * 3.14159265 / 180));
	velocity = velocity * 800.f;
	//std::cout << "velocity is (" << velocity.x << "," << velocity.y <<")" << std::endl;
	projectile->SetVelocity(velocity);

	projectile->setPosition(bulletPosition);
	projectile->SetVelocity(velocity);
	projectile->setRotation(getRotation());
	node.AttachChild(std::move(projectile));
}

sf::FloatRect Aircraft::GetBoundingRect() const
{
	sf::FloatRect bounds = GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
	
	//limit heigh and width of the bounding box to avoid rotation issues
	bounds.width = std::min(bounds.width, 40.f);
	bounds.height = std::min(bounds.height, 40.f);
	//std::cout << "bounds" << bounds.top << " " << bounds.left << " " << bounds.width << " " << bounds.height << std::endl;
	//center the bounding box around the sprite
	bounds.left = GetWorldPosition().x - bounds.width / 2.f;
	bounds.top = GetWorldPosition().y - bounds.height / 2.f;
	
	return bounds;
}

bool Aircraft::IsMarkedForRemoval() const
{
	return IsDestroyed() && (m_explosion.IsFinished() || !m_show_explosion);
}

void Aircraft::CheckPickupDrop(CommandQueue& commands)
{
	if (!IsAllied() && Utility::RandomInt(3) == 0 && !m_spawned_pickup)
	{
		commands.Push(m_drop_pickup_command);
	}
	m_spawned_pickup = true;
}

void Aircraft::CreatePickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<PickupType>(Utility::RandomInt(static_cast<int>(PickupType::kPickupCount)));
	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(GetWorldPosition());
	pickup->SetVelocity(0.f, 0.f);
	node.AttachChild(std::move(pickup));
}


void Aircraft::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (IsDestroyed() && m_show_explosion)
	{
		target.draw(m_explosion, states);
	}
	else
	{
		/*sf::FloatRect bounds = GetBoundingRect();
		sf::RectangleShape rectangle(sf::Vector2f(bounds.width, bounds.height));
		rectangle.setFillColor(sf::Color::Red);
		rectangle.setPosition(bounds.left, bounds.top);
		target.draw(rectangle);*/
		target.draw(m_sprite, states);
	}
}

void Aircraft::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	UpdateTexts();
	UpdateRollAnimation();
	if (IsDestroyed())
	{
		m_explosion.Update(dt);
		// Play explosion sound only once
		if (!m_explosion_began)
		{
			SoundEffect soundEffect = (Utility::RandomInt(2) == 0) ? SoundEffect::kExplosion1 : SoundEffect::kExplosion2;
			PlayLocalSound(commands, soundEffect);
			//Emit network game action for enemy explodes
			if (!IsAllied())
			{
				sf::Vector2f position = GetWorldPosition();

				Command command;
				command.category = static_cast<int>(ReceiverCategories::kNetwork);
				command.action = DerivedAction<NetworkNode>([position](NetworkNode& node, sf::Time)
				{
					node.NotifyGameAction(GameActions::kEnemyExplode, position);
				});

				commands.Push(command);
			}

			m_explosion_began = true;
		}
		return;
	}
	//Check if bullets or missiles are fired
	CheckProjectileLaunch(dt, commands);
	// Update enemy movement pattern; apply velocity
	UpdateMovementPattern(dt);
	Entity::UpdateCurrent(dt, commands);
	
}

void Aircraft::CheckProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	//Enemies try to fire as often as they can
	if (!IsAllied())
	{
		Fire();
	}

	if (m_is_firing && m_fire_countdown <= sf::Time::Zero)
	{
		PlayLocalSound(commands, IsAllied() ? SoundEffect::kAlliedGunfire : SoundEffect::kEnemyGunfire);
		commands.Push(m_fire_command);
		m_fire_countdown += Table[static_cast<int>(m_type)].m_fire_interval / (m_fire_rate + 1.f);
		m_is_firing = false;
	}
	else if (m_fire_countdown > sf::Time::Zero)
	{
		//Wait, can't fire
		m_fire_countdown -= dt;
		m_is_firing = false;
	}

	//Missile launch
	if (m_is_launching_missile)
	{
		PlayLocalSound(commands, SoundEffect::kLaunchMissile);
		commands.Push(m_missile_command);
		m_is_launching_missile = false;
	}

}

bool Aircraft::IsAllied() const
{
	return m_type == AircraftType::kEagle;
}

void Aircraft::Remove()
{
	Entity::Remove();
	m_show_explosion = false;
}

void Aircraft::UpdateRollAnimation()
{
	//if (Table[static_cast<int>(m_type)].m_has_roll_animation)
	//{
	//	sf::IntRect textureRect = Table[static_cast<int>(m_type)].m_texture_rect;

	//	// Roll left: Texture rect offset once
	//	if (GetVelocity().x < 0.f)
	//		textureRect.left += textureRect.width;

	//	// Roll right: Texture rect offset twice
	//	else if (GetVelocity().x > 0.f)
	//		textureRect.left += 2 * textureRect.width;

	//	m_sprite.setTextureRect(textureRect);
	//}
}

void Aircraft::PlayLocalSound(CommandQueue& commands, SoundEffect effect)
{
	sf::Vector2f world_position = GetWorldPosition();

	Command command;
	command.category = static_cast<int>(ReceiverCategories::kSoundEffect);
	command.action = DerivedAction<SoundNode>(
		[effect, world_position](SoundNode& node, sf::Time)
	{
		node.PlaySound(effect, world_position);
	});

	commands.Push(command);
}

void Aircraft::RotateInMouseDirection(sf::Vector2i mousePos, sf::RenderWindow& window)
{

	//log to console the mouse pos
	//std::cout << "mouse pos: " << mousePos.x << "," << mousePos.y << std::endl;

	//get current position in the screen (between 1920x1080)
	//sf::Vector2i curPos = window.mapCoordsToPixel(getPosition());
	float width = window.getSize().x;
	float height = window.getSize().y;
	sf::Vector2i centerOfScreen = sf::Vector2i(width / 2, height / 2);
	//std::cout << "curPos pos: " << curPos.x << "," << curPos.y << std::endl;
	const float PI = 3.14159265;

	float dx = centerOfScreen.x - mousePos.x;
	float dy = centerOfScreen.y - mousePos.y;
	float rotation = atan2f(dx, dy) * 180 / PI;


	//log to console
	//std::cout << "Rotation: " << rotation << std::endl;
	setRotation(-rotation);
}
