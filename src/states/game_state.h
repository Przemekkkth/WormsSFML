#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "state.h"
#include "../world.h"
#include "../world1.h"
#include "../world2.h"
#include "../player.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameState : public State
{
    public:
                            GameState(StateStack& stack, Context context);

        virtual void		draw();
        virtual bool		update(sf::Time dt);
        virtual bool		handleEvent(const sf::Event& event);

        static int choosenLevel;
        static sf::String score;

        enum class Mode { Sandbox1, Sandbox2, PvPC };
        static Mode sMode;
    private:
        World				mWorld;
        World1              mWorld1;
        World2              mWorld2;
        Player&				mPlayer;
        MusicPlayer&        mMusicPlayer;
        SoundPlayer&        mSoundPlayer;
};
#endif // GAME_STATE_H
