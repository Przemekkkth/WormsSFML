#include "sound_player.h"

#include <SFML/Audio/Listener.hpp>
#include "../application.h"
#include <cmath>


namespace
{
    // Sound coordinate system, point of view of a player in front of the screen:
    // X = left; Y = up; Z = back (out of the screen)
    const float ListenerZ = 300.f;
    const float Attenuation = 8.f;
    const float MinDistance2D = 200.f;
    const float MinDistance3D = std::sqrt(MinDistance2D*MinDistance2D + ListenerZ*ListenerZ);
}

SoundPlayer::SoundPlayer()
: mSoundBuffers()
, mSounds(),
  mMuted(false)
{
    mSoundBuffers.load(SoundEffect::Boom,     "res/growl.wav");
    mSoundBuffers.load(SoundEffect::Jump,     "res/jump.wav");
    mSoundBuffers.load(SoundEffect::Shoot,    "res/laser.wav");
    // Listener points towards the screen (default in SFML)
    sf::Listener::setDirection(0.f, 0.f, -1.f);
}

void SoundPlayer::play(SoundEffect::ID effect)
{
    if(mMuted)
    {
        return;
    }
    play(effect, getListenerPosition());
}

void SoundPlayer::play(SoundEffect::ID effect, sf::Vector2f position)
{
    mSounds.push_back(sf::Sound());
    sf::Sound& sound = mSounds.back();

    sound.setBuffer(mSoundBuffers.get(effect));
    sound.setPosition(position.x, -position.y, 0.f);
    sound.setAttenuation(Attenuation);
    sound.setMinDistance(MinDistance3D);

    sound.play();
}

void SoundPlayer::removeStoppedSounds()
{
    mSounds.remove_if([] (const sf::Sound& s)
    {
        return s.getStatus() == sf::Sound::Stopped;
    });
}

void SoundPlayer::setListenerPosition(sf::Vector2f position)
{
    sf::Listener::setPosition(position.x, -position.y, ListenerZ);
}

sf::Vector2f SoundPlayer::getListenerPosition() const
{
    sf::Vector3f position = sf::Listener::getPosition();
    return sf::Vector2f(position.x, -position.y);
}

bool SoundPlayer::isMuted() const
{
    return mMuted;
}

void SoundPlayer::setMuted(bool val)
{
    mMuted = val;
}
