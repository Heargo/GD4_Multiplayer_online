//HUGO REY D00262075 : Add the game over handling here since to change state I need to access the stack

#include "GameState.hpp"
#include "Player.hpp"

GameState::GameState(StateStack& stack, Context context)
    : State(stack, context)
    , m_world(*context.window, *context.fonts)
    , m_player(*context.player)
{
}

void GameState::Draw()
{
    m_world.Draw();
}

bool GameState::Update(sf::Time dt)
{
    m_world.Update(dt);
	AircraftType game_over = m_world.IsGameOver();
	if (game_over != AircraftType::kNone)
    {
        RequestStackPop();
		if (game_over == AircraftType::kPlayer1)
			EditContextCustomInfo("Player 2 wins!");
        else if(game_over == AircraftType::kPlayer2)
            EditContextCustomInfo("Player 1 wins!");
		
		RequestStackPush(StateID::kGameOver);
    }
    CommandQueue& commands = m_world.GetCommandQueue();
    m_player.HandleRealtimeInput(commands);
    return true;
}

bool GameState::HandleEvent(const sf::Event& event)
{
    CommandQueue& commands = m_world.GetCommandQueue();
    m_player.HandleEvent(event, commands);

    //Escape should bring up the pause menu
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
    {
        RequestStackPush(StateID::kPause);
    }
    return true;
}
