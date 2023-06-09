#ifndef DUMMY_H
#define DUMMY_H

#include "physics_object.h"
#include <vector>

class Dummy : public PhysicsObject
{
public:
    Dummy(float x = 0.0f, float y = 0.0f);
    virtual ~Dummy(){}
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    virtual int BounceDeathAction() override;
    virtual bool Damage(float d) override;
private:
    static std::vector<std::pair<float, float>> vecModel;
};

#endif // DUMMY_H
