#ifndef BRAKEDA3D_COLLISIONRESOLVERBETWEENENEMYPARTANDBSPMAP_H
#define BRAKEDA3D_COLLISIONRESOLVERBETWEENENEMYPARTANDBSPMAP_H


#include "../Render/BSPMap.h"
#include "CollisionResolver.h"
#include "../Render/Logging.h"
#include "../Game/NPCEnemyPartBody.h"
#include "../Objects/Decal.h"

class CollisionResolverBetweenEnemyPartAndBSPMap : public CollisionResolver {
public:
    NPCEnemyPartBody *enemyPart;

    std::vector<Object3D *> *gameObjects;
    btDiscreteDynamicsWorld *dynamicsWorld;
    ComponentWeapons *weaponManager;

    CollisionResolverBetweenEnemyPartAndBSPMap(btPersistentManifold *contactManifold, Object3D *objA, Object3D *objB,
                                               BSPMap *bspMap, std::vector<Object3D *> *gameObjects,
                                               btDiscreteDynamicsWorld *dynamicsWorld, ComponentWeapons *weaponManager,
                                               std::vector<Triangle *> &visibleTriangles) : CollisionResolver(
            contactManifold, objA, objB, bspMap, visibleTriangles) {
        this->enemyPart = getEnemyPart();
        this->bspMap = getBSPMap();

        this->gameObjects = gameObjects;
        this->dynamicsWorld = dynamicsWorld;
        this->weaponManager = weaponManager;
    }

    void dispatch() {
        if (EngineSetup::getInstance()->LOG_COLLISION_OBJECTS) {
            Logging::Log("CollisionResolverBetweenEnemyPartAndBSPMap", "Collision");
        }

        if (enemyPart->doneGore) return;
        enemyPart->doneGore = true;

        getEnemyPart()->getCurrentTextureAnimation()->setPaused(true);

        makeGoreDecals();

        dynamicsWorld->removeCollisionObject(getEnemyPart()->getRigidBody());
    }

    BSPMap *getBSPMap() {
        auto *bspMapA = dynamic_cast<BSPMap *> (this->objA);
        if (bspMapA != NULL) {
            return bspMapA;
        }

        auto *bspMapB = dynamic_cast<BSPMap *> (this->objB);
        if (bspMapB != NULL) {
            return bspMapB;
        }
    }

    NPCEnemyPartBody *getEnemyPart() {
        auto *NPCPartA = dynamic_cast<NPCEnemyPartBody *> (this->objA);
        if (NPCPartA != NULL) {
            return NPCPartA;
        }

        auto *NPCPartB = dynamic_cast<NPCEnemyPartBody *> (this->objB);
        if (NPCPartB != NULL) {
            return NPCPartB;
        }
    }

    void makeGoreDecals() {
        auto *decal = new Decal();
        decal->setPosition(getEnemyPart()->getPosition());
        decal->setupCube(10, 10, 10);

        btVector3 linearVelocity;
        btVector3 ptA, ptB;
        for (int x = 0; x < contactManifold->getNumContacts(); x++) {
            btManifoldPoint manifoldPoint = contactManifold->getContactPoint(x);
            linearVelocity = manifoldPoint.m_normalWorldOnB;
        }

        //linearVelocity = projectile->getRigidBody()->getLinearVelocity().normalized();
        Vertex3D direction = Vertex3D(linearVelocity.x(), linearVelocity.y(), linearVelocity.z());
        direction = direction.getInverse();

        M3 rotDecal = M3::getFromVectors(direction.getNormalize(), EngineSetup::getInstance()->up);
        decal->setRotation(rotDecal.getTranspose());

        decal->getSprite()->linkTextureAnimation(EngineBuffers::getInstance()->goreDecalTemplates);
        decal->getSprite()->setAnimation(Tools::random(0, 10));
        decal->cube->setPosition(decal->getPosition());
        decal->cube->updateGeometry();
        decal->getTriangles(*visibleTriangles,
                            Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera());
        Brakeza3D::get()->addObject3D(decal, "decal");
    }
};


#endif //BRAKEDA3D_COLLISIONRESOLVERBETWEENENEMYPARTANDBSPMAP_H
