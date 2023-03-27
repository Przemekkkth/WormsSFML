#ifndef WORLD_H
#define WORLD_H
#include "entity/physics_object.h"
#include "SFX/sound_player.h"
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <array>
#include <queue>
#include <list>
#include <memory>

// Forward declaration
namespace sf
{
    class RenderTarget;
    class Event;
}

class World : private sf::NonCopyable
{
    public:
                                            World(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds);
        void								update(sf::Time);
        void								draw();

        void processInput(const sf::Event& event);
        bool isGameOver() const;

    private:
        void								loadTextures();
        bool                                onUserCreate();
    private:
        void perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput);
        void createMap();
        void boom(float fWorldX, float fWorldY, float fRadius);
        void drawLandscape();
        void drawObjects();
        void setCameraPos(sf::Time dt);
        void updatePhysics(sf::Time dt);

        sf::RenderWindow&					mTarget;
        TextureHolder						mTextures;
        FontHolder&							mFonts;
        SoundPlayer&						mSounds;

        void render();
        // Terrain size
        int nMapWidth = 1024;
        int nMapHeight = 512;
        unsigned char *map = nullptr;

        // Camera coordinates
        float fCameraPosX = 0.0f;
        float fCameraPosY = 0.0f;

        // list of things that exist in game world
        std::list<std::unique_ptr<PhysicsObject>> listObjects;

        //image for background
        sf::Texture mBgTex;
        sf::Sprite mBgSprite;
        void prepareBG();
};

#endif // WORLD_H
