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
    int scalePixmap = bPixel ? 4 : 8;
    sf::Vector2f p = sf::Vector2f(px-fOffsetX-radius, py-fOffsetY-radius);
    sf::Sprite sprite;
    sprite.setTexture(wormTex);
    sprite.setTextureRect(sf::IntRect(0,0,62,65));
    sprite.setPosition(p.x*UNIT_SIZE, p.y*UNIT_SIZE);
    sprite.setScale(1.0f/(62.0f/32.0f), 1.0f/(65.0f/32.0f));
    float factorScale = radius*UNIT_SIZE/2.0f;
    float pixelScale = 0.5f*UNIT_SIZE/2.0f;
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
