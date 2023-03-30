#include "game_state.h"
#include "../SFX/music_player.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

GameState::Mode GameState::sMode = GameState::Mode::Sandbox2;
int GameState::choosenLevel = 0;
sf::String GameState::score;

GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, mWorld(*context.window, *context.fonts, *context.sounds),
  mWorld1(*context.window, *context.fonts, *context.sounds),
  mWorld2(*context.window, *context.fonts, *context.sounds),
  mPlayer(*context.player),
  mMusicPlayer(*context.music),
  mSoundPlayer(*context.sounds)
{
    if(sMode == GameState::Mode::Sandbox1)
    {
        mWorld.onUserCreate();
    }
    else if(sMode == GameState::Mode::Sandbox2)
    {
        mWorld1.onUserCreate();
    }
    else
    {
        mWorld2.onUserCreate();
    }
}

void GameState::draw()
{
    if(sMode == GameState::Mode::Sandbox1)
    {
        mWorld.draw();
    }
    else if(sMode == GameState::Mode::Sandbox2)
    {
        mWorld1.draw();
    }
    else
    {
        mWorld2.draw();
    }
}

bool GameState::update(sf::Time dt)
{
    if(sMode == GameState::Mode::Sandbox1)
    {
        mWorld.update(dt);
    }
    else if(sMode == GameState::Mode::Sandbox2)
    {
        mWorld1.update(dt);
    }
    else
    {
        mWorld2.update(dt);
    }
    return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyReleased)
    {
        if(event.key.code == sf::Keyboard::BackSpace)
        {
            requestStackPop();
            requestStackPush(States::Menu);
        }
        else if(event.key.code == sf::Keyboard::M)
        {
            mMusicPlayer.setPaused(!mMusicPlayer.paused());
            //mSoundPlayer.setMuted(mMusicPlayer.paused());
        }
    }

    if(sMode == GameState::Mode::Sandbox1)
    {
        mWorld.processInput(event);
    }
    else if(sMode == GameState::Mode::Sandbox2)
    {
        mWorld1.processInput(event);
    }
    else
    {
        mWorld2.processInput(event);
    }
     return true;
}
