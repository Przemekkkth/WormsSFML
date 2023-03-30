#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include "../utils/resource_holder.h"
#include "../utils/resource_identifiers.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>

#include <list>


class SoundPlayer : private sf::NonCopyable
{
    public:
                                    SoundPlayer();

        void						play(SoundEffect::ID effect);
        void						play(SoundEffect::ID effect, sf::Vector2f position);

        void						removeStoppedSounds();
        void						setListenerPosition(sf::Vector2f position);
        sf::Vector2f				getListenerPosition() const;

        bool                        isMuted() const;
        void                        setMuted(bool val);
    private:
        SoundBufferHolder			mSoundBuffers;
        std::list<sf::Sound>		mSounds;
        bool                        mMuted;
};
#endif // SOUNDPLAYER_H
