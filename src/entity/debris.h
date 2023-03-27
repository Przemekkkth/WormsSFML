#ifndef DEBRIS_H
#define DEBRIS_H
#include "physics_object.h"

class Debris : public PhysicsObject
{
public:
    Debris(float x = 0.0f, float y = 0.0f);
    virtual ~Debris(){}
    // Drawable interface
protected:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    virtual int BounceDeathAction() override;
    virtual bool Damage(float d) override;
};

#endif // DEBRIS_H
