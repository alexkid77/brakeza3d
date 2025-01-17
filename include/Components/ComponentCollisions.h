
#ifndef BRAKEDA3D_COMPONENTCOLLISIONS_H
#define BRAKEDA3D_COMPONENTCOLLISIONS_H

#include "../Render/BSPMap.h"
#include "../Render/PhysicsDebugDraw.h"
#include "../Physics/Mesh3DBody.h"
#include "../Physics/Mesh3DGhost.h"
#include "../Physics/SpriteDirectional3DBody.h"
#include "Component.h"

class ComponentCollisions : public Component {
public:
    ComponentCollisions();

    void onStart();

    void preUpdate();

    void onUpdate();

    void postUpdate();

    void onEnd();

    void onSDLPollEvent(SDL_Event *event, bool &finish);

    BSPMap *bspMap;
    std::vector<Triangle *> *visibleTriangles;

    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    btDefaultCollisionConfiguration *collisionConfiguration;
    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    btCollisionDispatcher *dispatcher;
    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    btBroadphaseInterface *overlappingPairCache;
    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver *solver;
    btDiscreteDynamicsWorld *dynamicsWorld;
    PhysicsDebugDraw *debugDraw;

    void initBulletSystem();

    void checkCollisionsForAll();

    btDiscreteDynamicsWorld *getDynamicsWorld() const;

    void setDynamicsWorld(btDiscreteDynamicsWorld *dynamicsWorld);

    BSPMap *getBspMap() const;

    void setBSPMap(BSPMap *bspMap);

    std::vector<Triangle *> &getVisibleTriangles() const;

    void setVisibleTriangles(std::vector<Triangle *> &newVisibleTriangles);

    void updatePhysicObjects();

    void stepSimulation();

    void demoProjectile(int type);

    virtual ~ComponentCollisions();

};


#endif //BRAKEDA3D_COMPONENTCOLLISIONS_H
