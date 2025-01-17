
#ifndef BRAKEDA3D_BODY_H
#define BRAKEDA3D_BODY_H

#include <btBulletDynamicsCommon.h>
#include "../Objects/Vertex3D.h"
#include "../../src/Collisions/Collisionable.h"

class Body: public Collisionable {

public:
    Body();

    float mass;

    btRigidBody *body;

    btRigidBody *getRigidBody() const;

    void setMass(float m);

    void removeCollisionObject() const;

    virtual ~Body();
};


#endif //BRAKEDA3D_BODY_H
