#include "world2.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Sleep.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "states/game_state.h"
#include "entity/debris.h"
#include "entity/dummy.h"
#include "entity/missile.h"
#include "entity/worm.h"
#include "const/constants.h"

World2::World2(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
    : mTarget(outputTarget)
    , mTextures()
    , mFonts(fonts)
    , mSounds(sounds)

{

}

void World2::update(sf::Time dt)
{
    setCameraPos(dt);
    controlSupervisor();
    controlAIStateMachine();
    // Decrease
    fTurnTime -= dt.asSeconds();
    handlePlayerInput(dt);
    //setCameraTargetObject(dt);

    clampMapBoundaries(dt);
    updatePhysics(dt);
    checkGameStability();
    //
    for (auto &p : listObjects)
    {
        if(p->px < 0.f)
        {
            p->px = 0.0f;
        }
        else if(p->px > 1024*UNIT_SIZE - 60)
        {
            p->px = 1024*UNIT_SIZE - 60;
        }
    }
    // Update State Machine
    nGameState = nNextState;
    nAIState = nAINextState;
}

void World2::draw()
{
    render();
}

void World2::processInput(const sf::Event &event)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(mTarget);
    if(event.type == sf::Event::MouseButtonReleased)
    {
        if(event.mouseButton.button == sf::Mouse::Left)
        {
            boom((mousePos.x/UNIT_SIZE) + fCameraPosX,
                 (mousePos.y/UNIT_SIZE) + fCameraPosY, 10.0f);
        }
        else if(event.mouseButton.button == sf::Mouse::Right)
        {
            listObjects.push_back(std::unique_ptr<Missile>(new Missile((mousePos.x/UNIT_SIZE) + fCameraPosX,
                                                                       (mousePos.y/UNIT_SIZE) + fCameraPosY)));
        }
        else if(event.mouseButton.button == sf::Mouse::Middle)
        {
            listObjects.push_back(std::unique_ptr<Worm>(new Worm(mTextures, (mousePos.x/UNIT_SIZE) + fCameraPosX,
                                                                            (mousePos.y/UNIT_SIZE) + fCameraPosY)));
        }
    }

    if(event.type == sf::Event::KeyPressed)
    {
        if(event.key.code == sf::Keyboard::Space)
        {
            spacePressed = true;
        }
    }
    else if(event.type == sf::Event::KeyReleased)
    {
        if(event.key.code == sf::Keyboard::Z)
        {
            zReleased = true;
        }
        if(event.key.code == sf::Keyboard::Space)
        {
            spaceReleased = true;
        }
    }
}

bool World2::isGameOver() const
{
}

void World2::loadTextures()
{
    mTextures.load(Textures::ALL, "res/all.png");
}

void World2::prepareBG()
{
    sf::Image bgImage;
    bgImage.create(LOGIC_W, LOGIC_H);
    // prepare Landscape
    for (int x = 0; x < LOGIC_W; x++)
    {
        for (int y = 0; y < LOGIC_H; y++)
        {
            // Offset screen coordinates into World2 coordinates

            switch (map[(y + (int)fCameraPosY)*nMapWidth + (x + (int)fCameraPosX)])
            {
            case 0:
                bgImage.setPixel(x, y, sf::Color::Cyan);
                break;
            case 1:
                bgImage.setPixel(x, y, sf::Color::Green);
                break;
            default:
                std::cout << "Invalid " << std::endl;
            }
        }
    }
    mBgTex.loadFromImage(bgImage);
    mBgSprite.setTexture(mBgTex);
    mBgSprite.setScale(UNIT_SIZE, UNIT_SIZE);
}

bool World2::onUserCreate()
{
    loadTextures();
    // Create Map
    map = new  char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof( char));

    // Set initial states for state machines
    nGameState = GS_RESET;
    nNextState = GS_RESET;
    nAIState = AI_ASSESS_ENVIRONMENT;
    nAINextState = AI_ASSESS_ENVIRONMENT;

    bGameIsStable = false;


    return true;
}

void World2::perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput)
{
    // Used 1D Perlin Noise
    for (int x = 0; x < nCount; x++)
    {
        float fNoise = 0.0f;
        float fScaleAcc = 0.0f;
        float fScale = 1.0f;

        for (int o = 0; o < nOctaves; o++)
        {
            int nPitch = nCount >> o;
            int nSample1 = (x / nPitch) * nPitch;
            int nSample2 = (nSample1 + nPitch) % nCount;
            float fBlend = (float)(x - nSample1) / (float)nPitch;
            float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];
            fScaleAcc += fScale;
            fNoise += fSample * fScale;
            fScale = fScale / fBias;
        }

        // Scale to seed range
        fOutput[x] = fNoise / fScaleAcc;
    }
}

