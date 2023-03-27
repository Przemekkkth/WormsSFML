#include "debris.h"
#include "../const/constants.h"
#include <cmath>
#include <list>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Debris::Debris(float x, float y)
    : PhysicsObject(x, y)
{
    // Set velocity to random direction and size for "boom" effect
    vx = 10.0f * cosf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
    vy = 10.0f * sinf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
    radius = 1.0f;
    fFriction = 0.4f;
    nBounceBeforeDeath = 5; // After 5 bounces, dispose
}

void Debris::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::RectangleShape rectangle(sf::Vector2f(UNIT_SIZE, UNIT_SIZE));
    sf::Vector2f p = sf::Vector2f(px-fOffsetX, py-fOffsetY);
    rectangle.setPosition(p.x*UNIT_SIZE, p.y*UNIT_SIZE);
    rectangle.setRotation(std::atan2(vy, vx)* (180.0f / 3.14159f));

    float factorScale = radius*UNIT_SIZE/2.0f;
    float pixelScale = 0.5f*UNIT_SIZE/2.0f;
    rectangle.setScale(bPixel ? pixelScale : factorScale, bPixel ? pixelScale : factorScale);
    rectangle.setFillColor(sf::Color(1, 50, 32));
    rectangle.setOutlineColor(sf::Color(1, 50, 32));

    target.draw(rectangle);
}

int Debris::BounceDeathAction()
{
    return 0;
}

bool Debris::Damage(float d)
{
    return true;
}

