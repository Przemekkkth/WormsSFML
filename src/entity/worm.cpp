#include "worm.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include "../utils/resource_holder.h"
#include "../const/constants.h"
#include <cmath>

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
    sprite.setTexture(wormTex);
    sprite.setTextureRect(sf::IntRect(0,0, TEX_W, TEX_H));
    sprite.setPosition(p.x*UNIT_SIZE, p.y*UNIT_SIZE);
    //Scale to 32x32pixels
    sprite.setScale(1.0f/(62.0f/32.0f), 1.0f/(65.0f/32.0f));
    if(fShootAngle >= -PI_2 && fShootAngle <= PI_2)
    {
        sprite.setTextureRect(sf::IntRect(TEX_W,0,-TEX_W,TEX_H));
    }
    target.draw(sprite);
}

int Worm::BounceDeathAction()
{
    return 0;
}

bool Worm::Damage(float d)
{
    return true;
}
