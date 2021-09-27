//
// Created by darkhead on 2/1/20.
//

#ifndef BRAKEDA3D_COLLISIONRESOLVERBETWEENCAMERA3DANDITEMHEALTH_H
#define BRAKEDA3D_COLLISIONRESOLVERBETWEENCAMERA3DANDITEMHEALTH_H


#include "CollisionResolver.h"
#include "../Game/ItemHealthGhost.h"
#include "../EngineBuffers.h"
#include "../../src/Game/Player.h"

class CollisionResolverBetweenCamera3DAndItemHealth : public CollisionResolver {
    ComponentCamera *camera;
    ItemHealthGhost *itemHealth;
    Player *player;

    std::vector<Object3D *> *gameObjects;
    btDiscreteDynamicsWorld *dynamicsWorld;
    ComponentWeapons *weaponManager;

    ComponentCamera *getCamera() {
        if (objA->getLabel() == EngineSetup::getInstance()->cameraNameIdentifier) {
            ComponentCamera *camera = dynamic_cast<ComponentCamera *> (this->objA);
            return camera;
        }

        if (objB->getLabel() == EngineSetup::getInstance()->cameraNameIdentifier) {
            ComponentCamera *camera = dynamic_cast<ComponentCamera *> (this->objB);
            return camera;
        }
    }

    ItemHealthGhost *getItemHealthBody() {
        auto *itemHealthA = dynamic_cast<ItemHealthGhost *> (this->objA);
        if (itemHealthA != nullptr) {
            return itemHealthA;
        }

        auto *itemHealthB = dynamic_cast<ItemHealthGhost *> (this->objB);
        if (itemHealthB != nullptr) {
            return itemHealthB;
        }
    }

public:
    CollisionResolverBetweenCamera3DAndItemHealth(btPersistentManifold *contactManifold, Object3D *objA, Object3D *objB,
                                                  BSPMap *bspMap, std::vector<Object3D *> *gameObjects,
                                                  btDiscreteDynamicsWorld *dynamicsWorld,
                                                  ComponentWeapons *weaponManager,
                                                  std::vector<Triangle *> &visibleTriangles, Player *player)
            : CollisionResolver(contactManifold, objA, objB, bspMap, visibleTriangles) {
        this->camera = getCamera();
        this->itemHealth = getItemHealthBody();

        this->gameObjects = gameObjects;
        this->dynamicsWorld = dynamicsWorld;
        this->weaponManager = weaponManager;
        this->player = player;
    }

    void dispatch() {
        if (EngineSetup::getInstance()->LOG_COLLISION_OBJECTS) {
            Logging::Log("CollisionResolverBetweenCamera3DAndItemHealth", "Collision");
        }

        // Skipping if max health
        if (player->getStamina() >= EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL) return;

        // Remove item for physics engine
        dynamicsWorld->removeCollisionObject((btCollisionObject *) itemHealth->ghostObject);

        // add aid
        player->getAid(itemHealth->getAid());

        // Remove item health from scene
        this->itemHealth->setRemoved(true);
        Tools::playMixedSound(EngineBuffers::getInstance()->soundPackage->getSoundByLabel("firstAid"),
                              EngineSetup::SoundChannels::SND_ENVIRONMENT, 0);
    }
};


#endif //BRAKEDA3D_COLLISIONRESOLVERBETWEENCAMERA3DANDITEMHEALTH_H
