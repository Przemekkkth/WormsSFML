#include "world1.h"
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

World1::World1(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
    : mTarget(outputTarget)
    , mTextures()
    , mFonts(fonts)
    , mSounds(sounds)

{

}

World1::~World1()
{
    delete [] map;
}


void World1::update(sf::Time dt)
{
    setCameraPos(dt);
    controlSupervisor();
    handlePlayerInput(dt);
    setCameraTargetObject(dt);
    clampMapBoundaries(dt);
    updatePhysics(dt);
    checkGameStability();
    // Update State Machine
    nGameState = nNextState;
}

void World1::draw()
{
    render();
}

void World1::processInput(const sf::Event &event)
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

bool World1::isGameOver() const
{
}

void World1::loadTextures()
{
    mTextures.load(Textures::ALL, "res/all.png");
}

void World1::prepareBG()
{
    sf::Image bgImage;
    bgImage.create(LOGIC_W, LOGIC_H);
    // prepare Landscape
    for (int x = 0; x < LOGIC_W; x++)
    {
        for (int y = 0; y < LOGIC_H; y++)
        {
            // Offset screen coordinates into World1 coordinates

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

bool World1::onUserCreate()
{
    loadTextures();
    srand(time(0));
    // Create Map
    map = new unsigned char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof(unsigned char));

    nGameState = GS_RESET;
    nNextState = GS_RESET;


    return true;
}

void World1::perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput)
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

void World1::createMap()
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

void World1::boom(float fWorld1X, float fWorld1Y, float fRadius)
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

    int bx = (int)fWorld1X;
    int by = (int)fWorld1Y;

    // Erase Terrain to form crater
    CircleBresenham(fWorld1X, fWorld1Y, fRadius);

    // Shockwave other entities in range
    for (auto &p : listObjects)
    {
        float dx = p->px - fWorld1X;
        float dy = p->py - fWorld1Y;
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
        listObjects.push_back(std::unique_ptr<Debris>(new Debris(fWorld1X, fWorld1Y)));

}

void World1::drawLandscape()
{
    mTarget.draw(mBgSprite);
}

void World1::drawObjects()
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

void World1::drawStabilityIndicator()
{
    if (bGameIsStable)
    {
        sf::RectangleShape rect(sf::Vector2f(6*UNIT_SIZE, 6*UNIT_SIZE));
        rect.setFillColor(sf::Color::Red);
        rect.setOutlineColor(sf::Color::Red);
        rect.setPosition(2*UNIT_SIZE, 2*UNIT_SIZE);
        mTarget.draw(rect);
    }
}

void World1::controlSupervisor()
{
    switch (nGameState)
    {
    case GS_RESET: // Set game variables to know state
    {
        bGameIsStable = false;
        bPlayerActionComplete = false;
        bPlayerHasControl = false;
        nNextState = GS_GENERATE_TERRAIN;
    }
    break;

    case GS_GENERATE_TERRAIN: // Create a new terrain
    {
        bPlayerHasControl = false;
        createMap();
        nNextState = GS_GENERATING_TERRAIN;
    }
    break;

    case GS_GENERATING_TERRAIN: // Does nothing, for now ;)
    {
        bPlayerHasControl = false;
        nNextState = GS_ALLOCATE_UNITS;
    }
    break;

    case GS_ALLOCATE_UNITS: // Add a unit to the top of the screen
    {
        bPlayerHasControl = false;
        Worm *worm = new Worm(mTextures, 32.0f, 1.0f);
        listObjects.push_back(std::unique_ptr<Worm>(worm));
        pObjectUnderControl = worm;
        pCameraTrackingObject = pObjectUnderControl;
        nNextState = GS_ALLOCATING_UNITS;
    }
    break;

    case GS_ALLOCATING_UNITS: // Stay in this state whilst units are deploying
    {
        bPlayerHasControl = false;

        if (bGameIsStable) // Can only leave state once game is stable
        {
            bPlayerActionComplete = false;
            nNextState = GS_START_PLAY;
        }
    }
    break;

    case GS_START_PLAY: // Player is in control of unit
    {
        bPlayerHasControl = true;
        if (bPlayerActionComplete) // Can only leave state when the player action has completed
            nNextState = GS_CAMERA_MODE;
    }
    break;

    case GS_CAMERA_MODE: // Camera is tracking on-screen action
    {
        bPlayerHasControl = false;
        bPlayerActionComplete = false;

        if (bGameIsStable) // Can only leave state when action has finished, and engine is stable
        {
            pCameraTrackingObject = pObjectUnderControl;
            nNextState = GS_START_PLAY;
        }

    }
    break;
    }
}

void World1::setCameraPos(sf::Time elapsedTime)
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

void World1::setCameraTargetObject(sf::Time )
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

void World1::updatePhysics(sf::Time )
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

void World1::clampMapBoundaries(sf::Time )
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

void World1::handlePlayerInput(sf::Time )
{
    float fElapsedTime = 0.15f;
    if (bPlayerHasControl)
    {
        if (pObjectUnderControl != nullptr)
        {
            if (pObjectUnderControl->bStable)
            {
                if (zReleased) // Jump in direction of cursor
                {
                    float a = ((Worm*)pObjectUnderControl)->fShootAngle;
                    pObjectUnderControl->vx = 6.0f * cosf(a);
                    pObjectUnderControl->vy = 10.0f * sinf(a);
                    pObjectUnderControl->bStable = false;
                    zReleased = false;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) // Rotate cursor CCW
                {
                    Worm* worm = (Worm*)pObjectUnderControl;
                    worm->fShootAngle -= 1.0f * fElapsedTime;
                    if (worm->fShootAngle < -3.14159f) worm->fShootAngle += 3.14159f * 2.0f;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S  )) // Rotate cursor CW
                {
                    Worm* worm = (Worm*)pObjectUnderControl;
                    worm->fShootAngle += 1.0f * fElapsedTime;
                    if (worm->fShootAngle > 3.14159f) worm->fShootAngle -= 3.14159f * 2.0f;
                }

                if (spacePressed && !spaceHeld) // Start to charge weapon
                {
                    bEnergising = true;
                    bFireWeapon = false;
                    fEnergyLevel = 0.0f;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space  )) // Weapon is charging
                {
                    spaceHeld = true;
                    if (bEnergising)
                    {
                        fEnergyLevel += 0.75f * fElapsedTime/4.0f;
                        if (fEnergyLevel >= 1.0f) // If it maxes out, Fire!
                        {
                            fEnergyLevel = 1.0f;
                            bFireWeapon = true;
                        }
                    }
                }

                if (spaceReleased) // If it is released before maxing out, Fire!
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
                listObjects.push_back(std::unique_ptr<Missile>(m));
                pCameraTrackingObject = m;

                // Reset flags involved with firing weapon
                bFireWeapon = false;
                fEnergyLevel = 0.0f;
                bEnergising = false;

                // Indicate the player has completed their action for this unit
                bPlayerActionComplete = true;
            }
        }
    }
}

void World1::checkGameStability()
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

void World1::render()
{
    prepareBG();
    drawLandscape();
    drawObjects();
    drawStabilityIndicator();
}
