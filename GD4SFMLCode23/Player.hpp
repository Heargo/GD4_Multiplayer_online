#pragma once
#include "Command.hpp"
#include "Action.hpp"
#include <SFML/Window/Event.hpp>
#include <map>

class CommandQueue;

class Player
{
public:
	Player();
	void HandleEvent(const sf::Event& event, CommandQueue& command);
	void HandleRealtimeInput(CommandQueue& command);

	void AssignKey(Action action, sf::Keyboard::Key key);
	sf::Keyboard::Key GetAssignedKey(Action action) const;

private:
	void InitializeActions();
	static bool IsRealtimeAction(Action action);

private:
	std::map<sf::Keyboard::Key, Action> m_key_binding;
	//mouse bindings
	std::map<sf::Mouse::Button, Action> m_mouse_binding;
	std::map<Action, Command> m_action_binding;

};

