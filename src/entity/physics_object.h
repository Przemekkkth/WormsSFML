#ifndef PHYSICSOBJECT_H
#define PHYSICSOBJECT_H
#include <SFML/Graphics/Drawable.hpp>

class PhysicsObject : public sf::Drawable
{
public:
    PhysicsObject(float x = 0.0f, float y = 0.0f);
    virtual ~PhysicsObject(){}

    //position
    float px = 0.0f;
    float py = 0.0f;
    // velocity
    float vx = 0.0f;
    float vy = 0.0f;
    // acelaration
    float ax = 0.0f;
    float ay = 0.0f;

    // bounding circle for collision
    float radius = 4.0f;
    // has object stopped moving
    bool bStable = false;
    // acctualy a dampening factor is a more accurate name
    float fFriction = 0.8f;
    // how many time object can bounce before death
    // - 1 = infinite
    int nBounceBeforeDeath = -1;
    // Flag to indicate object should be removed
    bool bDead = false;

    float fOffsetX, fOffsetY;
    bool bPixel;
    //virtual void Draw(GameScene* scene, float fOffsetX, float fOffsetY, bool bPixel = false) = 0;
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override = 0;
    virtual int BounceDeathAction() = 0;
    virtual bool Damage(float d) = 0;
};

#endif // PHYSICSOBJECT_H