void World2::createMap()
{
    // Used 1D Perlin Noise
    float *fSurface = new float[nMapWidth];
    float *fNoiseSeed = new float[nMapWidth];

    // Populate with noise
    for (int i = 0; i < nMapWidth; i++)
        fNoiseSeed[i] = (float)rand() / (float)RAND_MAX;

    // Clamp noise to half way up screen
    fNoiseSeed[0] = 0.5f;

    // Generate 1D map
    perlinNoise1D(nMapWidth, fNoiseSeed, 8, 2.0f, fSurface);

    // Fill 2D map based on adjacent 1D map
    for (int x = 0; x < nMapWidth; x++)
        for (int y = 0; y < nMapHeight; y++)
        {
            if (y >= fSurface[x] * nMapHeight)
            {
                map[y * nMapWidth + x] = 1;
            }
            else
            {
//                // Shade the sky according to altitude - we only do top 1/3 of map
//                // as the Boom() function will just paint in 0 (cyan)
//                if ((float)y < (float)nMapHeight / 3.0f)
//                    map[y * nMapWidth + x] = (-8.0f * ((float)y / (nMapHeight / 3.0f))) -1.0f;
//                else
                    map[y * nMapWidth + x] = 0;
            }
        }

    // Clean up!
    delete[] fSurface;
    delete[] fNoiseSeed;
}

void World2::boom(float fWorld2X, float fWorld2Y, float fRadius)
{
    auto CircleBresenham = [&](int xc, int yc, int r)
    {
        int x = 0;
        int y = r;
        int p = 3 - 2 * r;
        if (!r) return;

        auto drawline = [&](int sx, int ex, int ny)
        {
            for (int i = sx; i < ex; i++)
                if(ny >=0 && ny < nMapHeight && i >=0 && i < nMapWidth)
                    map[ny*nMapWidth + i] = 0;
        };

        while (y >= x) // only formulate 1/8 of circle
        {
            //Filled Circle
            drawline(xc - x, xc + x, yc - y);
            drawline(xc - y, xc + y, yc - x);
            drawline(xc - x, xc + x, yc + y);
            drawline(xc - y, xc + y, yc + x);
            if (p < 0) p += 4 * x++ + 6;
            else p += 4 * (x++ - y--) + 10;
        }
    };

    int bx = (int)fWorld2X;
    int by = (int)fWorld2Y;

    // Erase Terrain to form crater
    CircleBresenham(fWorld2X, fWorld2Y, fRadius);

    // Shockwave other entities in range
    for (auto &p : listObjects)
    {
        float dx = p->px - fWorld2X;
        float dy = p->py - fWorld2Y;
        float fDist = sqrt(dx*dx + dy*dy);
        if (fDist < 0.0001f) fDist = 0.0001f;
        if (fDist < fRadius)
        {
            p->vx = (dx / fDist) * fRadius;
            p->vy = (dy / fDist) * fRadius;
            p->Damage(((fRadius - fDist) / fRadius) * 0.8f); // Corrected ;)
            p->bStable = false;
        }
    }
    // Launch debris proportional to blast size
    for (int i = 0; i < (int)fRadius; i++)
        listObjects.push_back(std::unique_ptr<Debris>(new Debris(fWorld2X, fWorld2Y)));

}

void World2::drawLandscape()
{
    mTarget.draw(mBgSprite);
}

