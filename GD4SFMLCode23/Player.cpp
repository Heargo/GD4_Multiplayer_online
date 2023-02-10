// HUGO REY D00262075 : changes name of variable in InitializeActions to make it fit what's happening (player increase acceleration)
// add second player to the game (player 2) wich include new actions and new controls
// Fix errors in the shooting feature

#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Aircraft.hpp"
#include <map>
#include <string>
#include <algorithm>

struct AircraftMover
{
	AircraftMover(float speed, AircraftType type)
        : velocity(speed)
    {
		this->type = type;
    }
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        if (type == aircraft.GetType())
        {
            aircraft.AccelerateForward(velocity);
        };
    }
    
	
    float velocity;
	AircraftType type;
};

struct AircraftShooter
{
    AircraftShooter(AircraftType type)
    {
        this->type = type;
    }
    void operator()(Aircraft& aircraft, sf::Time) const
    {
        if (type == aircraft.GetType())
        {
            aircraft.Fire();
        };
    }
    AircraftType type;
};

Player::Player()
{
	//mouse mouvement
    m_mouse_binding[sf::Mouse::Right] = Action::kAccelerate1;
	m_mouse_binding[sf::Mouse::Left] = Action::kShoot1;
	

    //Set initial key bindings for player 1
    m_key_binding[sf::Keyboard::Left] = Action::kMoveLeft1;
    m_key_binding[sf::Keyboard::Right] = Action::kMoveRight1;
    m_key_binding[sf::Keyboard::Up] = Action::kMoveUp1;
    m_key_binding[sf::Keyboard::Down] = Action::kMoveDown1;
    m_key_binding[sf::Keyboard::Numpad0] = Action::kShoot1;

	//player 2
    m_key_binding[sf::Keyboard::A] = Action::kAccelerate2;
    m_key_binding[sf::Keyboard::D] = Action::kMoveRight2;
    m_key_binding[sf::Keyboard::W] = Action::kMoveUp2;
    m_key_binding[sf::Keyboard::S] = Action::kMoveDown2;
    m_key_binding[sf::Keyboard::Space] = Action::kShoot2;

    //Set initial action bindings
    InitializeActions();

    //Assign all categories to a player's aircraft
    for (auto& pair : m_action_binding)
    {
        pair.second.category = static_cast<unsigned int>(ReceiverCategories::kPlayerAircraft);
    }


}


void Player::HandleEvent(const sf::Event& event, CommandQueue& command)
{
    if (event.type == sf::Event::KeyPressed)
    {
        auto found = m_key_binding.find(event.key.code);
        if (found != m_key_binding.end() && !IsRealtimeAction(found->second))
        {
            command.Push(m_action_binding[found->second]);
        }
    }

	//mouse event
	if (event.type == sf::Event::MouseButtonPressed)
	{
		auto found = m_mouse_binding.find(event.mouseButton.button);
		if (found != m_mouse_binding.end() && !IsRealtimeAction(found->second))
		{
			command.Push(m_action_binding[found->second]);
		}
	}
}

void Player::HandleRealtimeInput(CommandQueue& command)
{
    //Check if any of the key binding keys are pressed
    for (auto pair : m_key_binding)
    {
        if (sf::Keyboard::isKeyPressed(pair.first) && IsRealtimeAction(pair.second))
        {
            command.Push(m_action_binding[pair.second]);
        }
    }

	//mouse event
	for (auto pair : m_mouse_binding)
	{
		if (sf::Mouse::isButtonPressed(pair.first) && IsRealtimeAction(pair.second))
		{
			command.Push(m_action_binding[pair.second]);
		}
	}
}

void Player::AssignKey(Action action, sf::Keyboard::Key key)
{
    //Remove all keys that are currently bound to the action
    for (auto itr = m_key_binding.begin(); itr != m_key_binding.end();)
    {
        if (itr->second == action)
        {
            m_key_binding.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
    m_key_binding[key] = action;
}

sf::Keyboard::Key Player::GetAssignedKey(Action action) const
{
    for (auto pair : m_key_binding)
    {
        if (pair.second == action)
        {
            return pair.first;
        }
    }
    return sf::Keyboard::Unknown;
}

void Player::InitializeActions()
{
    //TODO Normalize to avoid faster movement along diagonals
    const float kPlayerAcceleration = 5.f;

    m_action_binding[Action::kAccelerate1].action = DerivedAction<Aircraft>(AircraftMover(kPlayerAcceleration, AircraftType::kPlayer1));
    m_action_binding[Action::kShoot1].action = DerivedAction<Aircraft>(AircraftShooter(AircraftType::kPlayer1));

    m_action_binding[Action::kAccelerate2].action = DerivedAction<Aircraft>(AircraftMover(kPlayerAcceleration, AircraftType::kPlayer2)); 
    m_action_binding[Action::kShoot2].action = DerivedAction<Aircraft>(AircraftShooter(AircraftType::kPlayer2));
}

bool Player::IsRealtimeAction(Action action)
{
    switch (action)
    {
    case Action::kMoveDown1:
    case Action::kMoveUp1:
    case Action::kMoveLeft1:
    case Action::kMoveRight1:
    case Action::kMoveDown2:
    case Action::kMoveUp2:
    case Action::kMoveLeft2:
    case Action::kMoveRight2:
	case Action::kAccelerate1:
	case Action::kAccelerate2:
        return true;
    default:
        return false;
    }
}
