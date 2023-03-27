#include "world.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
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

World::World(sf::RenderWindow& outputTarget, FontHolder& fonts, SoundPlayer& sounds)
    : mTarget(outputTarget)
    , mTextures()
    , mFonts(fonts)
    , mSounds(sounds)

{
    loadTextures();
    srand(time(0));
    onUserCreate();
}

void World::update(sf::Time dt)
{
    setCameraPos(dt);
    updatePhysics(dt);
}

void World::draw()
{
    render();
}

void World::processInput(const sf::Event &event)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(mTarget);
    if(event.type == sf::Event::MouseButtonReleased)
    {
        if(event.mouseButton.button == sf::Mouse::Left)
        {
            boom((mousePos.x/UNIT_SIZE) + fCameraPosX,
                 (mousePos.y/UNIT_SIZE) + fCameraPosY, 10.0f);
            prepareBG();
        }
    }

    // Press 'M' key to regenerate map
    if(event.type == sf::Event::KeyReleased)
    {
        if(event.key.code == sf::Keyboard::M)
        {
            createMap();
        }
        else if(event.key.code == sf::Keyboard::Num1)
        {
            listObjects.push_back(std::unique_ptr<Missile>(new Missile((mousePos.x/UNIT_SIZE) + fCameraPosX,
                                                                       (mousePos.y/UNIT_SIZE) + fCameraPosY)));
        }
        else if(event.key.code == sf::Keyboard::Num2)
        {
            listObjects.push_back(std::unique_ptr<Dummy>(new Dummy((mousePos.x/UNIT_SIZE) + fCameraPosX,
                                                                       (mousePos.y/UNIT_SIZE) + fCameraPosY)));
        }
        else if(event.key.code == sf::Keyboard::Num3)
        {
            listObjects.push_back(std::unique_ptr<Worm>(new Worm(mTextures, (mousePos.x/UNIT_SIZE) + fCameraPosX,
                                                                            (mousePos.y/UNIT_SIZE) + fCameraPosY)));
        }
    }
}

bool World::isGameOver() const
{
}

void World::loadTextures()
{
    mTextures.load(Textures::ALL, "res/all.png");
}

void World::prepareBG()
{
    sf::Image bgImage;
    bgImage.create(LOGIC_W, LOGIC_H);
    // prepare Landscape
    for (int x = 0; x < LOGIC_W; x++)
    {
        for (int y = 0; y < LOGIC_H; y++)
        {
            // Offset screen coordinates into world coordinates

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

bool World::onUserCreate()
{
    // Create Map
    map = new unsigned char[nMapWidth * nMapHeight];
    memset(map, 0, nMapWidth*nMapHeight * sizeof(unsigned char));
    createMap();

    return true;
}

void World::perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput)
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

void World::createMap()
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

    //Prepare bg image
    prepareBG();

}

void World::boom(float fWorldX, float fWorldY, float fRadius)
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

    int bx = (int)fWorldX;
    int by = (int)fWorldY;

    // Erase Terrain to form crater
    CircleBresenham(fWorldX, fWorldY, fRadius);

    // Shockwave other entities in range
    for (auto &p : listObjects)
    {
        float dx = p->px - fWorldX;
        float dy = p->py - fWorldY;
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
        listObjects.push_back(std::unique_ptr<Debris>(new Debris(fWorldX, fWorldY)));

}

void World::drawLandscape()
{
    mTarget.draw(mBgSprite);
}

void World::drawObjects()
{
    // Draw Objects
    for (auto &p : listObjects)
    {
        //p->Draw(this, fCameraPosX, fCameraPosY);
        p->fOffsetX = fCameraPosX;
        p->fOffsetY = fCameraPosY;
        p->bPixel   = false;
        p->draw(mTarget, sf::RenderStates());
    }

}

void World::setCameraPos(sf::Time elapsedTime)
{
    float fElapsedTime = 1.0f;//elapsedTime.asSeconds();
    sf::Vector2i oldCamPos = sf::Vector2i(fCameraPosX, fCameraPosY);
    sf::Vector2i mousePos = sf::Mouse::getPosition(mTarget);
    // Mouse Edge Map Scroll
    float fMapScrollSpeed = 4.0f;
    if (mousePos.x < 10)
    {
        fCameraPosX -= fMapScrollSpeed * fElapsedTime;
        //prepareBG();
    }
    if (mousePos.x > SCREEN_SIZE.x - 10)
    {
        fCameraPosX += fMapScrollSpeed * fElapsedTime;
        //prepareBG();
    }
    if (mousePos.y < 10)
    {
        fCameraPosY -= fMapScrollSpeed * fElapsedTime;
        //prepareBG();
    }
    if (mousePos.y > SCREEN_SIZE.y - 10)
    {
        fCameraPosY += fMapScrollSpeed * fElapsedTime;
        //prepareBG();
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

    //if new camera pos prepare bg
    sf::Vector2i newCamPos = sf::Vector2i(fCameraPosX, fCameraPosY);
    if(oldCamPos != newCamPos)
    {
        prepareBG();
    }
}

void World::updatePhysics(sf::Time dt)
{
    float fElapsedTime = 0.01f;
    // Do 10 physics iterations per frame - this allows smaller physics steps
    // giving rise to more accurate and controllable calculations
    for (int z = 0; z < 10; z++)
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
                if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] != 0)
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
                            boom(p->px, p->py, nResponse);
                    }

                    // Prepare BG
                    prepareBG();
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

void World::render()
{
    drawLandscape();
    drawObjects();
}
