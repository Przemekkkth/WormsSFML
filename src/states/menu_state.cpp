#include "menu_state.h"

#include "../GUI/button.h"
#include "../utils/resource_holder.h"
#include "../SFX/music_player.h"
#include "../application.h"
#include "game_state.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <iostream>

MenuState::MenuState(StateStack& stack, Context context)
    : State(stack, context)
    , mGUIContainer()
{
    sf::Texture& texture = context.textures->get(Textures::Title);
    mTitleStringSprite.setTexture(texture);
    mTitleStringSprite.setPosition(192, 30);

    auto sandbox1Button = std::make_shared<GUI::Button>(context);
    sandbox1Button->setPosition(412, 300);
    sandbox1Button->setText("Sandbox 1");
    sandbox1Button->setCallback([this] ()
    {
        GameState::sMode = GameState::Mode::Sandbox1;
        requestStackPop();
        requestStackPush(States::Game);
    });


    auto sandbox2Button = std::make_shared<GUI::Button>(context);
    sandbox2Button->setPosition(412, 375);
    sandbox2Button->setText("Sandbox 2");
    sandbox2Button->setCallback([this] ()
    {
        GameState::sMode = GameState::Mode::Sandbox2;
        requestStackPop();
        requestStackPush(States::Game);
    });

    auto pVsPCButton = std::make_shared<GUI::Button>(context);
    pVsPCButton->setPosition(412, 450);
    pVsPCButton->setText("Player VS PC");
    pVsPCButton->setCallback([this] ()
    {
        requestStackPop();
        requestStackPush(States::Game);
    });

    auto exitButton = std::make_shared<GUI::Button>(context);
    exitButton->setPosition(412, 525);
    exitButton->setText("Exit");
    exitButton->setCallback([this] ()
    {
        requestStackPop();
    });



    mGUIContainer.pack(sandbox1Button);
    mGUIContainer.pack(sandbox2Button);
    mGUIContainer.pack(pVsPCButton);
    mGUIContainer.pack(exitButton);

}

void MenuState::draw()
{
    sf::RenderWindow& window = *getContext().window;

    window.setView(window.getDefaultView());

    window.draw(mTitleStringSprite);
    window.draw(mGUIContainer);
}

bool MenuState::update(sf::Time)
{
    return true;
}

bool MenuState::handleEvent(const sf::Event& event)
{
    mGUIContainer.handleEvent(event);
    return false;
}
