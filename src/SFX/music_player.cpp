#include "music_player.h"


MusicPlayer::MusicPlayer()
: mMusic()
, mFilenames()
, mVolume(20.f)
{
    mFilenames[Music::MenuTheme]    = "res/menu_theme.wav";
    mPause = false;
}

void MusicPlayer::play(Music::ID theme)
{
    std::string filename = mFilenames[theme];

    if (!mMusic.openFromFile(filename))
        throw std::runtime_error("Music " + filename + " could not be loaded.");

    mMusic.setVolume(mVolume);
    mMusic.setLoop(true);
    mMusic.play();
}

void MusicPlayer::stop()
{
    mMusic.stop();
}

void MusicPlayer::setVolume(float volume)
{
    mVolume = volume;
}

float MusicPlayer::volume() const
{
    return mVolume;
}

bool MusicPlayer::paused() const
{
    return mPause;
}

sf::SoundSource::Status MusicPlayer::status()
{
    return mMusic.getStatus();
}

void MusicPlayer::setPaused(bool paused)
{
    mPause = paused;
    if (paused)
        mMusic.pause();
    else
        mMusic.play();
}
