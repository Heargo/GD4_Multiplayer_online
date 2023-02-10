//HUGO REY D00262075 : Added custom_info in the context to pass the information between states. It's used 
//to display the player who wins in the game over state

#include "State.hpp"
#include "StateStack.hpp"



State::Context::Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, Player& player, std::string& customInfo)
	: window(&window)
	, textures(&textures)
	, fonts(&fonts)
	, player(&player)
	, customInfo(&customInfo)
{
}

State::State(StateStack& stack, Context context)
	: m_stack(&stack)
	, m_context(context)
{
}

State::~State()
{
}

void State::RequestStackPush(StateID state_id)
{
	m_stack->PushState(state_id);
}

void State::RequestStackPop()
{
	m_stack->PopState();
}

void State::RequestStackClear()
{
	m_stack->ClearState();
}

void State::EditContextCustomInfo(std::string new_value)
{
	*m_context.customInfo = new_value;
}

State::Context State::GetContext() const
{
	return m_context;
}


