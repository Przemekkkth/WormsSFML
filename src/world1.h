#ifndef WORLD1_H
#define WORLD1_H


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

class World1 : private sf::NonCopyable
{
    public:
                                            World1(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds);
        void								update(sf::Time);
        void								draw();

        void processInput(const sf::Event& event);
        bool isGameOver() const;
        bool                                onUserCreate();
    private:
        void								loadTextures();

    private:
        void perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput);
        void createMap();
        void boom(float fWorldX, float fWorldY, float fRadius);
        void drawLandscape();
        void drawObjects();
        void drawStabilityIndicator();
        void controlSupervisor();
        void setCameraPos(sf::Time dt);
        void setCameraTargetObject(sf::Time dt);
        void updatePhysics(sf::Time dt);
        void clampMapBoundaries(sf::Time dt);
        void handlePlayerInput(sf::Time dt);
        void checkGameStability();

        bool zReleased = false, aHeld = false,
             sHeld = false, spacePressed = false, spaceHeld = false, spaceReleased = false;

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
        float fCameraPosXTarget = 0.0f;
        float fCameraPosYTarget = 0.0f;

        enum GAME_STATE
        {
            GS_RESET = 0,
            GS_GENERATE_TERRAIN = 1,
            GS_GENERATING_TERRAIN,
            GS_ALLOCATE_UNITS,
            GS_ALLOCATING_UNITS,
            GS_START_PLAY,
            GS_CAMERA_MODE
        } nGameState, nNextState;

        bool bGameIsStable = false;
        bool bPlayerHasControl = false;
        bool bPlayerActionComplete = false;
        // list of things that exist in game world
        std::list<std::unique_ptr<PhysicsObject>> listObjects;

        PhysicsObject* pObjectUnderControl = nullptr;
        PhysicsObject* pCameraTrackingObject = nullptr;

        bool bEnergising = false;
        float fEnergyLevel = 0.0f;
        bool bFireWeapon = false;

        //image for background
        sf::Texture mBgTex;
        sf::Sprite mBgSprite;
        void prepareBG();
};

#endif // WORLD1_H
