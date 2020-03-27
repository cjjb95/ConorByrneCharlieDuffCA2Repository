/*
Charlie Duff
D00183790
*/
#include "MenuState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"
#include "OptionID.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

MenuState::MenuState(StateStack& stack, Context context)
	: State(stack, context)
	, mGUIContainer()
{
	sf::Texture& texture = context.textures->get(TextureID::TitleScreen);
	mBackgroundSprite.setTexture(texture);

	auto playButton = std::make_shared<GUI::Button>(context);
	playButton->setPosition(450, 300);
	playButton->setText("Play");
	playButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(StateID::Game);
	});

	auto hostPlayButton = std::make_shared<GUI::Button>(context);
	hostPlayButton->setPosition(450, 350);
	hostPlayButton->setText("Host");
	hostPlayButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(StateID::HostGame);
	});

	auto joinPlayButton = std::make_shared<GUI::Button>(context);
	joinPlayButton->setPosition(450, 400);
	joinPlayButton->setText("Join");
	joinPlayButton->setCallback([this]()
	{
		requestStackPop();
		requestStackPush(StateID::JoinGame);
	});

	auto settingsButton = std::make_shared<GUI::Button>(context);
	settingsButton->setPosition(450, 450);
	settingsButton->setText("Settings");
	settingsButton->setCallback([this]()
	{
		requestStackPush(StateID::Settings);
	});

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(450, 500);
	exitButton->setText("Exit");
	exitButton->setCallback([this]()
	{
		requestStackClear();
	});

	mGUIContainer.pack(playButton);
	mGUIContainer.pack(hostPlayButton);
	mGUIContainer.pack(joinPlayButton);
	mGUIContainer.pack(settingsButton);
	mGUIContainer.pack(exitButton);

	context.music->play(MusicID::MenuTheme);
}

void MenuState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.setView(window.getDefaultView());
	window.draw(mBackgroundSprite);
	window.draw(mGUIContainer);
}

bool MenuState::update(sf::Time dt)
{
	return true;
}

bool MenuState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

