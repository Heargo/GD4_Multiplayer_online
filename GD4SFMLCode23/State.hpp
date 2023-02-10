//HUGO REY D00262075 : Added custom_info in the context. See .cpp for more details

#pragma once
#include "StateID.hpp"
#include "ResourceIdentifiers.hpp"
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

namespace sf
{
	class RenderWindow;
}
class Player;
class StateStack;

#include <memory>

class State
{
public:
	typedef std::unique_ptr<State> Ptr;

	struct Context
	{
		Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, Player& player, std::string& customInfo);
		sf::RenderWindow* window;
		TextureHolder* textures;
		FontHolder* fonts;
		Player* player;
		std::string* customInfo;
	};

public:
	State(StateStack& stack, Context context);
	virtual ~State();
	virtual void Draw() = 0;
	virtual bool Update(sf::Time dt) = 0;
	virtual bool HandleEvent(const sf::Event& event) = 0;

protected:
	void RequestStackPush(StateID state_id);
	void RequestStackPop();
	void RequestStackClear();
	void EditContextCustomInfo(std::string new_value);
	Context GetContext() const;

private:
	StateStack* m_stack;
	Context m_context;


};

