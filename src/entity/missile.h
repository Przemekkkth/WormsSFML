#ifndef MISSILE_H
#define MISSILE_H
#include "physics_object.h"

class Missile : public PhysicsObject
{
public:
    Missile(float x = 0.0f, float y = 0.0f, float _vx = 0.0f, float _vy = 0.0f);

    // Drawable interface
public:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    virtual int BounceDeathAction() override;
    virtual bool Damage(float d) override;
private:
    static std::vector<std::pair<float, float>> vecModel;
};

#endif // MISSILE_H
