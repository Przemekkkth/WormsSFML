#include "missile.h"
#include "../const/constants.h"
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <cmath>
#include <iostream>

Missile::Missile(float x, float y, float _vx, float _vy)
    : PhysicsObject(x, y)
{
    radius = 2.5f;
    fFriction = 0.5f;
    vx = _vx;
    vy = _vy;
    bDead = false;
    nBounceBeforeDeath = 1;
}

void Missile::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::ConvexShape cs;
    cs.setPointCount( vecModel.size() );
    int idx = 0;
    for(auto val : vecModel)
    {
        cs.setPoint(idx, sf::Vector2f(val.first*UNIT_SIZE, val.second*UNIT_SIZE));
        idx++;
    }

    sf::Vector2f p = sf::Vector2f(px-fOffsetX, py-fOffsetY);
    cs.setPosition(p.x*UNIT_SIZE, p.y*UNIT_SIZE);
    cs.setRotation(std::atan2(vy, vx)* (180.0f / 3.14159f));

    float factorScale = radius*UNIT_SIZE/2.0f;
    float pixelScale = 0.5f*UNIT_SIZE/2.0f;
    cs.setScale(bPixel ? pixelScale : factorScale, bPixel ? pixelScale : factorScale);
    cs.setFillColor(sf::Color::Yellow);
    cs.setOutlineColor(sf::Color::Yellow);
    target.draw(cs);
}

int Missile::BounceDeathAction()
{
    return 20; // Explode Big
}

bool Missile::Damage(float d)
{
    return true;
}

std::vector<std::pair<float, float>> DefineMissile()
{
    // Defines a rocket like shape
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ 1.0f, 1.0f });
    vecModel.push_back({ 2.0f, 1.0f });
    vecModel.push_back({ 2.5f, 0.0f });
    vecModel.push_back({ 2.0f, -1.0f });
    vecModel.push_back({ 1.0f, -1.0f });
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ -1.0f, -1.0f });
    vecModel.push_back({ -2.5f, -1.0f });
    vecModel.push_back({ -2.0f, 0.0f });
    vecModel.push_back({ -2.5f, 1.0f });
    vecModel.push_back({ -1.0f, 1.0f });

    // Scale points to make shape unit sized
    for (auto &v : vecModel)
    {
        v.first /= 2.5f; v.second /= 2.5f;
    }
    return vecModel;
}

std::vector<std::pair<float, float>> Missile::vecModel = DefineMissile();
