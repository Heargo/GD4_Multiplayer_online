//HUGO REY D00262075 : Created this state is triggered when a player is dead.
//It's similar to the menu state, but with a custom message depending on the player who died.

#include "GameOverState.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "Button.hpp"
#include "TextNode.hpp"

GameOverState::GameOverState(StateStack& stack, Context context)
    :State(stack, context)
	, m_game_over_text()
{

	//load the background image
    sf::Texture& texture = context.textures->Get(Texture::kTitleScreen);
    m_background_sprite.setTexture(texture);
	
	sf::Vector2f viewSize = context.window->getView().getSize();

	//title 
	std::string txt = context.customInfo->c_str();
	m_game_over_text.setFont(context.fonts->Get(Font::kMain));
	m_game_over_text.setString(txt);
	m_game_over_text.setCharacterSize(70);
	Utility::CentreOrigin(m_game_over_text);
	m_game_over_text.setPosition(0.5f * viewSize.x, 0.4f * viewSize.y);


	
	auto play_button = std::make_shared<GUI::Button>(*context.fonts, *context.textures);
	play_button->setPosition(100, 250);
	play_button->SetText("Menu");
	play_button->SetCallback([this]()
		{
			RequestStackClear();
			RequestStackPush(StateID::kMenu);
		});

	auto exit_button = std::make_shared<GUI::Button>(*context.fonts, *context.textures);
	exit_button->setPosition(100, 300);
	exit_button->SetText("Exit");
	exit_button->SetCallback([this]()
		{
			RequestStackPop();
		});

	m_gui_container.Pack(play_button);
	m_gui_container.Pack(exit_button);
}

void GameOverState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.setView(window.getDefaultView());
	window.draw(m_background_sprite);
	window.draw(m_gui_container);
	window.draw(m_game_over_text);
}

bool GameOverState::Update(sf::Time dt)
{
    return false;
}

bool GameOverState::HandleEvent(const sf::Event& event)
{
	m_gui_container.HandleEvent(event);
	return false;
}
