#ifndef WORLD2_H
#define WORLD2_H

#include "entity/physics_object.h"
#include "entity/team.h"
#include "entity/worm.h"
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

class World2 : private sf::NonCopyable
{
    public:
                                            World2(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds);
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
        void drawTeamHealthBars();
        void drawCounter();
        void controlSupervisor();
        void controlAIStateMachine();
        void setCameraPos(sf::Time dt);
        void setCameraTargetObject(sf::Time dt);
        void updatePhysics(sf::Time dt);
        void clampMapBoundaries(sf::Time dt);
        void handlePlayerInput(sf::Time dt);
        void checkGameStability();

        bool zReleased = false,  spacePressed = false, spaceHeld = false, spaceReleased = false;

        sf::RenderWindow&					mTarget;
        TextureHolder						mTextures;
        FontHolder&							mFonts;
        SoundPlayer&						mSounds;

        void render();

        // Terrain size
        int nMapWidth = 1024;
        int nMapHeight = 512;
        char *map = nullptr;

        // Camera Coordinates
        float fCameraPosX = 0.0f;
        float fCameraPosY = 0.0f;
        float fCameraPosXTarget = 0.0f;
        float fCameraPosYTarget = 0.0f;

        // list of things that exist in game world
        std::list<std::unique_ptr<PhysicsObject>> listObjects;

        PhysicsObject* pObjectUnderControl = nullptr;		// Pointer to object currently under control
        PhysicsObject* pCameraTrackingObject = nullptr;	// Pointer to object that camera should track

        // Flags that govern/are set by game state machine
        bool bZoomOut = false;					// Render whole map
        bool bGameIsStable = false;				// All physics objects are stable
        bool bEnablePlayerControl = true;		// The player is in control, keyboard input enabled
        bool bEnableComputerControl = false;	// The AI is in control
        bool bEnergising = false;				// Weapon is charging
        bool bFireWeapon = false;				// Weapon should be discharged
        bool bShowCountDown = false;			// Display turn time counter on screen
        bool bPlayerHasFired = false;			// Weapon has been discharged

        float fEnergyLevel = 0.0f;				// Energy accumulated through charging (player only)
        float fTurnTime = 0.0f;					// Time left to take turn

        // Vector to store teams
        std::vector<Team> vecTeams;

        // Current team being controlled
        int nCurrentTeam = 0;

        // AI control flags
        bool bAI_Jump = false;				// AI has pressed "JUMP" key
        bool bAI_AimLeft = false;			// AI has pressed "AIM_LEFT" key
        bool bAI_AimRight = false;			// AI has pressed "AIM_RIGHT" key
        bool bAI_Energise = false;			// AI has pressed "FIRE" key


        float fAITargetAngle = 0.0f;		// Angle AI should aim for
        float fAITargetEnergy = 0.0f;		// Energy level AI should aim for
        float fAISafePosition = 0.0f;		// X-Coordinate considered safe for AI to move to
        Worm* pAITargetWorm = nullptr;		// Pointer to worm AI has selected as target
        float fAITargetX = 0.0f;			// Coordinates of target missile location
        float fAITargetY = 0.0f;

        enum GAME_STATE
        {
            GS_RESET = 0,
            GS_GENERATE_TERRAIN = 1,
            GS_GENERATING_TERRAIN,
            GS_ALLOCATE_UNITS,
            GS_ALLOCATING_UNITS,
            GS_START_PLAY,
            GS_CAMERA_MODE,
            GS_GAME_OVER1,
            GS_GAME_OVER2
        } nGameState, nNextState;


        enum AI_STATE
        {
            AI_ASSESS_ENVIRONMENT = 0,
            AI_MOVE,
            AI_CHOOSE_TARGET,
            AI_POSITION_FOR_TARGET,
            AI_AIM,
            AI_FIRE,
        } nAIState, nAINextState;

        //image for background
        sf::Texture mBgTex;
        sf::Sprite mBgSprite;
        void prepareBG();

};

#endif // WORLD2_H
