//HUGO REY D00262075 : Created this state. See .cpp for more details

#pragma once
#include "State.hpp"
#include "MenuOptions.hpp"
#include "Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class GameOverState : public State
{
public:
	GameOverState(StateStack& stack, Context context);
	virtual void Draw();
	virtual bool Update(sf::Time dt);
	virtual bool HandleEvent(const sf::Event& event);

private:
	sf::Sprite m_background_sprite;
	GUI::Container m_gui_container;
	sf::Text m_game_over_text;
};