void World2::drawObjects()
{
    for (auto &p : listObjects)
    {
        p->fOffsetX = fCameraPosX;
        p->fOffsetY = fCameraPosY;
        p->bPixel   = false;
        p->draw(mTarget, sf::RenderStates());

        Worm* worm = (Worm*)pObjectUnderControl;
        if(p.get() == worm)
        {
            float cx = worm->px + 8.0f * cosf(worm->fShootAngle) - fCameraPosX;
            float cy = worm->py + 8.0f * sinf(worm->fShootAngle) - fCameraPosY;

            sf::RectangleShape upR(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
            upR.setFillColor(sf::Color::Black);
            upR.setOutlineColor(sf::Color::Black);
            upR.setPosition(cx*UNIT_SIZE, (cy+1)*UNIT_SIZE);
            mTarget.draw(upR);

            sf::RectangleShape rR(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
            rR.setFillColor(sf::Color::Black);
            rR.setOutlineColor(sf::Color::Black);
            rR.setPosition((cx+1)*UNIT_SIZE, cy*UNIT_SIZE);
            mTarget.draw(rR);

            sf::RectangleShape lR(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
            lR.setFillColor(sf::Color::Black);
            lR.setOutlineColor(sf::Color::Black);
            lR.setPosition((cx-1)*UNIT_SIZE, cy*UNIT_SIZE);
            mTarget.draw(lR);

            sf::RectangleShape dR(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
            dR.setFillColor(sf::Color::Black);
            dR.setOutlineColor(sf::Color::Black);
            dR.setPosition(cx*UNIT_SIZE, (cy-1)*UNIT_SIZE);
            mTarget.draw(dR);

            sf::RectangleShape cR(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
            cR.setFillColor(sf::Color::Black);
            cR.setOutlineColor(sf::Color::Black);
            cR.setPosition(cx*UNIT_SIZE, cy*UNIT_SIZE);
            mTarget.draw(cR);
            for (int i = 0; i < 11 * fEnergyLevel; i++)
            {
                sf::RectangleShape green(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
                green.setFillColor(sf::Color::Green);
                green.setOutlineColor(sf::Color::Green);
                green.setPosition((worm->px - 5 + i - fCameraPosX)*UNIT_SIZE,
                                  (worm->py - 12 - fCameraPosY)*UNIT_SIZE);
                mTarget.draw(green);

                sf::RectangleShape red(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
                red.setFillColor(sf::Color::Red);
                red.setOutlineColor(sf::Color::Red);
                red.setPosition((worm->px - 5 + i - fCameraPosX)*UNIT_SIZE,
                                  (worm->py - 11 - fCameraPosY)*UNIT_SIZE);
                mTarget.draw(red);
            }
        }
    }
}

void World2::drawStabilityIndicator()
{
    if (bGameIsStable)
    {
        sf::RectangleShape rect(sf::Vector2f(6*UNIT_SIZE, 6*UNIT_SIZE));
        rect.setFillColor(sf::Color::Green);
        rect.setOutlineThickness(5);
        rect.setOutlineColor(sf::Color::Red);
        rect.setPosition(4*UNIT_SIZE, 20*UNIT_SIZE);
        mTarget.draw(rect);
    }
}

void World2::drawTeamHealthBars()
{
    sf::Color cols[] = { sf::Color::Green, sf::Color(102, 0, 51), sf::Color::Blue, sf::Color::Red };
    for (size_t t = 0; t < vecTeams.size(); t++)
     {
         float fTotalHealth = 0.0f;
         float fMaxHealth = (float)vecTeams[t].nTeamSize;
         for (auto w : vecTeams[t].vecMembers) // Accumulate team health
         {
             fTotalHealth += w->fHealth;
         }

         sf::RectangleShape rItem;
         rItem.setPosition(4*UNIT_SIZE, (4 + t * 4)* UNIT_SIZE);
         rItem.setSize(sf::Vector2f(((fTotalHealth / fMaxHealth) * (float)(LOGIC_SIZE.x - 8) + 1)*UNIT_SIZE,
                                              10));
        rItem.setFillColor(cols[t]);
        rItem.setOutlineColor(cols[t]);
        mTarget.draw(rItem);
    }
}

void World2::drawCounter()
{
    sf::Font font = mFonts.get(Fonts::Main);
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(25);
    text.setPosition(12*UNIT_SIZE, 20*UNIT_SIZE);
    text.setFillColor(sf::Color::Black);
    text.setStyle(sf::Text::Bold);


    float nearest = std::roundf(fTurnTime * 100) / 100;
    char res[20];
    std::sprintf(res, "%.2f", nearest);
    text.setString(res);
    mTarget.draw(text);
}

void World2::controlSupervisor()
{
    // Control Supervisor
    switch (nGameState)
    {
    case GS_RESET:
        {
            bEnablePlayerControl = false;
            bGameIsStable = false;
            bPlayerHasFired = false;
            bShowCountDown = false;
            nNextState = GS_GENERATE_TERRAIN;
        }
        break;

    case GS_GENERATE_TERRAIN:
        {
            bZoomOut = true;
            createMap();
            bGameIsStable = false;
            bShowCountDown = false;
            nNextState = GS_GENERATING_TERRAIN;
        }
        break;

    case GS_GENERATING_TERRAIN:
        {
            bShowCountDown = false;
            if (bGameIsStable)
                nNextState = GS_ALLOCATE_UNITS;
        }
        break;

    case GS_ALLOCATE_UNITS:
        {
            // Deploy teams
            int nTeams = 4;
            int nWormsPerTeam = 4;

            // Calculate spacing of worms and teams
            float fSpacePerTeam = (float)nMapWidth / (float)nTeams;
            float fSpacePerWorm = fSpacePerTeam / (nWormsPerTeam * 2.0f);

            // Create teams
            for (int t = 0; t < nTeams; t++)
            {
                vecTeams.emplace_back(Team());
                float fTeamMiddle = (fSpacePerTeam / 2.0f) + (t * fSpacePerTeam);
                for (int w = 0; w < nWormsPerTeam; w++)
                {
                    float fWormX = fTeamMiddle - ((fSpacePerWorm * (float)nWormsPerTeam) / 2.0f) + w * fSpacePerWorm;
                    float fWormY = 0.0f;

                    // Add worms to teams
                    Worm *worm = new Worm(mTextures, fWormX,fWormY);
                    worm->nTeam = t;
                    listObjects.push_back(std::unique_ptr<Worm>(worm));
                    vecTeams[t].vecMembers.push_back(worm);
                    vecTeams[t].nTeamSize = nWormsPerTeam;
                }

                vecTeams[t].nCurrentMember = 0;
            }

            // Select players first worm for control and camera tracking
            pObjectUnderControl = vecTeams[0].vecMembers[vecTeams[0].nCurrentMember];
            pCameraTrackingObject = pObjectUnderControl;
            bShowCountDown = false;
            nNextState = GS_ALLOCATING_UNITS;
        }
        break;

    case GS_ALLOCATING_UNITS: // Wait for units to "parachute" in
        {
            if (bGameIsStable)
            {
                bEnablePlayerControl = true;
                bEnableComputerControl = false;
                fTurnTime = 15.0f;
                bZoomOut = false;
                nNextState = GS_START_PLAY;
            }
        }
        break;

    case GS_START_PLAY:
        {
            bShowCountDown = true;

            // If player has discharged weapon, or turn time is up, move on to next state
            if (bPlayerHasFired || fTurnTime <= 0.0f)
                nNextState = GS_CAMERA_MODE;
        }
        break;

    case GS_CAMERA_MODE: // Camera follows object of interest until the physics engine has settled
        {
            bEnableComputerControl = false;
            bEnablePlayerControl = false;
            bPlayerHasFired = false;
            bShowCountDown = false;
            fEnergyLevel = 0.0f;

            if (bGameIsStable) // Once settled, choose next worm
            {
                // Get Next Team, if there is no next team, game is over
                int nOldTeam = nCurrentTeam;
                do {
                    nCurrentTeam++;
                    nCurrentTeam %= vecTeams.size();
                } while (!vecTeams[nCurrentTeam].IsTeamAlive());

                // Lock controls if AI team is currently playing
                if (nCurrentTeam == 0) // Player Team
                {
                    bEnablePlayerControl = true;	// Swap these around for complete AI battle
                    bEnableComputerControl = false;
                }
                else // AI Team
                {
                    bEnablePlayerControl = false;
                    bEnableComputerControl = true;
                }

                // Set control and camera
                pObjectUnderControl = vecTeams[nCurrentTeam].GetNextMember();
                pCameraTrackingObject = pObjectUnderControl;
                fTurnTime = 15.0f;
                bZoomOut = false;
                nNextState = GS_START_PLAY;

                // If no different team could be found...
                if (nCurrentTeam == nOldTeam)
                {
                    // ...Game is over, Current Team have won!
                    nNextState = GS_GAME_OVER1;
                }
            }
        }
        break;

        case GS_GAME_OVER1: // Zoom out and launch loads of missiles!
        {
            bEnableComputerControl = false;
            bEnablePlayerControl = false;
            bZoomOut = true;
            bShowCountDown = false;

            for (int i = 0; i < 100; i ++)
            {
                int nBombX = rand() % nMapWidth;
                int nBombY = rand() % (nMapHeight / 2);
                listObjects.push_back(std::unique_ptr<Missile>(new Missile(nBombX, nBombY, 0.0f, 0.5f)));
            }

            nNextState = GS_GAME_OVER2;
        }
        break;

        case GS_GAME_OVER2: // Stay here and wait for chaos to settle
        {
            bEnableComputerControl = false;
            bEnablePlayerControl = false;
            // No exit from this state!
        }
        break;

    }}

void World2::controlAIStateMachine()
{
    if (bEnableComputerControl)
    {
        zReleased = false;
        spacePressed = false;
        spaceHeld = false;
        spaceReleased = false;
        switch (nAIState)
        {
        case AI_ASSESS_ENVIRONMENT:
        {

            int nAction = rand() % 3;
            if (nAction == 0) // Play Defensive - move away from team
            {
                // Find nearest ally, walk away from them
                float fNearestAllyDistance = INFINITY; float fDirection = 0;
                Worm *origin = (Worm*)pObjectUnderControl;

                for (auto w : vecTeams[nCurrentTeam].vecMembers)
                {
                    if (w != pObjectUnderControl)
                    {
                        if (fabs(w->px - origin->px) < fNearestAllyDistance)
                        {
                            fNearestAllyDistance = fabs(w->px - origin->px);
                            fDirection = (w->px - origin->px) < 0.0f ? 1.0f : -1.0f;
                        }
                    }
                }

                if (fNearestAllyDistance < 50.0f)
                    fAISafePosition = origin->px + fDirection * 80.0f;
                else
                    fAISafePosition = origin->px;
            }

            if (nAction == 1) // Play Ballsy - move towards middle
            {
                Worm *origin = (Worm*)pObjectUnderControl;
                float fDirection = ((float)(nMapWidth / 2.0f) - origin->px) < 0.0f ? -1.0f : 1.0f;
                fAISafePosition = origin->px + fDirection * 200.0f;
            }

            if (nAction == 2) // Play Dumb - don't move
            {
                Worm *origin = (Worm*)pObjectUnderControl;
                fAISafePosition = origin->px;
            }

            // Clamp so dont walk off map
            if (fAISafePosition <= 20.0f) fAISafePosition = 20.0f;
            if (fAISafePosition >= nMapWidth - 20.0f) fAISafePosition = nMapWidth - 20.0f;
            nAINextState = AI_MOVE;
        }
        break;

        case AI_MOVE:
        {
            Worm *origin = (Worm*)pObjectUnderControl;
            if (fTurnTime >= 8.0f && origin->px != fAISafePosition)
            {
                // Walk towards target until it is in range
                if (fAISafePosition < origin->px && bGameIsStable)
                {
                    origin->fShootAngle = -3.14159f * 0.6f;
                    bAI_Jump = true;
                    nAINextState = AI_MOVE;
                }

                if (fAISafePosition > origin->px && bGameIsStable)
                {
                    origin->fShootAngle = -3.14159f * 0.4f;
                    bAI_Jump = true;
                    nAINextState = AI_MOVE;
                }
            }
            else
                nAINextState = AI_CHOOSE_TARGET;
        }
        break;

        case AI_CHOOSE_TARGET: // Worm finished moving, choose target
        {
            bAI_Jump = false;

            // Select Team that is not itself
            Worm *origin = (Worm*)pObjectUnderControl;
            int nCurrentTeam = origin->nTeam;
            int nTargetTeam = 0;
            do {
                nTargetTeam = rand() % vecTeams.size();
            } while (nTargetTeam == nCurrentTeam || !vecTeams[nTargetTeam].IsTeamAlive());

            // Aggressive strategy is to aim for opponent unit with most health
            Worm *mostHealthyWorm = vecTeams[nTargetTeam].vecMembers[0];
            for (auto w : vecTeams[nTargetTeam].vecMembers)
                if (w->fHealth > mostHealthyWorm->fHealth)
                    mostHealthyWorm = w;

            pAITargetWorm = mostHealthyWorm;
            fAITargetX = mostHealthyWorm->px;
            fAITargetY = mostHealthyWorm->py;
            nAINextState = AI_POSITION_FOR_TARGET;
        }
        break;

        case AI_POSITION_FOR_TARGET: // Calculate trajectory for target, if the worm needs to move, do so
        {
            Worm *origin = (Worm*)pObjectUnderControl;
            float dy = -(fAITargetY - origin->py);
            float dx = -(fAITargetX - origin->px);
            float fSpeed = 30.0f;
            float fGravity = 2.0f;

            bAI_Jump = false;

            float a = fSpeed * fSpeed*fSpeed*fSpeed - fGravity * (fGravity * dx * dx + 2.0f * dy * fSpeed * fSpeed);

            if (a < 0) // Target is out of range
            {
                if (fTurnTime >= 5.0f)
                {
                    // Walk towards target until it is in range
                    if (pAITargetWorm->px < origin->px && bGameIsStable)
                    {
                        origin->fShootAngle = -3.14159f * 0.6f;
                        bAI_Jump = true;
                        nAINextState = AI_POSITION_FOR_TARGET;
                    }

                    if (pAITargetWorm->px > origin->px && bGameIsStable)
                    {
                        origin->fShootAngle = -3.14159f * 0.4f;
                        bAI_Jump = true;
                        nAINextState = AI_POSITION_FOR_TARGET;
                    }
                }
                else
                {
                    // Worm is stuck, so just fire in direction of enemy!
                    // Its dangerous to self, but may clear a blockage
                    fAITargetAngle = origin->fShootAngle;
                    fAITargetEnergy = 0.75f;
                    nAINextState = AI_AIM;
                }
            }
            else
            {
                // Worm is close enough, calculate trajectory
                float b1 = fSpeed * fSpeed + sqrtf(a);
                float b2 = fSpeed * fSpeed - sqrtf(a);

                float fTheta1 = atanf(b1 / (fGravity * dx)); // Max Height
                float fTheta2 = atanf(b2 / (fGravity * dx)); // Min Height

                // We'll use max as its a greater chance of avoiding obstacles
                fAITargetAngle = fTheta1 - (dx > 0 ? 3.14159f : 0.0f);
                float fFireX = cosf(fAITargetAngle);
                float fFireY = sinf(fAITargetAngle);

                // AI is clamped to 3/4 power
                fAITargetEnergy = 0.75f;
                nAINextState = AI_AIM;
            }
        }
        break;

        case AI_AIM: // Line up aim cursor
        {
            Worm *worm = (Worm*)pObjectUnderControl;

            bAI_AimLeft = false;
            bAI_AimRight = false;
            bAI_Jump = false;

            if (worm->fShootAngle < fAITargetAngle)
                bAI_AimRight = true;
            else
                bAI_AimLeft = true;

            // Once cursors are aligned, fire - some noise could be
            // included here to give the AI a varying accuracy, and the
            // magnitude of the noise could be linked to game difficulty
            if (fabs(worm->fShootAngle - fAITargetAngle) <= 0.05f)
            {
                bAI_AimLeft = false;
                bAI_AimRight = false;
                fEnergyLevel = 0.0f;
                nAINextState = AI_FIRE;
            }
            else
                nAINextState = AI_AIM;
        }
        break;

        case AI_FIRE:
        {
            bAI_Energise = true;
            bFireWeapon = false;
            bEnergising = true;

            if (fEnergyLevel >= fAITargetEnergy)
            {
                bFireWeapon = true;
                bAI_Energise = false;
                bEnergising = false;
                bEnableComputerControl = false;
                nAINextState = AI_ASSESS_ENVIRONMENT;
            }
        }
        break;

        }
    }
}

void World2::setCameraPos(sf::Time elapsedTime)
{
    float fElapsedTime = 1.0f;//elapsedTime.asSeconds();
    sf::Vector2i oldCamPos = sf::Vector2i(fCameraPosX, fCameraPosY);
    sf::Vector2i mousePos = sf::Mouse::getPosition(mTarget);
    // Mouse Edge Map Scroll
    float fMapScrollSpeed = 4.0f;
    if (mousePos.x < 10)
    {
        fCameraPosX -= fMapScrollSpeed * fElapsedTime;
    }
    if (mousePos.x > SCREEN_SIZE.x - 10)
    {
        fCameraPosX += fMapScrollSpeed * fElapsedTime;
    }
    if (mousePos.y < 10)
    {
        fCameraPosY -= fMapScrollSpeed * fElapsedTime;
    }
    if (mousePos.y > SCREEN_SIZE.y - 10)
    {
        fCameraPosY += fMapScrollSpeed * fElapsedTime;
    }
    // Clamp map boundaries
    if (fCameraPosX < 0)
    {
        fCameraPosX = 0;
    }
    if (fCameraPosX >= nMapWidth - LOGIC_W)
    {
        fCameraPosX = nMapWidth - LOGIC_SIZE.x;
    }
    if (fCameraPosY < 0)
    {
        fCameraPosY = 0;
    }
    if (fCameraPosY >= nMapHeight - LOGIC_SIZE.y)
    {
        fCameraPosY = nMapHeight - LOGIC_SIZE.y;
    }
}

void World2::setCameraTargetObject(sf::Time )
{
    float fElapsedTime = 0.3f;
    if (pCameraTrackingObject != nullptr)
    {
        fCameraPosXTarget = pCameraTrackingObject->px - LOGIC_SIZE.x / 2;
        fCameraPosYTarget = pCameraTrackingObject->py - LOGIC_SIZE.y / 2;
        fCameraPosX += (fCameraPosXTarget - fCameraPosX) * 5.0f * fElapsedTime;
        fCameraPosY += (fCameraPosYTarget - fCameraPosY) * 5.0f * fElapsedTime;
        fCameraPosX = round(fCameraPosX * 100.0) / 100.0;
        fCameraPosY = round(fCameraPosY * 100.0) / 100.0;
    }
}

void World2::updatePhysics(sf::Time )
{
    float fElapsedTime = 0.01f;
    // Do 10 physics iterations per frame - this allows smaller physics steps
    // giving rise to more accurate and controllable calculations
    for (int z = 0; z < 15; z++)
    {
        // Update physics of all physical objects
        for (auto &p : listObjects)
        {
            // Apply Gravity
            p->ay += 2.0f;

            // Update Velocity
            p->vx += p->ax * fElapsedTime;
            p->vy += p->ay * fElapsedTime;

            // Update Position
            float fPotentialX = p->px + p->vx * fElapsedTime;
            float fPotentialY = p->py + p->vy * fElapsedTime;

            // Reset Acceleration
            p->ax = 0.0f;
            p->ay = 0.0f;
            p->bStable = false;

            // Collision Check With Map
            float fAngle = atan2f(p->vy, p->vx);
            float fResponseX = 0;
            float fResponseY = 0;
            bool bCollision = false;

            // Iterate through semicircle of objects radius rotated to direction of travel
            for (float r = fAngle - 3.14159f / 2.0f; r < fAngle + 3.14159f / 2.0f; r += 3.14159f / 8.0f)
            {
                // Calculate test point on circumference of circle
                float fTestPosX = (p->radius) * cosf(r) + fPotentialX;
                float fTestPosY = (p->radius) * sinf(r) + fPotentialY;

                // Constrain to test within map boundary
                if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
                if (fTestPosY >= nMapHeight) fTestPosY = nMapHeight - 1;
                if (fTestPosX < 0) fTestPosX = 0;
                if (fTestPosY < 0) fTestPosY = 0;

                // Test if any points on semicircle intersect with terrain
                if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] > 0)
                {
                    // Accumulate collision points to give an escape response vector
                    // Effectively, normal to the areas of contact
                    fResponseX += fPotentialX - fTestPosX;
                    fResponseY += fPotentialY - fTestPosY;
                    bCollision = true;
                }
            }

            // Calculate magnitudes of response and velocity vectors
            float fMagVelocity = sqrtf(p->vx*p->vx + p->vy*p->vy);
            float fMagResponse = sqrtf(fResponseX*fResponseX + fResponseY*fResponseY);

            // Collision occurred
            if (bCollision)
            {
                // Force object to be stable, this stops the object penetrating the terrain
                p->bStable = true;

                // Calculate reflection vector of objects velocity vector, using response vector as normal
                float dot = p->vx * (fResponseX / fMagResponse) + p->vy * (fResponseY / fMagResponse);

                // Use friction coefficient to dampen response (approximating energy loss)
                p->vx = p->fFriction * (-2.0f * dot * (fResponseX / fMagResponse) + p->vx);
                p->vy = p->fFriction * (-2.0f * dot * (fResponseY / fMagResponse) + p->vy);

                //Some objects will "die" after several bounces
                if (p->nBounceBeforeDeath > 0)
                {
                    p->nBounceBeforeDeath--;
                    p->bDead = p->nBounceBeforeDeath == 0;

                    // If object died, work out what to do next
                    if (p->bDead)
                    {
                        // Action upon object death
                        // = 0 Nothing
                        // > 0 Explosion
                        int nResponse = p->BounceDeathAction();
                        if (nResponse > 0)
                        {
                            boom(p->px, p->py, nResponse);
                            // Dead objects can no lobger be tracked by the camera
                            pCameraTrackingObject = nullptr;
                        }
                    }
                }

            }
            else
            {
                // No collision so update objects position
                p->px = fPotentialX;
                p->py = fPotentialY;
            }

            // Turn off movement when tiny
            if (fMagVelocity < 0.1f) p->bStable = true;
        }

        // Remove dead objects from the list, so they are not processed further. As the object
        // is a unique pointer, it will go out of scope too, deleting the object automatically. Nice :-)
        listObjects.remove_if([](std::unique_ptr<PhysicsObject> &o) {return o->bDead; });
    }
}

void World2::clampMapBoundaries(sf::Time )
{
//    if (fCameraPosX < 0) fCameraPosX = 0;
//    if (fCameraPosX >= nMapWidth - SCREEN_SIZE.x ) fCameraPosX = nMapWidth - SCREEN_SIZE.x;
//    if (fCameraPosY < 0) fCameraPosY = 0;
//    if (fCameraPosY >= nMapHeight - SCREEN_SIZE.y ) fCameraPosY = nMapHeight - SCREEN_SIZE.y;

    if (fCameraPosX < 0)
    {
        fCameraPosX = 0;
    }
    if (fCameraPosX >= nMapWidth - LOGIC_SIZE.x)
    {
        fCameraPosX = nMapWidth - LOGIC_SIZE.x;
    }
    if (fCameraPosY < 0)
    {
        fCameraPosY = 0;
    }
    if (fCameraPosY >= nMapHeight - LOGIC_SIZE.y)
    {
        fCameraPosY = nMapHeight - LOGIC_SIZE.y;
    }
}

void World2::handlePlayerInput(sf::Time )
{
    float fElapsedTime = 0.15f;
    if (pObjectUnderControl != nullptr)
    {
        pObjectUnderControl->ax = 0.0f;

        if (pObjectUnderControl->bStable)
        {
            if ((bEnablePlayerControl && zReleased) || (bEnableComputerControl && bAI_Jump))
            {
                float a = ((Worm*)pObjectUnderControl)->fShootAngle;

                pObjectUnderControl->vx = 6.0f * cosf(a);
                pObjectUnderControl->vy = 10.0f * sinf(a);
                pObjectUnderControl->bStable = false;

                bAI_Jump = false;
                zReleased = false;
            }

            if ((bEnablePlayerControl && sf::Keyboard::isKeyPressed(sf::Keyboard::S)) || (bEnableComputerControl && bAI_AimRight))
            {
                Worm* worm = (Worm*)pObjectUnderControl;
                fElapsedTime = 0.059f;
                worm->fShootAngle += 1.0f * fElapsedTime;
                if (worm->fShootAngle > 3.14159f) worm->fShootAngle -= 3.14159f * 2.0f;
            }

            if ((bEnablePlayerControl && sf::Keyboard::isKeyPressed(sf::Keyboard::A)) || (bEnableComputerControl && bAI_AimLeft))
            {
                Worm* worm = (Worm*)pObjectUnderControl;
                fElapsedTime = 0.059f;
                worm->fShootAngle -= 1.0f * fElapsedTime;
                if (worm->fShootAngle < -3.14159f) worm->fShootAngle += 3.14159f * 2.0f;
            }

            if ((bEnablePlayerControl &&  (spacePressed && !spaceHeld)))
            {
                bFireWeapon = false;
                bEnergising = true;
                fEnergyLevel = 0.0f;
            }

            if ((bEnablePlayerControl && sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ) || (bEnableComputerControl && bAI_Energise))
            {
                spaceHeld = true;
                if (bEnergising)
                {
                    fEnergyLevel += 0.75f * fElapsedTime/4.0f;
                    if (fEnergyLevel >= 1.0f)
                    {
                        fEnergyLevel = 1.0f;
                        bFireWeapon = true;
                    }
                }
            }

            if ((bEnablePlayerControl && spaceReleased))
            {
                if (bEnergising)
                {
                    bFireWeapon = true;
                }
                bEnergising = false;
                spaceReleased = false;
                spaceHeld = false;
            }
        }
        setCameraTargetObject(sf::Time());
        if (bFireWeapon)
        {
            Worm* worm = (Worm*)pObjectUnderControl;

            // Get Weapon Origin
            float ox = worm->px;
            float oy = worm->py;

            // Get Weapon Direction
            float dx = cosf(worm->fShootAngle);
            float dy = sinf(worm->fShootAngle);

            // Create Weapon Object
            Missile *m = new Missile(ox, oy, dx * 40.0f * fEnergyLevel, dy * 40.0f * fEnergyLevel);
            pCameraTrackingObject = m;
            listObjects.push_back(std::unique_ptr<Missile>(m));

            // Reset flags involved with firing weapon
            bFireWeapon = false;
            fEnergyLevel = 0.0f;
            bEnergising = false;
            bPlayerHasFired = true;

            if (rand() % 100 >= 50)
                bZoomOut = true;
        }
    }
    else
    {
        zReleased = false;
        spacePressed = false;
        spaceHeld = false;
        spaceReleased = false;
    }
}

void World2::checkGameStability()
{
    bGameIsStable = true;
    for (auto &p : listObjects)
    {
        if (!p->bStable)
        {
            bGameIsStable = false;
            break;
        }
    }
}

void World2::render()
{
    prepareBG();
    drawLandscape();
    drawObjects();
    drawStabilityIndicator();
    drawTeamHealthBars();
    drawCounter();
}
