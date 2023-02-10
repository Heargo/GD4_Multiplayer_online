// HUGO REY D00262075 : changes to cap the velocity of the entity and add friction to the entity
// Add a rotate method to the entity that will be used by the player to rotate the entity in the direction of its velocity
#pragma once
#include "SceneNode.hpp"
#include "CommandQueue.hpp"

class Entity : public SceneNode
{
public:
	Entity(int hitpoints);
	void SetVelocity(sf::Vector2f velocity);
	void SetVelocity(float vx, float vy);
	sf::Vector2f GetVelocity() const;
	void Accelerate(sf::Vector2f velocity);
	void Accelerate(float vx, float vy);
	void AccelerateForward(float speed);
	
	void ApplyFriction();
	void Rotate(sf::Vector2f velocity);

	int GetHitPoints() const;
	void Repair(unsigned int points);
	void Damage(unsigned int points);
	void Destroy();
	virtual bool IsDestroyed() const;

protected:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);

private:
	sf::Vector2f m_velocity;
	void RegulatePlayerSpeed();
	int m_hitpoints;
};
