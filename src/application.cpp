#include "application.h"

#include "const/state_identifiers.h"
#include "states/state.h"
#include "states/game_state.h"
#include "states/title_state.h"
#include "states/menu_state.h"
#include "const/constants.h"

const sf::Time Application::TimePerFrame = sf::seconds(1.f/60.f);


Application::Application()
: mWindow(sf::VideoMode(SCREEN_SIZE.x, SCREEN_SIZE.y), "Worms SFML", sf::Style::Close)
, mTextures()
, mFonts()
, mPlayer()
, mStateStack(State::Context(mWindow, mTextures, mFonts, mPlayer, mMusic, mSounds))
{
    mWindow.setKeyRepeatEnabled(false);

    mFonts.load(Fonts::Main, 	            "res/minecraft.ttf");

    mTextures.load(Textures::Buttons,		"res/buttons.png");
    mTextures.load(Textures::SFMLlogo,      "res/sfml-logo-small.png");
    mTextures.load(Textures::Title,         "res/title.png");

    registerStates();
    mStateStack.pushState(States::Title);
}

void Application::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow.isOpen())
    {
        sf::Time dt = clock.restart();
        timeSinceLastUpdate += dt;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;

            processInput();
            update(TimePerFrame);

            // Check inside this loop, because stack might be empty before update() call
            if (mStateStack.isEmpty())
                mWindow.close();
        }

        render();
    }
}

void Application::processInput()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        mStateStack.handleEvent(event);

        if (event.type == sf::Event::Closed)
            mWindow.close();
        if(event.type == sf::Event::KeyReleased)
        {
            //Uncomment if you want to make screenshots
//                if(event.key.code == sf::Keyboard::O)
//                {
//                    static int index = 0;
//                    sf::Texture texture;
//                    texture.create(mWindow.getSize().x, mWindow.getSize().y);
//                    texture.update(mWindow);
//                    std::string filename = "app" + std::to_string(index) + ".png";
//                    if (texture.copyToImage().saveToFile(filename))
//                    {
//                        index++;
//                    }
//                }
        }
    }
}

void Application::update(sf::Time dt)
{
    mStateStack.update(dt);
}

void Application::render()
{
    mWindow.clear();

    mStateStack.draw();

    mWindow.setView(mWindow.getDefaultView());

    mWindow.display();
}

void Application::registerStates()
{
    mStateStack.registerState<TitleState>(States::Title);
    mStateStack.registerState<MenuState>(States::Menu);
    mStateStack.registerState<GameState>(States::Game);
}
