#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SFML/System/Vector2.hpp>

static const float UNIT_SIZE = 4.0f;
static const float LOGIC_W   = 256.0f;
static const float LOGIC_H   = 160.0f;
static const sf::Vector2 LOGIC_SIZE = sf::Vector2(256, 160);
static const sf::Vector2 SCREEN_SIZE(UNIT_SIZE*LOGIC_SIZE.x, UNIT_SIZE*LOGIC_SIZE.y);
static const float PI_2 = 3.141592f/2.0f;

#endif // CONSTANTS_H
