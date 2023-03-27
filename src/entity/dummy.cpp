#include "dummy.h"
#include "../const/constants.h"
#include <cmath>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

std::vector<std::pair<float, float>> DefineDummy();

Dummy::Dummy(float x, float y)
    : PhysicsObject(x, y)
{

}

void Dummy::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::ConvexShape cs(vecModel.size());
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

    cs.setOutlineColor(sf::Color::White);
    target.draw(cs);
}

int Dummy::BounceDeathAction()
{
    return 0;
}

bool Dummy::Damage(float d)
{
    return true;
}

std::vector<std::pair<float, float>> DefineDummy()
{
    // Defines a circle with a line fom center to edge
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    for (int i = 0; i < 10; i++)
        vecModel.push_back({ cosf(i / 9.0f * 2.0f * 3.14159f) , sinf(i / 9.0f * 2.0f * 3.14159f) });
    return vecModel;
}

std::vector<std::pair<float, float>> Dummy::vecModel = DefineDummy();
