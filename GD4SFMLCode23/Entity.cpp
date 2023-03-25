#include "Entity.hpp"
#include <cassert>
#include <cmath>

Entity::Entity(int hitpoints)
    : m_hitpoints(hitpoints)
{
}

void Entity::SetVelocity(sf::Vector2f velocity)
{
    m_velocity = velocity;
}

void Entity::SetVelocity(float vx, float vy)
{
    m_velocity.x = vx;
    m_velocity.y = vy;
}

sf::Vector2f Entity::GetVelocity() const
{
    return m_velocity;
}


void Entity::Accelerate(sf::Vector2f velocity)
{
    m_velocity += velocity;
}

void Entity::Accelerate(float vx, float vy)
{
    m_velocity.x += vx;
    m_velocity.y += vy;
}

void Entity::AccelerateForward(float speed)
{
    //accelerate the entity in the direction of its rotation
    float rotation = getRotation();
    //get direction vector
    sf::Vector2f direction = sf::Vector2f(std::sin(rotation * 3.14159265 / 180), -std::cos(rotation * 3.14159265 / 180));
    //multiply by speed
    direction *= speed;
    //add to velocity
    m_velocity += direction;

    RegulatePlayerSpeed();
}

/**
 * Apply a "friction" to the entity's velocity. This allow the entity to slow down over time if the player do not press any key.
 * This is to apply a "drag" effect in space to the entity and make the controls more challenging.
 * For now the frictionIntensity is set to 0.1f, which means that the entity will slow down by 10% of its current velocity every time this methods is called.
 * @param No parameters
 * @return No return value
 */
void Entity::ApplyFriction()
{
    float frictionIntensity = 0.01f;
    m_velocity.x *= (1.f - frictionIntensity);
    m_velocity.y *= (1.f - frictionIntensity);
}

void Entity::Rotate(sf::Vector2f velocity)
{
    float PI = 3.14159265f;
    float angle = std::atan2(velocity.x, -velocity.y);
    setRotation(angle * 180.f / PI);
}


int Entity::GetHitPoints() const
{
    return m_hitpoints;
}

void Entity::SetHitpoints(int points)
{
    //assert(points > 0);
    m_hitpoints = points;
}

void Entity::Repair(unsigned int points)
{
    assert(points > 0);
    //TODO Limit hitpoints
    m_hitpoints += points;
}

void Entity::Damage(unsigned int points)
{
    assert(points > 0);
    m_hitpoints -= points;
}

void Entity::Destroy()
{
    m_hitpoints = 0;
}

bool Entity::IsDestroyed() const
{
    return m_hitpoints <=0;
}

void Entity::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
    move(m_velocity * dt.asSeconds());
}

void Entity::Remove()
{
    Destroy();
}

void Entity::RegulatePlayerSpeed()
{
    //limite the speed of the player
    float maxSpeed = 350.f;
    if (abs(m_velocity.x) > maxSpeed)
    {
        m_velocity.x = maxSpeed * (m_velocity.x / abs(m_velocity.x)); //keep sign
    }
    if (abs(m_velocity.y) > maxSpeed)
    {
        m_velocity.y = maxSpeed * (m_velocity.y / abs(m_velocity.y)); //keep sign
    }


    //if close to 0, set to 0
    float difference = 4.f;
    if (abs(m_velocity.x) < difference)
    {
        m_velocity.x = 0;
    }
    if (abs(m_velocity.y) < difference)
    {
        m_velocity.y = 0;
    }

}