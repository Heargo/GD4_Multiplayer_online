#pragma once
#include "Projectile.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>
#include <cassert>

#include "TextureHolder.hpp"

class Projectile :public Enemy
{
public:
	enum Type
	{
		//Don't know if I should add -t
		AlliedBullet,
		EnemyBullet,
		Missile,
		TypeCount
	};

public:
	Projectile(Type type, const TextureHolder& texture);
	void guideTowards(sf::Vector2f position);
	bool isGuided() const;
	virtual unsigned int getCategory() const;
	virtual sf::FloatRect getBoundingRect const;
	float getMaxSpeed() const;
	int getDamage() const;

	
private:
	virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
	virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
private:
	Type mType;
	sf::Sprite mSprite;
	sf::Vector2f mTargetDirection;



	//namespace
	Projectile
	{
		const std::vector<ProjectileData> Table = initializeProjectileData();
	}

	Projectile::Projectile(Type type, const TextureHolder& textures)
		: Entity(1)
		, mType(type)
		, mSprite(textures.get(Table[type].texture))
		, mTargetDirection()
	{
		centerOrigin(mSprite);
	}

	void Projectile::guideTowards(sf::Vector2f position)
	{
		assert(isGuided());
		mTargetDirection = unitVector(position - getWorldPosition());
	}

	bool Projectile::isGuided() const
	{
		return mType == Missile;
	}

	void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
	{
		if (isGuided())
		{
			const float approachRate = 200.f;

			sf::Vector2f newVelocity = unitVector(approachRate * dt.asSeconds() * mTargetDirection + getVelocity());
			newVelocity *= getMaxSpeed();
			float angle = std::atan2(newVelocity.y, newVelocity.x);

			setRotation(toDegree(angle) + 90.f);
			setVelocity(newVelocity);
		}

		Entity::updateCurrent(dt, commands);
	}

	void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(mSprite, states);
	}

	unsigned int Projectile::getCategory() const
	{
		if (mType == EnemyBullet)
			return Category::EnemyProjectile;
		else
			return Category::AlliedProjectile;
	}

	sf::FloatRect Projectile::getBoundingRect() const
	{
		return getWorldTransform().transformRect(mSprite.getGlobalBounds());
	}

	float Projectile::getMaxSpeed() const
	{
		return Table[mType].speed;
	}

	int Projectile::getDamage() const
	{
		return Table[mType].damage;
	}


};
