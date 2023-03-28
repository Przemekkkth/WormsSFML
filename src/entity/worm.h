#ifndef WORM_H
#define WORM_H
#include "physics_object.h"
#include "../utils/resource_identifiers.h"
#include <SFML/Graphics/Texture.hpp>

class Worm : public PhysicsObject
{
public:
    Worm(const TextureHolder& textures, float x = 0.0f, float y = 0.0f);
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    virtual int BounceDeathAction() override;
    virtual bool Damage(float d) override;
    sf::Texture wormTex;
    float fShootAngle = 0.0f;
private:
    const int TEX_W = 62.0f;
    const int TEX_H = 65.0f;
};

#endif // WORM_H
