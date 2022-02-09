#ifndef BRAKEDA3D_PARTICLE_H
#define BRAKEDA3D_PARTICLE_H


#include "Object3D.h"
#include "../Misc/Counter.h"
#include "../Misc/Color.h"

class Particle: public Object3D {
public:
    Particle(Object3D *parent, float force, float ttl, Color c);

    const Vertex3D &getVelocity() const;

    void onUpdate() override;
private:
    Timer timer;
    Counter timeToLive;
    Vertex3D velocity;
    Vertex3D finalVelocity;
    float force;
    float t;
    Color color;
};


#endif //BRAKEDA3D_PARTICLE_H
