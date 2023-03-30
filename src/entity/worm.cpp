#include "worm.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "../utils/resource_holder.h"
#include "../const/constants.h"
#include <cmath>
#include <iostream>

Worm::Worm(const TextureHolder& textures, float x, float y)
    : PhysicsObject(x, y)
{
    radius = 3.3f;
    fFriction = 0.2f;
    bDead = false;
    nBounceBeforeDeath = -1;
    wormTex = textures.get(Textures::ALL);
}

void Worm::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::Vector2f p = sf::Vector2f(px-fOffsetX-radius, py-fOffsetY-radius);
    sf::Sprite sprite;
    if(bIsPlayable)
    {
        sprite.setTexture(wormTex);
        sprite.setTextureRect(sf::IntRect(0,nTeam*TEX_H, TEX_W, TEX_H));
    }
    else
    {
        sprite.setTexture(wormTex);
        sprite.setTextureRect(sf::IntRect(TEX_W,nTeam*TEX_H, TEX_W, TEX_H));
    }
    sprite.setPosition(p.x*UNIT_SIZE, p.y*UNIT_SIZE);
    //Scale to 32x32pixels
    sprite.setScale(1.0f/(62.0f/32.0f), 1.0f/(65.0f/32.0f));
    if(bIsPlayable)
    {
        if(fShootAngle >= -PI_2 && fShootAngle <= PI_2)
        {
            sprite.setTextureRect(sf::IntRect(TEX_W, nTeam*TEX_H,-TEX_W,TEX_H));
        }
    }

    target.draw(sprite);
}

int Worm::BounceDeathAction()
{
    return 0;
}

bool Worm::Damage(float d)
{
    fHealth -= d;
    if (fHealth <= 0)
    { // Worm has died, no longer playable
        fHealth = 0.0f;
        bIsPlayable = false;
    }
    return fHealth > 0;
}
